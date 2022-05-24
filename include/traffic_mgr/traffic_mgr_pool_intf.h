/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TRAFFIC_MGR_POOLS_H__
#define __TRAFFIC_MGR_POOLS_H__

/**
 * @file traffic_mgr_pool_intf.h
 * @brief This file contains APIs for Traffic Manager application to
 *        program Pools meant to implement part of QoS behaviour based on
 *        traffic properties. Traffic with similar characteristics sourced
 *        from multiple ports can be grouped into an ingress pool. QoS
 *        parameters applied on pool will inturn apply to traffic flows mapped
 *        to the pool.
 *        Similarly traffic destined to various queues behind various
 *        ports having similar QoS requirements can be group into a egress pool.
 */

#include <traffic_mgr/traffic_mgr_types.h>
#include <bf_types/bf_types.h>

/**
 * @addtogroup tm-buffer-mgmt
 * @{
 *  Description of APIs for Traffic Manager application to manage buffer
 *  for pools from both ingress and egress ends.  Traffic with similar
 *  characteristics sourced from multiple ports can be grouped into a
 *  pool. QoS parameters applied on pool will inturn apply to traffic
 *  flows mapped to the pool. Similarly traffic destined to various
 *  queues behind various ports having similar QoS requirements can be
 *  group into a egress pool.
 */

/**
 * @brief Set application pool size.
 * When application pool size is set, the required number of cells are
 * obtained from unassigned portion of TM buffer cells.
 * Unassigned cells = (Total buffer cells - gmin size of every PPG or
 * Queue - sizes of all applicaiton pools).
 * When required number of cells are not available, the API fails.
 * PPGs or Queues can be mapped to this new application pool.
 *
 * Default:   By default application pool size is set to zero.
 *
 * Related APIs: bf_tm_pool_skid_size_set(), bf_tm_pool_size_get()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pool        Pool identifier. Any of BF_TM_IG_APP_POOL_0..3
 *                        or BF_TM_EG_APP_POOL_0..3 is valid application pool.
 * @param[in] cells       Size of pool in terms of cells.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_size_set(bf_dev_id_t dev,
                                bf_tm_app_pool_t pool,
                                uint32_t cells);

/**
 * @brief Enable Application pool's color drop condition.
 * If color drop condition is enabled, when pool color
 * threshold limit are reached packets are dropped.
 * When color drop is not enabled, packets do not get any
 * treatment based on their color at pool level.
 *
 * Default : Default policy is to trigger drops based on color.
 *
 * Related APIs: bf_tm_pool_color_drop_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_color_drop_enable(bf_dev_id_t dev,
                                         bf_tm_app_pool_t pool);

/**
 * @brief Disable Application pool's color drop condition.
 * By disabling color drop condition, packets do not get
 * any treatment based on their color at pool level.
 *
 * Default : Default policy is to trigger drops based on color.
 *
 * Related APIs:  bf_tm_pool_color_drop_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_color_drop_disable(bf_dev_id_t dev,
                                          bf_tm_app_pool_t pool);

/**
 * @brief Set Application pool color drop limit.
 *
 * Default : By default, color drop limits are set to 100% of pool size
 *           for all colors.
 *
 * Related APIs: bf_tm_pool_color_drop_enable(),
 *               bf_tm_pool_color_drop_limit_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool handle.
 * @param[in] color           Color (Green, Yellow, Red)
 * @param[in] limit           Limits in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_color_drop_limit_set(bf_dev_id_t dev,
                                            bf_tm_app_pool_t pool,
                                            bf_tm_color_t color,
                                            uint32_t limit);

/**
 * @brief Set color drop hysteresis limits.
 * The same hysteresis value is applied on all application pools.
 * Resume condition is triggered when pool usage drops by hysteresis
 * value from the limit value when color drop condition was set.
 *
 * Default : By deafult hysteresis value is set to zero; or no hysterisis
 *
 * Related APIs:  bf_tm_pool_color_drop_limit_set()
 *                bf_tm_pool_color_drop_hysteresis_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] color           Color (Green, Yellow, Red)
 * @param[in] limit           Limit in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_color_drop_hysteresis_set(bf_dev_id_t dev,
                                                 bf_tm_color_t color,
                                                 uint32_t limit);

/**
 * @brief Set per PFC level limit values.
 * PFC level limits are configurable on per applicaiton pool basis.
 * When PPG usage numbers hit pfc limits, PFC is triggered
 * for lossless traffic.
 *
 * Related APIs: bf_tm_pool_pfc_limit_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool handle for which limits are
 *                            configured.
 * @param[in] icos            Internal CoS (iCoS = ig_intr_md.ingress_cos) level
 *                            on which limits are applied.
 * @param[in] limit           Limit value in terms of cell count to increase.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_pfc_limit_set(bf_dev_id_t dev,
                                     bf_tm_app_pool_t pool,
                                     bf_tm_icos_t icos,
                                     uint32_t limit);

/**
 * @brief Set skid pool size.
 * Skid pool size can be oversubscribed. It need not be sum of per
 * ppg skid limits that are mapped to skid pool.
 * The reason being it is not practical scenario. Bursty behaviour could
 * be one or few PPGs at a time.
 *
 * Default:  At the start, default skid pool size is set zero.
 *
 * Related APIs: bf_tm_pool_size_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_skid_size_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief Set global skid pool hysteresis.
 *
 * Default : By deafult hysteresis value is set to zero; or no hysterisis
 *
 * Related APIs: bf_tm_pool_skid_size_set(), bf_tm_pool_skid_hysteresis_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      Number of cells set as skid pool hysteresis. If not a
 *                       multiple of 8, gets rounded down.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_skid_hysteresis_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief bf_tm_pre_fifo_limit_set
 * Set pre fifo limit in units of cells.
 *
 * Default : 0xc0
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      New size.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pre_fifo_limit_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief bf_tm_global_min_limit_set
 * Set Ingress total reserved buffer space in units of cells, at least larger
 * than total sum of 'ppg_min
 * Tofino2 and above support Only
 *
 * Default : 0x6000
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      New size.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_global_min_limit_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief Set negative mirror pool limit in units of cells.
 *
 * Default : Negative mirror pool size is zero.
 *
 * Related APIs: bf_tm_pool_mirror_on_drop_size_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      New size.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_mirror_on_drop_size_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief Set cut through size for unicast traffic.
 * This size determines total buffer set aside for cut through traffic.
 *
 * Default: All ports are in unicast cut through mode. The size is set
 *          accomodate worst case TM latency to transfer packet from ingress
 *          TM to egress TM.
 *
 * Related APIs:  bf_tm_port_uc_cut_through_limit_set(),
 *                bf_tm_pool_uc_cut_through_pool_size_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_uc_cut_through_size_set(bf_dev_id_t dev, uint32_t cells);

/**
 * @brief Set cut through size for Multicast traffic.
 * This size determines total buffer set aside for cut through traffic.
 *
 * Default: Using TM latency as metric and average replications
 *          default value will be set such that multicast traffic
 *          leverages cut through performance.
 *
 * Related APIs:  bf_tm_pool_mc_cut_through_size_get(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_pool_mc_cut_through_size_set(bf_dev_id_t dev, uint32_t cells);

/* @} */

#endif
