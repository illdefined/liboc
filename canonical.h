#pragma once
#ifndef OC_CANONICAL_H
#define OC_CANONICAL_H

/**
 * \file
 *
 * \brief Path name validation.
 */

/**
 * \brief Canonicalise and validate path name.
 *
 * Convert the path name specified in \a path into its canonical form
 * and validate it. The new path name will be allocated on the heap and
 * shound be released using \c free.
 *
 * \param prefix Mandatory path prefix.
 * \param path Path name.
 *
 * \return Pointer to the new path name or <tt>(char *) 0</tt> on failure.
 */
char *canonical(const char *restrict prefix, const char *restrict path);

#endif /* OC_CANONICAL_H */
