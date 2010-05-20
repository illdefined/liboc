#pragma once
#ifndef OC_EXPECT_H
#define OC_EXPECT_H

/**
 * \file
 *
 * \brief Branch prediction macros.
 */

/**
 * \def expect(expr, val)
 *
 * \brief Expect expression \a expr to yield value \a val.
 *
 * \param expr Expression to evaluate.
 * \param val  Expected value.
 *
 * \return Value of expression.
 */
#if defined(__clang__) || defined(__GNUC__)
# define expect(expr, val) __builtin_expect((expr), (val))
#else
# define expect(expr, val) (expr)
#endif

/**
 * \brief Expect expression \a expr to be \c true.
 *
 * \param expr Expression to evaluate.
 *
 * \return Truth value of expression.
 */
#define likely(expr)   expect(!!(expr), 1)

/**
 * \brief Expect expression \a expr to be \c false.
 *
 * \param expr Expression to evaluate.
 *
 * \return Truth value of expression.
 */
#define unlikely(expr) expect(!!(expr), 0)

#endif /* OC_EXPECT_H */
