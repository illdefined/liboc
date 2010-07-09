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

#define RETRIEVE EXEC_BASE "retrieve"
#define DEPOSIT  EXEC_BASE "deposit"
#define EFFACE   EXEC_BASE "efface"

extern char **environ;

bool retrieve(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int out) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Check permissions */
	if (unlikely(access(RETRIEVE, X_OK)))
		egress(0, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(0, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(1, false, errno);

	/* Write object to standard output */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		egress(1, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(1, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "retrieve", module, idstr, (char *) 0 };

	if (unlikely(posix_spawn(pid, RETRIEVE, &file_actions, (posix_spawnattr_t *) 0, (char **) argv, environ)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	posix_spawn_file_actions_destroy(&file_actions);

egress0:
	final();
}

bool deposit(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int in) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Check permissions */
	if (unlikely(access(DEPOSIT, X_OK)))
		egress(0, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(0, false, errno);

	/* Read object from standard input */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in, 0)))
		egress(1, false, errno);

	/* Standard output will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, "/dev/null", O_RDONLY, 0)))
		egress(1, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(1, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "deposit", module, idstr, (char *) 0 };

	if (unlikely(posix_spawnp(pid, DEPOSIT, &file_actions, (posix_spawnattr_t *) 0, (char **) argv, environ)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	posix_spawn_file_actions_destroy(&file_actions);

egress0:
	final();
}

bool efface(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Check permissions */
	if (unlikely(access(EFFACE, X_OK)))
		egress(0, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(0, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(1, false, errno);

	/* Standard output will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, "/dev/null", O_RDONLY, 0)))
		egress(1, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(1, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "efface", module, idstr, (char *) 0 };

	if (unlikely(posix_spawn(pid, EFFACE, &file_actions, (posix_spawnattr_t *) 0, (char **) argv, environ)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	posix_spawn_file_actions_destroy(&file_actions);

egress0:
	final();
}
