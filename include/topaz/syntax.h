#ifndef TOPAZ_SYNTAX_H
#define TOPAZ_SYNTAX_H

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

#include <stdint.h>
#include <topaz/buffer.h>

/** Syntax Tokens */
enum
{
  /* Sequence Tokens */
  
  /** Beginning of a list */
  TP_SWG_START_LIST  = 0xf0,
  
  /** End of a list */
  TP_SWG_END_LIST    = 0xf1,
  
  /** Start of a named (key-value) data type */
  TP_SWG_START_NAME  = 0xf2,
  
  /** End of a named (key-value) data type */
  TP_SWG_END_NAME    = 0xf3,
  
  /* Control Tokens */
  
  /** Beginning of a method (function) call */
  TP_SWG_CALL        = 0xf8,

  /** Separator between method call and method status */
  TP_SWG_END_OF_DATA = 0xf9,
  
  /** End current session */
  TP_SWG_END_SESSION = 0xfa,
  
  /** Start of transaction */
  TP_SWG_START_TRANS = 0xfb,
  
  /** End of transaction */
  TP_SWG_END_TRANS   = 0xfc
};

/** SWG Atom decoding information */
typedef struct
{
  /** Number of non-data header bytes prior to data */
  unsigned int header_bytes;

  /** Number of data header bytes following header */
  unsigned int data_bytes;
  
  /** Indicate data is a binary blob */
  unsigned int bin_flag;
  
  /** If not binary, whether integer is signed */
  unsigned int sign_flag;
  
} tp_syn_atom_info_t;

/**
 * \brief Encode Syntax Token
 *
 * Encode 1 byte raw token into target data stream
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Value of token
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_token(tp_buffer_t *tgt, uint8_t token);

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
tp_errno_t tp_syn_enc_tiny(tp_buffer_t *tgt, int sign_flag, uint64_t value);

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
			   void const *ptr, size_t len);

/**
 * \brief Encode Unsigned Integer
 *
 * Encode unsigned integer into SWG binary syntax 
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Input data value
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_uint(tp_buffer_t *tgt, uint64_t value);

/**
 * \brief Encode Signed Integer
 *
 * Encode signed integer into SWG binary syntax 
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Input data value
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_sint(tp_buffer_t *tgt, int64_t value);

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
tp_errno_t tp_syn_enc_bin(tp_buffer_t *tgt, void const *ptr, size_t len);

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
tp_errno_t tp_syn_enc_str(tp_buffer_t *tgt, char const *str);

/**
 * \brief Encode Half UID
 *
 * Encode Half UID in typical SWG form (4 byte binary blob)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Integer value of Half UID
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_half_uid(tp_buffer_t *tgt, uint32_t value);

/**
 * \brief Encode UID
 *
 * Encode UID in typical SWG form (8 byte binary blob)
 *
 * \param[in,out] buf Target data buffer
 * \param[in] value Integer value of UID
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_enc_uid(tp_buffer_t *tgt, uint64_t value);

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
			     uint64_t method_uid, tp_buffer_t const *args);

/**
 * \brief Decode Byte
 *
 * Remove next byte from buffer, and compare against expected SWG token
 *
 * \param[out] header Data encoding metadata
 * \param[in] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_byte(tp_buffer_t *tgt, uint8_t expected);

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
tp_errno_t tp_syn_dec_atom_header(tp_syn_atom_info_t *header, tp_buffer_t const *tgt);

/**
 * \brief Decode Unsigned Integer
 *
 * Decode unsigned integer from data buffer and advance pointers.
 *
 * \param[out] value Parsed value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_uint(uint64_t *value, tp_buffer_t *tgt);

/**
 * \brief Decode Signed Integer
 *
 * Decode signed integer from data buffer and advance pointers.
 *
 * \param[out] value Parsed value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_sint(int64_t *value, tp_buffer_t *tgt);

/**
 * \brief Decode Binary Blob
 *
 * Decode binary data segment from data buffer and advance pointers.
 *
 * \param[out] value Buffer describing parsed data
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_bin(tp_buffer_t *value, tp_buffer_t *tgt);

/**
 * \brief Decode UID
 *
 * Decode UID stored as binary blob, and advance pointers.
 *
 * \param[out] value Numeric UID value
 * \param[in,out] buf Input data stream
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_dec_uid(uint64_t *value, tp_buffer_t *tgt);

/**
 * \brief Display encoded data atom
 *
 * Print human readable version of encoded SWG data atom.
 *
 * \param[in,out] data SWG data to show
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_print_atom(tp_buffer_t *data);

/**
 * \brief Display encoded data stream
 *
 * Print human readable version of encoded SWG data.
 *
 * \param[in,out] data SWG data to show
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_syn_print(tp_buffer_t *data);

#endif
