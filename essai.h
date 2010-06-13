#pragma once
#ifndef OC_ESSAI_H
#define OC_ESSAI_H

/**
 * \file
 *
 * \brief Test functions.
 */

#include <stdio.h>
#include <stdlib.h>

#include "expect.h"

/**
 * \brief Verify expression.
 *
 * \param expr Expression.
 */
#define essaye(expr) \
	do { \
		printf("\x1b[K\x1b[0;33mEssayant \x1b[1;33m%s\x1b[0;33m…\x1b[m\r", #expr); \
		fflush(stdout); \
		if (likely(expr)) \
			printf("\x1b[K\x1b[1;32m%s\x1b[0;32m réussit.\x1b[m\n", #expr); \
		else { \
			printf("\x1b[K\x1b[1;31m%s\x1b[0;31m échoua!\x1b[m\n", #expr); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

#endif /* OC_ESSAI_H */
