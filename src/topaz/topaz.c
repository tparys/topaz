/*
 * Topaz - Main API
 *
 * This library implements functions and routines to control features on with
 * hard drives that implement TCG standard interfaces. Primarilly designed for
 * self-encrypting drives (SEDs), this allows fine grained user control and
 * strong authentication measures to prevent data access by unauthorized third
 * parties.
 *
 * Currently supported TCG standards (SSCs):
 *    - Enterprise
 *    - Opal 1 & 2
 *    - Opalite 
 *    - Pyrite (NOTE - no encryption)
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

#include <stdlib.h>
#include <topaz/topaz.h>
#include <topaz/transport_ata.h>
#include <topaz/security.h>
#include <topaz/discovery.h>

/**
 * \brief Open Drive / Trusted Peripheral (TPer)
 *
 * Opens a hard drive for use with topaz
 *
 * \param[in] path Path to target device
 * \return Pointer to allocated handle, or NULL on error
 */
tp_handle_t *tp_open(char const *path)
{
  tp_handle_t *handle = NULL;
  tp_errno_t rc = 0;
  
  /* alloc some memory for new handle */
  if ((handle = calloc(1, sizeof(handle))) == NULL)
  {
    rc = TP_ERR_ALLOC;
  }
  
  /* open ATA device */
  else if ((handle->ata = tp_ata_open(path)) == NULL)
  {
    rc = tp_errno;
  }
  
  /* check for TPM */
  else if (tp_ata_probe_tpm(handle->ata) != 0)
  {
    rc = tp_errno;
  }
  
  /* check for TPM security protocols */
  else if (tp_probe_security(handle) != 0)
  {
    rc = tp_errno;
  }
  
  /* check for supported SSC's and SWG features */
  else if (tp_probe_discovery(handle) != 0)
  {
    rc = tp_errno;
  }
  
  /* reset the SSC's ComID, if possible */
  else if ((handle->has_reset) &&
	   (tp_security_comid_reset(handle, handle->com_id) != 0))
  {
    rc = tp_errno;
  }
  
  /* otherwise everything's ok */
  else
  {
    tp_errno = TP_ERR_SUCCESS;
    return handle;
  }
  
  /* cleanup */
  tp_close(handle);
  tp_errno = rc;
  return NULL;
}

/**
 * \brief Close Drive / Trusted Peripheral (TPer)
 *
 * Closes & cleans up topaz device handle
 *
 * \param[in] path Path to target device
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_close(tp_handle_t *handle)
{
  /* sanity check */
  if (handle != NULL)
  {
    /* close ATA device */
    if (handle->ata != NULL)
    {
      tp_ata_close(handle->ata);
      handle->ata = NULL;
    }
    
    /* clear mem */
    free(handle);
    handle = NULL;
  }
  
  /* always return success for now .. */
  return tp_errno = TP_ERR_SUCCESS;
}
