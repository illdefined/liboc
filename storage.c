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

#include "egress.h"
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
	prime(bool);

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	if (unlikely(access(path, R_OK)))
		egress(1, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(1, false, errno);

	/* Read compressed object from standard input */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, path, O_RDONLY, 0)))
		egress(2, false, errno);

	/* Write uncompressed object to standard output */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		egress(2, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(2, false, errno);

	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (unlikely(posix_spawnattr_init(&attr)))
		egress(2, false, errno);

	if (unlikely(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF)))
		egress(3, false, errno);

	/* Empty signal mask */
	sigset_t sigmask;
	if (unlikely(sigemptyset(&sigmask)))
		egress(3, false, errno);

	if (unlikely(posix_spawnattr_setsigmask(&attr, &sigmask)))
		egress(3, false, errno);

	/* Default signal handlers */
	sigset_t sigdefault;
	if (unlikely(sigfillset(&sigdefault)))
		egress(3, false, errno);

	if (unlikely(posix_spawnattr_setsigdefault(&attr, &sigdefault)))
		egress(3, false, errno);

	char *argv[] = { "xz", "-d", "-c", (char *) 0 };
	char *envp[] = { (char *) 0 };

	if (unlikely(posix_spawnp(pid, "xz", &file_actions, &attr, argv, envp)))
		egress(3, false, errno);

	egress(3, true, errno);

egress3:
	posix_spawnattr_destroy(&attr);

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	final();
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
	prime(bool);

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	if (unlikely(access(STORE_BASE, W_OK)))
		egress(1, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(1, false, errno);

	/* Read uncompressed object from standard input */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in, 0)))
		egress(2, false, errno);

	/* Write compressed object to standard output */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, path, O_WRONLY | O_CREAT | O_EXCL, 0644)))
		egress(2, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(2, false, errno);

	/* Spawn attributes */
	posix_spawnattr_t attr;
	if (unlikely(posix_spawnattr_init(&attr)))
		egress(2, false, errno);

	if (unlikely(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF)))
		egress(3, false, errno);

	/* Empty signal mask */
	sigset_t sigmask;
	if (unlikely(sigemptyset(&sigmask)))
		egress(3, false, errno);

	if (unlikely(posix_spawnattr_setsigmask(&attr, &sigmask)))
		egress(3, false, errno);

	/* Default signal handlers */
	sigset_t sigdefault;
	if (unlikely(sigfillset(&sigdefault)))
		egress(3, false, errno);

	if (unlikely(posix_spawnattr_setsigdefault(&attr, &sigdefault)))
		egress(3, false, errno);

	char *argv[] = { "xz", "-z", "-c", "-7", (char *) 0 };
	char *envp[] = { (char *) 0 };

	if (unlikely(posix_spawnp(pid, "xz", &file_actions, &attr, argv, envp)))
		egress(3, false, errno);

	egress(3, true, errno);

egress3:
	posix_spawnattr_destroy(&attr);

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	final();
}


/**
 * \brief Efface object.
 *
 * \param ident Object identifier.
 *
 * \return \c true if successful or \c false on failure.
 */
bool efface(const uint8_t ident[restrict 32]) {
	prime(bool);

	char idstr[sizeof ident * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, sizeof ident);

	char *path = concat(STORE_BASE, idstr, ".xz", (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	if (unlikely(unlink(path)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	free(path);

egress0:
	final();
}
