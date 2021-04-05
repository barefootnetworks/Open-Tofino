/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TRAFFIC_MGR_PIPE_INTF_H__
#define __TRAFFIC_MGR_PIPE_INTF_H__

/**
 * @file traffic_mgr_pipe_intf.h
 *
 * @brief This file contains APIs for Traffic Manager application to
 *        program pipe threshold values meant to implement part of
 *        QoS behaviour.
 */

#include <traffic_mgr/traffic_mgr_types.h>
#include <bf_types/bf_types.h>

/**
 * @addtogroup tm-egress
 * @{
 */

/**
 * @brief Set egress pipe limit.
 * Default value of the pipe limit is set to maximum buffering capability
 * of the traffic manager. When admitting packet into Traffic manager,
 * apart from other checks, the packet has to also pass usage check on
 * per egress pipe usage limit. A packet destined to egress pipe whose
 * usage limit has crossed, will not be admitted.
 *
 * Default: Set to maximum TM buffer.
 *
 * Related APIs: bf_tm_pipe_egress_hysteresis_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[in] cells      Limit in terms of number of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pipe_egress_limit_set(bf_dev_id_t dev,
                                        bf_dev_pipe_t pipe,
                                        uint32_t cells);

/**
 * @brief Set Egress Pipe Queue States Reporting Mode
 * False: Trigger Qstat reporting from Q color threshold
 * True: Trigger Qstat reporting from Q any updates
 *
 * Default: 0
 *
 * Related APIs: bf_tm_pipe_egress_hysteresis_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[in] mode       Queue states reporting mode.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_qstat_report_mode_set(bf_dev_id_t dev,
                                        bf_dev_pipe_t pipe,
                                        bool mode);

/**
 * @brief Get Egress Pipe Queue States Reporting Mode
 * False: Trigger Qstat reporting from Q color threshold
 * True: Trigger Qstat reporting from Q any updates
 *
 * Default: 0
 *
 * Related APIs: bf_tm_pipe_egress_hysteresis_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[out] mode      Queue states reporting mode.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_qstat_report_mode_get(bf_dev_id_t dev,
                                        bf_dev_pipe_t pipe,
                                        bool *mode);

/**
 * @brief Set pipe hysteresis limit.
 * When usage of cells goes below the hysteresis limit, pipe level
 * drop condition will be cleared.
 *
 * Default : Hysteresis is set to zero or no hysteresis.
 *
 * Related APIs: bf_tm_pipe_egress_limit_set(),
 *               bf_tm_pipe_egress_hysteresis_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[in] cells      New threshold limit
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pipe_egress_hysteresis_set(bf_dev_id_t dev,
                                             bf_dev_pipe_t pipe,
                                             uint32_t cells);

/* @} */

#endif
