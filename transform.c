#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "expect.h"
#include "path.h"
#include "transform.h"

/**
 * \brief Convert 4bit integer to hexadecimal ASCII character.
 *
 * \param nibble Integer to convert.
 *
 * \return Hexadecimal ASCII character.
 */
static inline uint8_t hexchar(uint8_t nibble) {
	uint8_t result;

	assert(nibble <= 0xF);

	switch (nibble) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
		result = nibble + (uint8_t) '0';
		break;

	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		result = nibble + (uint8_t) 'a';
		break;
	}

	return result;
}

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
 * \brief Concatenate strings.
 *
 * The new string will be allocated on the heap and should be passed to
 * \c free to release the storage when it is no longer needed.
 *
 * \param prefix First string.
 *
 * \return Pointer to the concatenated string or <tt>(char *) 0</tt> on failure.
 */
static char *concat(const char *restrict prefix, ...) {
	char *result = (char *) 0;

	size_t size = strlen(prefix) + 1;
	char *buf = malloc(PATH_MAX);
	if (unlikely(!buf))
		goto egress0;

	strcpy(buf, prefix);

	va_list ap;
	va_start(ap, prefix);

	for (;;) {
		const char *arg = va_arg(ap, const char *);

		if (unlikely(!arg))
			break;

		void *nbuf = realloc(buf, size + strlen(arg));
		if (unlikely(!nbuf))
			goto egress1;
		else
			buf = nbuf;

		strcpy(&buf[size - 1], arg);
		size += strlen(arg);
	}

	va_end(ap);

	result = buf;
	goto egress0;

egress1:
	free(buf);

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
bool transform(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out, const int in[restrict], size_t num) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	for (size_t idx = 0; idx < sizeof ident; ++idx) {
		/* High nibble */
		idstr[idx + 0] = hexchar(ident[idx] / UINT8_C(0x10));

		/* Low nibble */
		idstr[idx + 1] = hexchar(ident[idx] % UINT8_C(0x10));
	}

	/* Zero‐terminate it */
	idstr[sizeof ident * 2] = '\0';

	/* Generate transformer specifier path */
	char *path = concat(SHARE_BASE, idstr, "/transformer", (char *) 0);
	if (unlikely(!path))
		goto egress0;

	/* Read transformer name */
	char *name = dump(path, NAME_MAX);
	if (unlikely(!name))
		goto egress1;

	free(path);

	/* Generate transformer path */
	path = concat(EXEC_BASE, name, (char *) 0);
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

	/* Source directory */
	char *source = concat(SHARE_BASE, idstr, (char *) 0);
	if (unlikely(!source))
		goto egress3;

	/* Check permissions */
	if (unlikely(access(source, R_OK | X_OK)))
		goto egress4;

	/* Cache directory */
	char *cache = concat(CACHE_BASE, idstr, (char *) 0);
	if (unlikely(!cache))
		goto egress4;

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST)
		goto egress5;

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		goto egress5;

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!temp))
		goto egress5;

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST)))
		goto egress6;

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		goto egress6;

	/* Get number of input descriptors as hexadecimal ASCII string */
	char narg[sizeof num * 2 + 1];
	for (size_t idx = 0; idx < sizeof num * 2; ++idx)
		narg[idx] = num >> sizeof num * 8 - idx * 4 & 0xf;
	narg[sizeof num * 2] = '\0';

	/* Set argument vector up */
	char *argv[] = { "sydbox", "-C", "-L", path, source, cache, temp, narg, (char *) 0 };

	/* Writable directories */
	char *sydwr = concat("SYDBOX_WRITE=/tmp/;" CACHE_BASE, idstr, ";" TEMP_BASE, idstr);
	if (unlikely(!sydwr))
		goto egress6;

	/* Set environment up */
	char *envp[] = { sydwr, (char *) 0 };

	/* Spawn sub‐process */
	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, (posix_spawnattr_t *) 0, argv, envp)))
		goto egress7;

	result = true;

egress7:
	free(sydwr);

egress6:
	free(temp);

egress5:
	free(cache);

egress4:
	free(source);

egress3:
	posix_spawn_file_actions_destroy(&file_actions);

egress2:
	free(name);

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
 * \brief Clean temporary directories up.
 *
 * \param ident Transformation identifier.
 * \param cache Remove cached files?
 *
 * \return \c true if successful or \c false on failure.
 */
bool cleanup(const uint8_t ident[restrict 32], bool cache) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	for (size_t idx = 0; idx < sizeof ident; ++idx) {
		/* High nibble */
		idstr[idx + 0] = hexchar(ident[idx] / UINT8_C(0x10));

		/* Low nibble */
		idstr[idx + 1] = hexchar(ident[idx] % UINT8_C(0x10));
	}

	/* Zero‐terminate it */
	idstr[sizeof ident * 2] = '\0';

	char *path = concat(TEMP_BASE, idstr, (char *) 0);
	if (unlikely(!path))
		goto egress0;

	if (unlikely(nftw(path, slave, 32, FTW_DEPTH | FTW_PHYS)))
		goto egress1;

	if (cache) {
		path = concat(CACHE_BASE, idstr, (char *) 0);
		if (unlikely(!path))
			goto egress0;

		if (unlikely(nftw(path, slave, 32, FTW_CHDIR | FTW_DEPTH | FTW_PHYS)))
			goto egress1;
	}

	result = true;

egress1:
	free(path);

egress0:
	return result;
}
