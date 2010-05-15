#pragma once
#ifndef OC_ENDIAN_H
#define OC_ENDIAN_H

#include <stdint.h>

#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER BIG_ENDIAN

#define swap16(x) \
	(((x) & UINT16_C(0xff00)) >> 8 | \
	 ((x) & UINT16_C(0x00ff)) << 8)

#define swap32(x) \
	(__builtin_constant_p(x) ? \
		((x) & UINT32_C(0xff000000)) >> 24 | \
		((x) & UINT32_C(0x00ff0000)) >>  8 | \
		((x) & UINT32_C(0x0000ff00)) <<  8 | \
		((x) & UINT32_C(0x000000ff)) << 24 : \
		__builtin_bswap32(x))

#define swap64(x) \
	(__builtin_constant_p(x) ? \
			((x) & UINT64_C(0(x)ff00000000000000)) >> 56 | \
			((x) & UINT64_C(0(x)00ff000000000000)) >> 40 | \
			((x) & UINT64_C(0(x)0000ff0000000000)) >> 24 | \
			((x) & UINT64_C(0(x)000000ff00000000)) >>  8 | \
			((x) & UINT64_C(0(x)00000000ff000000)) <<  8 | \
			((x) & UINT64_C(0(x)0000000000ff0000)) << 24 | \
			((x) & UINT64_C(0(x)000000000000ff00)) << 40 | \
			((x) & UINT64_C(0(x)00000000000000ff)) << 56 : \
			__builtin_bswap64((x))

#if BYTE_ORDER == BIG_ENDIAN

#define tobe16(x) x
#define tobe32(x) x
#define tobe64(x) x

#define tole16(x) swap16(x)
#define tole32(x) swap32(x)
#define tole64(x) swap64(x)

#elif BYTE_ORDER == LITTLE_ENDIAN

#define tobe16(x) swap16(x)
#define tobe32(x) swap32(x)
#define tobe64(x) swap64(x)

#define tole16(x) x
#define tole32(x) x
#define tole64(x) x

#else
#error "Unknown byte‐order!"
#endif

#endif /* OC_ENDIAN_H */
