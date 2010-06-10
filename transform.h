#pragma once
#ifndef OC_TRANSFORM_H
#define OC_TRANSFORM_H

/**
 * \file
 *
 * \brief Transformation functions.
 */

#include <stdbool.h>
#include <stdint.h>

/**
 * \brief Despatch transformation.
 *
 * \param pid Pointer to process ID variable.
 * \param ident Transformation identifier.
 * \param log Log file descriptor.
 * \param out Output file descriptor.
 * \param in Array of input file descriptors.
 * \param num Number of input file descriptors.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool transform(pid_t *restrict pid, const uint8_t ident[restrict 32], int log, int out, const int in[restrict], uint16_t num);

/**
 * \brief Clean temporary directory up.
 *
 * \param ident Transformation identifier.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool cleanup(const uint8_t ident[restrict 32]);

#endif /* OC_TRANSFORM_H */
