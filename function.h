#pragma once
#ifndef OC_FUNCTION_H
#define OC_FUNCTION_H

/**
 * \file
 *
 * \brief Extended function attributes.
 */

/**
 * \def check
 *
 * \brief Caller must consider the return value.
 *
 * When a caller neglects the return value, a warning will be issued.
 */

/**
 * \def cold
 *
 * \brief Function is unlikely executed.
 *
 * A function declared \c cold is unlikely executed.
 */

/**
 * \def constant
 *
 * \brief Constant function.
 *
 * A function declared \c constant does not examine any values except
 * their arguments and has no effects except the return value.
 * Pointer arguments are never dereferenced.
 */

/**
 * \def deprecated
 *
 * \brief Function is deprecated.
 *
 * A warning will be issued, if the function is called.
 */

/**
 * \def flatten
 *
 * \brief Attempt to inline all calls inside the function.
 *
 * All function calls inside the function are inlined, if possible.
 */

/**
 * \def hot
 *
 * \brief Function is likely executed.
 *
 * A function declared \c hot is likely executed.
 */

/**
 * \def noreturn
 *
 * \brief Function will never return.
 *
 * A function declared \c noreturn will never return.
 */

/**
 * \def pure
 *
 * \brief Function has no side‐effects.
 *
 * A function declared \c pure does not examine any values except their
 * arguments and global variables and has no effects except the return
 * value.
 */

/**
 * \def unused
 *
 * \brief Function may be possibly unused.
 *
 * A function declared \c unused may be possibly unused and the compiler
 * will not warn about it.
 */

/**
 * \def used
 *
 * \brief Function will be used.
 *
 * The compiler will always emit code for a function declared \c used,
 * even, if the function is not referenced.
 */

#if defined(__clang__) || defined(__GNUC__)
# define check      __attribute__((warn_unused_result))
# define cold       __attribute__((cold))
# define constant   __attribute__((const))
# define deprecated __attribute__((deprecated))
# define flatten    __attribute__((flatten))
# define hot        __attribute__((hot))
# define noreturn   __attribute__((noreturn))
# define pure       __attribute__((pure))
# define unused     __attribute__((unused))
# define used       __attribute__((used))
#else
# define check
# define cold
# define constant
# define deprecated
# define flatten
# define hot
# define noreturn
# define pure
# define unused
# define used
#endif

/* Work around sparse */
#ifdef __CHECKER__
# undef flatten
# define flatten
#endif

#endif /* OC_FUNCTION_H */
