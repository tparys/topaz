
/*
 * Topaz - Level 0 Discovery
 *
 * Implementation of the TCG SWG Discovery mechanism, which will allow the
 * topaz library to determine which TCG standards are supported, and what
 * their feature sets are.
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
#include <stdio.h>
#include <endian.h>
#include <topaz/debug.h>
#include <topaz/discovery.h>
#include <topaz/features.h>
#include <topaz/transport_ata.h>

/**
 * \brief Probe Drive's SWG Features
 *
 * Scan for available TCG SWG protocols supported
 *
 * \param[in] Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_probe_discovery(tp_handle_t *handle)
{
  char data[TP_ATA_BLOCK_SIZE], *feat_data;
  tp_header_t *header = (tp_header_t*)data;
  tp_feat_t *feat;
  uint32_t total_len;
  uint16_t major, minor, code;
  size_t offset = sizeof(tp_header_t);
  
  /* Level0 Discovery over IF-RECV */
  TP_DEBUG(1) printf("Establish Level 0 Comms - Discovery\n");
  if (tp_ata_if_recv(handle->ata, 1, 1, data, 1) != 0)
  {
    return tp_errno;
  }
  
  /* Verify structure format */
  total_len = 4 + be32toh(header->length);
  major = be16toh(header->major_ver);
  minor = be16toh(header->minor_ver);
  TP_DEBUG(2)
  {
    printf("  Level0 Size: %d\n",  total_len);
    printf("  Level0 Version: %d / %d\n", major, minor);
  }
  if ((major != 0) || (minor != 1))
  {
    return tp_errno = TP_ERR_TPM_VERSION;
  }
  
  /* until we find out otherwise */
  handle->ssc_type = TP_SSC_UNKNOWN;
  
  /* Tick through returned feature descriptors */
  for (offset = sizeof(tp_header_t);
       offset < (total_len - sizeof(tp_feat_t));
       offset += feat->length)
  {
    /* Set pointer to next feature */
    feat = (tp_feat_t*)(data + offset);
    
    /* Move to offset of feature data */
    offset += sizeof(tp_feat_t);
    feat_data = data + offset;
    
    /* Figure out what it is */
    code = be16toh(feat->code);
    TP_DEBUG(2) printf("  Feature 0x%04x v%d (%d bytes): ", code,
		       feat->version >> 4, feat->length);
    if (code == TP_FEAT_TPER)
    {
      TP_DEBUG(2)
      {
	printf("Trusted Peripheral (TPer)\n");
	printf("    Sync: %d\n",        0x01 & (data[offset]     ));
	printf("    Async: %d\n",       0x01 & (data[offset] >> 1));
	printf("    Ack/Nak: %d\n",     0x01 & (data[offset] >> 2));
	printf("    Buffer Mgmt: %d\n", 0x01 & (data[offset] >> 3));
	printf("    Streaming: %d\n",   0x01 & (data[offset] >> 4));
	printf("    ComID Mgmt: %d\n",  0x01 & (data[offset] >> 6));
      }
    }
    else if (code == TP_FEAT_LOCK)
    {
      TP_DEBUG(2)
      {
	printf("Locking\n");
	printf("    Supported: %d\n",        0x01 & (data[offset]     ));
	printf("    Enabled: %d\n",          0x01 & (data[offset] >> 1));
	printf("    Locked: %d\n",           0x01 & (data[offset] >> 2));
	printf("    Media Encryption: %d\n", 0x01 & (data[offset] >> 3));
	printf("    MBR Enabled: %d\n",      0x01 & (data[offset] >> 4));
	printf("    MBR Done: %d\n",         0x01 & (data[offset] >> 5));
      }
    }
    else if (code == TP_FEAT_GEO)
    {
      tp_feat_geo_t *geo = (tp_feat_geo_t*)feat_data;
      handle->lba_align = be64toh(geo->lowest_align);
      TP_DEBUG(2)
      {
	printf("Geometry Reporting\n");
	printf("    Align Required: %d\n",    0x01 & geo->align); 
	printf("    LBA Size: %d\n",          be32toh(geo->lba_size));
	printf("    Align Granularity: %u\n", (unsigned int)(handle->lba_align));
	printf("    Lowest Align: %u\n",      be32toh(geo->lowest_align));
      }
    }
    else if (code == TP_FEAT_ENTERPRISE)
    {
      handle->ssc_type = TP_SSC_ENTERPRISE;
      TP_DEBUG(2) printf("Enterprise SSC 1.0\n");
      tp_discovery_ssc1(handle, feat_data);
    }
    else if (code == TP_FEAT_OPAL1)
    {
      handle->ssc_type = TP_SSC_OPAL;
      TP_DEBUG(2) printf("Opal SSC 1.0\n");
      tp_discovery_ssc1(handle, feat_data);
    }
    else if (code == TP_FEAT_SINGLE)
    {
      TP_DEBUG(2)
      {
	tp_feat_single_t *single = (tp_feat_single_t*)feat_data;
	printf("Single User Mode\n");
	printf("    Locking Objects Supported: %d\n", be32toh(single->lock_obj_count));
	printf("    Single User Presence: ");
	switch (0x03 & single->bitmask)
	{
	  case 0:
	    printf("None\n");
	    break;
	    
	  case 1:
	    printf("Some\n");
	    break;
	    
	  default:
	    printf("All\n");
	    break;
	}
	printf("    Ownership Policy: %s\n",
	       (0x04 & single->bitmask ? "Admin" : "User"));
      }
    }
    else if (code == TP_FEAT_TABLES)
    {
      TP_DEBUG(2)
      {
	tp_feat_tables_t *tables = (tp_feat_tables_t*)feat_data;
	printf("Additional DataStore Tables\n");
	printf("    Max Tables: %d\n",     be16toh(tables->max_tables));
	printf("    Max Table Size: %d\n", be32toh(tables->max_size));
	printf("    Table Align: %d\n",    be32toh(tables->table_align));
      }
    }
    else if (code == TP_FEAT_OPAL2)
    {
      handle->ssc_type = TP_SSC_OPAL;
      TP_DEBUG(2) printf("Opal SSC 2.0\n");
      tp_discovery_ssc2(handle, feat_data);
    }
    else if (code == TP_FEAT_OPALITE)
    {
      handle->ssc_type = TP_SSC_OPAL;
      TP_DEBUG(2) printf("Opalite SSC 1.0\n");
      tp_discovery_ssc2(handle, feat_data);
    }
    else if (code == TP_FEAT_PYRITE)
    {
      handle->ssc_type = TP_SSC_OPAL;
      TP_DEBUG(2) printf("Pyrite SSC 1.0\n");
      tp_discovery_ssc2(handle, feat_data);
    }
    else if ((code >= 0x1000) && (code < 0x4000))
    {
      TP_DEBUG(2) printf("SSCs");
    }
    else if (code >= 0xc000)
    {
      TP_DEBUG(2) printf("Vendor Specific\n");
    }
    else
    {
      TP_DEBUG(2) printf("Reserved\n");
    }
  }
  
  /* make sure it supports a recognized messaging type (SSC) */
  if (handle->ssc_type == TP_SSC_UNKNOWN)
  {
    return tp_errno = TP_ERR_NO_SSC;
  }
  
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Parse SSC Feature Data (Format 1)
 *
 * \param[in] Target drive
 * \param[in] Feature Data
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_discovery_ssc1(tp_handle_t *handle, void const *feat_data)
{
  tp_feat_ssc1_t *ssc = (tp_feat_ssc1_t*)feat_data;
  handle->com_id = be16toh(ssc->comid_base);
  TP_DEBUG(2)
  {
    printf("    Base ComID: %u\n",            handle->com_id);
    printf("    Number of ComIDs: %d\n",      be16toh(ssc->comid_count));
    printf("    Range cross BHV: %d\n",       0x01 & (ssc->range_bhv));
  }
  return tp_errno = TP_ERR_SUCCESS;
}

/**
 * \brief Parse SSC Feature Data (Format 2)
 *
 * \param[in] Target drive
 * \param[in] Feature Data
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_discovery_ssc2(tp_handle_t *handle, void const *feat_data)
{
  tp_feat_ssc2_t *ssc = (tp_feat_ssc2_t*)feat_data;
  handle->com_id = be16toh(ssc->comid_base);
  int admin_count = be16toh(ssc->admin_count);
  int user_count = be16toh(ssc->user_count);
  TP_DEBUG(2)
  {
    printf("    Base ComID: %u\n",       handle->com_id);
    printf("    Number of ComIDs: %d\n", be16toh(ssc->comid_count));
    printf("    Range cross BHV: %d\n",  0x01 & (ssc->range_bhv));
    printf("    Max SP Admin: %d\n",     admin_count);
    printf("    Max SP User: %d\n",      user_count);
    printf("    C_PIN_SID Initial: ");
    if (ssc->init_pin == 0x00)
    {
      printf("C_PIN_MSID\n");
    }
    else if (ssc->init_pin == 0xff)
    {
      printf("Vendor Defined\n");
    }
    else
    {
      printf("Reserved (%02x)\n", ssc->init_pin);
    }
    printf("    C_PIN_SID Revert: ");
    if (ssc->revert_pin == 0x00)
    {
      printf("C_PIN_MSID\n");
    }
    else if (ssc->revert_pin == 0xff)
    {
      printf("Vendor Defined\n");
    }
    else
    {
      printf("Reserved (%02x)\n", ssc->revert_pin);
    }
  }
  return tp_errno = TP_ERR_SUCCESS;
}
