#ifdef TEST
#include <stdint.h>
#include <stdlib.h>

#include "essai.h"
#include "rotate.h"

int main(void) {
	essaye(rotl((uint8_t) 0xac, 0) == 0xac);
	essaye(rotl((uint8_t) 0xac, 1) == 0x59);
	essaye(rotl((uint8_t) 0xac, 2) == 0xb2);
	essaye(rotl((uint8_t) 0xac, 3) == 0x65);
	essaye(rotl((uint8_t) 0xac, 4) == 0xca);
	essaye(rotl((uint8_t) 0xac, 5) == 0x95);
	essaye(rotl((uint8_t) 0xac, 6) == 0x2b);
	essaye(rotl((uint8_t) 0xac, 7) == 0x56);
	essaye(rotl((uint8_t) 0xac, 8) == 0xac);

	essaye(rotr((uint8_t) 0xac, 0) == 0xac);
	essaye(rotr((uint8_t) 0xac, 1) == 0x56);
	essaye(rotr((uint8_t) 0xac, 2) == 0x2b);
	essaye(rotr((uint8_t) 0xac, 3) == 0x95);
	essaye(rotr((uint8_t) 0xac, 4) == 0xca);
	essaye(rotr((uint8_t) 0xac, 5) == 0x65);
	essaye(rotr((uint8_t) 0xac, 6) == 0xb2);
	essaye(rotr((uint8_t) 0xac, 7) == 0x59);
	essaye(rotr((uint8_t) 0xac, 8) == 0xac);

	essaye(rotl((uint16_t) 0xcafe,  0) == 0xcafe);
	essaye(rotl((uint16_t) 0xcafe,  4) == 0xafec);
	essaye(rotl((uint16_t) 0xcafe,  8) == 0xfeca);
	essaye(rotl((uint16_t) 0xcafe, 12) == 0xecaf);
	essaye(rotl((uint16_t) 0xcafe, 16) == 0xcafe);

	essaye(rotr((uint16_t) 0xcafe,  0) == 0xcafe);
	essaye(rotr((uint16_t) 0xcafe,  4) == 0xecaf);
	essaye(rotr((uint16_t) 0xcafe,  8) == 0xfeca);
	essaye(rotr((uint16_t) 0xcafe, 12) == 0xafec);
	essaye(rotr((uint16_t) 0xcafe, 16) == 0xcafe);

	essaye(rotl((uint32_t) 0xdeadbeef,  0) == 0xdeadbeef);
	essaye(rotl((uint32_t) 0xdeadbeef,  4) == 0xeadbeefd);
	essaye(rotl((uint32_t) 0xdeadbeef,  8) == 0xadbeefde);
	essaye(rotl((uint32_t) 0xdeadbeef, 12) == 0xdbeefdea);
	essaye(rotl((uint32_t) 0xdeadbeef, 16) == 0xbeefdead);
	essaye(rotl((uint32_t) 0xdeadbeef, 20) == 0xeefdeadb);
	essaye(rotl((uint32_t) 0xdeadbeef, 24) == 0xefdeadbe);
	essaye(rotl((uint32_t) 0xdeadbeef, 28) == 0xfdeadbee);
	essaye(rotl((uint32_t) 0xdeadbeef, 32) == 0xdeadbeef);

	essaye(rotr((uint32_t) 0xdeadbeef,  0) == 0xdeadbeef);
	essaye(rotr((uint32_t) 0xdeadbeef,  4) == 0xfdeadbee);
	essaye(rotr((uint32_t) 0xdeadbeef,  8) == 0xefdeadbe);
	essaye(rotr((uint32_t) 0xdeadbeef, 12) == 0xeefdeadb);
	essaye(rotr((uint32_t) 0xdeadbeef, 16) == 0xbeefdead);
	essaye(rotr((uint32_t) 0xdeadbeef, 20) == 0xdbeefdea);
	essaye(rotr((uint32_t) 0xdeadbeef, 24) == 0xadbeefde);
	essaye(rotr((uint32_t) 0xdeadbeef, 28) == 0xeadbeefd);
	essaye(rotr((uint32_t) 0xdeadbeef, 32) == 0xdeadbeef);

	essaye(rotl((uint64_t) 0x46c048d15e26af37,  0) == 0x46c048d15e26af37);
	essaye(rotl((uint64_t) 0x46c048d15e26af37,  4) == 0x6c048d15e26af374);
	essaye(rotl((uint64_t) 0x46c048d15e26af37,  8) == 0xc048d15e26af3746);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 12) == 0x048d15e26af3746c);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 16) == 0x48d15e26af3746c0);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 20) == 0x8d15e26af3746c04);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 24) == 0xd15e26af3746c048);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 28) == 0x15e26af3746c048d);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 32) == 0x5e26af3746c048d1);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 36) == 0xe26af3746c048d15);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 40) == 0x26af3746c048d15e);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 44) == 0x6af3746c048d15e2);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 48) == 0xaf3746c048d15e26);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 52) == 0xf3746c048d15e26a);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 56) == 0x3746c048d15e26af);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 60) == 0x746c048d15e26af3);
	essaye(rotl((uint64_t) 0x46c048d15e26af37, 64) == 0x46c048d15e26af37);

	essaye(rotr((uint64_t) 0x46c048d15e26af37,  0) == 0x46c048d15e26af37);
	essaye(rotr((uint64_t) 0x46c048d15e26af37,  4) == 0x746c048d15e26af3);
	essaye(rotr((uint64_t) 0x46c048d15e26af37,  8) == 0x3746c048d15e26af);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 12) == 0xf3746c048d15e26a);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 16) == 0xaf3746c048d15e26);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 20) == 0x6af3746c048d15e2);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 24) == 0x26af3746c048d15e);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 28) == 0xe26af3746c048d15);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 32) == 0x5e26af3746c048d1);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 36) == 0x15e26af3746c048d);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 40) == 0xd15e26af3746c048);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 44) == 0x8d15e26af3746c04);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 48) == 0x48d15e26af3746c0);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 52) == 0x048d15e26af3746c);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 56) == 0xc048d15e26af3746);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 60) == 0x6c048d15e26af374);
	essaye(rotr((uint64_t) 0x46c048d15e26af37, 64) == 0x46c048d15e26af37);
}
#endif /* TEST */
