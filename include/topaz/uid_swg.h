#ifndef TOPAZ_UID_SWG_H
#define TOPAZ_UID_SWG_H

/*
 * Topaz - SWG UIDs
 *
 * Definition of object and method UID's, as defined in the TCG SWG Core
 * Specification. These definitions are used for all SSC standards, such
 * as TCG Opal and TCG Enterprise.
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

/** Helper macro to build 64 bit UIDs out of a high and low component */
#define TP_UID(high, low) (((high) * 0x100000000ULL) + (low))

enum
{
  
  /* ========= OBJECTS ========= */
  
  /** Null UID */
  TP_SWG_NULL = TP_UID(0x0, 0x0),
  
  /** Current SP UID, or "This SP". Refers to SP of current session */
  TP_SWG_SPUID = TP_UID(0x0, 0x1),
  
  /* Session Manager */
  TP_SWG_SMUID = TP_UID(0x0, 0xff),
  
  /* Security Identifier (Drive Owner) */
  TP_SWG_SID = TP_UID(0x9, 0x6),
  
  /* PIN / Password of SID (Drive Owner) */
  TP_SWG_C_PIN_SID = TP_UID(0xb, 0x1),
  
  /* Admin SP (Lifecycle managment) */
  TP_SWG_SP_ADMIN = TP_UID(0x205, 0x1),
  
  /* ========= METHODS ========= */
  
  /** SMUID - Communication Properties */
  TP_SWG_PROPERTIES = TP_UID(0, 0xff01),
  
  /** SMUID - Session Startup */
  TP_SWG_START_SESSION = TP_UID(0, 0xff02),
  
  /** SMUID -  ACK / Response to Session Startup */
  TP_SWG_SYNC_SESSION  = TP_UID(0, 0xff03),
  
  /** Next */
  TP_SWG_NEXT = TP_UID(0x6, 0x8),
  
  /** Scramble data */
  TP_SWG_GenKey = TP_UID(0x6, 0x10),
  
  /** Get Data */
  TP_SWG_GET = TP_UID(0x6, 0x16),
  
  /** Set Data */
  TP_SWG_SET = TP_UID(0x6, 0x17),
  
  /** Authenticate / Login */
  TP_SWG_AUTHENTICATE = TP_UID(0x6, 0xc),
  
};

#endif
