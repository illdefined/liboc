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
		for (size_t word = 0; word < SKEIN_WORDS; ++word) {
			block[word] = 0;

			for (size_t byte = 0; byte < sizeof block[word]; ++byte)
				block[word] |= (uint64_t) mesg[word * sizeof block[word] + byte] << byte * 8;
		}

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

void skein_init(struct skein *restrict ctx) {
	ctx->tweak[0] = 0;
	ctx->tweak[1] = FLAG_FIRST | TYPE_MSG;

	memcpy(ctx->chain, skein_iv, SKEIN_BYTES);

	ctx->level = 0;
}

void skein_feed(struct skein *restrict ctx, const void *restrict blob, size_t size) {
	/* Catch integer overflow */
	assert(ctx->level + size >= size);

	const uint8_t *mesg = (const uint8_t *) blob;

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

void skein(uint8_t hash[restrict SKEIN_BYTES], const void *restrict mesg, size_t size) {
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
		be64(UINT64_C(0x0f42a0f1b17a98b1)),
		be64(UINT64_C(0x8264fa62a1286caa)),
		be64(UINT64_C(0xfa6039eb6b5a97c2)),
		be64(UINT64_C(0x7a1648fe72a76423))
	},
	{
		be64(UINT64_C(0xfdc9821a2dfc3bee)),
		be64(UINT64_C(0x8ad7b3d46e9797db)),
		be64(UINT64_C(0xaa482329b2f55042)),
		be64(UINT64_C(0xd35129901e561181))
	},
	{
		be64(UINT64_C(0x42a017086ff15aab)),
		be64(UINT64_C(0xcf2203c3dd6ff31e)),
		be64(UINT64_C(0xc20171f32a038fcd)),
		be64(UINT64_C(0x1a69dca1c23471c2))
	},
	{
		be64(UINT64_C(0x0267cc3514c4c38e)),
		be64(UINT64_C(0xb25fef4d7a7711fd)),
		be64(UINT64_C(0xb9cf666f17098f3d)),
		be64(UINT64_C(0xda2b96429f382c78))
	},
	{
		be64(UINT64_C(0x98144b8bbb065e7e)),
		be64(UINT64_C(0xe7f7f0abee5a7d1b)),
		be64(UINT64_C(0x8a7bb66cec1202dc)),
		be64(UINT64_C(0x84f6a975db969b42))
	},
	{
		be64(UINT64_C(0x322eabef955bea86)),
		be64(UINT64_C(0xfd1458451900ecfa)),
		be64(UINT64_C(0x43e51a3ccdbc3106)),
		be64(UINT64_C(0xc5bd502ceaa4c5ec))
	},
	{
		be64(UINT64_C(0x05362b92b65091c4)),
		be64(UINT64_C(0x615e0d5ab93cb1b8)),
		be64(UINT64_C(0xb935448954a78a21)),
		be64(UINT64_C(0x6a9454a19165dadd))
	},
	{
		be64(UINT64_C(0x164b38d4a65f8ae9)),
		be64(UINT64_C(0x4c7beadcdcff6941)),
		be64(UINT64_C(0x71db0fa6b6c51c1e)),
		be64(UINT64_C(0xeeb69ce699e5f91f))
	},
	{
		be64(UINT64_C(0xedd6aa093dedd935)),
		be64(UINT64_C(0x711b6dbdfa10d5f5)),
		be64(UINT64_C(0xcdf01d8e62370337)),
		be64(UINT64_C(0x51d858aa957a3bb4))
	},
	{
		be64(UINT64_C(0xc3951f697417cb12)),
		be64(UINT64_C(0x23e0a0d014a836d4)),
		be64(UINT64_C(0x46299c16e6e31f6e)),
		be64(UINT64_C(0xe19585f629bcc27a))
	},
	{
		be64(UINT64_C(0x4c2a142530deb3bf)),
		be64(UINT64_C(0x07911ffd38b91943)),
		be64(UINT64_C(0x8b6c527c889aa109)),
		be64(UINT64_C(0xfe40024edd9ca495))
	},
	{
		be64(UINT64_C(0x376cdea95646893e)),
		be64(UINT64_C(0x7080f74e4c372a09)),
		be64(UINT64_C(0x2f82fa9eb8970d76)),
		be64(UINT64_C(0xd6b7001ae15b8990))
	},
	{
		be64(UINT64_C(0xa08c03dbd2e58cad)),
		be64(UINT64_C(0x21b326bbe936da24)),
		be64(UINT64_C(0x2d9d988db2c48181)),
		be64(UINT64_C(0xb3be15c8626e2d2a))
	},
	{
		be64(UINT64_C(0x42033c7af61286b1)),
		be64(UINT64_C(0xfdc4a6049f01a2d8)),
		be64(UINT64_C(0xf50f86bd47c60a7b)),
		be64(UINT64_C(0xa3b7eb52ba46c97d))
	},
	{
		be64(UINT64_C(0x8cb67ac7189d99e8)),
		be64(UINT64_C(0x6d49980788240863)),
		be64(UINT64_C(0x08d5e4f3d46ad757)),
		be64(UINT64_C(0x7588bcd6efb6a3e4))
	},
	{
		be64(UINT64_C(0x2bf5d34ceeb07493)),
		be64(UINT64_C(0xcce70ed68db023bf)),
		be64(UINT64_C(0x5016586ae207f04e)),
		be64(UINT64_C(0x1a883a89606a2a68))
	},
	{
		be64(UINT64_C(0x186722d4a0b0c885)),
		be64(UINT64_C(0xc03d6186d461576b)),
		be64(UINT64_C(0x37beea117071d2f8)),
		be64(UINT64_C(0x37691b2f57204acf))
	},
	{
		be64(UINT64_C(0x06a0d76cc0f36053)),
		be64(UINT64_C(0x07a0286a11430bc7)),
		be64(UINT64_C(0x26fad97f4bad9247)),
		be64(UINT64_C(0x8a8488d7fc6886fa))
	},
	{
		be64(UINT64_C(0xa252ef865e16ce4c)),
		be64(UINT64_C(0xfd205489817bec26)),
		be64(UINT64_C(0xb4e92d49bfef1d07)),
		be64(UINT64_C(0xa1391ba7874c5399))
	},
	{
		be64(UINT64_C(0x504af2d75642d5d8)),
		be64(UINT64_C(0xb514b02f56779460)),
		be64(UINT64_C(0x651f2dc98e626f98)),
		be64(UINT64_C(0xa713868f54656927))
	},
	{
		be64(UINT64_C(0x0c346c52357840f6)),
		be64(UINT64_C(0x48f9582166ef133d)),
		be64(UINT64_C(0x3cc0b5999144cf59)),
		be64(UINT64_C(0x92920ed531a64f89))
	},
	{
		be64(UINT64_C(0x3e133027ea222f03)),
		be64(UINT64_C(0xff7a567d6e08e7b7)),
		be64(UINT64_C(0x8f53145e611ed499)),
		be64(UINT64_C(0xad995f9ff2fa4842))
	},
	{
		be64(UINT64_C(0x76704c3e2e79a4f7)),
		be64(UINT64_C(0xee65dff985892e7b)),
		be64(UINT64_C(0x50fbf726cc3a5dd8)),
		be64(UINT64_C(0xe72bd8b52fa7777c))
	},
	{
		be64(UINT64_C(0xa3cbdd7826739afe)),
		be64(UINT64_C(0xb6e2a594e3f202ba)),
		be64(UINT64_C(0xceefaf829c4b756b)),
		be64(UINT64_C(0xac5b138b6167a171))
	},
	{
		be64(UINT64_C(0x85a28a0794702153)),
		be64(UINT64_C(0xd6c24f9f873b4b6f)),
		be64(UINT64_C(0x9d0910e950cc2c89)),
		be64(UINT64_C(0xc928da7db513d005))
	},
	{
		be64(UINT64_C(0xac9c6c4a1d3f0c0e)),
		be64(UINT64_C(0x447b8b476ec59dd8)),
		be64(UINT64_C(0x398910a326efb7ed)),
		be64(UINT64_C(0xd08fc2fa39e0aa72))
	},
	{
		be64(UINT64_C(0xf8fa36dc114ce020)),
		be64(UINT64_C(0x09278dc1dd6863c5)),
		be64(UINT64_C(0xd0b548255f6e2a5c)),
		be64(UINT64_C(0xf82262e9f90f8322))
	},
	{
		be64(UINT64_C(0x0acbc82acb6ceca7)),
		be64(UINT64_C(0x565480a04dc1b037)),
		be64(UINT64_C(0x6822517e496f051e)),
		be64(UINT64_C(0xb1635de6d775492b))
	},
	{
		be64(UINT64_C(0x6805657b92ae41e4)),
		be64(UINT64_C(0xc101b6eb3b77df44)),
		be64(UINT64_C(0x5018091421be82ac)),
		be64(UINT64_C(0x4837053bce672e3a))
	},
	{
		be64(UINT64_C(0xd5870851133a6489)),
		be64(UINT64_C(0x16aaf66713851d49)),
		be64(UINT64_C(0x2f9d7c979f92f6fb)),
		be64(UINT64_C(0xcf63b8b5c9c83074))
	},
	{
		be64(UINT64_C(0x1d32964df6d74c54)),
		be64(UINT64_C(0xe29026ee8e7a1500)),
		be64(UINT64_C(0x73dbe2162b7e5207)),
		be64(UINT64_C(0x1d078f3ace69699b))
	}
};

/**
 * \brief Test data.
 */
static const uint8_t data[] = {
	UINT8_C(0xd3), UINT8_C(0x8e), UINT8_C(0x48), UINT8_C(0x6a),
	UINT8_C(0xeb), UINT8_C(0xfb), UINT8_C(0x4d), UINT8_C(0x51),
	UINT8_C(0x5b), UINT8_C(0x48), UINT8_C(0x60), UINT8_C(0xb9),
	UINT8_C(0xa5), UINT8_C(0x65), UINT8_C(0x19), UINT8_C(0xbe),
	UINT8_C(0xa8), UINT8_C(0x41), UINT8_C(0xf4), UINT8_C(0xf1),
	UINT8_C(0x31), UINT8_C(0x59), UINT8_C(0x59), UINT8_C(0x2a),
	UINT8_C(0xc5), UINT8_C(0x86), UINT8_C(0x20), UINT8_C(0xee),
	UINT8_C(0xc0), UINT8_C(0x7c), UINT8_C(0x9c), UINT8_C(0xdf)
};

/**
 * \brief Skein hash test routine.
 *
 * \return \c EXIT_SUCCESS if successful or \c EXIT_FAILURE on failure.
 */
int main(void) {
	assert(sizeof data + 1 >= TESTS);

	for (size_t size = 0; size < TESTS; ++size) {
		uint8_t hash[SKEIN_BYTES];

		skein(hash, data, size);

		if (unlikely(memcmp(hash, test[size], SKEIN_BYTES))) {
			fprintf(stderr, "Hash differs from test vector %lu!\n0x", (unsigned long) size);

			/* Print hash */
			for (size_t byte = 0; byte < SKEIN_BYTES; ++byte)
				fprintf(stderr, "%02" PRIx8, hash[byte]);

			fputs(" ≠ 0x", stderr);

			/* Print test vector */
			for (size_t byte = 0; byte < SKEIN_BYTES; ++byte)
				fprintf(stderr, "%02" PRIx8, ((uint8_t *) test[size])[byte]);

			fputc('\n', stderr);

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
#endif /* TEST */
