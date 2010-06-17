#pragma once
#ifndef OC_PATH_H
#define OC_PATH_H

/**
 * \file
 *
 * \brief File system paths.
 */

/**
 * \brief Base path for cached data.
 */
#define CACHE_BASE "/var/cache/opencorpus/"

/**
 * \brief Base path for executables.
 */
#define EXEC_BASE "/usr/libexec/opencorpus/"

/**
 * \brief Base path for lock files.
 */
#define LOCK_BASE "/var/lock/opencorpus/"

/**
 * \brief Base path for log files.
 */
#define LOG_BASE "/var/log/opencorpus/"

/**
 * \brief Base path for architecture‐independent data
 */
#define SHARE_BASE "/usr/share/opencorpus/"

/**
 * \brief Base path for data storage.
 */
#define STORE_BASE "/var/db/opencorpus/"

/**
 * \brief Base path for temporary files.
 */
#define TEMP_BASE "/var/tmp/opencorpus/"

/**
 * \brief Default writable path prefixes
 */
#define WRITE_DEFAULT "/dev/fd:/dev/full:/dev/null:/dev/stderr:/dev/stdout:/dev/shm:/dev/tty:/dev/zero:/pro    c/self/attr:/proc/self/fd:/proc/self/task:/tmp"

#endif
