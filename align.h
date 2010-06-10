#pragma once
#ifndef OC_ALIGN_H
#define OC_ALIGN_H

/**
 * \file
 *
 * \brief Alignment attributes.
 */

/**
 * \def aligned(bound)
 *
 * \brief Align to boundary.
 *
 * \param bound Boundary to align to.
 *
 * A variable declared \c aligned will be allocated on the boundary
 * specified by \a bound.
 */

/**
 * \def packed
 *
 * \brief Packed variable.
 *
 * A variable declared \c packed should have the smallest possible
 * alignment.
*/

#if defined(__clang__) || defined(__GNUC__)
# define packed         __attribute__((packed))
# define aligned(bound) __attribute__((aligned(bound)))
#else
# warning "Your compiler is almost certainly going to produce incorrect code!"
# define packed
# define aligned(bound)
#endif

#endif /* OC_FUNCTION_H */
