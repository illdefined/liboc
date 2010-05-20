#pragma once
#ifndef OC_HARDTICK_H
#define OC_HARDTICK_H

/**
 * \file
 *
 * \brief Hardware tick counter.
 */

#include <stdint.h>
#include <time.h>
#include <unistd.h>

/**
 * \brief Read hardware tick counter.
 *
 * \return Current value of hardware tick counter.
 */
inline uint64_t hardtick() {
	uint64_t ticks;

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
	register unsigned long low, high;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	ticks =
		(uint64_t) low |
		(uint64_t) high << 32;

/* SPARC */
#elif defined(__sparc__)
	/* Read from tick register */
	asm volatile ("rd %%tick, %0" : "=r" (ticks));

/* PowerPC */
#elif defined(__powerpc__)
	/* Read time base */
	asm volatile ("mftb %0" : "=r" (ticks));

/* MIPS */
#elif defined(__mips__)
	/* Read cycle counter from co-processor register */
	asm volatile ("mfc0 %0, $9" : "=r" (ticks));

/* ARM */
#elif defined(__arm__)
	/* Read cycle counter register from co‐processor */
	asm volatile ("mrc p15, 0, %0, c15, c12, 1" : "=r" (ticks));

/* PA-RISC */
#elif defined(__hppa__)
	/* Read interval timer from control register */
	asm volatile ("mfctl 16, %0" : "=r" (ticks));

/* Alpha */
#elif defined(__alpha__)
	/* Read process cycle counter */
	asm volatile ("rpcc %0" : "=r" (ticks));

/* IA-64 */
#elif defined(__ia64__)
	/* Read time counter */
	asm volatile ("mov %0 = ar.itc" : "=r" (ticks));

/* System/390 */
#elif defined(__s390__)
	/* Store clock */
	asm volatile ("stck 0(%1)" : "=m" (ticks) : "a" (&ticks) : "cc");

/* Architecture‐independent */
#elif defined(_POSIX_THREAD_CPUTIME)
	struct timespec tp;

	/* Get thread‐specific CPU‐time clock */
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp);
	ticks =
		(uint64_t) tp.tv_sec * UINT64_C(1000000000) +
		(uint64_t) tp.tv_nsec;

#else
#error "Your platform does not offer a way to read the CPU cycle counter"
#endif

	return ticks;
}

#endif /* OC_HARDTICK_H */
