#ifndef TOPAZ_ERRNO_H
#define TOPAZ_ERRNO_H

/**
 * Topaz - Error Numbers / Ids
 *
 * This file implements known error codes, as used by the topaz library.
 *
 * Copyright (c) 2016, T Parys
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** Last topaz error number */
extern unsigned int tp_errno;

/**
 * \brief Error Number String Lookup (Current)
 *
 * Returns a human readable interpretation of the current Topaz error code
 *
 * \param[in] id Valid error identifier
 * \return Pointer to static buffer containing error message
 */
char const *tp_errno_lookup_cur();

/**
 * \brief Error Number String Lookup (Arbitrary)
 *
 * Returns a human readable interpretation of a arbitrary Topaz error code
 *
 * \param[in] id Valid error identifier
 * \return Pointer to static buffer containing error message
 */
char const *tp_errno_lookup(unsigned int id);

/* === BEGIN AUTOGENERATED CONTENT === */

#define TP_ERR_SUCCESS         0x00000000 /** No error reported */
#define TP_ERR_UNSPECIFIED     0x00000001 /** Unspecified error */
#define TP_ERR_OPEN            0x00000002 /** Error opening file */
#define TP_ERR_CLOSE           0x00000003 /** Error opening file */
#define TP_ERR_INVALID         0x00000004 /** Invalid argument */
#define TP_ERR_NULL            0x00000005 /** NULL Pointer */
#define TP_ERR_ALLOC           0x00000006 /** Error allocating memory */

/* Tranport Errors */

#define TP_ERR_IOCTL           0x00001000 /** Failed to call ioctl */
#define TP_ERR_SENSE           0x00001001 /** Bad sense data */

/* Linux Specific Errors */

#define TP_ERR_SYSFS           0x00002000 /** Error reading from sysfs */
#define TP_ERR_LIBATA          0x00002001 /** Libata blocking TPM calls (add kernel argument 'libata.allow_tpm=1') */

/* === END AUTOGENERATED CONTENT === */

#endif
