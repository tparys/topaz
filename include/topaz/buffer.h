#ifndef TOPAZ_BUFFER_H
#define TOPAZ_BUFFER_H

/*
 * Topaz - Data Buffer
 *
 * Functions and routines for manipulating pre-sized static data buffers.
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

#include <stddef.h>
#include <stdint.h>
#include <topaz/errno.h>

/**
 * \brief Container for static buffers
 */
typedef struct
{
  /** Pointer to start of data buffer */
  union
  {
    void *ptr;
    uint8_t *byte_ptr;
  };
  
  /** Maximum length of data segment */
  size_t max_len;
  
  /** Current size of data segment */
  size_t cur_len;
  
  /** When parsing, how many bytes have been used? */
  size_t parse_idx;
  
} tp_buffer_t;

/**
 * \brief Add to buffer 
 *
 * Add data to pre-sized data buffer.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] src Source data buffer
 * \param[in] src_len Length of source data buffer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_add(tp_buffer_t *tgt, void const *src, size_t src_len);

/**
 * \brief Add buffer Byte
 *
 * Add one byte to pre-sized data buffer.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] byte Byte to add
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_add_byte(tp_buffer_t *tgt, uint8_t byte);

/**
 * \brief Add buffer string
 *
 * Append string to pre-sized data buffer (excluding NULL Terminator)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] src NULL terminated data to append
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_add_str(tp_buffer_t *tgt, char *src);

/**
 * \brief Append data buffer to another
 *
 * Combine contents of source data to end of target.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] src Source data buffer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_add_buf(tp_buffer_t *tgt, tp_buffer_t const *src);

/**
 * \brief Trim leading bytes from buffer
 *
 * Advance pointers to remove reference to early bytes in static buffer.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] count Number of bytes to removeSource data buffer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_trim_left(tp_buffer_t *tgt, size_t count);

/**
 * \brief Trim trailing bytes from buffer
 *
 * Change counters to reduce effective size of buffer.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] count Number of bytes to remove
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_trim_right(tp_buffer_t *tgt, size_t count);

/**
 * \brief Peek at next byte for parsing
 *
 * Peek at next byte for parsing
 *
 * \param[out] byte Next byte in stream
 * \param[in,out] buf Target data buffer
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_buf_peek(uint8_t *byte, tp_buffer_t *tgt);

/**
 * \brief Compare buffer against string
 *
 * Peek at next byte for parsing
 *
 * \param[in,out] buf Data buffer for comparison
 * \param[in] ref Reference string for comparison
 * \return Non zero on match, 0 otherwise
 */
int tp_buf_cmp_str(tp_buffer_t *tgt, char const *ref);

#endif
