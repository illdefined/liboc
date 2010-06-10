#pragma once
#ifndef OC_EGRESS_H
#define OC_EGRESS_H

/**
 * \file
 *
 * \brief Egress macros.
 */

#include <errno.h>

/**
 * \brief Prime function.
 *
 * \param type Return type.
 */
#define prime(type) \
	type egress_result; \
	int  egress_errnum = errno

/**
 * \brief Egress from function.
 *
 * \param point Egress point.
 * \param result Function result.
 * \param errnum Error number.
 */
#define egress(point, result, errnum) \
	do { \
		egress_errnum = (errnum); \
		egress_result = (result); \
		goto egress##point; \
	} while (0)

/**
 * \brief Finalise function.
 */
#define final() \
	do { \
		errno = egress_errnum; \
		return egress_result; \
	} while (0)

#endif /* OC_EGRESS_H */
