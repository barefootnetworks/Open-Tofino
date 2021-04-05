/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_PDFIXED_PD_TS_H
#define _TOFINO_PDFIXED_PD_TS_H

/**
 * @file pd_ts.h
 *
 * @brief PD fixed timestamp APIs.
 *
 */

#include <tofino/pdfixed/pd_common.h>

typedef int32_t p4_pd_ts_dev_t;
typedef int32_t p4_pd_ts_port_t;

/**
 * @brief Enable/disable global timestamp function
 *
 * @param[in] dev_id Device identifier
 * @param[in] enable Flag to enable or disable timestamp function
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_state_set(p4_pd_ts_dev_t dev, bool enable);

/**
 * @brief Get the state of global timestamp function
 *
 * @param[in] dev_id Device identifier
 * @param[out] enable Pointer to return the state of the timestamp function
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_state_get(p4_pd_ts_dev_t dev, bool *enable);

/**
 * @brief Configure the global timestamp counter
 *
 * @param[in] dev_id Device identifier
 * @param[in] global_ts Global timestamp value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_value_set(p4_pd_ts_dev_t dev,
                                            uint64_t global_ts);

/**
 * @brief Get the global timestamp counter
 *
 * @param[in] dev_id Device identifier
 * @param[out] global_ts Pointer to return the global timestamp value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_value_get(p4_pd_ts_dev_t dev,
                                            uint64_t *global_ts);

/**
 * @brief Configure the global timestamp counter inc value
 *
 * @param[in] dev_id Device identifier
 * @param[in] global_inc_ns Global timestamp counter inc value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_inc_value_set(p4_pd_ts_dev_t dev,
                                                uint32_t global_inc_ns);

/**
 * @brief Get the global timestamp counter inc value
 *
 * @param[in] dev_id Device identifier
 * @param[out] global_inc_ns Pointer to return the global timestamp counter
 * inc value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_inc_value_get(p4_pd_ts_dev_t dev,
                                                uint32_t *global_inc_ns);

/**
 * @brief Set global timestamp offset value
 *
 * @param[in] dev_id Device identifier
 * @param[in] global_ts Global timestamp offset value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_offset_value_set(p4_pd_ts_dev_t dev,
                                                   uint64_t global_ts);

/**
 * @brief Get global timestamp offset value
 *
 * @param[in] dev_id Device identifier
 * @param[in] global_ts Pointer to return global timestamp offset value in ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_ts_offset_value_get(p4_pd_ts_dev_t dev,
                                                   uint64_t *global_ts);

/**
 * @brief Get global timestamp offset value and baresync timestamp value
 *
 * @param[in] dev_id Device identifier
 * @param[in] global_ts Pointer to return global timestamp offset value in ns
 * @param[in] baresync_ts Pointer to return baresync timestamp offset value in
 * ns
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_global_baresync_ts_get(p4_pd_ts_dev_t dev,
                                               uint64_t *global_ts,
                                               uint64_t *baresync_ts);

/**
 * @brief Set 1588 Tx delta on a Tofino port
 *
 * @param[in] dev Device identifier
 * @param[in] dev_port Port identifier
 * @param[in] delta Time delta to set (0-65535)
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_1588_timestamp_delta_tx_set(p4_pd_ts_dev_t dev,
                                                    p4_pd_ts_port_t port,
                                                    uint16_t delta);

/**
 * @brief Get 1588 Tx delta on a Tofino port
 *
 * @param[in] dev Device identifier
 * @param[in] dev_port Port identifier
 * @param[out] delta Pointer to return the time delta to set (0-65535)
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_1588_timestamp_delta_tx_get(p4_pd_ts_dev_t dev,
                                                    p4_pd_ts_port_t port,
                                                    uint16_t *delta);

/**
 * @brief Set 1588 Rx delta on a Tofino port
 *
 * @param[in] dev Device identifier
 * @param[in] dev_port Port identifier
 * @param[in] delta Time delta to set (0-65535)
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_1588_timestamp_delta_rx_set(p4_pd_ts_dev_t dev,
                                                    p4_pd_ts_port_t port,
                                                    uint16_t delta);

/**
 * @brief Get 1588 Rx delta on a Tofino port
 *
 * @param[in] dev Device identifier
 * @param[in] dev_port Port identifier
 * @param[out] delta Pointer to return the time delta to set (0-65535)
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_1588_timestamp_delta_rx_get(p4_pd_ts_dev_t dev,
                                                    p4_pd_ts_port_t port,
                                                    uint16_t *delta);

/**
 * @brief Get 1588 timestamp value on a Tofino port
 *
 * @param[in] dev Device identifier
 * @param[in] dev_port Port identifier
 * @param[out] ts Pointer to return the timestamp value
 * @param[out] ts_valid Pointer to return the timestamp valid
 * @param[out] ts_id Pointer to return the timestamp id
 *
 * @return Status of the API call
 */
p4_pd_status_t p4_pd_ts_1588_timestamp_tx_get(p4_pd_ts_dev_t dev,
                                              p4_pd_ts_port_t port,
                                              uint64_t *ts,
                                              bool *ts_valid,
                                              int *ts_id);

#endif
