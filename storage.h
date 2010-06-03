#pragma once
#ifndef OC_STORAGE_H
#define OC_STORAGE_H

#include <stdbool.h>
#include <stdint.h>

extern bool retrieve(pid_t *restrict, const uint8_t[restrict 32], int, int);
extern bool deposit(pid_t *restrict, const uint8_t[restrict 32], int, int);

#endif
