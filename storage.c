#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "canonical.h"
#include "egress.h"
#include "expect.h"
#include "path.h"
#include "storage.h"
#include "string.h"

bool retrieve(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int out) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Generate storage module path */
	char *path = concat(EXEC_BASE "storage/", module, (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	/* Validate path */
	char *canon = canonical(EXEC_BASE "storage/", path);
	if (unlikely(!canon))
		egress(1, false, errno);

	free(path);
	path = canon;

	/* Check permissions */
	if (unlikely(access(path, X_OK)))
		egress(1, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(1, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(2, false, errno);

	/* Write object to standard output */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, out, 1)))
		egress(2, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(2, false, errno);

#if 0
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
#endif

	/* Store directory */
	char *store = concat(STORE_BASE, module, (char *) 0);
	if (unlikely(!store))
		egress(3, false, errno);

	/* Create directory */
	if (unlikely(mkdir(store, 0755) && errno != EEXIST))
		egress(4, false, errno);

	/* Check permissions */
	if (unlikely(access(store, R_OK | W_OK | X_OK)))
		egress(4, false, errno);

	/* Cache directory */
	char *cache = concat(CACHE_BASE "storage/", module, (char *) 0);
	if (unlikely(!cache))
		egress(4, false, errno);

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST))
		egress(5, false, errno);

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		egress(5, false, errno);

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE "storage/", idstr, (char *) 0);
	if (unlikely(!temp))
		egress(5, false, errno);

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST))
		egress(6, false, errno);

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		egress(6, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "sydbox", "-C", "-L", path, store, cache, temp, idstr, "retrieve", (char *) 0 };

	/* Writable directories */
	char *sydwr = concat("SYDBOX_WRITE=/dev/fd;/dev/full;/dev/null;/dev/stderr;/dev/stdout;/dev/shm;/dev/zero;/proc/self/attr;/proc/self/fd;/proc/self/task;/tmp;", store, ";", cache, ";", temp, (char *) 0);

	/* Set environment up */
	const char *envp[] = { sydwr, (char *) 0 };

	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, (posix_spawnattr_t *) 0, (char **) argv, (char **) envp)))
		egress(6, false, errno);

	egress(6, true, errno);

egress6:
	free(temp);

egress5:
	free(cache);

egress4:
	free(store);

egress3:
#if 0
	posix_spawnattr_destroy(&attr);
#endif

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	final();
}

bool deposit(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int in) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Generate storage module path */
	char *path = concat(EXEC_BASE "storage/", module, (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	/* Validate path */
	char *canon = canonical(EXEC_BASE "storage/", path);
	if (unlikely(!canon))
		egress(1, false, errno);

	free(path);
	path = canon;

	/* Check permissions */
	if (unlikely(access(path, X_OK)))
		egress(1, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(1, false, errno);

	/* Read object from standard input */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, in, 0)))
		egress(2, false, errno);

	/* Standard output will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, "/dev/null", O_RDONLY, 0)))
		egress(2, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(2, false, errno);

#if 0
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
#endif

	/* Store directory */
	char *store = concat(STORE_BASE, module, (char *) 0);
	if (unlikely(!store))
		egress(3, false, errno);

	/* Create directory */
	if (unlikely(mkdir(store, 0755) && errno != EEXIST))
		egress(4, false, errno);

	/* Check permissions */
	if (unlikely(access(store, R_OK | W_OK | X_OK)))
		egress(4, false, errno);

	/* Cache directory */
	char *cache = concat(CACHE_BASE "storage/", module, (char *) 0);
	if (unlikely(!cache))
		egress(4, false, errno);

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST))
		egress(5, false, errno);

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		egress(5, false, errno);

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE "storage/", idstr, (char *) 0);
	if (unlikely(!temp))
		egress(5, false, errno);

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST))
		egress(6, false, errno);

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		egress(6, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "sydbox", "-C", "-L", path, store, cache, temp, idstr, "deposit", (char *) 0 };

	/* Writable directories */
	char *sydwr = concat("SYDBOX_WRITE=/dev/fd;/dev/full;/dev/null;/dev/stderr;/dev/stdout;/dev/shm;/dev/zero;/proc/self/attr;/proc/self/fd;/proc/self/task;/tmp;", store, ";", cache, ";", temp, (char *) 0);

	/* Set environment up */
	const char *envp[] = { sydwr, (char *) 0 };

	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, (posix_spawnattr_t *) 0, (char **) argv, (char **) envp)))
		egress(6, false, errno);

	egress(6, true, errno);

egress6:
	free(temp);

egress5:
	free(cache);

egress4:
	free(store);

egress3:
#if 0
	posix_spawnattr_destroy(&attr);
#endif

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	final();
}

bool efface(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log) {
	prime(bool);

	char idstr[32 * 2 + 1];

	/* Convert identifier to hexadecimal ASCII string */
	inthexs(idstr, ident, 32);

	/* Generate storage module path */
	char *path = concat(EXEC_BASE "storage/", module, (char *) 0);
	if (unlikely(!path))
		egress(0, false, errno);

	/* Validate path */
	char *canon = canonical(EXEC_BASE "storage/", path);
	if (unlikely(!canon))
		egress(1, false, errno);

	free(path);
	path = canon;

	/* Check permissions */
	if (unlikely(access(path, X_OK)))
		egress(1, false, errno);

	posix_spawn_file_actions_t file_actions;

	/* Set file descriptors up */
	if (unlikely(posix_spawn_file_actions_init(&file_actions)))
		egress(1, false, errno);

	/* Standard input will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0)))
		egress(2, false, errno);

	/* Standard output will not be used */
	if (unlikely(posix_spawn_file_actions_addopen(&file_actions, 1, "/dev/null", O_RDONLY, 0)))
		egress(2, false, errno);

	/* Use standard error for logging */
	if (unlikely(posix_spawn_file_actions_adddup2(&file_actions, log, 2)))
		egress(2, false, errno);

#if 0
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
#endif

	/* Store directory */
	char *store = concat(STORE_BASE, module, (char *) 0);
	if (unlikely(!store))
		egress(3, false, errno);

	/* Create directory */
	if (unlikely(mkdir(store, 0755) && errno != EEXIST))
		egress(4, false, errno);

	/* Check permissions */
	if (unlikely(access(store, R_OK | W_OK | X_OK)))
		egress(4, false, errno);

	/* Cache directory */
	char *cache = concat(CACHE_BASE "storage/", module, (char *) 0);
	if (unlikely(!cache))
		egress(4, false, errno);

	/* Create directory */
	if (unlikely(mkdir(cache, 0755) && errno != EEXIST))
		egress(5, false, errno);

	/* Check permissions */
	if (unlikely(access(cache, R_OK | W_OK | X_OK)))
		egress(5, false, errno);

	/* Temporary file directory */
	char *temp = concat(TEMP_BASE "storage/", idstr, (char *) 0);
	if (unlikely(!temp))
		egress(5, false, errno);

	/* Create directory */
	if (unlikely(mkdir(temp, 0755) && errno != EEXIST))
		egress(6, false, errno);

	/* Check permissions */
	if (unlikely(access(temp, R_OK | W_OK | X_OK)))
		egress(6, false, errno);

	/* Set argument vector up */
	const char *argv[] = { "sydbox", "-C", "-L", path, store, cache, temp, idstr, "efface", (char *) 0 };

	/* Writable directories */
	char *sydwr = concat("SYDBOX_WRITE=/dev/fd;/dev/full;/dev/null;/dev/stderr;/dev/stdout;/dev/shm;/dev/zero;/proc/self/attr;/proc/self/fd;/proc/self/task;/tmp;", store, ";", cache, ";", temp, (char *) 0);

	/* Set environment up */
	const char *envp[] = { sydwr, (char *) 0 };

	if (unlikely(posix_spawnp(pid, "sydbox", &file_actions, (posix_spawnattr_t *) 0, (char **) argv, (char **) envp)))
		egress(6, false, errno);

	egress(6, true, errno);

egress6:
	free(temp);

egress5:
	free(cache);

egress4:
	free(store);

egress3:
#if 0
	posix_spawnattr_destroy(&attr);
#endif

egress2:
	posix_spawn_file_actions_destroy(&file_actions);

egress1:
	free(path);

egress0:
	final();
}
