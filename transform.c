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

#define DESPATCHER EXEC_BASE "/despatch"

extern char **environ;

bool transform(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out, const int in[restrict], uint16_t num) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Check permissions */
	if (unlikely(access(DESPATCHER, X_OK)))
		egress(0, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(0, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(1, false, errno);

	/* Standard out will be used for the output tree */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		egress(1, false, errno);

	/* Standard error will be used for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(1, false, errno);

	/* Input file descriptors */
	for (size_t iter = 0; iter < num; ++iter)
		if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in[iter], iter + 3)))
			egress(1, false, errno);

	/* Get number of input descriptors as hexadecimal ASCII string */
	char narg[sizeof num * 2 + 1];
	num = be16(num);
	inthexs(narg, &num, sizeof num);

	/* Set argument vector up */
	const char *argv[] = { "despatch", DESPATCHER, idstr, narg, (char *) 0 };

	/* Spawn sub‐process */
	if (unlikely(posix_spawn(pid, DESPATCHER, &file_actions, (posix_spawnattr_t *) 0, (char **) argv, environ)))
		egress(1, false, errno);

	egress(1, true, errno);

egress1:
	posix_spawn_file_actions_destroy(&file_actions);

egress0:
	final();
}
