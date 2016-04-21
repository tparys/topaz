/*
 * Topaz - ATA Transport (Linux SGIO)
 *
 * This file is an implementation of a Linux specific OS API for
 * basic ATA commands.
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <topaz/debug.h>
#include <topaz/errno.h>
#include <topaz/transport_ata.h>

/** Linux device handle */
struct tp_ata_handle
{
  int fd; /** POSIX file descriptor */
};

/**
 * \brief Open ATA Device (OS Specific)
 *
 * OS-agnostic API to provide an ATA device handle
 *
 * \param[in] path Path to device
 * \return Pointer to new device, or NULL on error
 */
struct tp_ata_handle *tp_ata_open(char const *path)
{
  struct tp_ata_handle *handle = NULL;
  int rc, fd = -1;
  char in;
  
  /*
   * First check that libata is playing nice ...
   */
  
  /* open pesudofile in sysfs */
  TP_DEBUG(1) printf("Probe libata configuration\n");
  fd = open("/sys/module/libata/parameters/allow_tpm", O_RDONLY);
  if (fd == -1)
  {
    rc = TP_ERR_SYSFS;
    goto cleanup;
  }
  
  /* check kernel's setting */
  if ((read(fd, &in, 1) != 1) || (in == '0'))
  {
    /* libata blocking TPM calls ... */
    rc = TP_ERR_LIBATA;
    goto cleanup;
  }
  
  /* cleanup */
  close(fd);
  
  /*
   * Next, let's open the device ...
   */
  
  /* open block device */
  fd = open(path, O_RDWR);
  if (fd == -1)
  {
    rc = TP_ERR_OPEN;
    goto cleanup;
  }
  
  /* allocate some memory for device handle */
  handle = (struct tp_ata_handle*)calloc(sizeof(struct tp_ata_handle), 1);
  if (handle == NULL)
  {
    rc = TP_ERR_ALLOC;
    goto cleanup;
  }
  
  /* all done */
  tp_errno = TP_ERR_SUCCESS;
  handle->fd = fd;
  return handle;
  
  cleanup: /* on failure */
  
  /* close handle if open */
  if (fd != -1)
  {
    close(fd);
  }
  
  /* clear memory if allocated */
  if (handle != NULL)
  {
    free(handle);
  }
  
  /* set errno and return */
  tp_errno = rc;
  return NULL;
}

/**
 * \brief Close ATA Device (OS Specific)
 *
 * Linux specific API to close a ATA device handle
 *
 * \param[in] handle Device handle
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_ata_close(struct tp_ata_handle *handle)
{
  /* sanity check */
  if (handle == NULL)
  {
    return -1;
  }
  
  /* cleanup */
  close(handle->fd);
  free(handle);
  return 0;
}

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
tp_errno_t tp_ata_exec12(struct tp_ata_handle *handle, tp_ata_cmd12_t const *cmd,
			 tp_ata_oper_type_t optype, void *data,
			 uint8_t bcount, int wait)
{
  struct sg_io_hdr sg_io;  // ioctl data structure
  unsigned char cdb[12];   // Command descriptor block
  unsigned char sense[32]; // SCSI sense (error) data
  int rc;
  
  // Initialize structures
  memset(&sg_io, 0, sizeof(sg_io));
  memset(&cdb, 0, sizeof(cdb));
  memset(&sense, 0, sizeof(sense));
  
  ////
  // Fill in ioctl data for ATA12 pass through
  //
  
  // Mandatory per interface
  sg_io.interface_id    = 'S';
  
  // Location, size of command descriptor block (command)
  sg_io.cmdp            = cdb;
  sg_io.cmd_len         = sizeof(cdb);
  
  // Command data transfer (optional)
  sg_io.dxferp          = data;
  sg_io.dxfer_len       = bcount * TP_ATA_BLOCK_SIZE;
  
  // Sense (error) data
  sg_io.sbp             = sense;
  sg_io.mx_sb_len       = sizeof(sense);
  
  // Timeout (ms)
  sg_io.timeout         = wait * 1000;
  
  ////
  // Fill in SCSI command
  //
  
  // Byte 0: ATA12 pass through
  cdb[0] = 0xA1;
  
  // Byte 1: ATA protocol (read/write/none)
  // Byte 2: Check condition, blocks, size, I/O direction
  // Final direction specific bits
  switch (optype)
  {
    case TP_ATA_OPER_READ:
      sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
      cdb[1] = 4 << 1; // ATA PIO-in
      cdb[2] = 0x2e;   // Check, blocks, size in sector count, read
      break;

    case TP_ATA_OPER_WRITE:
      sg_io.dxfer_direction = SG_DXFER_TO_DEV;
      cdb[1] = 5 << 1; // ATA PIO-out
      cdb[2] = 0x26;   // Check, blocks, size in sector count
      break;
      
    default: // Invalid
      return tp_errno = TP_ERR_INVALID;
      break;
  }
  
  // Rest of ATA12 command get copied here (7 bytes)
  memcpy(cdb + 3, cmd, 7);
  
  ////
  // Run ioctl
  //
  
  // Debug output command
  TP_DEBUG(4)
  {
    // Command descriptor block
    printf("ATA Command:\n");
    tp_debug_dump(cmd, sizeof(*cmd));
    
    // Command descriptor block
    printf("SCSI CDB:\n");
    tp_debug_dump(cdb, sizeof(cdb));
    
    // Data out?
    if (optype == TP_ATA_OPER_WRITE)
    {
      printf("Write Data:\n");
      tp_debug_dump(data, bcount * TP_ATA_BLOCK_SIZE);
    }
  }
  
  // System call
  rc = ioctl(handle->fd, SG_IO, &sg_io);
  if (rc != 0)
  {
    return tp_errno = TP_ERR_IOCTL;
  }
  
  // Debug input
  if (optype == TP_ATA_OPER_READ)
  { 
    TP_DEBUG(4)
    {
      printf("Read Data:\n");
      tp_debug_dump(data, bcount * TP_ATA_BLOCK_SIZE);
    }
  }
  
  // Check sense data
  if (sense[0] != 0x72 || sense[7] != 0x0e || sense[8] != 0x09
      || sense[9] != 0x0c || sense[10] != 0x00)
  {
    //fprintf(stderr, "error  = %02x\n", sense[11]);    // 0x00 means success
    //fprintf(stderr, "status = %02x\n", sense[21]);    // 0x50 means success
    return tp_errno = TP_ERR_SENSE;
  }
  
  // Otherwise ok
  return 0;
}
