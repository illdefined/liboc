#pragma once
#ifndef OC_SKEIN_H
#define OC_SKEIN_H

#include <stdint.h>

#include "variable.h"

/**
 * \brief Skein context structure.
 */
struct skein {
	uint64_t tweak[2]; /**< Tweak words. */
	uint64_t state[4]; /**< State. */
	uint8_t  block[4 * sizeof (uint64_t)] __aligned(sizeof (uint64_t)); /**< Partial block buffer. */
	size_t   level;    /**< Buffer fill level. */
};

void skein_init(struct skein *restrict);
void skein_feed(struct skein *restrict, const uint8_t *restrict __aligned(sizeof (uint64_t)), size_t);
void skein_plug(struct skein *restrict, uint64_t[restrict 4]);

void skein(const uint64_t[restrict 4], uint64_t[restrict 4]);

#endif /* OC_SKEIN_H */
