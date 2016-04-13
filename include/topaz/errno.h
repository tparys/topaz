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

enum
{
  /* General Errors */
  
  TP_ERR_SUCCESS = 0x000, /** No reported error */
  TP_ERR_UNSPEC,          /** Unspecified error */
  TP_ERR_OPEN,            /** Cannot open file */
  TP_ERR_CLOSE,           /** Cannot close file */
  TP_ERR_INVALID,         /** Invalid argument */
  TP_ERR_NULL,            /** NULL pointer */
  TP_ERR_ALLOC,           /** Cannot close file */

  /* Low level transport errors */

  TP_ERR_IOCTL = 0x100,   /** Failed call to ioctl */
  TP_ERR_SENSE,           /** Bad sense data */
  
  /* Linux Errors */
  
  TP_ERR_LIBATA = 0x200,  /** Libata layer is configured to block TPM calls */
};

#endif
