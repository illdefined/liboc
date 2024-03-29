#include <stdint.h>

#include "binary.h"

const uint8_t magic[16] = {
	UINT8_C(0x1b), UINT8_C(0x5b), UINT8_C(0x31), UINT8_C(0x3b),
	UINT8_C(0x33), UINT8_C(0x35), UINT8_C(0x6d), UINT8_C(0x4f),
	UINT8_C(0x43), UINT8_C(0x42), UINT8_C(0x30), UINT8_C(0x30),
	UINT8_C(0x0d), UINT8_C(0x0a), UINT8_C(0x1a), UINT8_C(0x0a)
};
