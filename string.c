#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "egress.h"
#include "expect.h"
#include "function.h"
#include "string.h"

/**
 * \brief Convert 4bit integer to hexadecimal ASCII character.
 *
 * \param nibble Integer to convert.
 *
 * \return Hexadecimal ASCII character.
 */
static inline constant uint8_t inthexc(uint8_t nibble) {
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
		result = nibble - 0xa + (uint8_t) 'a';
		break;
	}

	return result;
}

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
static inline constant uint8_t hexcint(uint8_t nibble) {
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
		result = nibble - (uint8_t) 'A' + 0xa;
		break;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		result = nibble - (uint8_t) 'a' + 0xa;
		break;
	}

	assert(result <= 0xF);

	return result;
}

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

#ifdef TEST
#include <stdlib.h>

#include "essai.h"

int main(void) {
	essaye(inthexc(0x0) == '0');
	essaye(inthexc(0x1) == '1');
	essaye(inthexc(0x2) == '2');
	essaye(inthexc(0x3) == '3');
	essaye(inthexc(0x4) == '4');
	essaye(inthexc(0x5) == '5');
	essaye(inthexc(0x6) == '6');
	essaye(inthexc(0x7) == '7');
	essaye(inthexc(0x8) == '8');
	essaye(inthexc(0x9) == '9');
	essaye(inthexc(0xa) == 'a');
	essaye(inthexc(0xb) == 'b');
	essaye(inthexc(0xc) == 'c');
	essaye(inthexc(0xd) == 'd');
	essaye(inthexc(0xe) == 'e');
	essaye(inthexc(0xf) == 'f');

	essaye(hexcint('0') == 0x0);
	essaye(hexcint('1') == 0x1);
	essaye(hexcint('2') == 0x2);
	essaye(hexcint('3') == 0x3);
	essaye(hexcint('4') == 0x4);
	essaye(hexcint('5') == 0x5);
	essaye(hexcint('6') == 0x6);
	essaye(hexcint('7') == 0x7);
	essaye(hexcint('8') == 0x8);
	essaye(hexcint('9') == 0x9);
	essaye(hexcint('a') == 0xa);
	essaye(hexcint('A') == 0xa);
	essaye(hexcint('b') == 0xb);
	essaye(hexcint('B') == 0xb);
	essaye(hexcint('c') == 0xc);
	essaye(hexcint('C') == 0xc);
	essaye(hexcint('d') == 0xd);
	essaye(hexcint('D') == 0xd);
	essaye(hexcint('e') == 0xe);
	essaye(hexcint('E') == 0xe);
	essaye(hexcint('f') == 0xf);
	essaye(hexcint('F') == 0xf);

	char *test;
	essaye((test = concat("foo", (char *) 0)) && !strcmp(test, "foo"));
	free(test);

	essaye((test = concat("foo", "bar", (char *) 0)) && !strcmp(test, "foobar"));
	free(test);

	essaye((test = concat("foo", "bar", "foo", (char *) 0)) && !strcmp(test, "foobarfoo"));
	free(test);

	return EXIT_SUCCESS;
}
#endif /* TEST */
