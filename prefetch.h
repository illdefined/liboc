#pragma once
#ifndef OC_PREFETCH_H
#define OC_PREFETCH_H

/**
 * \file
 *
 * \brief Memory prefetching macros.
 */

/**
 * \def prefetch(addr, ...)
 *
 * \brief Prefetch memory.
 *
 * \param addr Memory address.
 */
#if defined(__clang__) || defined(__GNUC__)
# define prefetch(addr, ...) __builtin_prefetch(addr, __VA_ARGS__)
#else
# define prefetch(addr, ...)
#endif

#endif /* OC_PREFETCH_H */
