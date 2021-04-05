/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TRAFFIC_MGR_Q_INTF_H__
#define __TRAFFIC_MGR_Q_INTF_H__

/**
 * @file traffic_mgr_q_intf.h
 * @brief This file contains APIs for Traffic Manager application to
 *        program Queues meant to implement part of QoS behaviour based on
 *        traffic properties. Traffic with similar characteristics destined
 *        to egress ports can be queued together.
 *        APIs in this file facilitate queue QoS management.
 */

#include <traffic_mgr/traffic_mgr_types.h>
#include <bf_types/bf_types.h>

/**
 * @addtogroup tm-egress
 * @{
 *  Description of APIs for Traffic Manager application to manage
 *  egress buffer and queues meant to implement part of QoS behaviour
 *  based on traffic properties.
 */

/**
 * @brief Queue Mapping Array
 * q_mapping is array of 32/128 (TOF1/TOF2) integers specifying queue numbering
 * for finer queue allocation to ports. Return type should be checked.
 * Depending upon hardware resources available at the time of API invocation,
 * special queue carving may or may not be possible to accomodate.
 *
 * Related APIs :
 *    dvm_add_port_with_queues()
 *    bf_tm_allocate_deafult_queues()
 *    bf_tm_allocate_queues()
 *
 */

/**
 * @brief Allocates Queues and Sets Queue Mapping per Port
 * This API can be used to allocate flexible number of queues behind a port.
 * By default straight mapping is established. If flexible queue mapping is
 * desired, this API can be used.
 * Default mapping or when q_mapping is NULL, queue mapping is according to
 * following calculation.
 *    queue# = Ig_intr_md.qid % q_count.
 *    Example Queue mapping in default mode:
 *                   Ig_intr_md.qid 0 --> port qid 0
 *                   Ig_intr_md.qid 1 --> port qid 1
 *                         :
 *                   Ig_intr_md.qid 7 --> port qid 7
 *                         :
 *
 * NOTE: If this API gets called for changing the queue carving (queue count),
 * then application should
 *   - always call queue mapping APIs strictly in increasing order of ports
 *      within a port group
 *   - if queue count gets changed for a port/channel in a port group,
 *      then application should call the queue mapping APIs
 *      for rest of the ports after it (if present) in that port group in
 *      increasing order
 *   - if queue count gets changed, then calling this API while traffic
 *      running on any port within the port group is not recommended as
 *      traffic would be disrupted for a short period of time and buffer
 *      accounting would be inconsistent
 * The above restrictions are not applicable if just queue mapping gets changed
 *
 *
 * Related APIs: bf_tm_port_q_mapping_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] q_count    Number of queues being mapped.
 * @param[in] q_mapping  Array of integer values specifying queue mapping
 *                       Mapping is indexed by ig_intr_md.qid.
 *                       Value q_mapping[ig_intr_md.qid] is port's QID
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_q_mapping_set(bf_dev_id_t dev,
                                     bf_dev_port_t port,
                                     uint8_t q_count,
                                     uint8_t *q_mapping);

/**
 * @brief bf_tm_port_q_mapping_speed_set
 * This API can be used to allocate flexible number of queues behind a port.
 * By default straight mapping is established. If flexible queue mapping is
 * desired, this API can be used.
 * Default mapping or when q_mapping is NULL, queue mapping is according to
 * following calculation.
 *    queue# = Ig_intr_md.qid % q_count.
 *    Example Queue mapping in default mode:
 *                   Ig_intr_md.qid 0 --> port qid 0
 *                   Ig_intr_md.qid 1 --> port qid 1
 *                         :
 *                   Ig_intr_md.qid 7 --> port qid 7
 *                         :
 *
 * NOTE: If this API gets called for changing the queue carving (queue count),
 * then application should
 *   - always call queue mapping APIs strictly in increasing order of ports
 *      within a port group
 *   - if queue count gets changed for a port/channel in a port group,
 *      then application should call the queue mapping APIs
 *      for rest of the ports after it (if present) in that port group in
 *      increasing order
 *   - if queue count gets changed, then calling this API while traffic
 *      running on any port within the port group is not recommended as
 *      traffic would be disrupted for a short period of time and buffer
 *      accounting would be inconsistent
 * The above restrictions are not applicable if just queue mapping gets changed
 *
 * Queue speeds must be a multiple of 50G in the range 50G - 400G. Resources
 * allow for up to 6400G of queue speed to be allocated across a port group.
 *
 * Related APIs: bf_tm_port_q_mapping_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] q_count    Number of queues being mapped.
 * @param[in] q_speeds   Array of integer values specifying queue speeds
 * @param[in] q_mapping  Array of integer values specifying queue mapping
 *                       Mapping is indexed by ig_intr_md.qid.
 *                       Value q_mapping[ig_intr_md.qid] is port's QID
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_q_mapping_speed_set(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           uint8_t q_count,
                                           uint16_t *q_speeds,
                                           uint8_t *q_mapping);

/**
 * @brief Configues queue with pool and limit properties
 * A queue can be optionally assigned to any application pool.
 * When assigned to application pool, static or dynamic shared limit
 * can be set. This API aids to achieve that. If it is desired to not
 * assign queue to any pool, then this API need not be invoked.
 * Advantage of belonging to a pool of queues is that queue can share
 * resources. Hence can grow to dynamic limit depending on burst absorption
 * factor and pool usage number. Dynamic limit will be higher than its own
 * queue limit. If static limit is used, then  queue can usage upto its
 * own limit before tail drop condition occurs.
 *
 * Default: Queues are not assigned to any application pool.
 *
 * Related APIs: bf_tm_q_app_pool_usage_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port handle.
 * @param[in] queue           Queue identifier. Valid range [ 0..31 ]
 * @param[in] pool            Application pool to which queue is assigned to.
 *                            Valid values are BF_TM_EG_POOL0..3.
 * @param[in] base_use_limit  Limit to which PPG can grow inside application
 *                            pool. Limit is specified in terms of cells.
 *                            Once this limit is crossed, if queue burst
 *                            absroption factor (BAF) is non zero, depending
 *                            availability of buffer, queue is allowed to
 *                            use buffer upto BAF limit. If BAF limit is zero,
 *                            queue is treated as static and no dynamic
 *                            buffering is possible.
 * @param[in] dynamic_baf     One of the values listed in bf_tm_queue_baf_t.
 *                            When BF_TM_QUEUE_BAF_DISABLE is used, queue uses
 *                            static limit.
 * @param[in] hysteresis      Hysteresis value of queue in cells .
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

bf_status_t bf_tm_q_app_pool_usage_set(bf_dev_id_t dev,
                                       bf_dev_port_t port,
                                       bf_tm_queue_t queue,
                                       bf_tm_app_pool_t pool,
                                       uint32_t base_use_limit,
                                       bf_tm_queue_baf_t dynamic_baf,
                                       uint32_t hysteresis);

/**
 * @brief Disable Queue Pool Usage
 * This API can be used to disable queue participation in application
 * pool. In such case, once q's gmin limits are used, queue tail drop
 * can occur.
 *
 * Related APIs: bf_tm_q_app_pool_usage_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue identifier
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

bf_status_t bf_tm_q_app_pool_usage_disable(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           bf_tm_queue_t queue);

/**
 * @brief Set queue min limits.
 * Inorder to increase min limits, MIN pool should have that many free cells
 * unused. If not available, limits cannot be increased and API fails.
 * When queue limits are decreased, in the absence of shared/application pools,
 * decreased number of cells will be unused and earmarked for future use.
 * These unused cells can be used in future to increase queue limits.
 * If application pools are carved out, then the decreased number of cells are
 * equally distributed to all application pools by raising their pool limits.
 *
 * Default: TM buffer is equally distribted to all queues assuming all queues
 *          are active.
 *
 * Related APIs: bf_tm_q_guaranteed_min_limit_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue identifier. Valid range [ 0..31 ]
 * @param[in] cells      Queue limits specified in cell count
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_guaranteed_min_limit_set(bf_dev_id_t dev,
                                             bf_dev_port_t port,
                                             bf_tm_queue_t queue,
                                             uint32_t cells);

/**
 * @brief  Set color drop limits for queue.
 * Color drop limits for red should be less than color drop limits of yellow,
 * which inturn is less than color drop limits of green.
 *
 * Default: Color drop limits for yellow and red are set to 75% of gmin
 *          size of queue.
 *
 * Related APIs: bf_tm_q_color_drop_enable(), bf_tm_q_color_limit_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop limit to be set.
 * @param[in] color      Color (RED, YELLOW)
 * @param[in] limit      Color Limit is specified in percentage of
 *                       guaranteed queue limit.
 *                       Green Color limit is equal to queue limit.
 *                       For yellow, red, limit can be set as percentage of
 *                       overall queue limit. Once queue usage reaches the
 *                       limit, appropriate colored packets are dropped.
 *                       To set GREEN Color limit use
 *                       bf_tm_q_guaranteed_min_limit_set()
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_color_limit_set(bf_dev_id_t dev,
                                    bf_dev_port_t port,
                                    bf_tm_queue_t queue,
                                    bf_tm_color_t color,
                                    bf_tm_queue_color_limit_t limit);

/**
 * @brief Set queue's color drop hysteresis. When queue's usage for a color
 * falls below by hysteresis value, tail drop condition is cleared.
 *
 * Default: Color hysteresis limits are set to  zero.
 *
 * Related APIs: bf_tm_q_color_drop_enable(), bf_tm_q_color_limit_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop limit to be set.
 * @param[in] color      Color (RED, YELLOW)
 * @param[in] limit      Number of cells queue usage should reduce before
 *                       drop condition for appropriate colored packets is
 *                       cleared.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_color_hysteresis_set(bf_dev_id_t dev,
                                         bf_dev_port_t port,
                                         bf_tm_queue_t queue,
                                         bf_tm_color_t color,
                                         bf_tm_queue_color_limit_t limit);

/**
 * @brief Enable queue tail drop condition. When queue
 * threshold limits (guranteed min limit + shared limit)
 * are reached, packets are dropped.
 *
 * Default : Trigger drops when queue threshold limits are reached.
 *
 * Related APIs: bf_tm_q_tail_drop_disable()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for which tail drop has to be enabled.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_tail_drop_enable(bf_dev_id_t dev,
                                     bf_dev_port_t port,
                                     bf_tm_queue_t queue);

/**
 * @brief Disable queue tail drop condition.
 * When queue threshold limits (guranteed min limit + shared limit)
 * are reached, packets are not dropped in Egress. This
 * will lead to Ingress drops eventually.
 *
 * Default : Trigger drops when queue threshold limits are reached.
 *
 * Related APIs: bf_tm_q_tail_drop_enable()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for which tail drop has to be disabled.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_tail_drop_disable(bf_dev_id_t dev,
                                      bf_dev_port_t port,
                                      bf_tm_queue_t queue);

/**
 * @brief Enable queue color drop condition.
 * Based on packet color, when queue color threshold limit are reached,
 * packets are dropped. When color drop is not enabled, packets do not
 * get any treatment based on their color.
 *
 * Default : Trigger drops based on color.
 *
 * Related APIs: bf_tm_q_color_drop_disable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop is to set.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_color_drop_enable(bf_dev_id_t dev,
                                      bf_dev_port_t port,
                                      bf_tm_queue_t queue);

/**
 * @brief Disable queue color drop condition.
 * Based on packet color, when queue color threshold limit are reached,
 * packets are dropped. When color drop is not enabled, packets do not
 * get any treatment based on their color.
 *
 * Related APIs: bf_tm_q_color_drop_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop is to set.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_color_drop_disable(bf_dev_id_t dev,
                                       bf_dev_port_t port,
                                       bf_tm_queue_t queue);

/**
 * @brief Set Queue visible condition.
 * Visible queues will be reported to ingress
 * MAU for Queue length cross any color threshold. (TM Visibility feature)
 *
 * Default : False
 *
 * Related APIs: bf_tm_q_visible_get()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for port.
 * param[in] visible    Desired visiblity state.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_visible_set(bf_dev_id_t dev,
                                bf_dev_port_t port,
                                bf_tm_queue_t queue,
                                bool visible);

/**
 * @brief Get queue visible condition.
 * Visible queues will be reported to ingress
 * MAU for Queue length cross any color threshold. (TM Visibility feature)
 *
 * Default : False
 *
 * Related APIs: bf_tm_q_visible_set()
 *
 * param[in] dev            ASIC device identifier.
 * param[in] port           Port handle.
 * param[in] queue          Queue for port.
 * param[out] visible_sw    Visible Status in Driver SW.
 * param[out] visible_hw    Visible Status in HW.
 * return                   Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_visible_get(bf_dev_id_t dev,
                                bf_dev_port_t port,
                                bf_tm_queue_t queue,
                                bool *visible_sw,
                                bool *visible_hw);

/**
 * @brief bf_tm_q_visible_get_default
 * Default visible condition. Visible queues will be reported to ingress
 * MAU for Queue length cross any color threshold. (TM Visibility feature)
 *
 * Related APIs: bf_tm_q_visible_get()
 *
 * param[in] dev       ASIC device identifier.
 * param[out] visible  Default visiblity state.
 * return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_visible_get_default(bf_dev_id_t dev, bool *visible);

/**
 * @brief Set queue hysteresis value.
 *
 * Related APIs: bf_tm_q_app_pool_usage_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose hysteresis is to set.
 * @param[in] cells      Hysteresis value of queue in cells .
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_hysteresis_set(bf_dev_id_t dev,
                                   bf_dev_port_t port,
                                   bf_tm_queue_t queue,
                                   uint32_t cells);

/**
 * @brief Set dest port() queue) for negative mirror traffic
 * Use this API to set (port, queue) used for egressing out
 * negative mirror traffic. Its possible to set one such
 * (port,queue) value for each pipe.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[in] port       Negative Mirror port.
 * @param[in] queue      Queue where negative mirror traffic is enqueued.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_mirror_on_drop_dest_set(bf_dev_id_t dev,
                                               bf_dev_pipe_t pipe,
                                               bf_dev_port_t port,
                                               bf_tm_queue_t queue);

/* @} */

/**
 * @addtogroup tm-flowcontrol
 * @{
 */
/**
 * @brief Set Queue PFC Cos Mapping
 * When egress queues need to honour received PFC from downstream,
 * by mapping cos to queue using the API below, queues
 * will not participate in scheduling until PFC gets cleared.
 *
 * Default: All queues are mapping CoS zero.
 *
 * Related APIs: bf_tm_ppg_icos_mapping_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @param[in] queue       Queue whose color drop is to set.
 * @param[in] cos         CoS associated with the queue.
 * @return            Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_q_pfc_cos_mapping_set(bf_dev_id_t dev,
                                        bf_dev_port_t port,
                                        bf_tm_queue_t queue,
                                        uint8_t cos);

/* @} */

#endif
