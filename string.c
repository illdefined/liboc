/**
 * \file
 *
 * \brief String manipulation.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "expect.h"
#include "string.h"

/**
 * \brief Convert 4bit integer to hexadecimal ASCII character.
 *
 * \param nibble Integer to convert.
 *
 * \return Hexadecimal ASCII character.
 */
static inline uint8_t hexchar(uint8_t nibble) {
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
 */
void hexstr(char *restrict buf, const void *src, size_t size) {
	const uint8_t *num = (const uint8_t *) src;

	for (size_t idx = 0; idx < size; ++idx) {
		/* High nibble */
		buf[idx + 0] = hexchar(num[idx] / UINT8_C(0x10));

		/* Low nibble */
		buf[idx + 1] = hexchar(num[idx] % UINT8_C(0x10));
	}

	/* Zero‐terminate result */
	buf[size * 2] = '\0';
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
	char *result = (char *) 0;

	size_t size = strlen(prefix) + 1;
	char *buf = malloc(size);
	if (unlikely(!buf))
		goto egress0;

	strcpy(buf, prefix);

	va_list ap;
	va_start(ap, prefix);

	for (;;) {
		const char *arg = va_arg(ap, const char *);

		if (unlikely(!arg))
			break;

		void *nbuf = realloc(buf, size + strlen(arg));
		if (unlikely(!nbuf))
			goto egress1;
		else
			buf = nbuf;

		strcpy(&buf[size - 1], arg);
		size += strlen(arg);
	}

	va_end(ap);

	result = buf;
	goto egress0;

egress1:
	free(buf);

egress0:
	return result;
}

