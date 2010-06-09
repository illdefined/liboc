#pragma once
#ifndef OC_STRING_H
#define OC_STRING_H

#include <stdbool.h>
#include <stdint.h>

extern void inthexs(char *restrict, const void *restrict, size_t);
extern bool hexsint(void *restrict, const char *restrict, size_t);
extern char *concat(const char *restrict, ...);

#endif
