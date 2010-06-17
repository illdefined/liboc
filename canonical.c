#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "canonical.h"
#include "egress.h"
#include "expect.h"

char *canonical(const char *restrict prefix, const char *restrict path) {
	prime(char *);

	char *canon = realpath(path, (char *) 0);
	if (unlikely(!canon))
		egress(0, (char *) 0, errno);

	if (unlikely(strncmp(canon, prefix, strlen(prefix))))
		egress(1, (char *) 0, EPERM);

	egress(0, canon, errno);

egress1:
	free(canon);

egress0:
	final();
}
