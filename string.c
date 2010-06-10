/**
 * \file
 *
 * \brief String manipulation.
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "egress.h"
#include "expect.h"
#include "string.h"

/**
 * \brief Convert 4bit integer to hexadecimal ASCII character.
 *
 * \param nibble Integer to convert.
 *
 * \return Hexadecimal ASCII character.
 */
static inline uint8_t inthexc(uint8_t nibble) {
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
 * \brief Convert big‐endian integer to hexadecimal ASCII string.
 *
 * \param buf Buffer to hold ASCII string.
 * \param src Pointer to integer.
 * \param size Size of integer.
 *
 * The string will be zero‐terminated and therefore \a buf must be able
 * to hold at least 2 × \a size + 1 bytes.
 */
void inthexs(char *restrict buf, const void *restrict src, size_t size) {
	const uint8_t *num = (const uint8_t *) src;

	for (size_t idx = 0; idx < size; ++idx) {
		/* High nibble */
		buf[2 * idx + 0] = inthexc(num[idx] / UINT8_C(0x10));

		/* Low nibble */
		buf[2 * idx + 1] = inthexc(num[idx] % UINT8_C(0x10));
	}

	/* Zero‐terminate result */
	buf[size * 2] = '\0';
}

/**
 * \brief Convert hexadecimal ASCII character to 4bit integer.
 *
 * \param nibble Character to convert.
 *
 * \return 4bit integer.
 */
static inline uint8_t hexcint(uint8_t nibble) {
	uint8_t result;

	assert(nibble >= '0' && nibble <= '9' || nibble >= 'A' && nibble <= 'F' || nibble >= 'a' && nibble <= 'f');

	switch (nibble) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		result = nibble - (uint8_t) '0';
		break;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		result = nibble - (uint8_t) 'A';
		break;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		result = nibble - (uint8_t) 'a';
		break;
	}

	assert(result <= 0xF);

	return result;
}

/**
 * \brief Convert hexadecimal ASCII string to big‐endian integer.
 *
 * \param dest Pointer to integer.
 * \param src ASCII string.
 * \param size Size of integer.
 *
 * The string must hold exactly 2 × \a size hexadecimal characters, but
 * does not need to be zero‐terminated.
 *
 * \return \c true on success or \c false if \a src is invalid.
 */
bool hexsint(void *restrict dest, const char *restrict src, size_t size) {
	prime(bool);

	uint8_t *num = (uint8_t *) dest;

	for (size_t idx = 0; idx < size; ++idx) {
		/* Check character range */
		for (size_t byte = 0; byte < 2; ++byte) {
			if (unlikely(
				src[2 * idx + byte] < '0' ||
				src[2 * idx + byte] > '9' && src[2 * idx + byte] < 'A' ||
				src[2 * idx + byte] > 'F' && src[2 * idx + byte] < 'a' ||
				src[2 * idx + byte] > 'f')) {
				egress(0, false, EINVAL);
			}
		}

		num[idx] =
			hexcint(src[2 * idx]) * UINT8_C(0x10) |
			hexcint(src[2 * idx + 1]);
	}

	egress(0, true, errno);

egress0:
	final();
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
char *concat(const char *restrict prefix, ...) {
	prime(char *);

	size_t size = strlen(prefix) + 1;
	char *buf = malloc(size);
	if (unlikely(!buf))
		egress(0, (char *) 0, errno);

	strcpy(buf, prefix);

	va_list ap;
	va_start(ap, prefix);

	for (;;) {
		const char *arg = va_arg(ap, const char *);

		if (unlikely(!arg))
			break;

		void *nbuf = realloc(buf, size + strlen(arg));
		if (unlikely(!nbuf))
			egress(1, (char *) 0, errno);
		else
			buf = nbuf;

		strcpy(&buf[size - 1], arg);
		size += strlen(arg);
	}

	va_end(ap);

	egress(0, buf, errno);

egress1:
	free(buf);

egress0:
	final();
}
