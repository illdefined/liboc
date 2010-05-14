#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "endian.h"
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
 * \brief Generate unique idifier.
 *
 * \param id Buffer to hold idifier.
 *
 * \return True on success or false on failure.
 */
bool unique(uint64_t id[restrict 4]) {
	/* Open random device */
	int rnd = open(RANDOM_DEVICE, O_RDONLY);
	if (rnd < 0)
		return false;

	/* Read random data */
	ssize_t cnt = read(rnd, id + 2, sizeof id - 2 * sizeof *id);
	if (cnt < sizeof id - 2 * sizeof *id) {
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

	if (clock_gettime(CLOCK_REALTIME, &now))
		return false;

	/* TAI in nanoseconds */
	id[0] = tobe64((now.tv_sec + TAI_OFFSET) * UINT64_C(1000000000) +
		now.tv_nsec);

	/* CPU cycle counter */
	id[1] = tobe64(hardtick());

	return true;
}
