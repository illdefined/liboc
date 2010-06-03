/**
 * \file
 *
 * \brief Trivial name database.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <tchdb.h>

#include "expect.h"
#include "path.h"
#include "trivial.h"

#define TRIVIAL_PATH CACHE_BASE "trivial.db"
#define TRIVIAL_CACHE 64

/**
 * \brief Initialise trivial name database context.
 *
 * \param ctx Database context.
 *
 * \return \c true if successful or \c false on failure.
 */
bool trivial_init(struct trivial *restrict ctx) {
	bool result = false;

	ctx->hdb = tchdbnew();
	if (unlikely(!ctx->hdb))
		goto egress0;

	if (unlikely(!tchdbsetcache(ctx->hdb, TRIVIAL_CACHE)))
		goto egress1;

	if (unlikely(!tchdbopen(ctx->hdb, TRIVIAL_PATH, HDBOREADER)))
		goto egress1;

	result = true;
	goto egress0;

egress1:
	tchdbdel(ctx->hdb);

egress0:
	return result;
}

/**
 * \brief Resolve trivial name.
 *
 * \param ctx Database context.
 * \param ident Buffer to hold identifier.
 * \param name Trivial name.
 *
 * \return \c true if successful or \c false on failure.
 */
bool trivial_resolve(struct trivial *restrict ctx, uint8_t ident[restrict 32], const char *restrict name) {
	bool result = false;

	if (unlikely(tchdbget3(ctx->hdb, name, strlen(name), ident, sizeof ident) != sizeof ident))
		goto egress0;

	result = true;

egress0:
	return result;
}

/**
 * \brief Free trivial name database context.
 *
 * \param ctx Database context.
 */
void trivial_free(struct trivial *restrict ctx) {
	tchdbclose(ctx->hdb);
	tchdbdel(ctx->hdb);
}
