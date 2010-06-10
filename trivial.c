#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <tchdb.h>

#include "egress.h"
#include "expect.h"
#include "path.h"
#include "trivial.h"

#define TRIVIAL_PATH CACHE_BASE "trivial.db"
#define TRIVIAL_CACHE 64

bool trivial_init(struct trivial *restrict ctx) {
	prime(bool);

	/* FIXME: Error codes */

	ctx->hdb = tchdbnew();
	if (unlikely(!ctx->hdb))
		egress(0, false, errno);

	if (unlikely(!tchdbsetcache(ctx->hdb, TRIVIAL_CACHE)))
		egress(1, false, errno);

	if (unlikely(!tchdbopen(ctx->hdb, TRIVIAL_PATH, HDBOREADER)))
		egress(1, false, errno);

	egress(0, true, errno);

egress1:
	tchdbdel(ctx->hdb);

egress0:
	final();
}

bool trivial_resolve(struct trivial *restrict ctx, uint8_t ident[restrict 32], const char *restrict name) {
	prime(bool);

	/* FIXME: Error codes */

	if (unlikely(tchdbget3(ctx->hdb, name, strlen(name), ident, sizeof ident) != sizeof ident))
		egress(0, false, errno);

	egress(0, true, errno);

egress0:
	final();
}

void trivial_free(struct trivial *restrict ctx) {
	tchdbclose(ctx->hdb);
	tchdbdel(ctx->hdb);
}
