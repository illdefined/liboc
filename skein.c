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
 * \brief Initialise context for incremental hashing.
 *
 * \param ctx Skein context.
 */
__flatten void skein_init(struct skein *restrict ctx) {
}

/**
 * \brief Hash data incrementally.
 *
 * \param ctx  Skein context.
 * \param data Data to hash.
 * \param size Size of data.
 */
__flatten void skein_feed(struct skein *restrict ctx, const uint8_t *restrict data, size_t size) {
}

/**
 * \brief Finalise incremental hashing.
 *
 * \param ctx  Skein context.
 * \param hash Buffer to hold hash.
 *
 * The \c skein_plug function finalises the incremental hashing and
 * writes the hash value to \a hash.
 */
__flatten void skein_plug(struct skein *restrict ctx, uint64_t hash[restrict 4]) {
}

/**
 * \brief Hash data.
 *
 * \param hash Buffer to hold hash.
 * \param data Data to hash.
 * \param size Size of data.
 */
void skein(uint64_t hash[restrict 4], const uint8_t *restrict data, size_t size) {
	struct skein ctx;

	skein_init(&ctx);
	skein_feed(&ctx, data, size);
	skein_plug(&ctx, hash);
}
