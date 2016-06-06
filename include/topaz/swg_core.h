#ifndef TOPAZ_SWG_CORE_H
#define TOPAZ_SWG_CORE_H

/*
 * Topaz - SWG Core
 *
 * Functions and routines to implement the TCG Storage Working Group (SWG)
 * Core Specification (including obsolete portions from v0.9, which happen
 * to still be in use in TCG Enterprise).
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
#include <topaz/buffer.h>
#include <topaz/defs.h>

/** ComPackets are the primary unit of communication with the Trusted
 * Peripheral (TPer). They may contain 0 or more Packets */
typedef struct
{
  uint32_t reserved;
  uint16_t com_id;
  uint16_t com_id_ext;
  uint32_t tper_left;   /* size of data left in TPer */
  uint32_t min_xfer;    /* min recv to receive tper_left data */
  uint32_t length;      /* size of data in this xmit */
} tp_swg_com_packet_header_t;

/* Packets are associated with particular sessions, and may contain
 * zero or more subpackets */
typedef struct
{
  uint32_t tper_session_id;
  uint32_t host_session_id;
  uint32_t seq_number;
  uint16_t reserved;
  uint16_t ack_type;
  uint32_t ack;
  uint32_t length;
} tp_swg_packet_header_t;
  
/* SubPackets may contain zero or more tokens, where tokens may span multiple
 * SubPackets. Note that SubPackets may not span multiple Packets or ComPackets */
typedef struct
{
  uint8_t  reserved[6];
  uint16_t kind;
  uint32_t length;
} tp_swg_sub_packet_header_t;

/** Though for all intents and purposes, all these headers are used together
 * for one unit, and comprise the header metadata for TCG SWG comms */
typedef struct
{
  tp_swg_com_packet_header_t com;
  tp_swg_packet_header_t     pkt;
  tp_swg_sub_packet_header_t sub;
} tp_swg_header_t;

/**
 * \brief Send payload via SWG comms
 *
 * Send data within payload buffer to Trusted Peripheral (TPer) in target device.
 *
 * \param[out] dev Target device for data payload
 * \param[in] payload Buffer describing data to transmit
 * \param[in] use_session_ids If non-zero, include current session IDs in transmission
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_send(tp_handle_t *dev, tp_buffer_t const *payload,
		       int use_session_ids);

/**
 * \brief Receive payload via SWG comms
 *
 * Receive data from Trusted Peripheral (TPer) in target device to payload buffer.
 *
 * \param[out] payload Buffer describing data to transmit
 * \param[in] dev Target device for data payload
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_recv(tp_buffer_t *payload, tp_handle_t *dev);

/**
 * \brief Invoke Method
 *
 * Invoke method in SWG communication stream upon object.
 *
 * \param[in,out] dev Target drive
 * \param[out] response Buffer to catch encoded return (or NULL to ignore)
 * \param[in] obj_uid UID of object for method call
 * \param[in] method_uid UID of method to call
 * \param[in] args Encoded arguments to pass to method (or NULL for none)
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_invoke(tp_handle_t *dev, tp_buffer_t *response,
			 uint64_t obj_uid, uint64_t method_uid,
			 tp_buffer_t const *args);

/**
 * \brief Host Properties
 *
 * Establish level 1 communications by exchanging communication properties
 * with TPM on drive.
 *
 * \param[in,out] dev Target drive
 * \param[out] response Buffer to catch encoded return (or NULL to ignore)
 * \param[in] obj_uid UID of object for method call
 * \param[in] method_uid UID of method to call
 * \param[in] args Encoded arguments to pass to method (or NULL for none)
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_do_properties(tp_handle_t *dev);

/**
 * \brief Start Session
 *
 * Begin anonymous session with target Security Provider (SP)
 *
 * \param[in,out] dev Target drive
 * \param[in] UID of SP object
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_session_start(tp_handle_t *dev, uint64_t sp_uid);

/**
 * \brief End Session
 *
 * Cleanly terminate current session
 *
 * \param[in,out] dev Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_session_end(tp_handle_t *dev);

/**
 * \brief Forget Session
 *
 * Mark current session as terminated, without performing handshake
 *
 * \param[in,out] dev Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_session_forget(tp_handle_t *dev);

#endif
