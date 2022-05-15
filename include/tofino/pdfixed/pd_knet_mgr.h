/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_PDFIXED_PD_KNET_MGR_H
#define _TOFINO_PDFIXED_PD_KNET_MGR_H

#include <tofino/pdfixed/pd_common.h>
#include <knet_mgr/bf_knet_if.h>

typedef bf_knet_cpuif_t p4_pd_knet_cpuif_t;
typedef bf_knet_hostif_t p4_pd_knet_hostif_t;
typedef bf_knet_filter_t p4_pd_knet_filter_t;
typedef bf_knet_priority_t p4_pd_priority_t;
typedef bf_knet_action_count_t p4_pd_action_count_t;
typedef bf_knet_data_offset_t p4_pd_data_offset_t;
typedef bf_knet_data_size_t p4_pd_data_size_t;
typedef bf_knet_rx_filter_t p4_pd_knet_rx_filter_t;
typedef bf_knet_tx_action_t p4_pd_knet_tx_action_t;
typedef bf_knet_packet_mutation_t p4_pd_knet_packet_mutation_t;
typedef bf_knet_count_t p4_pd_knet_count_t;
typedef bf_knet_cpuif_list_t p4_pd_knet_cpuif_list_t;
typedef bf_knet_hostif_list_t p4_pd_knet_hostif_list_t;

p4_pd_status_t p4_knet_cpuif_ndev_add(const char *cpuif_netdev_name,
                                      char *cpuif_knetdev_name,
                                      p4_pd_knet_cpuif_t *knet_cpuif_id);

p4_pd_status_t p4_knet_cpuif_ndev_delete(
    const p4_pd_knet_cpuif_t knet_cpuif_id);

p4_pd_status_t p4_knet_hostif_kndev_add(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                        const char *hostif_name,
                                        p4_pd_knet_hostif_t *knet_hostif_id);

p4_pd_status_t p4_knet_hostif_kndev_delete(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    const p4_pd_knet_hostif_t knet_hostif_id);

p4_pd_status_t p4_knet_rx_filter_add(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    p4_pd_knet_rx_filter_t *const knet_rx_filter);

p4_pd_status_t p4_knet_rx_filter_delete(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                        const p4_pd_knet_filter_t filter_id);

p4_pd_status_t p4_knet_rx_filter_get(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                     const p4_pd_knet_filter_t filter_id,
                                     p4_pd_knet_rx_filter_t *rx_filter,
                                     p4_pd_knet_count_t rx_mutation_count);

p4_pd_status_t p4_knet_tx_action_add(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                     const p4_pd_knet_hostif_t knet_hostif_id,
                                     p4_pd_knet_tx_action_t *tx_action);

p4_pd_status_t p4_knet_tx_action_delete(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    const p4_pd_knet_hostif_t knet_hostif_id);

bool p4_knet_module_is_inited();

p4_pd_status_t p4_knet_get_cpuif_cnt(p4_pd_knet_count_t *const obj_count);

p4_pd_status_t p4_knet_get_hostif_cnt(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                      p4_pd_knet_count_t *const obj_count);

p4_pd_status_t p4_knet_get_rx_filter_cnt(const p4_pd_knet_cpuif_t knet_cpuif_id,
                                         p4_pd_knet_count_t *const obj_count);

p4_pd_status_t p4_knet_get_rx_mutation_cnt(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    const p4_pd_knet_filter_t filter_id,
    p4_pd_knet_count_t *const obj_count);

p4_pd_status_t p4_knet_get_tx_mutation_cnt(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    const p4_pd_knet_hostif_t hostif_id,
    p4_pd_knet_count_t *const obj_count);

p4_pd_status_t p4_knet_get_rx_filter_list(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    p4_pd_knet_filter_t *const filter_list,
    p4_pd_knet_count_t *const filter_count);

p4_pd_status_t p4_knet_get_cpuif_list(p4_pd_knet_cpuif_list_t *const cpuif_list,
                                      p4_pd_knet_count_t *const cpuif_count);

p4_pd_status_t p4_knet_get_hostif_list(
    p4_pd_knet_cpuif_t knet_cpuif_id,
    p4_pd_knet_hostif_list_t *const hostif_list,
    p4_pd_knet_count_t *const hostif_count);

p4_pd_status_t p4_knet_tx_action_get(
    const p4_pd_knet_cpuif_t knet_cpuif_id,
    const p4_pd_knet_hostif_t knet_hostif_id,
    p4_pd_knet_tx_action_t *const tx_action,
    const p4_pd_knet_count_t tx_mutation_count);

#endif
