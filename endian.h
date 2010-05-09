#pragma once
#ifndef OC_ENDIAN_H
#define OC_ENDIAN_H

#include <stdint.h>

#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER BIG_ENDIAN

extern inline uint16_t swap16(uint16_t x) {
	return
		(x & UINT16_C(0xff00)) >> 8 |
		(x & UINT16_C(0x00ff)) << 8;
}

extern inline uint32_t swap32(uint32_t x) {
	if (__builtin_constant_p(x))
		return
			(x & UINT32_C(0xff000000)) >> 24 |
			(x & UINT32_C(0x00ff0000)) >>  8 |
			(x & UINT32_C(0x0000ff00)) <<  8 |
			(x & UINT32_C(0x000000ff)) << 24;
	else
		return __builtin_bswap32(x);
}

extern inline uint32_t swap64(uint64_t x) {
	if (__builtin_constant_p(x))
		return
			(x & UINT64_C(0xff00000000000000)) >> 56 |
			(x & UINT64_C(0x00ff000000000000)) >> 40 |
			(x & UINT64_C(0x0000ff0000000000)) >> 24 |
			(x & UINT64_C(0x000000ff00000000)) >>  8 |
			(x & UINT64_C(0x00000000ff000000)) <<  8 |
			(x & UINT64_C(0x0000000000ff0000)) << 24 |
			(x & UINT64_C(0x000000000000ff00)) << 40 |
			(x & UINT64_C(0x00000000000000ff)) << 56;
	else
		return __builtin_bswap64(x);
}

#if BYTE_ORDER == BIG_ENDIAN

extern inline uint16_t tobe16(uint16_t x) {
	return x;
}

extern inline uint32_t tobe32(uint32_t x) {
	return x;
}

extern inline uint64_t tobe64(uint64_t x) {
	return x;
}

extern inline uint16_t tole16(uint16_t x) {
	return swap16(x);
}

extern inline uint32_t tole32(uint32_t x) {
	return swap32(x);
}

extern inline uint64_t tole64(uint64_t x) {
	return swap64(x);
}

#elif BYTE_ORDER == LITTLE_ENDIAN

extern inline uint16_t tobe16(uint16_t x) {
	return swap16(x);
}

extern inline uint32_t tobe32(uint32_t x) {
	return swap32(x);
}

extern inline uint64_t tobe64(uint64_t x) {
	return swap64(x);
}

extern inline uint16_t tole16(uint16_t x) {
	return x;
}

extern inline uint32_t tole32(uint32_t x) {
	return x;
}

extern inline uint64_t tole64(uint64_t x) {
	return x;
}

#else
#error "Unknown byte‐order!"
#endif

#endif /* OC_ENDIAN_H */
