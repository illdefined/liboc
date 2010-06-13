#pragma once
#ifndef OC_ROTATE_H
#define OC_ROTATE_H

/**
 * \file
 *
 * \brief Bitwise rotation operations.
 */

/**
 * \brief Rotate integer left.
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
#define rotl(x, s) (((x) << ((s) & sizeof (x) * 8 - 1) | (x) >> sizeof (x) * 8 - ((s) & sizeof (x) * 8 - 1)) & (1 << sizeof (x) * 8) - 1)

/**
 * \brief Rotate integer right.
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
#define rotr(x, s) (((x) >> ((s) & sizeof (x) * 8 - 1) | (x) << sizeof (x) * 8 - ((s) & sizeof (x) * 8 - 1)) & (1 << sizeof (x) * 8) - 1)

#endif
