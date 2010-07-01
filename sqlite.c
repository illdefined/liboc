#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <sqlite3.h>

#include "expect.h"
#include "string.h"

/**
 * \brief Size of I/O buffer.
 */
#define BUFSIZE 4096

/**
 * \brief I/O buffer.
 */
static uint8_t buf[BUFSIZE];

/**
 * \brief SQLite database handle.
 */
static sqlite3 *db;

/**
 * \brief SQLite cleanup routine.
 */
static void atexit_sqlite() {
	sqlite3_close(db);
}

/**
 * \brief Main routine.
 *
 * \param argc Number of arguments.
 * \param argv Argument vector.
 *
 * \return EXIT_SUCCESS if successful or any other value on failure.
 */
int main(int argc, char *argv[]) {
	if (unlikely(argc != 6)) {
		fputs("Invalid number of command line arguments!\n", stderr);
		return EXIT_FAILURE;
	}

	/* Change to storage directory */
	if (unlikely(chdir(argv[1]))) {
		perror("Unable to change to storage directory");
		return EXIT_FAILURE;
	}

	uint8_t ident[32];
	if (!hexsint(ident, argv[4], sizeof ident)) {
		perror("Failed to parse identifier");
		return EXIT_FAILURE;
	}

	/* Close database upon exit */
	atexit(atexit_sqlite);

	/* Open database */
	if (unlikely(sqlite3_open("corpus", &db) != SQLITE_OK)) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	/* Create table */
	char *errmsg;
	if (unlikely(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS corpus (ident BLOB PRIMARY KEY, object BLOB)",
		(void *) 0, (void *) 0, &errmsg) != SQLITE_OK)) {
		fprintf(stderr, "Unable to create table: %s\n", errmsg);
		return EXIT_FAILURE;
	}

	/* Parse operation string */
	if (!strcmp(argv[5], "assay")) {
		const char *query = "SELECT object FROM corpus WHERE ident=?";

		sqlite3_stmt *stmt;
		if (unlikely(sqlite3_prepare_v2(db, query, -1, &stmt, (const char **) 0) != SQLITE_OK)) {
			fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_bind_blob(stmt, 1, ident, sizeof ident, SQLITE_STATIC) != SQLITE_OK)) {
			fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		int rc = sqlite3_step(stmt);
		switch (rc) {
		case SQLITE_DONE:
			sqlite3_finalize(stmt);
			return 3;

		case SQLITE_ROW:
			break;

		default:
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			return EXIT_FAILURE;
		}

		sqlite3_finalize(stmt);
	}

	else if (!strcmp(argv[5], "retrieve")) {
		/* Get row ID */
		const char *query = "SELECT ROWID FROM corpus WHERE ident=?";

		sqlite3_stmt *stmt;
		if (unlikely(sqlite3_prepare_v2(db, query, -1, &stmt, (const char **) 0) != SQLITE_OK)) {
			fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_bind_blob(stmt, 1, ident, sizeof ident, SQLITE_STATIC) != SQLITE_OK)) {
			fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		int rc = sqlite3_step(stmt);
		switch (rc) {
		case SQLITE_DONE:
			return 3;
			sqlite3_finalize(stmt);

		case SQLITE_ROW:
			break;

		default:
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			return EXIT_FAILURE;
		}

		sqlite3_int64 row = sqlite3_column_int64(stmt, 0);
		sqlite3_finalize(stmt);

		/* Open BLOB */
		sqlite3_blob *blob;
		if (unlikely(sqlite3_blob_open(db, (char *) 0, "corpus", "object", row, 0, &blob) != SQLITE_OK)) {
			fprintf(stderr, "Cannot open BLOB: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		int index = 0;
		int bytes = sqlite3_blob_bytes(blob);

#define BUFFILL (BUFSIZE > bytes - index ? bytes - index : BUFSIZE)

		while (index < bytes) {
			if (unlikely(sqlite3_blob_read(blob, buf, BUFFILL, index) != SQLITE_OK)) {
				fprintf(stderr, "Cannot read from BLOB: %s\n", sqlite3_errmsg(db));
				return EXIT_FAILURE;
			}

			if (unlikely(write(1, buf, BUFFILL) != BUFFILL)) {
				perror("Write error");
				return EXIT_FAILURE;
			}

			index += BUFFILL;
		}
	}

	else if (!strcmp(argv[5], "deposit")) {
		/* Write to temporary file */
		if (unlikely(chdir(argv[3]))) {
			perror("Failed to change to temporary directory");
			return EXIT_FAILURE;
		}

		char template[] = "temp-XXXXXX";
		int fd = mkstemp(template);
		if (unlikely(fd < 0)) {
			perror("Cannot create temporary file");
			return EXIT_FAILURE;
		}

		unlink(template);

		ssize_t in, out;

		do {
			in = read(0, buf, BUFSIZE);
			if (unlikely(in < 0)) {
				perror("Read error");
				return EXIT_FAILURE;
			}

			out = write(fd, buf, in);
			if (unlikely(out < 0)) {
				perror("Write error");
				return EXIT_FAILURE;
			}

			else if (unlikely(out < in)) {
				fputs("Write error!\n", stderr);
				return EXIT_FAILURE;
			}
		} while (likely(in > 0));

		/* Close standard input */
		close(0);

		struct stat st;
		if (unlikely(fstat(fd, &st))) {
			perror("Cannot stat file");
			return EXIT_FAILURE;
		}

		void *object = mmap((void *) 0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (unlikely(object == MAP_FAILED)) {
			perror("Unable to mmap file");
			return EXIT_FAILURE;
		}

		close(fd);

		const char *query = "INSERT OR REPLACE INTO corpus (ident, object) VALUES(?, ?)";

		sqlite3_stmt *stmt;
		if (unlikely(sqlite3_prepare_v2(db, query, -1, &stmt, (const char **) 0) != SQLITE_OK)) {
			fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
			munmap(object, st.st_size);
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_bind_blob(stmt, 1, ident, sizeof ident, SQLITE_STATIC) != SQLITE_OK)) {
			fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			munmap(object, st.st_size);
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_bind_blob(stmt, 2, object, st.st_size, SQLITE_STATIC) != SQLITE_OK)) {
			fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			munmap(object, st.st_size);
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_step(stmt) != SQLITE_DONE)) {
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			munmap(object, st.st_size);
			return EXIT_FAILURE;
		}

		sqlite3_finalize(stmt);
		munmap(object, st.st_size);
	}

	else if (!strcmp(argv[5], "efface")) {
		const char *query = "DELETE FROM corpus WHERE ident=?";

		sqlite3_stmt *stmt;
		if (unlikely(sqlite3_prepare_v2(db, query, -1, &stmt, (const char **) 0) != SQLITE_OK)) {
			fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_bind_blob(stmt, 1, ident, sizeof ident, SQLITE_STATIC) != SQLITE_OK)) {
			fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}

		if (unlikely(sqlite3_step(stmt) != SQLITE_DONE)) {
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			return EXIT_FAILURE;
		}
	}

	else {
		fprintf(stderr, "Invalid storage operation “%s”!\n", argv[5]);
		return 2;
	}

	return EXIT_SUCCESS;
}
