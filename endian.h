#pragma once
#ifndef OC_ENDIAN_H
#define OC_ENDIAN_H

/**
 * \file
 *
 * \brief Byte‐order conversion macros.
 */

#include <stdint.h>

#include "constancy.h"

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN
#undef BYTE_ORDER

#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER LITTLE_ENDIAN

/**
 * \brief Swap octets of a 16bit integer.
 *
 * \param x Integer.
 *
 * \return Swapped integer.
 */
#define swap16(x) \
	(((x) & UINT16_C(0xff00)) >> 8 | \
	 ((x) & UINT16_C(0x00ff)) << 8)

/**
 * \brief Swap octets of a 32bit integer.
 *
 * \param x Integer.
 *
 * \return Swapped integer.
 */
#define swap32(x) \
	(constant_p(x) ? \
		((x) & UINT32_C(0xff000000)) >> 24 | \
		((x) & UINT32_C(0x00ff0000)) >>  8 | \
		((x) & UINT32_C(0x0000ff00)) <<  8 | \
		((x) & UINT32_C(0x000000ff)) << 24 : \
		__builtin_bswap32(x))

/* Work around sparse issue */
#ifdef __CHECKER__
# define __builtin_bswap64(x) (x)
#endif

/**
 * \brief Swap octets of a 64bit integer.
 *
 * \param x Integer.
 *
 * \return Swapped integer.
 */
#define swap64(x) \
	(constant_p(x) ? \
			((x) & UINT64_C(0xff00000000000000)) >> 56 | \
			((x) & UINT64_C(0x00ff000000000000)) >> 40 | \
			((x) & UINT64_C(0x0000ff0000000000)) >> 24 | \
			((x) & UINT64_C(0x000000ff00000000)) >>  8 | \
			((x) & UINT64_C(0x00000000ff000000)) <<  8 | \
			((x) & UINT64_C(0x0000000000ff0000)) << 24 | \
			((x) & UINT64_C(0x000000000000ff00)) << 40 | \
			((x) & UINT64_C(0x00000000000000ff)) << 56 : \
			__builtin_bswap64(x))

/**
 * \def be16(x)
 *
 * \brief Convert native 16bit integer from or to big‐endian byte‐order.
 *
 * \return Converted integer.
 */

/**
 * \def be32(x)
 *
 * \brief Convert native 32bit integer from or to big‐endian byte‐order.
 *
 * \return Converted integer.
 */

/**
 * \def be64(x)
 *
 * \brief Convert native 64bit integer from or to big‐endian byte‐order.
 *
 * \return Converted integer.
 */

/**
 * \def le16(x)
 *
 * \brief Convert native 16bit integer from or to little‐endian byte‐order.
 *
 * \return Converted integer.
 */

/**
 * \def le32(x)
 *
 * \brief Convert native 32bit integer from or to little‐endian byte‐order.
 *
 * \return Converted integer.
 */

/**
 * \def le64(x)
 *
 * \brief Convert native 64bit integer from or to little‐endian byte‐order.
 *
 * \return Converted integer.
 */

#if BYTE_ORDER == BIG_ENDIAN

#define be16(x) x
#define be32(x) x
#define be64(x) x

#define le16(x) swap16(x)
#define le32(x) swap32(x)
#define le64(x) swap64(x)

#elif BYTE_ORDER == LITTLE_ENDIAN

#define be16(x) swap16(x)
#define be32(x) swap32(x)
#define be64(x) swap64(x)

#define le16(x) x
#define le32(x) x
#define le64(x) x

#else
#error "Unknown byte‐order!"
#endif

#endif /* OC_ENDIAN_H */
