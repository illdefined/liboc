/**
 * \file
 *
 * \brief Skein hash function
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "endian.h"
#include "function.h"
#include "prefetch.h"
#include "rotate.h"

#include "skein.h"

#define FLAG_FIRST (UINT64_C(1) << 62)
#define FLAG_FINAL (UINT64_C(1) << 63)

#define TYPE_MSG (UINT64_C(48) << 56)
#define TYPE_OUT (UINT64_C(48) << 63)

/**
 * \brief Process full blocks of message.
 */
static void skein_block(struct skein *restrict ctx, const uint8_t *restrict mesg, size_t nblk, size_t blen) {
	uint64_t tweak[TWEAK_WORDS + 1];
	uint64_t chain[SKEIN_WORDS + 1];
	uint64_t state[SKEIN_WORDS];
	uint64_t block[SKEIN_WORDS];

#define inject(rnd) \
	for (size_t jter = 0; jter < SKEIN_WORDS; ++jter) \
		state[jter] += chain[((rnd) + jter) % (SKEIN_WORDS + 1)]; \
	state[SKEIN_WORDS - 3] += tweak[((rnd) + 0) % (TWEAK_WORDS + 1)]; \
	state[SKEIN_WORDS - 2] += tweak[((rnd) + 1) % (TWEAK_WORDS + 1)]; \
	state[SKEIN_WORDS - 1] += (rnd);

#define round(i0, i1, i2, i3, r0, r1) \
	state[i0] += state[i1]; \
	state[i1] = rotl(state[i1], r0); \
	state[i1] ^= state[i0]; \
	state[i2] += state[i3]; \
	state[i3] = rotl(state[i3], r1); \
	state[i3] ^= state[i2]; \

	for (size_t iter = 0; iter < nblk; ++iter) {
		/* Catch integer overflow */
		assert(ctx->tweak[0] + blen >= blen);

		ctx->tweak[0] += blen;

		/* Precompute key schedule */
		chain[SKEIN_WORDS] = UINT64_C(0x5555555555555555);

		for (size_t jter = 0; jter < SKEIN_WORDS; ++jter) {
			chain[jter]         = ctx->chain[jter];
			chain[SKEIN_WORDS] ^= ctx->chain[jter];
		}

		tweak[TWEAK_WORDS] = 0;

		for (size_t jter = 0; jter < TWEAK_WORDS; ++jter) {
			tweak[jter]         = ctx->tweak[jter];
			tweak[TWEAK_WORDS] ^= ctx->tweak[jter];
		}

		/* Get message block in little‐endian byte‐order */
		for (size_t jter = 0; jter < SKEIN_WORDS; ++jter)
			for (size_t kter = 0; kter < sizeof block[jter]; ++kter)
				block[jter] = mesg[jter * sizeof block[jter] + kter] << kter * 8;

		/* First full key injection */
		for (size_t jter = 0; jter < SKEIN_WORDS; ++jter)
			state[jter] = block[jter] + chain[jter];

		state[SKEIN_WORDS - 3] += tweak[0];
		state[SKEIN_WORDS - 2] += tweak[1];

		for (size_t round = 0; round < 72 / 8; ++round) {
			/* First tetrad */
			round(0, 1, 2, 3,  5, 56);
			round(0, 3, 2, 1, 36, 28);
			round(0, 1, 2, 3, 13, 46);
			round(0, 3, 2, 1, 58, 44);

			/* Key injection */
			inject(2 * round + 1);

			/* Second tetrad */
			round(0, 1, 2, 3, 26, 20);
			round(0, 3, 2, 1, 53, 35);
			round(0, 1, 2, 3, 11, 42);
			round(0, 3, 2, 1, 59, 50);

			/* Key injection */
			inject(2 * round + 2);
		}

		/* Feedforward XOR */
		for (size_t jter = 0; jter < SKEIN_WORDS; ++jter)
			ctx->chain[jter] = state[jter] ^ block[jter];

		ctx->tweak[1] &= ~FLAG_FIRST;

		mesg += sizeof block;
	}
}

/**
 * \brief Initialise context for incremental hashing.
 *
 * \param ctx Skein context.
 */
void skein_init(struct skein *restrict ctx) {
	ctx->tweak[0] = 0;
	ctx->tweak[1] = FLAG_FIRST | TYPE_MSG;

	/* Initialisation vector */
	ctx->chain[0] = UINT64_C(0x388512680e660046);
	ctx->chain[1] = UINT64_C(0x4b72d5dec5a8ff01);
	ctx->chain[2] = UINT64_C(0x281a9298ca5eb3a5);
	ctx->chain[3] = UINT64_C(0x54ca5249f46070c4);

	ctx->level = 0;
}

/**
 * \brief Hash message incrementally.
 *
 * \param ctx  Skein context.
 * \param mesg Message to hash.
 * \param size Size of message.
 */
void skein_feed(struct skein *restrict ctx, const uint8_t *restrict mesg, size_t size) {
	/* Catch integer overflow */
	assert(ctx->level + size >= size);

	if (ctx->level + size > SKEIN_BYTES) {
		if (ctx->level) {
			size_t rem = SKEIN_BYTES - ctx->level;

			if (rem) {
				memcpy(&ctx->block[ctx->level], mesg, rem);
				size -= rem;
				mesg += rem;
			}

			skein_block(ctx, ctx->block, 1, SKEIN_BYTES);
			ctx->level = 0;
		}

		/* Process any remaining blocks */
		if (size > SKEIN_BYTES) {
			size_t blk = (size - 1) / SKEIN_BYTES;

			skein_block(ctx, mesg, blk, SKEIN_BYTES);
			size -= blk * SKEIN_BYTES;
			mesg += blk * SKEIN_BYTES;
		}
	}

	/* Copy remaining partial block */
	if (size) {
		memcpy(&ctx->block[ctx->level], mesg, size);
		ctx->level += size;
	}
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
void skein_plug(struct skein *restrict ctx, uint64_t hash[restrict SKEIN_WORDS]) {
	/* Mark as the final block */
	ctx->tweak[1] |= FLAG_FINAL;

	/* Zero‐pad final block */
	if (ctx->level < SKEIN_BYTES)
		memset(&ctx->block[ctx->level], 0, SKEIN_BYTES - ctx->level);
	skein_block(ctx, ctx->block, 1, ctx->level);

	/* Generate output */
	ctx->tweak[0] = 0;
	ctx->tweak[1] = FLAG_FIRST | FLAG_FINAL | TYPE_OUT;

	memset(ctx->block, 0, SKEIN_BYTES);

	skein_block(ctx, ctx->block, 1, sizeof (uint64_t));

	/* Write hash value */
	for (size_t iter = 0; iter < SKEIN_WORDS; ++iter)
		hash[iter] = le64(ctx->chain[iter]);
}

/**
 * \brief Hash message.
 *
 * \param hash Buffer to hold hash.
 * \param mesg Message to hash.
 * \param size Size of message.
 */
void skein(uint64_t hash[restrict SKEIN_WORDS], const uint8_t *restrict mesg, size_t size) {
	struct skein ctx;

	skein_init(&ctx);
	skein_feed(&ctx, mesg, size);
	skein_plug(&ctx, hash);
}

#ifdef TEST
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * \brief Test vector.
 *
 * Going to add further test vectors in the future.
 */
static const uint64_t test[] = {
	be64(UINT64_C(0xbc2763f707e262b8)),
	be64(UINT64_C(0x0e0313791543a7ab)),
	be64(UINT64_C(0x0a4b6cd083270afb)),
	be64(UINT64_C(0x2fce4272e1bb0aa9))
};

/**
 * \brief Skein hash test routine.
 *
 * \return \c EXIT_SUCCESS if successful or \c EXIT_FAILURE on failure.
 */
int main(void) {
	uint64_t hash[SKEIN_WORDS];

	skein(hash, (uint8_t *) 0, 0);

	if (memcmp(hash, test, sizeof hash)) {
		fputs("Hash differs from test vector!\n0x", stderr);

		for (size_t iter = 0; iter < sizeof hash; ++iter)
			fprintf(stderr, "%02" PRIx8, ((uint8_t *) hash)[iter]);
		putc('\n', stderr);

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#endif /* TEST */
