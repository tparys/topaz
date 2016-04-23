/*
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

#include <topaz/errno.h>

/** Last topaz error number */
tp_errno_t tp_errno = 0;

/** Known topaz error codes */
struct
{
  tp_errno_t id;
  char const *msg;
} tp_errno_db[] =
{
/* === BEGIN AUTOGENERATED CONTENT === */

  { TP_ERR_SUCCESS        , "No error reported" },
  { TP_ERR_UNSPECIFIED    , "Unspecified error" },
  { TP_ERR_OPEN           , "Error opening file" },
  { TP_ERR_CLOSE          , "Error opening file" },
  { TP_ERR_INVALID        , "Invalid argument" },
  { TP_ERR_NULL           , "NULL Pointer" },
  { TP_ERR_ALLOC          , "Error allocating memory" },

  /* TPM Errors */

  { TP_ERR_NO_TPM         , "Target drive does not contain a TPM" },
  { TP_ERR_TPM_PROTO      , "TPM does not support a known protocol" },
  { TP_ERR_TPM_VERSION    , "TPM using an unknown protocol version" },
  { TP_ERR_TPM_SSC        , "Unrecognized SSC messaging type" },

  /* Tranport Errors */

  { TP_ERR_IOCTL          , "Failed to call ioctl" },
  { TP_ERR_SENSE          , "Bad sense data" },

  /* Linux Specific Errors */

  { TP_ERR_SYSFS          , "Error reading from sysfs" },
  { TP_ERR_LIBATA         , "Libata blocking TPM calls (add kernel argument 'libata.allow_tpm=1')" },

/* === END AUTOGENERATED CONTENT === */
};

/**
 * \brief Error Number String Lookup (Current)
 *
 * Returns a human readable interpretation of the current Topaz error code
 *
 * \param[in] id Valid error identifier
 * \return Pointer to static buffer containing error message, or NULL on error
 */
char const *tp_errno_lookup_cur()
{
  return tp_errno_lookup(tp_errno);
}

/**
 * \brief Error Number String Lookup (Arbitrary)
 *
 * Returns a human readable interpretation of a arbitrary Topaz error code
 *
 * \param[in] id Valid error identifier
 * \return Pointer to static buffer containing error message, or NULL on error
 */
char const *tp_errno_lookup(tp_errno_t id)
{
  unsigned int db_size = sizeof(tp_errno_db) / sizeof(tp_errno_db[0]);
  unsigned int i;
  
  /* Look for matching ID */
  for (i = 0; i < db_size; i++)
  {
    if (tp_errno_db[i].id == id)
    {
      return tp_errno_db[i].msg;
    }
  }
  
  /* Did not find it */
  return "Invalid topaz errno";
}
