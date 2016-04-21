#ifndef TOPAZ_H
#define TOPAZ_H

/*
 * Topaz - Main API
 *
 * This library implements functions and routines to control features on with
 * hard drives that implement TCG standard interfaces. Primarilly designed for
 * self-encrypting drives (SEDs), this allows fine grained user control and
 * strong authentication measures to prevent data access by unauthorized third
 * parties.
 *
 * Currently supported TCG standards (SSCs):
 *    - Enterprise
 *    - Opal 1 & 2
 *    - Opalite 
 *    - Pyrite (NOTE - no encryption)
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

#include <stdint.h>
#include <topaz/transport_ata.h>
#include <topaz/errno.h>

/** Messaging sets supported by drive */
typedef enum
{
  /** Messaging set is unknown */
  TP_MSG_UNKNOWN    = 0,
  
  /** Enterprise SSC */
  TP_MSG_ENTERPRISE = 1,
  
  /** Opal SSC (and everything else) */
  TP_MSG_OPAL       = 2

} tp_msg_type_t;

/** Trusted Peripheral (TPer) handle */
typedef struct
{
  /** Raw OS device handle */
  struct tp_ata_handle *ata;
  
  /** Supports security protocol 2 (com & prog resets) */
  int has_reset;
  
  /** ComID to use for TCG SWG messaging */
  uint32_t com_id;
  
  /** Supported messaging set */
  tp_msg_type_t msg_type;
  
} tp_handle_t;

/**
 * \brief Open Drive / Trusted Peripheral (TPer)
 *
 * Opens a hard drive for use with topaz
 *
 * \param[in] path Path to target device
 * \return Pointer to allocated handle, or NULL on error
 */
tp_handle_t *tp_open(char const *path);

/**
 * \brief Close Drive / Trusted Peripheral (TPer)
 *
 * Closes & cleans up topaz device handle
 *
 * \param[in] path Path to target device
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_close(tp_handle_t *handle);

#endif
