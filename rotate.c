#ifdef TEST
#include <stdint.h>
#include <stdlib.h>

#include "essai.h"
#include "rotate.h"

int main(void) {
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
