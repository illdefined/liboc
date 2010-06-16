#pragma once
#ifndef OC_ROTATE_H
#define OC_ROTATE_H

#include <stdint.h>

#include "function.h"

/**
 * \file
 *
 * \brief Bitwise rotation operations.
 */

/**
 * \brief Rotate integer left.
 *
 * \param num Integer to rotate.
 * \param shift Shift to rotate by.
 *
 * \return Rotated integer
 */
inline constant uint64_t rotl(uint64_t num, uint8_t shift) {
	return num << (shift & sizeof num * 8 - 1) |
		   num >> sizeof num * 8 - (shift & sizeof num * 8 - 1);
}

/**
 * \brief Rotate integer right.
 *
 * \param num Integer to rotate.
 * \param shift Shift to rotate by.
 *
 * \return Rotated integer
 */
inline constant uint64_t rotr(uint64_t num, uint8_t shift) {
	return num >> (shift & sizeof num * 8 - 1) |
		   num << sizeof num * 8 - (shift & sizeof num * 8 - 1);
}

#endif
