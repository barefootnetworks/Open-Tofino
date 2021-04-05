/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _BF_KNET_IF_H_
#define _BF_KNET_IF_H_

/**
 * @file pd_ms.h
 *
 * @brief PD Fixed APIs for KNET module management.
 *
 */

#include <stdint.h>
#include <netinet/in.h>
#include <knet_mgr/bf_knet_ioctl.h>
#include <bf_types/bf_types.h>

/**
 * @brief Add the KNET CPU interface
 *
 * @param[in] cpuif_netdev_name CPU interface name
 * @param[out] cpuif_knetdev_name Pointer to return the KNET
 * CPU interface name
 * @param[out] knet_cpuif_id Pointer to return the assigned KNET
 * CPU interface id
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_cpuif_ndev_add(const char *cpuif_netdev_name,
                                   char *cpuif_knetdev_name,
                                   bf_knet_cpuif_t *knet_cpuif_id);

/**
 * @brief Delete the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_cpuif_ndev_delete(const bf_knet_cpuif_t knet_cpuif_id);

/**
 * @brief Add the KNET host interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[out] hostif_knetdev Pointer to return the KNET host interface added
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_hostif_kndev_add(const bf_knet_cpuif_t knet_cpuif_id,
                                     bf_knet_hostif_knetdev_t *hostif_knetdev);

/**
 * @brief Delete the KNET host interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] knet_hostif_id KNET host interface id
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_hostif_kndev_delete(const bf_knet_cpuif_t knet_cpuif_id,
                                        const bf_knet_hostif_t knet_hostif_id);

/**
 * @brief Add the RX filter to KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] rx_filter Pointer to the RX filter to be added
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_rx_filter_add(const bf_knet_cpuif_t knet_cpuif_id,
                                  bf_knet_rx_filter_t *rx_filter);

/**
 * @brief Delete the RX filter from KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] filter_id RX filter id to be deleted
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_rx_filter_delete(const bf_knet_cpuif_t knet_cpuif_id,
                                     const bf_knet_filter_t filter_id);

/**
 * @brief Get the RX filter for KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] filter_id RX filter id
 * @param[out] rx_filter Pointer to return the RX filter
 * @param[in] rx_mutation_count RX filter mutation count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_rx_filter_get(const bf_knet_cpuif_t knet_cpuif_id,
                                  const bf_knet_filter_t filter_id,
                                  bf_knet_rx_filter_t *rx_filter,
                                  bf_knet_count_t rx_mutation_count);

/**
 * @brief Add TX action for KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] knet_hostif_id KNET host interface id
 * @param[in] tx_action Pointer to TX action to be added
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_tx_action_add(const bf_knet_cpuif_t knet_cpuif_id,
                                  const bf_knet_hostif_t knet_hostif_id,
                                  bf_knet_tx_action_t *const tx_action);

/**
 * @brief Delete the TX action for KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] knet_hostif_id KNET host interface id
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_tx_action_delete(const bf_knet_cpuif_t knet_cpuif_id,
                                     const bf_knet_hostif_t knet_hostif_id);

/**
 * @brief Get the KNET CPU interface count
 *
 * @param[in] obj_count Pointer to return the count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_get_cpuif_cnt(bf_knet_count_t *const obj_count);

/**
 * @brief Get the KNET host interface count for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[out] obj_count Pointer to return the count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_get_hostif_cnt(const bf_knet_cpuif_t knet_cpuif_id,
                                   bf_knet_count_t *const obj_count);

/**
 * @brief Get the RX filter count for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[out] obj_count Pointer to return the count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_get_rx_filter_cnt(const bf_knet_cpuif_t knet_cpuif_id,
                                      bf_knet_count_t *const obj_count);

/**
 * @brief Get the RX mutation count for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] filter_id KNET RX filter id
 * @param[out] obj_count Pointer to return the count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_get_rx_mutation_cnt(const bf_knet_cpuif_t knet_cpuif_id,
                                        const bf_knet_filter_t filter_id,
                                        bf_knet_count_t *const obj_count);

/**
 * @brief Get the TX mutation count for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] hostif_id KNET host interface id
 * @param[out] obj_count Pointer to return the count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_get_tx_mutation_cnt(const bf_knet_cpuif_t knet_cpuif_id,
                                        const bf_knet_hostif_t hostif_id,
                                        bf_knet_count_t *const obj_count);

/**
 * @brief Get the RX filter list for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[out] filter_list Pointer to return the filter list
 * @param[out] filter_count Pointer to return the filter count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_rx_filter_list_get(const bf_knet_cpuif_t knet_cpuif_id,
                                       bf_knet_filter_t *const filter_list,
                                       bf_knet_count_t *const filter_count);

/**
 * @brief Get the KNET CPU interface list
 *
 * @param[out] cpuif_list Pointer to return the CPU interface list
 * @param[out] cpuif_count Pointer to return the CPU interface count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_cpuif_list_get(bf_knet_cpuif_list_t *const cpuif_list,
                                   bf_knet_count_t *const cpuif_count);

/**
 * @brief Get the host interface list for the KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[out] hostif_list Pointer to return the host interface list
 * @param[out] hostif_count Pointer to return the host interface count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_hostif_list_get(bf_knet_cpuif_t knet_cpuif_id,
                                    bf_knet_hostif_list_t *const hostif_list,
                                    bf_knet_count_t *const hostif_count);

/**
 * @brief Get the TX action for KNET CPU interface
 *
 * @param[in] knet_cpuif_id KNET CPU interface id
 * @param[in] knet_hostif_id KNET host interface id
 * @param[out] rx_filter Pointer to return the TX action
 * @param[in] tx_mutation_count TX mutation count
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_knet_tx_action_get(const bf_knet_cpuif_t knet_cpuif_id,
                                  const bf_knet_hostif_t knet_hostif_id,
                                  bf_knet_tx_action_t *tx_action,
                                  bf_knet_count_t tx_mutation_count);

/**
 * @brief Check if the KNET module is initialized or not
 *
 * @return true: if the KNET module is initialized, false: otherwise
 *
 */
bool bf_knet_module_is_inited();
#endif
