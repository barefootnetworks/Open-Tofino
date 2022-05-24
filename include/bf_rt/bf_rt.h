/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt.h
 *
 *  @brief One-stop C header file for applications to include for
 *  using BFRT C-frontend
 */
#ifndef _BF_RT_H
#define _BF_RT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_info.h>
#include <bf_rt/bf_rt_init.h>
#include <bf_rt/bf_rt_learn.h>
#include <bf_rt/bf_rt_session.h>
#include <bf_rt/bf_rt_table.h>
#include <bf_rt/bf_rt_table_attributes.h>
#include <bf_rt/bf_rt_table_data.h>
#include <bf_rt/bf_rt_table_key.h>
#include <bf_rt/bf_rt_table_operations.h>

#ifdef __cplusplus
}
#endif

#endif  //_BF_RT_H
