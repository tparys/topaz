#ifndef TOPAZ_SECURITY_H
#define TOPAZ_SECURITY_H

/*
 * Topaz - TPM Security API
 *
 * This file implements various Security Protocols / APIs built upon the TPM's
 * IF-SEND and IF-RECV calls. In practice, this can identify a TCG compliant
 * SED, as well as providing some low level reset capabilities, and other
 * miscellaneous capabilities.
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

#include <topaz/defs.h>

/** HANDLE_COMID_REQUEST Command */
typedef struct
{
  uint16_t com_id;
  uint16_t com_id_ext;
  uint32_t req_code;
} tp_comid_req_t;

/** HANDLE_COMID_REQUEST Response */
typedef struct
{
  uint16_t com_id;
  uint16_t com_id_ext;
  uint32_t req_code;
  uint32_t avail_data;
  uint32_t failed;
} tp_comid_resp_t;

/**
 * \brief Probe TPM Security Protocols
 *
 * Scan for available protocols supported via IF-SEND / IF-RECV
 *
 * \param[in] handle Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_probe_security(tp_handle_t *handle);

/**
 * \brief Look up TPM Security Protocols
 *
 * Return human readable representation of TPM protocol
 *
 * \param[in] proto Protocol number
 * \return Pointer to static buffer describing protocol
 */
char const *tp_security_proto_lookup(unsigned char proto);

/**
 * \brief Communication Stack Reset
 *
 * Function to reset state of communication ID within TCG SWG interface
 *
 * \param[in] handle Target drive
 * \param[in] com_id Communication ID to reset
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_security_comid_reset(tp_handle_t *handle, uint32_t com_id);

#endif
