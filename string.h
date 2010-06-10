#pragma once
#ifndef OC_STRING_H
#define OC_STRING_H

/**
 * \file
 *
 * \brief String manipulation.
 */

#include <stdbool.h>
#include <stdint.h>

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
extern void inthexs(char *restrict buf, const void *restrict src, size_t size);

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
extern bool hexsint(void *restrict dest, const char *restrict src, size_t size);

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
extern char *concat(const char *restrict prefix, ...);

#endif
