#ifndef TOPAZ_L0_DISCOVERY_H
#define TOPAZ_L0_DISCOVERY_H

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

#include <topaz/defs.h>

/**
 * \brief Probe Drive's SWG Features
 *
 * Scan for available TCG SWG protocols supported
 *
 * \param[in] Target drive
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_probe_discovery(tp_handle_t *handle);

/**
 * \brief Parse SSC Feature Data (Format 1)
 *
 * \param[in] Target drive
 * \param[in] Feature Data
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_discovery_ssc1(tp_handle_t *handle, void const *feat_data);

/**
 * \brief Parse SSC Feature Data (Format 2)
 *
 * \param[in] Target drive
 * \param[in] Feature Data
 * \return 0 on success, error code indicating failure
 */
tp_errno_t tp_discovery_ssc2(tp_handle_t *handle, void const *feat_data);


#endif
