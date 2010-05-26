#pragma once
#ifndef OC_SKEIN_H
#define OC_SKEIN_H

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
 * \brief Skein context structure.
 */
struct skein {
	uint64_t tweak[TWEAK_WORDS]; /**< Tweak words. */
	uint64_t chain[SKEIN_WORDS]; /**< Chaining variables. */
	uint8_t  block[SKEIN_WORDS * sizeof (uint64_t)]; /**< Partial block buffer. */
	size_t   level;    /**< Buffer fill level. */
};

void skein_init(struct skein *restrict);
void skein_feed(struct skein *restrict, const uint8_t *restrict, size_t);
void skein_plug(struct skein *restrict, uint64_t[restrict SKEIN_WORDS]);

void skein(uint64_t[restrict SKEIN_WORDS], const uint8_t *restrict, size_t);

#endif /* OC_SKEIN_H */
