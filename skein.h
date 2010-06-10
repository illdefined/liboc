#pragma once
#ifndef OC_SKEIN_H
#define OC_SKEIN_H

/**
 * \file
 *
 * \brief Skein hash function.
 */

#include <stdint.h>

/**
 * \brief Number of tweak words.
 */
#define TWEAK_WORDS 2

/**
 * \brief Words per block.
 */
#define SKEIN_WORDS 4

/**
 * \brief Bytes per block.
 */
#define SKEIN_BYTES (SKEIN_WORDS * sizeof (uint64_t))

/**
 * \brief Skein context structure.
 */
struct skein {
	uint64_t tweak[TWEAK_WORDS]; /**< Tweak words. */
	uint64_t chain[SKEIN_WORDS]; /**< Chaining variables. */
	uint8_t  block[SKEIN_BYTES]; /**< Partial block buffer. */
	size_t   level;    /**< Buffer fill level. */
};

/**
 * \brief Initialise context for incremental hashing.
 *
 * \param ctx Skein context.
 */
extern void skein_init(struct skein *restrict ctx);

/**
 * \brief Hash message incrementally.
 *
 * \param ctx  Skein context.
 * \param mesg Message to hash.
 * \param size Size of message.
 */
extern void skein_feed(struct skein *restrict ctx, const void *restrict mesg, size_t size);

/**
 * \brief Finalise incremental hashing.
 *
 * \param ctx  Skein context.
 * \param hash Buffer to hold hash.
 *
 * The \c skein_plug function finalises the incremental hashing and
 * writes the hash value to \a hash.
 */
extern void skein_plug(struct skein *restrict ctx, uint8_t hash[restrict SKEIN_BYTES]);

/**
 * \brief Hash message.
 *
 * \param hash Buffer to hold hash.
 * \param mesg Message to hash.
 * \param size Size of message.
 */
extern void skein(uint8_t hash[restrict SKEIN_BYTES], const void *restrict mesg, size_t size);

#endif /* OC_SKEIN_H */
