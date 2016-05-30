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

#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <topaz/debug.h>
#include <topaz/uid_swg.h>
#include <topaz/swg_core.h>
#include <topaz/transport_ata.h>
#include <topaz/syntax.h>

#include <topaz/debug.h>

/* Pad to value to mutiple of another */
#define TP_PAD_MULTIPLE(val, mult) (((val + (mult - 1)) / mult) * mult)

// How often to poll the device for data (millisecs)
#define POLL_MS 1

// How long to wait before timeout thrown
#define TIMEOUT_SECS 10

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
		       int use_session_ids)
{
  size_t sub_size, pkt_size, com_size, tot_size;
  tp_swg_header_t *header;
  
  /* check for NULL pointers */
  if ((dev == NULL) || (payload == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* Sub Packet contains the actual payload data */
  sub_size = payload->cur_len;
  
  /* Packet includes Sub Packet header */
  pkt_size = sub_size + sizeof(tp_swg_sub_packet_header_t);
  
  /* .. and gets padded to a multiple of 4 bytes */
  pkt_size = TP_PAD_MULTIPLE(pkt_size, 4);
  
  /* Com Packet includes Packet header */
  com_size = pkt_size + sizeof(tp_swg_packet_header_t);
  
  /* Grand total include Com Packet header */
  tot_size = com_size + sizeof(tp_swg_com_packet_header_t);
  
  // ... and gets padded to multiple of 512 bytes
  tot_size = TP_PAD_MULTIPLE(tot_size, TP_ATA_BLOCK_SIZE);
  
  /* Make sure the drive can handle this data */
  if (tot_size > dev->max_com_packet_size)
  {
    return tp_errno = TP_ERR_PACKET_SIZE;
  }
  
  /* using the buffer from device handle */
  memset(dev->io_block, 0, sizeof(dev->io_block));
  header = (tp_swg_header_t*)dev->io_block;
  
  /* fill in headers */
  header->com.com_id = htobe16(dev->com_id);
  header->com.length = htobe32(com_size);
  header->pkt.length = htobe32(pkt_size);
  header->sub.length = htobe32(sub_size);
  
  /* if desired, include current session IDs?
   * (not used for session manager) */
  if (use_session_ids)
  {
    //header->pkt.tper_session_id = htobe32(tper_session_id);
    //header->pkt.host_session_id = htobe32(host_session_id);
  }
  
  /* copy over payload data */
  memcpy(dev->io_block + sizeof(tp_swg_header_t),
	 payload->ptr, payload->cur_len);
  
  return tp_ata_if_send(dev->ata, 1, dev->com_id,
			dev->io_block, tot_size / TP_ATA_BLOCK_SIZE);
}

/**
 * \brief Receive payload via SWG comms
 *
 * Receive data from Trusted Peripheral (TPer) in target device to payload buffer.
 *
 * \param[out] payload Buffer describing data to transmit
 * \param[in] dev Target device for data payload
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_swg_recv(tp_buffer_t *payload, tp_handle_t *dev)
{
  /* Maximum poll attempts before timeout */
  int max_iters = (TIMEOUT_SECS * 1000) / POLL_MS;
  tp_swg_header_t *header;
  
  /* check for NULL pointers */
  if ((dev == NULL) || (payload == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }

  /* using the buffer from device handle */
  memset(dev->io_block, 0, sizeof(dev->io_block));
  header = (tp_swg_header_t*)dev->io_block;
  
  /* if still processing, drive may respond with "no data yet" */
  do
  {
    /* Receive formatted Com Packet */
    if (tp_ata_if_recv(dev->ata, 1, dev->com_id, dev->io_block,
		       dev->max_com_packet_size / TP_ATA_BLOCK_SIZE))
    {
      return tp_errno;
    }
    
    /* Do some cursory verification here */
    if (be16toh(header->com.com_id) != dev->com_id)
    {
      tp_debug_dump(dev->io_block, sizeof(dev->io_block));
      return tp_errno = TP_ERR_BAD_COMID;
    }
    if (be32toh(header->com.length) == 0)
    {
      /* Response is not yet ready ... wait a bit and try again */
      usleep(POLL_MS * 1000);
    }
  } while ((be32toh(header->com.length) == 0) && (--max_iters > 0));
  
  /* Check for timeout */
  if (max_iters == 0)
  {
    return tp_errno = TP_ERR_TIMEOUT;
  }
  
  /* Ready the receiver buffer */
  memset(payload, 0, sizeof(tp_buffer_t));
  payload->ptr = dev->io_block + sizeof(tp_swg_header_t);
  payload->cur_len = be32toh(header->sub.length);
  payload->max_len = be32toh(header->sub.length);
  
  return tp_errno = TP_ERR_SUCCESS;
}


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
			 tp_buffer_t const *args)
{
  int use_session_ids;
  uint8_t call_status;
  tp_buffer_t work;
  char work_raw[MAX_IO_BLOCK];
  
  /* check for NULL pointers */
  if (dev == NULL)
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* set up static buffer */
  memset(work_raw, 0, sizeof(work_raw));
  memset(&work, 0, sizeof(work));
  work.ptr = work_raw;
  work.max_len = sizeof(work_raw);
  
  /* session ID's with everything but session manager */
  use_session_ids = (obj_uid == TP_SWG_SMUID ? 0 : 1);
  
  /* encode method, and perform I/O */
  if ((tp_syn_enc_method(&work, TP_SWG_SMUID, TP_SWG_PROPERTIES, args)) ||
      (tp_swg_send(dev, &work, use_session_ids)) ||
      (tp_swg_recv(&work, dev)))
  {
    return tp_errno;
  }
  
  /* NOTE - work.ptr now points within dev->io_block via tp_swg_recv() */
  
  /* skip method signature, if present (session manager stuff) */
  if (work.byte_ptr[0] == 0xf8)
  {
    /* remove leading 19 bytes from buffer */
    if (tp_buf_trim_left(&work, 19))
    {
      return tp_errno;
    }
  }
  
  /* last 5 bytes contain method status code */
  call_status = work.byte_ptr[work.cur_len - 4];
  if (call_status)
  {
    /* convert to appropriate error code */
    return tp_errno = (TP_ERR_CALL_SUCCESS + call_status);
  }
  
  /* if response is wanted, extract from remaining bytes */
  if (response)
  {
    if ((tp_buf_trim_left(&work, 1)) ||
	(tp_buf_trim_right(&work, 7)))
    {
      return tp_errno;
    }
    
    tp_debug_dump(work.ptr, work.cur_len);
    if (tp_syn_print(&work))
    {
      printf("Did not parse! - %s\n", tp_errno_lookup_cur());
    }
    
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}

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
tp_errno_t tp_swg_do_properties(tp_handle_t *dev)
{
  return tp_errno = TP_ERR_UNSPECIFIED;
}
