#pragma once
#ifndef OC_STRING_H
#define OC_STRING_H

#include <stdint.h>

extern void hexstr(char *restrict, const void *restrict, size_t);
extern char *concat(const char *restrict, ...);

#endif
