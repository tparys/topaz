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

#define _BSD_SOURCE
#include <stdio.h>
#include <endian.h>
#include <topaz/security.h>
#include <topaz/transport_ata.h>
#include <topaz/debug.h>

/**
 * \brief Probe TPM Security Protocols
 *
 * Scan for available protocols supported via IF-SEND / IF-RECV
 *
 * \param[in] handle Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_probe_security(tp_handle_t *handle)
{
  unsigned proto_count, has_tcg = 0, i;
  unsigned char buf[512];
  
  /* query protocol info */
  TP_DEBUG(1) printf("Probe TPM Security Protocols\n");
  if (tp_ata_if_recv(handle->ata, 0, 0, buf, 1) != 0)
  {
    return tp_errno;
  }
  
  /* query number of protocols supported */
  proto_count = (buf[6] << 8) + buf[7];
  
  /* tick through each protocol */
  for (i = 0; i < proto_count; i++)
  {
    unsigned char proto = buf[i + 8];
    
    /* TCG comms? */
    if (proto == 0x01)
    {
      has_tcg = 1;
    }
    
    /* TCG resets? */
    if (proto == 0x02)
    {
      handle->has_reset = 1;
    }
    
    TP_DEBUG(2)
    {
      printf("  (0x%02x) %s\n", proto, tp_security_proto_lookup(proto));
    }
  }
  
  /* return status */
  if (has_tcg == 0)
  {
    return tp_errno = TP_ERR_TPM_PROTO;
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Look up TPM Security Protocols
 *
 * Return human readable representation of TPM protocol
 *
 * \param[in] proto Protocol number
 * \return Pointer to static buffer describing protocol
 */
char const *tp_security_proto_lookup(unsigned char proto)
{
  if (proto == 0)
  {
    return "Security Protocol Discovery";
  }
  else if (proto == 1)
  {
    return "TCG SWG (General Comms)";
  }
  else if (proto == 2)
  {
    return "TCG SWG (Proto Reset)";
  }
  else if (proto <= 6)
  {
    return "TCG SWG (Reserved)";
  }
  else if ((proto == 0x20) || (proto == 0xef))
  {
    return "T10 (Reserved)";
  }
  else if (proto == 0xee)
  {
    return "IEEE P1667";
  }
  else if (proto >= 0xf0)
  {
    return "Vendor Specific";
  }
  else
  {
    return "Reserved";
  }
}

/**
 * \brief Communication Stack Reset
 *
 * Function to reset state of communication ID within TCG SWG interface
 *
 * \param[in] handle Target drive
 * \param[in] com_id Communication ID to reset
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_security_comid_reset(tp_handle_t *handle, uint32_t com_id)
{
  unsigned char block[TP_ATA_BLOCK_SIZE] = {0};
  tp_comid_req_t *cmd = (tp_comid_req_t*)block;
  tp_comid_resp_t *resp = (tp_comid_resp_t*)block;
  
  TP_DEBUG(1) printf("Reset ComID 0x%x\n", com_id);
  
  /* Cook up the COMID management packet */
  cmd->com_id = htobe16(com_id);
  cmd->req_code = htobe32(0x02);     /* STACK_RESET */
  
  /* Hit the reset */
  if ((tp_ata_if_send(handle->ata, 2, com_id, block, 1) != 0) ||
      (tp_ata_if_recv(handle->ata, 2, com_id, block, 1) != 0))
  {
    return tp_errno;
  }
  
  /* Check result */
  if ((htobe32(resp->avail_data) != 4) || (htobe32(resp->failed) != 0))
  {
    return tp_errno = TP_ERR_TPM_COMID_RESET;
  }
  
  TP_DEBUG(2) printf("  Completed\n");
  return tp_errno = TP_ERR_SUCCESS;
}
