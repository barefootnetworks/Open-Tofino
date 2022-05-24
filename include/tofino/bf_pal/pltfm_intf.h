/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_BF_PAL_PLATFORM_INTF_H
#define _TOFINO_BF_PAL_PLATFORM_INTF_H

#include <bf_types/bf_types.h>

/**
 * @brief Get the type of the device (model or asic)
 * @param[in] dev_id Device id
 * @param[out] is_sw_model Pointer to bool flag to return true for model and
                      false for asic devices
 * @return Status of the API call
 */
bf_status_t bf_pal_pltfm_type_get(bf_dev_id_t dev_id, bool *is_sw_model);

#endif
