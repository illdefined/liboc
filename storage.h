#pragma once
#ifndef OC_STORAGE_H
#define OC_STORAGE_H

/**
 * \file
 *
 * \brief Simple storage.
 */

#include <stdbool.h>
#include <stdint.h>

/**
 * \brief Retrieve object.
 *
 * \param pid Pointer to process ID variable.
 * \param module Storage module name.
 * \param ident Object identifier.
 * \param log Log file descriptor.
 * \param out Output file descriptor.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool retrieve(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int out);

/**
 * \brief Deposit object.
 *
 * \param pid Pointer to process ID variable.
 * \param module Storage module name.
 * \param ident Object identifier.
 * \param log Log file descriptor.
 * \param in Input file descriptor.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool deposit(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log, int in);

/**
 * \brief Efface object.
 *
 * \param pid Pointer to process ID variable.
 * \param module Storage module name.
 * \param ident Object identifier.
 * \param log Log file descriptor.
 *
 * \return \c true if successful or \c false on failure.
 */
extern bool efface(pid_t *restrict pid, const char *restrict module, const uint8_t ident[restrict 32], int log);

#endif
