#pragma once
#ifndef OC_TRIVIAL_H
#define OC_TRIVIAL_H

/**
 * \file
 *
 * \brief Trivial name database.
 */

#include <stdbool.h>
#include <stdint.h>

#include <tchdb.h>

struct trivial {
	TCHDB *hdb;
};

/**
 * \brief Initialise trivial name database context.
 *
 * \param ctx Database context.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool trivial_init(struct trivial *restrict ctx);

/**
 * \brief Resolve trivial name.
 *
 * \param ctx Database context.
 * \param ident Buffer to hold identifier.
 * \param name Trivial name.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool trivial_resolv(struct trivial *restrict ctx, uint8_t ident[restrict 32], const char *restrict name);

/**
 * \brief Free trivial name database context.
 *
 * \param ctx Database context.
 */
extern void trivial_free(struct trivial *restrict ctx);

#endif /* OC_TRIVIAL_H */
