#ifndef TOPAZ_DEFS_H
#define TOPAZ_DEFS_H

/*
 * Topaz - Common Definitions and Structures
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
#include <topaz/errno.h>

/* Maximum number of bytes for an I/O operation */
#define MAX_IO_BLOCK (64 * 1024)

/** SSCs (Messaging sets) supported by drive */
typedef enum
{
  /** Messaging set is unknown */
  TP_SSC_UNKNOWN    = 0,
  
  /** Enterprise SSC */
  TP_SSC_ENTERPRISE = 1,
  
  /** Opal SSC (and everything else) */
  TP_SSC_OPAL       = 2

} tp_ssc_type_t;

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
  tp_ssc_type_t ssc_type;
  
  /** LBA alignment granularity */
  uint64_t lba_align;
  
  /** Largest valid ComPacketSize for session */
  size_t max_com_pkt_size;
  
  /** Largest valid ComPacketSize for session */
  size_t max_token_size;
  
  /** Space for doing I/O (non-reentrant) */
  char io_block[MAX_IO_BLOCK];
  
} tp_handle_t;

#endif
