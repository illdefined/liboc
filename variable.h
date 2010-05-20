#pragma once
#ifndef OC_VARIABLE_H
#define OC_VARIABLE_H

/**
 * \file
 *
 * \brief Extended variable attributes.
 */

/**
 * \def __aligned(bound)
 *
 * \brief Align to boundary.
 *
 * \param bound Boundary to align to.
 *
 * A variable declared \c __aligned will be allocated on the boundary
 * specified by \p bound.
 */

/**
 * \def __packed
 *
 * \brief Packed variable.
 *
 * A variable declared \c __packed should have the smallest possible
 * alignment.
*/

#if defined(__clang__) || defined(__GNUC__)
# define __packed         __attribute__((packed))
# define __aligned(bound) __attribute__((aligned(bound)))
#else
# warning "Your compiler is almost certainly going to produce incorrect code!"
# define __packed
# define __aligned(bound)
#endif

#endif /* OC_FUNCTION_H */
