/**
 * \file
 *
 * \brief Transformation functions.
 */

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
	char *result = (char *) 0;

	/* Open file in read‐only mode */
	int fd = open(path, O_RDONLY);
	if (unlikely(fd < 0))
		goto egress0;

	/* Determine file size */
	struct stat st;
	if (unlikely(fstat(fd, &st)))
		goto egress1;

	if (unlikely(st.st_size > max)) {
		errno = EFBIG;
		goto egress1;
	}

	/* Allocate file buffer */
	char *buf = malloc(st.st_size + 1);
	if (unlikely(!buf))
		goto egress1;

	/* Dump file */
	ssize_t ret = read(fd, buf, st.st_size + 1);
	if (unlikely(ret < 0))
		goto egress2;
	else if (unlikely(ret > st.st_size)) {
		errno = EFBIG;
		goto egress2;
	}

	/* Zero‐terminate buffer */
	buf[ret] = '\0';

	result = buf;
	goto egress1;

egress2:
	free(buf);

egress1:
	close(fd);

egress0:
	return result;
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
	char *result = (char *) 0;

	char *canon = realpath(path, (char *) 0);
	if (unlikely(!canon))
		goto egress0;

	if (unlikely(strncmp(canon, prefix, strlen(prefix)))) {
		errno = EPERM;
		goto egress1;
	}

	result = canon;
	goto egress0;

egress1:
	free(canon);

egress0:
	return result;
}

/**
 * \brief Despatch transformation.
 *
 * \param pid Pointer to process ID variable.
 * \param ident Transformation identifier.
 * \param log Log file descriptor.
 * \param out Output file descriptor.
 * \param in Array of input file descriptors.
 * \param num Number of input file descriptors.
 *
 * \return \c true if successful or \c false on failure.
 */
bool transform(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out, const int in[restrict], uint16_t num) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	hexstr(idstr, ident, sizeof ident);

	/* Generate TRE specifier path */
	char *path = concat(SHARE_BASE, idstr, "/tre", (char *) 0);
	if (unlikely(!path))
		goto egress0;

	/* Read TRE name */
	char *tre = dump(path, NAME_MAX);
	if (unlikely(!tre))
		goto egress1;

	free(path);

	/* Generate TRE path */
	path = concat(EXEC_BASE, tre, (char *) 0);
	if (unlikely(!path))
		goto egress2;

	/* Validate path */
	char *canon = canonicalise(EXEC_BASE, path);
	if (unlikely(!canon))
		goto egress2;

	free(path);
	path = canon;

	/* Check permissions */
	if (unlikely(access(path, X_OK)))
		goto egress2;

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		goto egress2;

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		goto egress3;

	/* Standard out will be used for the output tree */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		goto egress3;

	/* Standard error will be used for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		goto egress3;

	/* Input file descriptors */
	for (size_t iter = 0; iter < num; ++iter)
		if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in[iter], iter + 3)))
			goto egress3;

	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (posix_spawnattr_init(&attr))
		goto egress3;

	if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF))
		goto egress4;

	/* Empty signal mask */
	sigset_t sigmask;
	if (sigemptyset(&sigmask))
		goto egress4;

	if (posix_spawnattr_setsigmask(&attr, &sigmask))
		goto egress4;

	/* Default signal handlers */
	sigset_t sigdefault;
	if (sigfillset(&sigdefault))
		goto egress4;

	if (posix_spawnattr_setsigdefault(&attr, &sigdefault))
		goto egress4;

	/* Source directory */
	char *source = concat(SHARE_BASE, idstr, (char *) 0);
	if (unlikely(!source))
		goto egress4;

	/* Check permissions */
	if (unlikely(access(source, R_OK | X_OK)))
		goto egress5;

	/* Cache directory */
	char *cache = concat(CACHE_BASE, tre, (char *) 0);
	if (unlikely(!cache))
		goto egress5;

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST))
		goto egress6;

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		goto egress6;

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!temp))
		goto egress6;

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST))
		goto egress7;

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		goto egress7;

	/* Get number of input descriptors as hexadecimal ASCII string */
	char narg[sizeof num * 2 + 1];
	num = be16(num);
	hexstr(narg, &num, sizeof num);

	/* Set argument vector up */
	char *argv[] = { "sydbox", "-C", "-L", path, source, cache, temp, narg, (char *) 0 };

	/* Writable directories (FIXME: Make this configurable) */
	char *sydwr = concat("SYDBOX_WRITE=/dev/fd;/dev/full;/dev/null;/dev/stderr;/dev/stdout;/dev/shm;/dev/zero;/proc/self/attr;/proc/self/fd;/proc/self/task;/tmp;" CACHE_BASE, idstr, ";" TEMP_BASE, idstr);
	if (unlikely(!sydwr))
		goto egress7;

	/* Set environment up */
	char *envp[] = { sydwr, (char *) 0 };

	/* Spawn sub‐process */
	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, &attr, argv, envp)))
		goto egress8;

	result = true;

egress8:
	free(sydwr);

egress7:
	free(temp);

egress6:
	free(cache);

egress5:
	free(source);

egress4:
	posix_spawnattr_destroy(&attr);

egress3:
	posix_spawn_file_actions_destroy(&file_actions);

egress2:
	free(tre);

egress1:
	free(path);

egress0:
	return result;
}

/**
 * \brief Cleanup slave routine.
 */
static int slave(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int result = -1;

	switch (typeflag) {
	case FTW_DP:
		if (unlikely(rmdir(fpath)))
			goto egress0;

	case FTW_SL:
	case FTW_F:
	case FTW_SLN:
		if (unlikely(unlink(fpath)))
			goto egress0;
	}

	result = 0;

egress0:
	return result;
}

/**
 * \brief Clean temporary directory up.
 *
 * \param ident Transformation identifier.
 *
 * \return \c true if successful or \c false on failure.
 */
bool cleanup(const uint8_t ident[restrict 32]) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	hexstr(idstr, ident, sizeof ident);

	char *path = concat(TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!path))
		goto egress0;

	if (unlikely(nftw(path, slave, 32, FTW_DEPTH | FTW_PHYS)))
		goto egress1;

	result = true;

egress1:
	free(path);

egress0:
	return result;
}
