/**
 * \file
 *
 * \brief Unique identifiers.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "endian.h"
#include "expect.h"
#include "hardtick.h"

#include "unique.h"

/**
 * \brief Random device
 */
#define RANDOM_DEVICE "/dev/random"

/**
 * \brief Offset between UTC and TAI.
 *
 * Naïve, but sufficient for now.
 */
#define TAI_OFFSET UINT64_C(946728000)

/**
 * \brief Generate unique identifier.
 *
 * \param id Buffer to hold identifier.
 *
 * \return True on success or false on failure.
 */
bool unique(uint64_t id[restrict 4]) {
	/* Open random device */
	int rnd = open(RANDOM_DEVICE, O_RDONLY);
	if (unlikely(rnd < 0))
		return false;

	/* Read random data */
	ssize_t cnt = read(rnd, id + 2, sizeof id - 2 * sizeof *id);
	if (unlikely(cnt < sizeof id - 2 * sizeof *id)) {
		/* Specify error in errno */
		if (cnt >= 0)
			errno = EIO;

		close(rnd);
		return false;
	}

	/* Close the device */
	close(rnd);

	/* Determine current time */
	struct timespec now;

	if (unlikely(clock_gettime(CLOCK_REALTIME, &now)))
		return false;

	/* TAI in nanoseconds */
	id[0] = be64((now.tv_sec + TAI_OFFSET) * UINT64_C(1000000000) +
		now.tv_nsec);

	/* CPU cycle counter */
	id[1] = be64(hardtick());

	return true;
}

#ifdef TEST
#include <stdio.h>
#include <stdlib.h>

/**
 * \brief Number of test identifiers to generate.
 */
#define UNIQUE_COUNT 16384

/**
 * \brief Calculate hamming distance between two identifiers.
 *
 * \param x First identifier.
 * \param y Second identifier.
 *
 * \return Hamming distance between \a x and \a y.
 */
static inline unsigned int hamming(uint64_t x[restrict 4], uint64_t y[restrict 4]) {
	unsigned int distance = 0;

	for (size_t iter = 0; iter < 4; ++iter) {
#if defined(__clang__) || defined(__GNUC__)
	distance += (unsigned int) __builtin_popcountll(x[iter] ^ y[iter]);
#else
	for (size_t jter = 0; jter < sizeof (uint64_t) * 8; ++jter)
		if ((x[iter] ^ y[iter]) & UINT64_C(1) << jter)
			++distance;
#endif
	}

	return distance;
}

/**
 * \brief Unique identifier test routine.
 *
 * \return \c EXIT_SUCCESS if successful or \c EXIT_FAILURE on failure.
 */
int main(void) {
	uint64_t *ids = calloc(UNIQUE_COUNT, 4 * sizeof (uint64_t));
	if (unlikely(!ids)) {
		perror("calloc");
		return EXIT_FAILURE;
	}

	/* Generate test identifiers */
	for (size_t iter = 0; iter < UNIQUE_COUNT; ++iter) {
		if (unlikely(!unique(&ids[iter * 4]))) {
			perror("unique");
			return EXIT_FAILURE;
		}
	}

	unsigned long acc = 0;

	/* Calculate hamming distance */
	for (size_t iter = 0; iter < UNIQUE_COUNT; ++iter)
		for (size_t jter = iter + 1; jter < UNIQUE_COUNT; ++jter)
			acc += hamming(&ids[iter * 4], &ids[jter * 4]);

	double avg = (double) acc / (double) (UNIQUE_COUNT * UNIQUE_COUNT - UNIQUE_COUNT);
	printf("unique: Average hamming distance %f\n", avg);

	/* FIXME: This value was choosen arbitrarily */
	if (unlikely(avg < 8.0)) {
		fputs("Average hamming distance unreasonably low\n", stderr);
		return EXIT_FAILURE;
	}

	free(ids);

	return EXIT_SUCCESS;
}
#endif
