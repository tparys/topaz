#ifndef TOPAZ_L0_FEATURES_H
#define TOPAZ_L0_FEATURES_H

/*
 * Topaz - Level 0 Discovery Features
 *
 * Feature ID and structure definitions for TCG SWG Level 0 Discovery
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

/** Enumerations of known TCG SWG features */
typedef enum
{
  /** Trusted Peripheral (TPer) General Info */
  TP_FEAT_TPER       = 0x0001,
  
  /** Locking & Cryptography Feature */
  TP_FEAT_LOCK       = 0x0002,
  
  /** Geometry Reporting (Advanced format drives) */
  TP_FEAT_GEO        = 0x0003,
  
  /** TCG Enterprise 1.x (Enterprise SSC) */
  TP_FEAT_ENTERPRISE = 0x0100,
  
  /** TCG Opal 1.x (Opal SSC) */
  TP_FEAT_OPAL1      = 0x0200,
  
  /** Single User Mode */
  TP_FEAT_SINGLE     = 0x0201,
  
  /** Additional Datastore Tables */
  TP_FEAT_TABLES     = 0x0202,
  
  /** TCG Opal 2.x (Opal SSC) */
  TP_FEAT_OPAL2      = 0x0203,
  
  /** TCG Opalite 1.x (Opalite SSC) */
  TP_FEAT_OPALITE    = 0x0301,
  
  /** TCG Pyrite 1.x (Pyrite SSC) */
  TP_FEAT_PYRITE     = 0x0302
  
} tp_feat_id_t;

/** TCG SWG Level 0 Discovery Header */
typedef struct
{
  /** Valid data length (excluding this field) */
  uint32_t length;
  
  /** 0x0000 for now */
  uint16_t major_ver;
  
  /** 0x0001 for now */
  uint16_t minor_ver;
  
  /** Reserved for later use */
  char reserved[8];
  
  /** Vendor specific data */
  char vendor[32];
  
} tp_header_t;

/** SWG Level 0 Feature Descriptor */
typedef struct
{
  /** Feature ID */
  uint16_t code;
  
  /** Feature version */
  uint8_t  version;
  
  /** Length of feature data */
  uint8_t  length;
  
} tp_feat_t;

// Geometry Feature Data (0x003)
typedef struct
{
  uint8_t  align;
  uint8_t  reserved[7];
  uint32_t lba_size;     // Logical Block Size
  uint64_t align_gran;   // Alignment Granularity
  uint64_t lowest_align; // Lowest Aligned LBA
} tp_feat_geo_t;

// TCG Enterprise SSC Feature Data (0x100)
// TCG Opal 1.0 SSC Feature Data (0x200
typedef struct
{
  uint16_t comid_base;
  uint16_t comid_count;
  uint8_t  range_bhv;   // bits 1-7 reserved
} tp_feat_ssc1_t;
  
// TCG Opal Single User Mode Data (0x201)
typedef struct
{
  uint32_t lock_obj_count; // Number of locking objects supported
  uint8_t  bitmask;        // If any objects are in single user & lock policy
} tp_feat_single_t;

// TCG Opal Additional DataStore Tables (0x202)
typedef struct
{
  uint16_t reserved;
  uint16_t max_tables;  // Max number of DataStore Tables
  uint32_t max_size;    // Max size of all DataStore Tables
  uint32_t table_align; // DataStore Table Alignment
} tp_feat_tables_t;
  
// TCG Opal 2.0 SSC Feature Data (0x203)
// TCG Opalite 1.0 SSC Feature Data (0x301)
// TCG Pyrite 1.0 SSC Feature Data (0x302)
typedef struct
{
  uint16_t comid_base;
  uint16_t comid_count;
  uint8_t  range_bhv;   // bits 1-7 reserved
  uint16_t admin_count; // number locking SP admin supported
  uint16_t user_count;  // number locking SP user supported
  uint8_t  init_pin;    // Initial PIN Indicator
  uint8_t  revert_pin;  // behavior on PIN revert
} tp_feat_ssc2_t;
  
#endif
