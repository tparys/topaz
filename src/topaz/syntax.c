/*
 * Topaz - SWG Binary Syntax
 *
 * Implementation of the TCG Storage Working Group (SWG) binary syntax, as
 * outlined in the SWG Core Specification, and as used within the TCG Opal,
 * Enterprise, and other SED interface standards.
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
#include <string.h>
#include <endian.h>
#include <topaz/syntax.h>

/**
 * \brief Encode Tiny Atom
 *
 * Encode signed or unsigned 6 bit integer into a 1 byte Atom.
 *
 * \param[in,out] buf Target data buffer
 * \param[in] sign_flag Indicate signed, rather than unsigned data type
 * \param[in] value Input data value (only low 6 bits used)
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_tiny(tp_buffer_t *tgt, int sign_flag, uint64_t value)
{
  uint8_t atom;
  
  /* bit 7 always 0 */
  atom = 0;
  
  /* bit 6 indicates sign */
  atom |= (sign_flag & 0x1) << 6;
  
  /* bits 5-0 are data */
  atom |= 0x3f & value;
  
  /* tiny atoms are always a single byte */
  return tp_buf_add_byte(tgt, atom);
}

/**
 * \brief Encode Atom
 *
 * Encode binary or integer data to binary syntax
 *
 * \param[in,out] buf Target data buffer
 * \param[in] bin_flag Indicate binary data
 * \param[in] sign_flag Indicate signed, rather than unsigned data type
 * \param[in] ptr Data to encode as atom
 * \param[in] len Data length
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_atom(tp_buffer_t *tgt, int bin_flag, int sign_flag,
			   void const *ptr, size_t len)
{
  uint8_t header[4];
  int header_bytes;
  
  memset(header, 0, sizeof(header));
  
  /* choose SWG atom encoding for requested data type and size */
  if (len < 16)
  {
    /* short atom (1 byte header) */
    header_bytes = 1;

    /* bit pattern - 10.. */
    header[0] = 0x80;
    
    /* datatype flags */
    header[0] |= (bin_flag & 0x01) << 5;
    header[0] |= (sign_flag & 0x01) << 4;

    /* encode length */
    header[0] |= len & 0xff;
  }
  else if (len < 2048)
  {
    /* medium atom (2 byte header) */
    header_bytes = 2;

    /* bit pattern - 110.. */
    header[0] = 0xc0;
    
    /* data type flags */
    header[0] |= (bin_flag & 0x01) << 4;
    header[0] |= (sign_flag & 0x01) << 3;
    
    /* encode length */
    header[0] |= (len >> 8) & 0x07;
    header[1] |= len & 0xff;
  }
  else if (len < 16777216)
  {
    /* long atom (4 byte header) */
    header_bytes = 4;
    
    /* bit pattern - 111000.. */
    header[0] = 0xe0;
    
    /* data type flags */
    header[0] |= (bin_flag & 0x01) << 1;
    header[0] |= sign_flag & 0x01;
    
    /* encode length */
    header[1] |= (len >> 16) & 0xff;
    header[2] |= (len >> 8) & 0xff;
    header[3] |= len & 0xff;
  }
  else
  {
    /* really? */
    return tp_errno = TP_ERR_REPRESENT;
  }
  
  /* encode the header, then data */
  if (!tp_buf_add(tgt, header, header_bytes))
  {
    tp_buf_add(tgt, ptr, len);
  }
  
  return tp_errno;
}

/**
 * \brief Encode Unsigned Integer
 *
 * Encode unsigned integer into SWG binary syntax 
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Input data value
 */
tp_errno_t tp_syn_enc_uint(tp_buffer_t *tgt, uint64_t value)
{
  uint8_t raw[8];
  int skip;
  
  /* check for trivial encoding */
  if (value < 0x40)
  {
    return tp_syn_enc_tiny(tgt, 0, value);
  }
  
  /* pull out as big-endian bytes */
  value = htobe64(value);
  memcpy(raw, &value, sizeof(raw));
  
  /* to use minimum encoding, we drop leading 0x00's */
  for (skip = 0; (skip < 8) && (raw[skip] == 0x00); skip++) {}
  
  /* encode whatever's left */
  return tp_syn_enc_atom(tgt, 0, 0, raw + skip, sizeof(raw) - skip);
}

/**
 * \brief Encode Signed Integer
 *
 * Encode signed integer into SWG binary syntax 
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Input data value
 */
tp_errno_t tp_syn_enc_sint(tp_buffer_t *tgt, int64_t value)
{
  uint8_t raw[8];
  int skip;
  
  /* check for trivial encoding */
  if ((value < 0x20) && (value >= -0x20))
  {
    return tp_syn_enc_tiny(tgt, 1, value);
  }
  
  /* keep track of negative sign for byteflip later */
  int neg = (value < 0 ? 1 : 0);
  
  /* pull out as big-endian bytes */
  value = htobe64(value);
  memcpy(raw, &value, sizeof(raw));
  
  /* logic differs based on sign to use minimum encoding */
  if (neg)
  {
    /*
     * Negative condition is to drop 0xff bytes,
     * provided the remaining value is still negative
     * (most significant remaining bit is a 1)
     */
    for (skip = 0; (skip < 8) && (raw[skip] == 0xff) &&
	   ((raw[skip + 1] & 0x80) == 0x80); skip++) {}
  }
  else
  {
    /*
     * Positive condition is to drop 0x00 bytes,
     * provided the remaining value is still positive
     * (most significant remaining bit is a 0)
     */
    for (skip = 0; (skip < 8) && (raw[skip] == 0x00) &&
	   ((raw[skip + 1] & 0x80) == 0x00); skip++) {}
  }
  
  /* encode whatever's left */
  return tp_syn_enc_atom(tgt, 0, 1, raw + skip, sizeof(raw) - skip);
}

/**
 * \brief Encode Binary Data
 *
 * Encode binary data SWG binary syntax
 *
 * \param[in,out] buf Target data buffer
 * \param[in] ptr Data to encode as atom
 * \param[in] len Data length
 */
tp_errno_t tp_syn_enc_bin(tp_buffer_t *tgt, void const *ptr, size_t len)
{
  return tp_syn_enc_atom(tgt, 1, 0, ptr, len);
}
