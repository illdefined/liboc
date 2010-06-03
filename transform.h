#pragma once
#ifndef OC_TRANSFORM_H
#define OC_TRANSFORM_H

#include <stdbool.h>
#include <stdint.h>

extern bool transform(pid_t *restrict, const uint8_t[restrict 32], int, int, const int[restrict], uint16_t);
extern bool cleanup(const uint8_t[restrict 32], bool);

#endif /* OC_TRANSFORM_H */
