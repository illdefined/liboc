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
#define TYPE_OUT (UINT64_C(63) << 56)

#define PARITY UINT64_C(0x5555555555555555)
#define ROUNDS 72

/**
 * \brief Skein rotation constants
 */
static const uint_least8_t skein_rot[][2] = {
	{  5, 56 },
	{ 36, 28 },
	{ 13, 46 },
	{ 58, 44 },
	{ 26, 20 },
	{ 53, 35 },
	{ 11, 42 },
	{ 59, 50 }
};

/**
 * \brief Skein initialisation vector
 */
static const uint64_t skein_iv[] = {
	UINT64_C(0x388512680e660046),
	UINT64_C(0x4b72d5dec5a8ff01),
	UINT64_C(0x281a9298ca5eb3a5),
	UINT64_C(0x54ca5249f46070c4)
};

/**
 * \brief Process full blocks of message.
 */
static void skein_block(struct skein *restrict ctx, const uint8_t *restrict mesg, size_t nblk, size_t blen) {
	uint64_t tweak[TWEAK_WORDS + 1];
	uint64_t chain[SKEIN_WORDS + 1];
	uint64_t state[SKEIN_WORDS];
	uint64_t block[SKEIN_WORDS];

	while (nblk--) {
		/* Catch integer overflow */
		assert(ctx->tweak[0] + blen >= blen);

		ctx->tweak[0] += blen;

		/* Precompute key schedule */
		chain[SKEIN_WORDS] = PARITY;

		for (size_t word = 0; word < SKEIN_WORDS; ++word) {
			chain[word]         = ctx->chain[word];
			chain[SKEIN_WORDS] ^= ctx->chain[word];
		}

		tweak[TWEAK_WORDS] = 0;

		for (size_t word = 0; word < TWEAK_WORDS; ++word) {
			tweak[word]         = ctx->tweak[word];
			tweak[TWEAK_WORDS] ^= ctx->tweak[word];
		}

		/* Get message block in little‐endian byte‐order */
		for (size_t word = 0; word < SKEIN_WORDS; ++word)
			for (size_t kter = 0; kter < sizeof block[word]; ++kter)
				block[word] = mesg[word * sizeof block[word] + kter] << kter * 8;

		/* First full key injection */
		for (size_t word = 0; word < SKEIN_WORDS; ++word)
			state[word] = block[word] + chain[word];

		state[SKEIN_WORDS - 3] += tweak[0];
		state[SKEIN_WORDS - 2] += tweak[1];

		for (size_t rnd = 0; rnd < ROUNDS / 4; ++rnd) {
			for (size_t sub = 0; sub < 4; ++sub) {
				state[0] += state[1 + sub % 2 * 2];
				state[1 + sub % 2 * 2] =
					rotl(state[1 + sub % 2 * 2], skein_rot[rnd % 2 * 4 + sub][0]);
				state[1 + sub % 2 * 2] ^= state[0];

				state[2] += state[3 - sub % 2 * 2];
				state[3 - sub % 2 * 2] =
					rotl(state[3 - sub % 2 * 2], skein_rot[rnd % 2 * 4 + sub][1]);
				state[3 - sub % 2 * 2] ^= state[2];
			}

			/* Key injection */
			for (size_t word = 0; word < SKEIN_WORDS; ++word)
				state[word] += chain[(rnd + word + 1) % (SKEIN_WORDS + 1)];

			state[SKEIN_WORDS - 3] += tweak[(rnd + 1) % (TWEAK_WORDS + 1)];
			state[SKEIN_WORDS - 2] += tweak[(rnd + 2) % (TWEAK_WORDS + 1)];
			state[SKEIN_WORDS - 1] += rnd + 1;
		}

		/* Feedforward XOR */
		for (size_t word = 0; word < SKEIN_WORDS; ++word)
			ctx->chain[word] = state[word] ^ block[word];

		ctx->tweak[1] &= ~FLAG_FIRST;
		mesg += SKEIN_BYTES;
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

	memcpy(ctx->chain, skein_iv, SKEIN_BYTES);

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
void skein_plug(struct skein *restrict ctx, uint8_t *restrict hash) {
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
	for (size_t byte = 0; byte < SKEIN_BYTES; ++byte)
		hash[byte] = ctx->chain[byte / sizeof (uint64_t)] >> byte % sizeof (uint64_t) * 8;
}

/**
 * \brief Hash message.
 *
 * \param hash Buffer to hold hash.
 * \param mesg Message to hash.
 * \param size Size of message.
 */
void skein(uint8_t *restrict hash, const uint8_t *restrict mesg, size_t size) {
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
	uint8_t hash[SKEIN_BYTES];

	skein(hash, (uint8_t *) 0, 0);

	if (memcmp(hash, test, sizeof hash)) {
		fputs("Hash differs from test vector!\n0x", stderr);

		for (size_t iter = 0; iter < sizeof hash; ++iter)
			fprintf(stderr, "%02" PRIx8, hash[iter]);
		putc('\n', stderr);

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#endif /* TEST */
