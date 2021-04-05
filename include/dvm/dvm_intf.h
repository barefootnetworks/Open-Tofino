/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef DVM_INTF_H_INCLUDED
#define DVM_INTF_H_INCLUDED

/**
 * @file dvm_intf.h
 *
 * @brief Device manager interface APIs
 *
 */

/**
 * @brief Check whether the device is a virtual device or not.
 *
 * @param[in] dev_id Device identifier.
 *
 * @return true - if the device is a virtual device, false -otherwise.
 */
bool bf_drv_is_device_virtual(bf_dev_id_t dev_id);

#endif  // DVM_INTF_H
