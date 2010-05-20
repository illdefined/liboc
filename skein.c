/**
 * \file
 *
 * \brief Skein hash function
 */

#include <stdint.h>
#include <string.h>

#include "endian.h"
#include "function.h"
#include "rotate.h"

#include "skein.h"

/**
 * \brief Initialisation vector.
 */
static const uint64_t iv[] = {
	UINT64_C(0x388512680e660046),
	UINT64_C(0x4b72d5dec5a8ff01),
	UINT64_C(0x281a9298ca5eb3a5),
	UINT64_C(0x54ca5249f46070c4),
};

/**
 * \brief Initialise skein context.
 *
 * \param ctx Skein context.
 */
__flatten void skein_init(struct skein *restrict ctx) {
}

/**
 * \brief Hash data.
 *
 * \param ctx  Skein context.
 * \param data Data to hash.
 * \param size Size of data.
 */
__flatten void skein_feed(struct skein *restrict ctx, const uint8_t *restrict data __aligned(sizeof (uint64_t)), size_t size) {
}

/**
 * \brief Finalise hash.
 *
 * \param ctx  Skein context.
 * \param hash Buffer to hold hash.
 */
__flatten void skein_plug(struct skein *restrict ctx, uint64_t[restrict 4]) {
}

/**
 * \brief Hash a single data block.
 *
 * \param block Data block to hash.
 * \param hash  Buffer to hold hash.
 */
__flatten void skein(const uint64_t block[restrict 4], uint64_t hash[restrict 4]) {
	struct skein ctx;

	skein_init(&ctx);
	skein_feed(&ctx, (const uint8_t *) block);
	skein_plug(&ctx, hash);
}
