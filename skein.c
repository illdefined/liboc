/**
 * \file
 *
 * \brief Skein hash function
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "endian.h"
#include "expect.h"
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
void skein_plug(struct skein *restrict ctx, uint8_t hash[restrict SKEIN_BYTES]) {
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
void skein(uint8_t hash[restrict SKEIN_BYTES], const uint8_t *restrict mesg, size_t size) {
	struct skein ctx;

	skein_init(&ctx);
	skein_feed(&ctx, mesg, size);
	skein_plug(&ctx, hash);
}

#ifdef TEST
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define TESTS (sizeof test / sizeof *test)

/**
 * \brief Test vectors.
 */
static const uint64_t test[][4] = {
	{
		be64(UINT64_C(0xbc2763f707e262b8)),
		be64(UINT64_C(0x0e0313791543a7ab)),
		be64(UINT64_C(0x0a4b6cd083270afb)),
		be64(UINT64_C(0x2fce4272e1bb0aa9))
	},
	{
		be64(UINT64_C(0xa6cabf2db01a6f27)),
		be64(UINT64_C(0x424042936439928c)),
		be64(UINT64_C(0xa83854935294df98)),
		be64(UINT64_C(0x19e7c4c661fc319f))
	},
	{
		be64(UINT64_C(0x4cc83ba92b0397fa)),
		be64(UINT64_C(0x74cb833495f6294c)),
		be64(UINT64_C(0x3f8a64dc0760c1d9)),
		be64(UINT64_C(0xb631e54b857b5fbe))
	},
	{
		be64(UINT64_C(0x1856cc63d731759e)),
		be64(UINT64_C(0x62d882b91bfd4bfb)),
		be64(UINT64_C(0x3e2b166d0cae8baa)),
		be64(UINT64_C(0x508bc5f025213a56))
	},
	{
		be64(UINT64_C(0xa81745a0d15d1032)),
		be64(UINT64_C(0x4a4450694123ed63)),
		be64(UINT64_C(0x2ae36a69308b8512)),
		be64(UINT64_C(0xf56ea53c4b74d118))
	},
	{
		be64(UINT64_C(0x5e1e3302b3428502)),
		be64(UINT64_C(0x1b9dad5f07e4f358)),
		be64(UINT64_C(0x760adfc0c68b74c3)),
		be64(UINT64_C(0x1ec6504e88f9ce04))
	},
	{
		be64(UINT64_C(0xa12f4afee0a4ed3f)),
		be64(UINT64_C(0x22d3134dcd5fb331)),
		be64(UINT64_C(0xe0bd71bbf3997a6d)),
		be64(UINT64_C(0xcf7af21f2177e015))
	},
	{
		be64(UINT64_C(0xdffdac63d4caa4a3)),
		be64(UINT64_C(0xeda096e86b016aa5)),
		be64(UINT64_C(0x214c01f625b131e3)),
		be64(UINT64_C(0x73e3f2d0f68e070a))
	},
	{
		be64(UINT64_C(0x70035edf235a449c)),
		be64(UINT64_C(0x4336b8222df889fb)),
		be64(UINT64_C(0x017cfa7035770d0f)),
		be64(UINT64_C(0xd98f6787acc97b6c))
	},
	{
		be64(UINT64_C(0xd7331f1ffebb0dd8)),
		be64(UINT64_C(0x7bbd8bda77c6930f)),
		be64(UINT64_C(0x0e162ef752cb127f)),
		be64(UINT64_C(0x9f2acfdf1ee7ebc3))
	},
	{
		be64(UINT64_C(0x8e6c4c9e736429a0)),
		be64(UINT64_C(0x4a51a0e998845204)),
		be64(UINT64_C(0x7fb5a29cc10dfa6b)),
		be64(UINT64_C(0x7b15ddbe67ec6db9))
	},
	{
		be64(UINT64_C(0x58e64ac3fbdf57fe)),
		be64(UINT64_C(0x42ab45350d59226c)),
		be64(UINT64_C(0x893ed29f0dcb3de5)),
		be64(UINT64_C(0xcd1278c38824ca0b))
	},
	{
		be64(UINT64_C(0x2ac2361fbe94db69)),
		be64(UINT64_C(0xf14a5c0e4b947201)),
		be64(UINT64_C(0x1c7735286722eead)),
		be64(UINT64_C(0x6a54b44f72b6edfb))
	},
	{
		be64(UINT64_C(0xda001660604856b1)),
		be64(UINT64_C(0xf9aeb46206ca37c4)),
		be64(UINT64_C(0x691acbd4e8a385fe)),
		be64(UINT64_C(0xcbefb2a8bed2cd77))
	},
	{
		be64(UINT64_C(0xea052923e443e68a)),
		be64(UINT64_C(0x0432d6c0d3470827)),
		be64(UINT64_C(0xcb1fd44758dfbd5d)),
		be64(UINT64_C(0xcf1d5f362f9450fa)),
	},
	{
		be64(UINT64_C(0x3f4bceac06ffebac)),
		be64(UINT64_C(0x005bf6acde69b7b4)),
		be64(UINT64_C(0xa3d714422e848741)),
		be64(UINT64_C(0x538ab6dfe2a86564))
	},
	{
		be64(UINT64_C(0xf6c83970cf27d7c1)),
		be64(UINT64_C(0xc6fba88bd8dbdc6e)),
		be64(UINT64_C(0x739eec4f7454aef0)),
		be64(UINT64_C(0x683ac52a2ae28d4e))
	},
	{
		be64(UINT64_C(0x9d40aca56dd9db78)),
		be64(UINT64_C(0x9a38a8e28fba249e)),
		be64(UINT64_C(0x2cf58c912259157c)),
		be64(UINT64_C(0xd8c574e3a18dfe31))
	},
	{
		be64(UINT64_C(0x87f0835b8730bf38)),
		be64(UINT64_C(0x81b796e16abdbe27)),
		be64(UINT64_C(0x8239ac3b6858d532)),
		be64(UINT64_C(0x92be40d20ff9fbeb))
	},
	{
		be64(UINT64_C(0xcf61568d48e14c47)),
		be64(UINT64_C(0x119b31f1af8f22da)),
		be64(UINT64_C(0x00c2730a01ed120c)),
		be64(UINT64_C(0xebb27ff3ac4fabed))
	},
	{
		be64(UINT64_C(0x7cc6498224bfd5d1)),
		be64(UINT64_C(0x2e8da97599488d06)),
		be64(UINT64_C(0xbcc1d28c08914761)),
		be64(UINT64_C(0xba7f86ca16c96f1a))
	},
	{
		be64(UINT64_C(0xf390b63e0737fe99)),
		be64(UINT64_C(0x766bcd538733a0e2)),
		be64(UINT64_C(0x27d0a2c95b3d16cd)),
		be64(UINT64_C(0x6005f031744ee4f1))
	},
	{
		be64(UINT64_C(0x92679e52ff50629f)),
		be64(UINT64_C(0xc5aeb0b91e200a40)),
		be64(UINT64_C(0x813de7c1e6064701)),
		be64(UINT64_C(0x1b5eba3db53ba1ab))
	},
	{
		be64(UINT64_C(0x8f873815a8601814)),
		be64(UINT64_C(0xafb63ca4bee9a44b)),
		be64(UINT64_C(0x8337295b936d464e)),
		be64(UINT64_C(0x509ad34eb648e10b))
	},
	{
		be64(UINT64_C(0x5acebd60d88c56ef)),
		be64(UINT64_C(0x3e224a535eb1b883)),
		be64(UINT64_C(0x14cca855f4ae114a)),
		be64(UINT64_C(0x71338649fa1c23bd))
	},
	{
		be64(UINT64_C(0xe07af5e26b9e3594)),
		be64(UINT64_C(0x3a99231bf9f892d1)),
		be64(UINT64_C(0xf6b55f9951b22963)),
		be64(UINT64_C(0x996353fc170da4c5))
	},
	{
		be64(UINT64_C(0xb5b22057e67a776e)),
		be64(UINT64_C(0x3312623653c9bb90)),
		be64(UINT64_C(0x6481b28e583ec6d1)),
		be64(UINT64_C(0x3944f5430d036520))
	},
	{
		be64(UINT64_C(0xb14765176dfabfba)),
		be64(UINT64_C(0x2986ef009d546321)),
		be64(UINT64_C(0x60a2b8b832a5fe41)),
		be64(UINT64_C(0xaabec957e21d5abc))
	},
	{
		be64(UINT64_C(0x1b8ca31f8cc1bd7e)),
		be64(UINT64_C(0x98c49168d857a5ec)),
		be64(UINT64_C(0xf4c1868dc31c8e39)),
		be64(UINT64_C(0x814beb8807c7c92d))
	},
	{
		be64(UINT64_C(0x449f8c003db3f95b)),
		be64(UINT64_C(0x33c7abafe7dbf3ca)),
		be64(UINT64_C(0x159e691f6a53a1cb)),
		be64(UINT64_C(0xce3f73a41b111cf4))
	},
	{
		be64(UINT64_C(0x515316d3de430956)),
		be64(UINT64_C(0x060dc22926655ed3)),
		be64(UINT64_C(0x8fe5cb84a29d743d)),
		be64(UINT64_C(0x2a968fd5416323fd))
	},
	{
		be64(UINT64_C(0x79fd17f877eb80ba)),
		be64(UINT64_C(0x5894601d344adbe2)),
		be64(UINT64_C(0xc7b47a5473145746)),
		be64(UINT64_C(0x9ae5948c7db9537a))
	}
};

/**
 * \brief Null vector.
 */
static const uint8_t null[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * \brief Skein hash test routine.
 *
 * \return \c EXIT_SUCCESS if successful or \c EXIT_FAILURE on failure.
 */
int main(void) {
	assert(sizeof null + 1 >= TESTS);

	for (size_t size = 0; size < TESTS; ++size) {
		uint8_t hash[SKEIN_BYTES];

		skein(hash, null, size);

		if (unlikely(memcmp(hash, test[size], SKEIN_BYTES))) {
			fprintf(stderr, "Hash differs from test vector %lu!\n0x", (unsigned long) size);

			/* Print hash */
			for (size_t byte = 0; byte < SKEIN_BYTES; ++byte)
				fprintf(stderr, "%02" PRIx8, hash[byte]);

			fputs(" ≠ 0x", stderr);

			/* Print test vector */
			for (size_t byte = 0; byte < SKEIN_BYTES; ++byte)
				fprintf(stderr, "%02" PRIx8, ((uint8_t *) hash)[byte]);

			fputc('\n', stderr);

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
#endif /* TEST */
