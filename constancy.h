#pragma once
#ifndef OC_CONSTANCY_H
#define OC_CONSTANCY_H

/**
 * \file
 *
 * \brief Compile‐time constancy checks.
 */

/**
 * \def constant_p(expr)
 *
 * \brief Determine if expression \a expr is a compile‐time constant.
 *
 * \param expr Expression to evaluate.
 *
 * \return True if expression is a compile‐time constant or false otherwise.
 */

#if defined(__clang__) || defined(__GNUC__)
#define constant_p(expr) __builtin_constant_p(expr)
#else
#define constant_p(expr) 0
#endif

#endif /* OC_CONSTANCY_H */
