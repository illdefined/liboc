#pragma once
#ifndef OC_ROTATE_H
#define OC_ROTATE_H

/**
 * \file
 *
 * \brief Bitwise rotation operations.
 */

#include <stdint.h>

/**
 * \brief Rotate 8bit integer left
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint8_t rotl8(uint8_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x << s | x >> sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rolb %b1, %h0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x << s | x >> sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 16bit integer left
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint16_t rotl16(uint16_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x << s | x >> sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rolw %b1, %w0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x << s | x >> sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 32bit integer left
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint32_t rotl32(uint32_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x << s | x >> sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("roll %b1, %k0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x << s | x >> sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 64bit integer left
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint64_t rotl64(uint64_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x << s | x >> sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rolq %b1, %q0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x << s | x >> sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 8bit integer right
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint8_t rotr8(uint8_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x >> s | x << sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rorb %b1, %h0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x >> s | x << sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 16bit integer right
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint16_t rotr16(uint16_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x >> s | x << sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rorw %b1, %w0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x >> s | x << sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 32bit integer right
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint32_t rotr32(uint32_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x >> s | x << sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rorl %b1, %k0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x >> s | x << sizeof x * 8 - s;
#endif
	}
}

/**
 * \brief Rotate 64bit integer right
 *
 * \param x Integer to rotate.
 * \param s Shift to rotate by.
 *
 * \return Rotated integer
 */
inline uint64_t rotr64(uint64_t x, uint8_t s) {
	if (__builtin_constant_p(x) && __builtin_constant_p(s))
		return x >> s | x << sizeof x * 8 - s;
	else {

/* x86 */
#if defined(__i386__) || defined(__x86_64__)
		asm ("rorq %b1, %q0" : "=q" (x) : "ci" (s) : "cc");
		return x;

#else
		return x >> s | x << sizeof x * 8 - s;
#endif
	}
}

#endif
