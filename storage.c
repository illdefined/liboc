/**
 * \file
 *
 * \brief Simple storage.
 */

#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "expect.h"
#include "path.h"
#include "storage.h"
#include "string.h"

/**
 * \brief Retrieve object.
 *
 * \param pid Pointer to process ID variable.
 * \param ident Object identifier.
 * \param log Log file descriptor.
 * \param out Output file descriptor.
 *
 * \return \c true if successful or \c false on failure.
 */
bool retrieve(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		goto egress0;

	if (unlikely(access(path, R_OK)))
		goto egress1;

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		goto egress1;

	/* Read compressed object from standard input */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, path, O_RDONLY, 0)))
		goto egress2;

	/* Write uncompressed object to standard output */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		goto egress2;

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		goto egress2;

	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (unlikely(posix_spawnattr_init(&attr)))
		goto egress2;

	if (unlikely(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF)))
		goto egress3;

	/* Empty signal mask */
	sigset_t sigmask;
	if (unlikely(sigemptyset(&sigmask)))
		goto egress3;

	if (unlikely(posix_spawnattr_setsigmask(&attr, &sigmask)))
		goto egress3;

	/* Default signal handlers */
	sigset_t sigdefault;
	if (unlikely(sigfillset(&sigdefault)))
		goto egress3;

	if (unlikely(posix_spawnattr_setsigdefault(&attr, &sigdefault)))
		goto egress3;

	char *argv[] = { "xz", "-d", "-c", (char *) 0 };
	char *envp[] = { (char *) 0 };

	if (unlikely(posix_spawnp(pid, "xz", &file_actions, &attr, argv, envp)))
		goto egress3;

	result = true;

egress3:
	posix_spawnattr_destroy(&attr);

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	return result;
}

/**
 * \brief Deposit object.
 *
 * \param pid Pointer to process ID variable.
 * \param ident Object identifier.
 * \param log Log file descriptor.
 * \param in Input file descriptor.
 *
 * \return \c true if successful or \c false on failure.
 */
bool deposit(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int in) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		goto egress0;

	if (unlikely(access(STORE_BASE, W_OK)))
		goto egress1;

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		goto egress1;

	/* Read uncompressed object from standard input */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in, 0)))
		goto egress2;

	/* Write compressed object to standard output */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, path, O_WRONLY | O_CREAT | O_EXCL, 0644)))
		goto egress2;

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		goto egress2;

	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (unlikely(posix_spawnattr_init(&attr)))
		goto egress2;

	if (unlikely(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF)))
		goto egress3;

	/* Empty signal mask */
	sigset_t sigmask;
	if (unlikely(sigemptyset(&sigmask)))
		goto egress3;

	if (unlikely(posix_spawnattr_setsigmask(&attr, &sigmask)))
		goto egress3;

	/* Default signal handlers */
	sigset_t sigdefault;
	if (unlikely(sigfillset(&sigdefault)))
		goto egress3;

	if (unlikely(posix_spawnattr_setsigdefault(&attr, &sigdefault)))
		goto egress3;

	char *argv[] = { "xz", "-z", "-c", "-7", (char *) 0 };
	char *envp[] = { (char *) 0 };

	if (unlikely(posix_spawnp(pid, "xz", &file_actions, &attr, argv, envp)))
		goto egress3;

	result = true;

egress3:
	posix_spawnattr_destroy(&attr);

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	return result;
}


/**
 * \brief Efface object.
 *
 * \param ident Object identifier.
 *
 * \return \c true if successful or \c false on failure.
 */
bool efface(const uint8_t ident[restrict 32]) {
	bool result = false;

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		goto egress0;

	if (unlikely(unlink(path)))
		goto egress1;

	result = true;

egress1:
	free(path);

egress0:
	return result;
}
