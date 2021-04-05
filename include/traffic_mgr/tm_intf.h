/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TM_INTF_H__
#define __TM_INTF_H__

#include <traffic_mgr/traffic_mgr_types.h>

bf_status_t bf_tm_init();

// Used for UT purposes only..
void bf_tm_set_ut_mode_as_model(bf_dev_id_t dev);
void bf_tm_set_ut_mode_as_asic(bf_dev_id_t dev);

#endif
