#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "egress.h"
#include "endian.h"
#include "expect.h"
#include "path.h"
#include "string.h"
#include "transform.h"

/**
 * \brief Dump file content.
 *
 * The file buffer will be allocated on the heap and should be passed to
 * \c free to release the storage when it is no longer needed.
 *
 * \param path Path name of file.
 * \param max Maximum number of bytes to dump.
 *
 * \return Pointer to buffer or <tt>(char *) 0</tt> on failure.
 */
static char *dump(const char *restrict path, size_t max) {
	prime(char *);

	/* Open file in read‐only mode */
	int fd = open(path, O_RDONLY);
	if (unlikely(fd < 0))
		egress(0, (char *) 0, errno);

	/* Determine file size */
	struct stat st;
	if (unlikely(fstat(fd, &st)))
		egress(1, (char *) 0, errno);

	if (unlikely(st.st_size > max))
		egress(1, (char *) 0, EFBIG);

	/* Allocate file buffer */
	char *buf = malloc(st.st_size + 1);
	if (unlikely(!buf))
		egress(1, (char *) 0, errno);

	/* Dump file */
	ssize_t ret = read(fd, buf, st.st_size + 1);
	if (unlikely(ret < 0))
		egress(2, (char *) 0, errno);
	else if (unlikely(ret > st.st_size))
		egress(2, (char *) 0, EFBIG);

	/* Zero‐terminate buffer */
	buf[ret] = '\0';

	egress(1, buf, errno);

egress2:
	free(buf);

egress1:
	close(fd);

egress0:
	final();
}

/**
 * \brief Canonicalise and validate path name.
 *
 * Convert the path name specified in \a path into its canonical form
 * and validate it. The new path name will be allocated on the heap and
 * shound be released using \c free.
 *
 * \param prefix Mandatory path prefix.
 * \param path Path name.
 *
 * \return Pointer to the new path name or <tt>(char *) 0</tt> on failure.
 */
static char *canonicalise(const char *restrict prefix, const char *restrict path) {
	prime(char *);

	char *canon = realpath(path, (char *) 0);
	if (unlikely(!canon))
		egress(0, (char *) 0, errno);

	if (unlikely(strncmp(canon, prefix, strlen(prefix))))
		egress(1, (char *) 0, EPERM);

	egress(0, canon, errno);

egress1:
	free(canon);

egress0:
	final();
}

bool transform(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out, const int in[restrict], uint16_t num) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Generate TRE specifier path */
	char *path = concat(SHARE_BASE, idstr, "/runtime", (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	/* Read TRE name */
	char *runtime = dump(path, NAME_MAX);
	if (unlikely(!runtime))
		egress(1, false, errno);

	free(path);

	/* Generate TRE path */
	path = concat(EXEC_BASE "runtime/", runtime, (char *) 0);
	if (unlikely(!path))
		egress(2, false, errno);

	/* Validate path */
	char *canon = canonicalise(EXEC_BASE "runtime/", path);
	if (unlikely(!canon))
		egress(2, false, errno);

	free(path);
	path = canon;

	/* Check permissions */
	if (unlikely(access(path, X_OK)))
		egress(2, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(2, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(3, false, errno);

	/* Standard out will be used for the output tree */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		egress(3, false, errno);

	/* Standard error will be used for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(3, false, errno);

	/* Input file descriptors */
	for (size_t iter = 0; iter < num; ++iter)
		if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in[iter], iter + 3)))
			egress(3, false, errno);

#if 0
	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (posix_spawnattr_init(&attr))
		egress(3, false, errno);

	if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF))
		egress(4, false, errno);

	/* Empty signal mask */
	sigset_t sigmask;
	if (sigemptyset(&sigmask))
		egress(4, false, errno);

	if (posix_spawnattr_setsigmask(&attr, &sigmask))
		egress(4, false, errno);

	/* Default signal handlers */
	sigset_t sigdefault;
	if (sigfillset(&sigdefault))
		egress(4, false, errno);

	if (posix_spawnattr_setsigdefault(&attr, &sigdefault))
		egress(4, false, errno);
#endif

	/* Source directory */
	char *source = concat(SHARE_BASE, idstr, (char *) 0);
	if (unlikely(!source))
		egress(4, false, errno);

	/* Check permissions */
	if (unlikely(access(source, R_OK | X_OK)))
		egress(5, false, errno);

	/* Cache directory */
	char *cache = concat(CACHE_BASE "runtime/", runtime, (char *) 0);
	if (unlikely(!cache))
		egress(5, false, errno);

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST))
		egress(6, false, errno);

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		egress(6, false, errno);

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE "runtime/", idstr, (char *) 0);
	if (unlikely(!temp))
		egress(6, false, errno);

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST))
		egress(7, false, errno);

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		egress(7, false, errno);

	/* Get number of input descriptors as hexadecimal ASCII string */
	char narg[sizeof num * 2 + 1];
	num = be16(num);
	inthexs(narg, &num, sizeof num);

	/* Set argument vector up */
	const char *argv[] = { "sydbox", "-C", "-L", path, source, cache, temp, narg, (char *) 0 };

	/* Writable directories (FIXME: Make this configurable) */
	char *sydwr = concat("SYDBOX_WRITE=/dev/fd;/dev/full;/dev/null;/dev/stderr;/dev/stdout;/dev/shm;/dev/zero;/proc/self/attr;/proc/self/fd;/proc/self/task;/tmp;" CACHE_BASE, idstr, ";" TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!sydwr))
		egress(7, false, errno);

	/* Set environment up */
	const char *envp[] = { sydwr, (char *) 0 };

	/* Spawn sub‐process */
	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, (posix_spawnattr_t *) 0, (char **) argv, (char **) envp)))
		egress(8, false, errno);

	egress(8, true, errno);

egress8:
	free(sydwr);

egress7:
	free(temp);

egress6:
	free(cache);

egress5:
	free(source);

egress4:
#if 0
	posix_spawnattr_destroy(&attr);
#endif

egress3:
	posix_spawn_file_actions_destroy(&file_actions);

egress2:
	free(runtime);

egress1:
	free(path);

egress0:
	final();
}

/**
 * \brief Cleanup slave routine.
 */
static int slave(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	prime(int);

	switch (typeflag) {
	case FTW_DP:
		if (unlikely(rmdir(fpath)))
			egress(0, -1, errno);

	case FTW_SL:
	case FTW_F:
	case FTW_SLN:
		if (unlikely(unlink(fpath)))
			egress(0, -1, errno);
	}

	egress(0, 0, errno);

egress0:
	final();
}

bool cleanup(const uint8_t ident[restrict 32]) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	char *path = concat(TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	if (unlikely(nftw(path, slave, 32, FTW_DEPTH | FTW_PHYS)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	free(path);

egress0:
	final();
}
