/**
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
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <topaz/transport_ata.h>

/** Linux device handle */
struct TP_ATA_DRIVE
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
struct TP_ATA_DRIVE *tp_ata_open(char const *path)
{
  struct TP_ATA_DRIVE *handle = NULL;
  int fd = -1;
  char in;
  
  /*
   * First check that libata is playing nice ...
   */
  
  /* open pesudofile in sysfs */ 
  fd = open("/sys/module/libata/parameters/allow_tpm", O_RDONLY);
  if (fd == -1)
  {
    return NULL;
  }
  
  /* check kernel's setting */
  if ((read(fd, &in, 1) != 1) || (in == '0'))
  {
    /* libata blocking TPM calls ... */
    close(fd);
    return NULL;
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
    return NULL;
  }
  
  /* allocate some memory for device handle */
  handle = (struct TP_ATA_DRIVE*)calloc(sizeof(struct TP_ATA_DRIVE), 1);
  if (handle == NULL)
  {
    close(fd);
    return NULL;
  }
  
  /* all done */
  handle->fd = fd;
  return handle;
}

/**
 * \brief Close ATA Device (OS Specific)
 *
 * Linux specific API to close a ATA device handle
 *
 * \param[in] handle Device handle
 * \return 0 on success, error code indicating failure
 */
int tp_ata_close(struct TP_ATA_DRIVE *handle)
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
int tp_ata_exec12(struct TP_ATA_DRIVE *handle, tp_ata_cmd12_t const *cmd,
		  tp_ata_oper_type_t optype, void *data,
		  uint8_t bcount, int wait)
{
  // TBD
  return -1;
}
