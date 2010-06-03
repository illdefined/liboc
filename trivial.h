#pragma once
#ifndef OC_TRIVIAL_H
#define OC_TRIVIAL_H

#include <stdbool.h>
#include <stdint.h>

#include <tchdb.h>

struct trivial {
	TCHDB *hdb;
};

extern bool trivial_init(struct trivial *restrict);
extern bool trivial_resolv(struct trivial *restrict, uint8_t[restrict 32], const char *restrict);
extern void trivial_free(struct trivial *restrict);

#endif /* OC_TRIVIAL_H */
