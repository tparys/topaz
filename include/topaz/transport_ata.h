#ifndef TOPAZ_TRANSPORT_ATA_H
#define TOPAZ_TRANSPORT_ATA_H

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

#include <stdint.h>
#include <topaz/errno.h>

/** Single ATA block (Note sector size may still be 4k) */
#define TP_ATA_BLOCK_SIZE 512

/** ATA operation direction */
typedef enum
{
  TP_ATA_OPER_READ = 1,
  TP_ATA_OPER_WRITE
} tp_ata_oper_type_t;

/** ATA12 Command */
typedef struct
{
  uint8_t feature;
  uint8_t count;
  uint8_t lba_low;
  uint8_t lba_mid;
  uint8_t lba_high;
  uint8_t device;
  uint8_t command;
} tp_ata_cmd12_t;

/** Opaque ATA device data handle (OS-Agnostic) */
struct TP_ATA_DRIVE;

/**
 * \brief Open ATA Device (OS Specific)
 *
 * OS-agnostic API to provide an ATA device handle
 *
 * \param[in] path Path to device
 * \return Pointer to new device, or NULL on error
 */
struct TP_ATA_DRIVE *tp_ata_open(char const *path);

/**
 * \brief Close ATA Device (OS Specific)
 *
 * OS-agnostic API to close a ATA device handle
 *
 * \param[in] handle Device handle
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_close(struct TP_ATA_DRIVE *handle);

/**
 * \brief Execute ATA12 Command (OS Specific)
 *
 * OS-agnostic API to execute an ATA12 command
 *
 * \param[in] handle Device handle
 * \param[in] cmd Pointer to ATA12 command structure
 * \param[in] optype Operation type / direction
 * \param[in,out] data Data buffer for operation
 * \param[in] bcount Count of 512 byte blocks to transfer
 * \param[in] wait Timeout in seconds
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_exec12(struct TP_ATA_DRIVE *handle, tp_ata_cmd12_t const *cmd,
			 tp_ata_oper_type_t optype, void *data,
			 uint8_t bcount, int wait);

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
tp_errno_t tp_ata_get_identify(struct TP_ATA_DRIVE *handle, void *data);

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
tp_errno_t tp_ata_if_send(struct TP_ATA_DRIVE *handle, uint8_t proto,
			  uint16_t comid, void *data, uint8_t bcount);

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
tp_errno_t tp_ata_if_recv(struct TP_ATA_DRIVE *handle, uint8_t proto,
			  uint16_t comid, void *data, uint8_t bcount);

#endif
