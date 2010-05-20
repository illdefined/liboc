#pragma once
#ifndef OC_FUNCTION_H
#define OC_FUNCTION_H

/**
 * \file
 *
 * \brief Extended function attributes.
 */

/**
 * \def __check
 *
 * \brief Caller must consider the return value.
 *
 * When a caller neglects the return value, a warning will be issued.
 */

/**
 * \def __cold
 *
 * \brief Function is unlikely executed.
 *
 * A function declared \c __cold is unlikely executed.
 */

/**
 * \def __const
 *
 * \brief Constant function.
 *
 * A function declared \c __const does not examine any values except
 * their arguments and has no effects except the return value.
 * Pointer arguments are never dereferenced.
 */

/**
 * \def __deprecated
 *
 * \brief Function is deprecated.
 *
 * A warning will be issued, if the function is called.
 */

/**
 * \def __flatten
 *
 * \brief Attempt to inline all calls inside the function.
 *
 * All function calls inside the function are inlined, if possible.
 */

/**
 * \def __hot
 *
 * \brief Function is likely executed.
 *
 * A function declared \c __hot is likely executed.
 */

/**
 * \def __noreturn
 *
 * \brief Function will never return.
 *
 * A function declared \c __noreturn will never return.
 */

/**
 * \def __pure
 *
 * \brief Function has no side‐effects.
 *
 * A function declared \c __pure does not examine any values except their
 * arguments and global variables and has no effects except the return
 * value.
 */

/**
 * \def __unused
 *
 * \brief Function may be possibly unused.
 *
 * A function declared \c __unused may be possibly unused and the
 * compiler will not warn about it.
 */

/**
 * \def __used
 *
 * \brief Function will be used.
 *
 * The compiler will always emit code for a function declared \c __used,
 * even, if the function is not referenced.
 */

#if defined(__clang__) || defined(__GNUC__)
# define __check      __attribute__((warn_unused_result))
# define __cold       __attribute__((cold))
# define __const      __attribute__((const))
# define __deprecated __attribute__((deprecated))
# define __flatten    __attribute__((flatten))
# define __hot        __attribute__((hot))
# define __noreturn   __attribute__((noreturn))
# define __pure       __attribute__((pure))
# define __unused     __attribute__((unused))
# define __used       __attribute__((used))
#else
# define __check
# define __cold
# define __const
# define __deprecated
# define __flatten
# define __hot
# define __noreturn
# define __pure
# define __unused
# define __used
#endif

/* Work around sparse */
#ifdef __CHECKER__
# undef __flatten
# define __flatten
#endif

#endif /* OC_FUNCTION_H */
