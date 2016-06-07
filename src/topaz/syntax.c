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
#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <ctype.h>
#include <inttypes.h>
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
    header[0] |= len & 0x0f;
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
 * \return 0 on success, error code indicating failure
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
 * \return 0 on success, error code indicating failure
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
 * \brief Encode Binary Blob
 *
 * Encode binary data segment SWG binary syntax
 *
 * \param[in,out] buf Target data buffer
 * \param[in] ptr Data to encode as atom
 * \param[in] len Data length
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_bin(tp_buffer_t *tgt, void const *ptr, size_t len)
{
  return tp_syn_enc_atom(tgt, 1, 0, ptr, len);
}

/**
 * \brief Encode String
 *
 * Encode string as binary blob in SWG binary syntax
 *
 * \param[in,out] buf Target data buffer
 * \param[in] ptr Data to encode as atom
 * \param[in] len Data length
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_str(tp_buffer_t *tgt, char const *str)
{
  return tp_syn_enc_bin(tgt, str, strlen(str));
}

/**
 * \brief Encode Half UID
 *
 * Encode Half UID in typical SWG form (4 byte binary blob)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Integer value of Half UID
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_half_uid(tp_buffer_t *tgt, uint32_t value)
{
  /* byteflip and encode */
  value = htobe32(value);
  return tp_syn_enc_bin(tgt, &value, sizeof(uint32_t));
}

/**
 * \brief Encode UID
 *
 * Encode UID in typical SWG form (8 byte binary blob)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Integer value of UID
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_uid(tp_buffer_t *tgt, uint64_t value)
{
  /* byteflip and encode */
  value = htobe64(value);
  return tp_syn_enc_bin(tgt, &value, sizeof(uint64_t));
}

/**
 * \brief Encode Method Call
 *
 * Encode UID in typical SWG form (8 byte binary blob)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] obj_uid UID of object for method call
 * \param[in] method_uid UID of method to call
 * \param[in] args If non-NULL, encoded arguments to pass to method
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_method(tp_buffer_t *tgt, uint64_t obj_uid,
			     uint64_t method_uid, tp_buffer_t const *args)
{
  /* check for NULL pointers */
  if ((tgt == NULL) || (tgt->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* build the method call */
  if ((tp_buf_add_byte(tgt, TP_SWG_CALL)) ||
      
      /* followed by object / method UIDs */
      (tp_syn_enc_uid(tgt, obj_uid)) ||
      (tp_syn_enc_uid(tgt, method_uid)) ||
      
      /* start of argument list */
      (tp_buf_add_byte(tgt, TP_SWG_START_LIST)) ||
      
      /* arguments themselves are optional */
      ((args != NULL) && (tp_buf_add_buf(tgt, args))) ||
      
      /* end of argument list */
      (tp_buf_add_byte(tgt, TP_SWG_END_LIST)) ||
    
      /*
       * the end of the method call can be used for terminating long-running
       * processes (re-encryption of data bands, for example). But outside of
       * that, these bytes are generally constant, and we're going to ignore
       * them for now ...
       */
      (tp_buf_add_byte(tgt, TP_SWG_END_OF_DATA)) ||
    
      /* nominally, method status is basically a list of three zeros ... */
      (tp_buf_add_byte(tgt, TP_SWG_START_LIST)) ||
      (tp_syn_enc_uint(tgt, 0)) ||
      (tp_syn_enc_uint(tgt, 0)) ||
      (tp_syn_enc_uint(tgt, 0)) ||
      (tp_buf_add_byte(tgt, TP_SWG_END_LIST)))
  {
    return tp_errno;
  }
  else
  {
    return tp_errno = TP_ERR_SUCCESS;
  }
}

/**
 * \brief Decode Byte
 *
 * Remove next byte from buffer, and compare against expected SWG token
 *
 * \param[out] header Data encoding metadata
 * \param[in] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_byte(tp_buffer_t *tgt, uint8_t expected)
{
  uint8_t actual;

  /* check for NULL */
  if (tgt == NULL)
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* grab next byte */
  if (tp_buf_peek(&actual, tgt))
  {
    return tp_errno;
  }

  /* compare to expected */
  if (expected != actual)
  {
    return tp_errno = TP_ERR_SYNTAX;
  }

  tgt->parse_idx++;
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Decode Atom Header
 *
 * Decode header data from datastream, and determine type of next atom,
 * ensuring all bytes are accounted for in buffer. Note, this function
 * does NOT advance any pointers within the buffer 
 *
 * \param[out] header Data encoding metadata
 * \param[in] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_atom_header(tp_syn_atom_info_t *header, tp_buffer_t const *tgt)
{
  uint8_t *atom;
  size_t bytes_left;

  /* check for NULL pointers */
  if ((header == NULL) || (tgt == NULL) || (tgt->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* otherwise safe to use this pointer */
  atom = (uint8_t*)tgt->ptr + tgt->parse_idx;
  bytes_left = tgt->cur_len - tgt->parse_idx;
  
  /* check that there's something to parse */
  if (bytes_left < 1)
  {
    return tp_errno = TP_ERR_BUFFER_END;
  }
  
  /* figure out encoding ... */
  
  /* tiny atoms start with a binary "0" */
  if ((atom[0] & 0x80) == 0x00)
  {
    /* header combined into data byte */
    header->header_bytes = 0;
    header->data_bytes = 1;
    
    /* always integer, sign in bit 6 */
    header->bin_flag = 0;
    header->sign_flag = (atom[0] >> 6) & 0x01;
  }
  
  /* small atoms start with a binary "10" */
  else if ((atom[0] & 0xc0) == 0x80)
  {
    /* one header byte */
    header->header_bytes = 1;
    
    /* binary / sign flags in bits 5 & 4 */
    header->bin_flag = (atom[0] >> 5) & 0x01;
    header->sign_flag = (atom[0] >> 4) & 0x01;
    
    /* data byte count in bits 0-3 */
    header->data_bytes = (atom[0] & 0x0f);
  }
  
  /* medium atoms start with a binary "110" */
  else if ((atom[0] & 0xe0) == 0xc0)
  {
    /* two header bytes */
    header->header_bytes = 2;
    if (bytes_left < 2)
    {
      return tp_errno = TP_ERR_BUFFER_END;
    }
    
    /* binary / sign flags in bits 4 & 3 of first byte */
    header->bin_flag = (atom[0] >> 4) & 0x01;
    header->sign_flag = (atom[0] >> 3) & 0x01;
    
    /* 11 bit data byte count */
    header->data_bytes = (atom[0] & 0x07) << 8;
    header->data_bytes += atom[1];
  }
  
  /* long atoms start with a binary "111000" (incl. reserved bits) */
  else if ((atom[0] & 0xfc) == 0xe0)
  {
    /* four header bytes */
    header->header_bytes = 4;
    if (bytes_left < 4)
    {
      return tp_errno = TP_ERR_BUFFER_END;
    }
    
    /* binary / sign flags in bits 1 & 0 of first byte */
    header->bin_flag = (atom[0] >> 1) & 0x01;
    header->sign_flag = atom[0] & 0x01;
    
    /* 24 bit data byte count */
    header->data_bytes = atom[1] << 16;
    header->data_bytes += atom[2] << 8;
    header->data_bytes += atom[3];
  }
  
  /* probably some other type of token */
  else
  {
    return tp_errno = TP_ERR_DATATYPE;
  }
  
  /* ensure data bytes exist */
  if (bytes_left < (header->header_bytes + header->data_bytes))
  {
    return tp_errno = TP_ERR_BUFFER_END;
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Decode Unsigned Integer
 *
 * Decode unsigned integer from data buffer and advance pointers.
 *
 * \param[out] value Parsed value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_uint(uint64_t *value, tp_buffer_t *tgt)
{
  tp_syn_atom_info_t header_info;
  uint8_t *data_ptr, raw[8];
  
  /* check for NULL pointers */
  if ((value == NULL) || (tgt == NULL) || (tgt->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  data_ptr = (uint8_t*)tgt->ptr + tgt->parse_idx;
  
  /* figure out what's there */
  if (tp_syn_dec_atom_header(&header_info, tgt))
  {
    return tp_errno;
  }
  
  /* ensure it's a valid unsigned int */
  if ((header_info.bin_flag == 1) ||
      (header_info.sign_flag == 1))
  {
    return tp_errno = TP_ERR_DATATYPE;
  }
  if ((header_info.data_bytes == 0) ||
      (header_info.data_bytes > 8))
  {
    return tp_errno = TP_ERR_REPRESENT;
  }
  
  /* trivial case (tiny atom) */
  if (header_info.header_bytes == 0)
  {
    *value = *data_ptr & 0x3f;
    tgt->parse_idx += 1;
    return TP_ERR_SUCCESS;
  }
  
  /* copy data out */
  memset(raw, 0, 8);
  memcpy(raw + 8 - header_info.data_bytes,
	 data_ptr + header_info.header_bytes,
	 header_info.data_bytes);
  memcpy(value, raw, 8);
  
  /* byteflip to native endianess */
  *value = be64toh(*value);
  
  /* advance pointers */
  tgt->parse_idx += header_info.header_bytes;
  tgt->parse_idx += header_info.data_bytes;
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Decode Signed Integer
 *
 * Decode signed integer from data buffer and advance pointers.
 *
 * \param[out] value Parsed value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_sint(int64_t *value, tp_buffer_t *tgt)
{
  tp_syn_atom_info_t header_info;
  uint8_t *data_ptr, raw[8];
  
  /* check for NULL pointers */
  if ((value == NULL) || (tgt == NULL) || (tgt->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  data_ptr = (uint8_t*)tgt->ptr + tgt->parse_idx;
  
  /* figure out what's there */
  if (tp_syn_dec_atom_header(&header_info, tgt))
  {
    return tp_errno;
  }
  
  /* ensure it's a valid signed int */
  if ((header_info.bin_flag == 1) ||
      (header_info.sign_flag == 0))
  {
    return tp_errno = TP_ERR_DATATYPE;
  }
  if ((header_info.data_bytes == 0) ||
      (header_info.data_bytes > 8))
  {
    return tp_errno = TP_ERR_REPRESENT;
  }
  
  /* trivial case (tiny atom) */
  if (header_info.header_bytes == 0)
  {
    /* sign extend */
    *value = *data_ptr & 0x3f;
    if (*value & 0x20)
    {
      *value -= 0x40;
    }
    tgt->parse_idx += 1;
    return TP_ERR_SUCCESS;
  }

  /* sign extend */
  if (data_ptr[header_info.header_bytes] & 0x80)
  {
    memset(raw, 0xff, 8);
  }
  else
  {
    memset(raw, 0x00, 8);
  }
  
  /* copy data out */
  memcpy(raw + 8 - header_info.data_bytes,
	 data_ptr + header_info.header_bytes,
	 header_info.data_bytes);
  memcpy(value, raw, 8);
  
  /* byteflip to native endianess */
  *value = be64toh(*value);
  
  /* advance pointers */
  tgt->parse_idx += header_info.header_bytes;
  tgt->parse_idx += header_info.data_bytes;
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Decode UID
 *
 * Decode UID stored as binary blob, and advance pointers.
 *
 * \param[out] value Numeric UID value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_uid(uint64_t *value, tp_buffer_t *tgt)
{
  /* store buffer state in case this doesn't look like a UID */
  tp_buffer_t bin_data, save = *tgt;
  
  /* retrieve binary data */
  if (tp_syn_dec_bin(&bin_data, tgt))
  {
    return tp_errno;
  }
  
  /* check to see if it looks like a UID */
  if ((bin_data.cur_len != 8) ||
      (bin_data.byte_ptr[bin_data.parse_idx + 0] != 0) ||
      (bin_data.byte_ptr[bin_data.parse_idx + 4] != 0))
  {
    /* restore state */
    *tgt = save;
    return tp_errno = TP_ERR_DATATYPE;
  }
  
  /* otherwise convert to integer */
  memcpy(value, bin_data.byte_ptr + bin_data.parse_idx, 8);
  *value = be64toh(*value);
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Decode Binary Blob
 *
 * Decode binary data segment from data buffer and advance pointers.
 *
 * \param[out] value Buffer describing parsed data
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_bin(tp_buffer_t *value, tp_buffer_t *tgt)
{
  tp_syn_atom_info_t header_info;
  uint8_t *data_ptr;
  
  /* check for NULL pointers */
  if ((value == NULL) || (tgt == NULL) || (tgt->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  data_ptr = (uint8_t*)tgt->ptr + tgt->parse_idx;
  
  /* figure out what's there */
  if (tp_syn_dec_atom_header(&header_info, tgt))
  {
    return tp_errno;
  }
  
  /* ensure it's a valid binary segment,
   * note sign flag must NOT be set! */
  if ((header_info.bin_flag == 0) ||
      (header_info.sign_flag == 1))
  {
    return tp_errno = TP_ERR_DATATYPE;
  }
  
  /* set up return buffer (zero copy) */
  memset(value, 0, sizeof(tp_buffer_t));
  value->ptr = data_ptr + header_info.header_bytes;
  value->max_len = header_info.data_bytes;
  value->cur_len = header_info.data_bytes;
  
  /* advance pointers */
  tgt->parse_idx += header_info.header_bytes;
  tgt->parse_idx += header_info.data_bytes;
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Display encoded data atom
 *
 * Print human readable version of encoded SWG data atom.
 *
 * \param[in,out] data SWG data to show
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_print_atom(tp_buffer_t *data)
{
  tp_syn_atom_info_t info;
  
  /* check for NULL pointers */
  if ((data == NULL) || (data->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* What's the next item? */
  if (tp_syn_dec_atom_header(&info, data) == 0)
  {
    /* atom */
    if (info.bin_flag)
    {
      tp_buffer_t bin_data;
      unsigned i, is_print;

      /* binary data - UIDs, strings, and binary blobs */
      if (tp_syn_dec_bin(&bin_data, data))
      {
	return tp_errno;
      }
      
      /* scan if the whole buffer is printable */
      is_print = 1;
      for (i = 0; i < bin_data.cur_len; i++)
      {
	if (!(isprint(bin_data.byte_ptr[i])))
	{
	  is_print = 0;
	}
      }
      
      /* if non-zero and printable, it's probably a string */
      if ((bin_data.cur_len > 0) &&
	  (is_print == 1))
      {
	/* Careful, not NULL terminated! */
	printf(" \'");
	fwrite(bin_data.ptr, bin_data.cur_len, 1, stdout);
	printf("\'");
      }
      
      /*
       * If it's 8 bytes, and looks like 32 bit int's crammed
       * together, it's probably a UID
       */
      else if ((bin_data.cur_len == 8) &&
	       (bin_data.byte_ptr[bin_data.parse_idx] == 0) &&
	       (bin_data.byte_ptr[bin_data.parse_idx + 4] == 0))
      {
	/* Looks like a UID, try to print it as it's original 32 bit ints */
	uint32_t upper, lower;
	memcpy(&upper, bin_data.byte_ptr + 0, 4);
	upper = be32toh(upper);
	memcpy(&lower, bin_data.byte_ptr + 4, 4);
	lower = be32toh(lower);
	printf(" %x:%x", upper, lower);
      }
      
      /* otherwise, dump the first few bytes of the binary data */
      else
      {
	printf(" {");
	for (i = 0; (i < 16) && (i < bin_data.cur_len); i++)
	{
	  printf("%02x", bin_data.byte_ptr[i]);
	}
	if (bin_data.cur_len > 16)
	{
	  printf("..");
	}
	printf("}");
      }
    }
    else if (info.sign_flag)
    {
      /* signed integer */
      int64_t value = 5;
      if (tp_syn_dec_sint(&value, data))
      {
	return tp_errno;
      }
      printf(" %" PRId64, value);
    }
    else
    {
      /* unsigned integer */
      uint64_t value = 5;
      if (tp_syn_dec_uint(&value, data))
      {
	return tp_errno;
      }
      printf(" %" PRIu64, value);
    }
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Display encoded data stream
 *
 * Print human readable version of encoded SWG data.
 *
 * \param[in,out] data SWG data to show
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_print(tp_buffer_t *data)
{
  tp_syn_atom_info_t info;
  uint64_t obj_uid, method_uid;
  uint8_t next, first_in_loop = 1;
  
  /* check for NULL pointers */
  if ((data == NULL) || (data->ptr == NULL))
  {
    return tp_errno = TP_ERR_NULL;
  }
  
  /* and that there's some room to work with */
  if (data->cur_len <= data->parse_idx)
  {
    return tp_errno = TP_ERR_BUFFER_END;
  }
  
  /* basic data atom? */
  if (tp_syn_dec_atom_header(&info, data) == 0)
  {
    return tp_syn_print_atom(data);
  }

  switch (data->byte_ptr[data->parse_idx])
  {
    case TP_SWG_START_LIST: /* a list of items */
      
      /* start of list */
      printf(" [");
      data->parse_idx++;
      
      /* items within */
      while (1)
      {
	/* check next byte */
	if (tp_buf_peek(&next, data))
	{
	  return tp_errno;
	}
	
	/* kick out of loop */
	if (next == TP_SWG_END_LIST)
	{
	  break;
	}
	
	/* comma separator after the first */
	if (first_in_loop)
	{
	  first_in_loop = 0;
	}
	else
	{
	  printf(",");
	}
	
	/* otherwise recurse */
	if (tp_syn_print(data))
	{
	  return tp_errno;
	}
      }
      
      /* end of list */
      printf(" ]");
      data->parse_idx++;
      
      break;
      
    case TP_SWG_START_NAME: /* named data (key/value data) */
      
      /* start of name */
      data->parse_idx++;
      
      /* first data item (name) */
      if (tp_syn_print(data))
      {
	return tp_errno;
      }
      
      printf(" =");
      
      /* second data item (value) */
      if (tp_syn_print(data))
      {
	return tp_errno;
      }
      
      /* ensure next byte is end name */
      if (tp_buf_peek(&next, data))
      {
	return tp_errno;
      }
      if (next != TP_SWG_END_NAME)
      {
	return tp_errno = TP_ERR_DATATYPE;
      }
      data->parse_idx++;
      break;
      
    case TP_SWG_CALL: /* method call */
      
      /* start of method call */
      data->parse_idx++;
      
      /* object & method uids */
      if ((tp_syn_dec_uid(&obj_uid, data)) ||
	  (tp_syn_dec_uid(&method_uid, data)))
      {
	return tp_errno;
      }
      
      /* dump UIDs in a similar form to TCG docs */
      printf(" %x:%x.%x:%x",
	     (unsigned int)(obj_uid >> 32),
	     (unsigned int)(obj_uid),
	     (unsigned int)(method_uid >> 32),
	     (unsigned int)(method_uid));
	     
      /* lazy way to print the argument list ... */
      if (tp_syn_print(data))
      {
	return tp_errno;
      }
      
      break;
      
    default:
      printf("\n\nUnpexted token %u / 0x%02x\n", next, next);
      
      return tp_errno = TP_ERR_DATATYPE;
      break;
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}
