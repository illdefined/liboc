/**
 * \file
 *
 * \brief Compile‐time byte‐order tests
 */

#include <stdint.h>

uint16_t be[] = {
	UINT16_C(0x4249), UINT16_C(0x4765),
	UINT16_C(0x6e44), UINT16_C(0x6961),
	UINT16_C(0x6e53), UINT16_C(0x7953),
	UINT16_C(0x0000)
};

uint16_t le[] = {
	UINT16_C(0x694c), UINT16_C(0x5454),
	UINT16_C(0x656c), UINT16_C(0x6e45),
	UINT16_C(0x6944), UINT16_C(0x6e61),
	UINT16_C(0x0000)
};
