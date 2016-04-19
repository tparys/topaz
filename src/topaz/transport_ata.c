/*
 * Topaz - ATA Transport
 *
 * This file implements OS abstracted API to implement TCG IF-SEND and IF-RECV
 * calls, along with other basic ATA commands.
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

#include <stdio.h>
#include <string.h>
#include <topaz/transport_ata.h>

/**
 * \brief ATA Identify
 *
 * Implementation of ATA Identify command to query
 * drive self-identification data.
 *
 * \param[in] handle Device handle
 * \param[out] data Pointer to 512 byte buffer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_get_identify(struct TP_ATA_HANDLE *handle, void *data)
{
  /* ATA12 Command - Identify Device (0xec) */
  tp_ata_cmd12_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.command     = 0xec;
  
  /* Off it goes */
  return tp_ata_exec12(handle, &cmd, TP_ATA_OPER_READ, data, 1, 1);
}

/**
 * \brief ATA IF-SEND
 *
 * Implementation of TCG SWG IF-SEND method to send
 * data to a particular Communication ID via a specified
 * security protocol.
 *
 * \param[in] handle Device handle
 * \param[in] proto Security protocol
 * \param[in] comid Communication ID
 * \param[in] data I/O data buffer
 * \param[in] bcount Count of 512 byte blocks to transfer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_if_send(struct TP_ATA_HANDLE *handle, uint8_t proto,
			  uint16_t comid, void *data, uint8_t bcount)
{
  /* Build ATA12 Command - Trusted Send (0x5e) */
  tp_ata_cmd12_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.feature      = proto;
  cmd.count        = bcount;
  cmd.lba_mid      = comid & 0xff;
  cmd.lba_high     = comid >> 8;
  cmd.command      = 0x5e;
  
  /* Off it goes */
  return tp_ata_exec12(handle, &cmd, TP_ATA_OPER_WRITE, data, bcount, 5);
}

/**
 * \brief ATA IF-RECV
 *
 * Implementation of TCG SWG IF-RECV method to receive
 * data from a particular Communication ID via a specified
 * security protocol.
 *
 * \param[in] handle Device handle
 * \param[in] proto Security protocol
 * \param[in] comid Communication ID
 * \param[out] data I/O data buffer
 * \param[in] bcount Count of 512 byte blocks to transfer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_if_recv(struct TP_ATA_HANDLE *handle, uint8_t proto,
			  uint16_t comid, void *data, uint8_t bcount)
{
  /* Build ATA12 command - Trusted Receive (0x5c) */
  tp_ata_cmd12_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.feature      = proto;
  cmd.count        = bcount;
  cmd.lba_mid      = comid & 0xff;
  cmd.lba_high     = comid >> 8;
  cmd.command      = 0x5c;
  
  /* Off it goes */
  return tp_ata_exec12(handle, &cmd, TP_ATA_OPER_READ, data, bcount, 5);
}
