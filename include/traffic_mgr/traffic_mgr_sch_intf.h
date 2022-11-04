/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TRAFFIC_MGR_SCH_INTF_H__
#define __TRAFFIC_MGR_SCH_INTF_H__

/**
 * @file traffic_mgr_sched_intf.h
 * @brief This file contains APIs for Traffic Manager application to
 *        program scheduler paramters to implement part of QoS behaviour based
 *        on traffic properties.
 */

#include <bf_types/bf_types.h>
#include <traffic_mgr/traffic_mgr_types.h>

/**
 * @addtogroup tm-sched
 * @{
 *  Description of APIs for Traffic Manager application to
 *  program scheduler paramters to implement desired QoS behaviour based
 *  on traffic properties.
 */

/**
 * @brief Set queue scheduling priority.
 * Scheduling priority level used when serving guaranteed bandwidth.
 * Higher the number, higher the  priority to select the queue for
 * scheduling.
 *
 * Default: Queue scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_set ()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[in] priority              Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_priority_set(bf_dev_id_t dev,
                                       bf_dev_port_t port,
                                       bf_tm_queue_t queue,
                                       bf_tm_sched_prio_t priority);

/**
 * @brief Set queue DWRR weights.
 * These weights are used by queues at same priority level.
 * Across prioirty these weights serve as ratio to
 * share excess or remaining bandwidth.
 *
 * Default: Queue scheduling weights set to 1023
 *
 * Related APIs: bf_tm_sched_q_priority_set(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] weight          Weight value. Supported range [ 0.. 1023 ]
 *                            Weight 0 is used to disable the DWRR especially
 *                            when Max Rate Leakybucket is used.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_dwrr_weight_set(bf_dev_id_t dev,
                                          bf_dev_port_t port,
                                          bf_tm_queue_t queue,
                                          uint16_t weight);

/**
 * @brief Set queue shaping rate in units of kbps or pps.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_rate_set(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           bf_tm_queue_t queue,
                                           bool pps,
                                           uint32_t burst_size,
                                           uint32_t rate);

/**
 * @brief Set queue shaping rate in units of kbps or pps using provisioning
 *type.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @param[in] prov_type       Shaper provisioning type {UPPER, LOWER, MIN_ERR}
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_rate_set_provisioning(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool pps,
    uint32_t burst_size,
    uint32_t rate,
    bf_tm_sched_shaper_prov_type_t prov_type);

/**
 * @briefEnable token bucket that assures queue shaping rate (pps or bps)
 *
 * Default: Queue shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_q_max_shaping_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_max_shaping_rate_enable(bf_dev_id_t dev,
                                                  bf_dev_port_t port,
                                                  bf_tm_queue_t queue);

/**
 * @brief Disable token bucket that assures queue shaping rate (pps or bps)
 *
 * Default: Queue shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_q_max_shaping_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_max_shaping_rate_disable(bf_dev_id_t dev,
                                                   bf_dev_port_t port,
                                                   bf_tm_queue_t queue);

/**
 * @brief Set queue guaranteed rate in terms of pps or kbps.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_rate_set(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              bf_tm_queue_t queue,
                                              bool pps,
                                              uint32_t burst_size,
                                              uint32_t rate);

/**
 * @brief Set scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the queue for
 * scheduling.
 *
 * Default: Queue scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_get ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] priority        Scheduling priority of queue.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_remaining_bw_priority_set(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bf_tm_sched_prio_t priority);

/**
 * @brief Enable token bucket that assures queue guaranteed rate (pps or bps)
 *
 * Default: Queue guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_rate_enable(bf_dev_id_t dev,
                                                 bf_dev_port_t port,
                                                 bf_tm_queue_t queue);

/**
 * @brief Disable token bucket that assures queue guaranteed rate (pps or bps)
 *
 * Default: Queue guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_rate_disable(bf_dev_id_t dev,
                                                  bf_dev_port_t port,
                                                  bf_tm_queue_t queue);

/**
 * @brief Set port shaping rate in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_get()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port
 * @param[in] pps           If set to true, values are in terms of pps
 *                          and packets, else in terms of kbps and bytes.
 * @param[in] burst_size    Burst size in packets or bytes.
 * @param[in] rate          Shaper value in pps or kbps.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_rate_set(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              bool pps,
                                              uint32_t burst_size,
                                              uint32_t rate);

/**
 * @brief Set port shaping rate in units of kbps or pps using
 * provisioning type.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @param[in] prov_type       Shaper provisioning type {UPPER, LOWER, MIN_ERR}
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_rate_set_provisioning(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bool pps,
    uint32_t burst_size,
    uint32_t rate,
    bf_tm_sched_shaper_prov_type_t prov_type);

/**
 * @brief Set number of bytes added per packet to packet length by shaper
 *
 * Default: Zero bytes are added to packet to length.
 *
 * Related APIs: bf_tm_sched_pkt_ifg_compensation_get
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pipe            Pipe identifier.
 * @param[in] adjustment      Byte adjustment done on every packet.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_pkt_ifg_compensation_set(bf_dev_id_t dev,
                                                 bf_dev_pipe_t pipe,
                                                 uint8_t adjustment);

/**
 * @brief Enable queue scheduling.
 * If disabled, queue will not participate in scheduling.
 *
 * Default: By deafult, queue is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_disable_q
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_enable(bf_dev_id_t dev,
                                 bf_dev_port_t port,
                                 bf_tm_queue_t queue);

/**
 * @brief Disable queue scheduling.
 * If disabled, queue will not participate in scheduling.
 *
 * Default: By deafult, queue is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_enable_q
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_disable(bf_dev_id_t dev,
                                  bf_dev_port_t port,
                                  bf_tm_queue_t queue);

/**
 * @brief Associate queue with l1 node for further scheduling.
 *
 * Default: By deafult, queue is set to schedule with the default l1 node
 * for a port.
 *
 * Related APIs: bf_tm_sched_q_l1_reset
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_l1_set(bf_dev_id_t dev,
                                 bf_dev_port_t port,
                                 bf_tm_l1_node_t l1_node,
                                 bf_tm_queue_t queue);

/**
 * @brief Set queue to default l1 node for its port.
 * Note that the port must have at least 1 l1 node assigned to it.
 *
 * Related APIs: bf_tm_sched_q_l1_set
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_l1_reset(bf_dev_id_t dev,
                                   bf_dev_port_t port,
                                   bf_tm_queue_t queue);

/**
 * @brief Set l1 node scheduling priority.
 * Scheduling priority level used when serving guaranteed bandwidth.
 * Higher the number, higher the  priority to select the l1 node for
 * scheduling.
 *
 * Default: l1 node scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_l1_remaining_bw_priority_set ()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] l1_node               L1 node
 * @param[in] priority              Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_priority_set(bf_dev_id_t dev,
                                        bf_dev_port_t port,
                                        bf_tm_l1_node_t l1_node,
                                        bf_tm_sched_prio_t priority);

/**
 * @brief Set l1 node DWRR weights.
 * These weights are used by l1 nodes at same priority level.
 * Across priority these weights serve as ratio to
 * share excess or remaining bandwidth.
 *
 * Default: l1 node scheduling weights set to 1023
 *
 * Related APIs: bf_tm_sched_l1_priority_set(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1 node         L1 node
 * @param[in] weight          Weight value. Supported range [ 0.. 1023 ]
 *                            Weight 0  is used to disable the DWRR especially
 *                            when Max Rate Leakybucket is used.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_dwrr_weight_set(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           bf_tm_l1_node_t l1_node,
                                           uint16_t weight);

/**
 * @brief Set l1 node shaping rate in units of kbps or pps.
 *
 * Default: l1 node shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_shaping_rate_set(bf_dev_id_t dev,
                                            bf_dev_port_t port,
                                            bf_tm_l1_node_t l1_node,
                                            bool pps,
                                            uint32_t burst_size,
                                            uint32_t rate);

/**
 * @brief Enable token bucket that assures l1 node shaping rate (pps or bps)
 *
 * Default: l1 node shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_l1_max_shaping_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_max_shaping_rate_enable(bf_dev_id_t dev,
                                                   bf_dev_port_t port,
                                                   bf_tm_l1_node_t l1_node);

/**
 * @brief Disable token bucket that assures l1 node shaping rate (pps or bps)
 *
 * Default: l1 node shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_l1_max_shaping_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_max_shaping_rate_disable(bf_dev_id_t dev,
                                                    bf_dev_port_t port,
                                                    bf_tm_l1_node_t l1_node);

/**
 * @brief Enable priority propagation from child queues
 *
 * Default: priority propagation is disabled
 *
 * Related APIs: bf_tm_sched_l1_priority_prop_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_priority_prop_enable(bf_dev_id_t dev,
                                                bf_dev_port_t port,
                                                bf_tm_l1_node_t l1_node);

/**
 * @brief Disable priority propagation from child queues
 *
 * Default: priority propagation is disabled
 *
 * Related APIs: bf_tm_sched_l1_priority_prop_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

bf_status_t bf_tm_sched_l1_priority_prop_disable(bf_dev_id_t dev,
                                                 bf_dev_port_t port,
                                                 bf_tm_l1_node_t l1_node);

/**
 * @brief Set l1 node guaranteed rate in terms of pps or kbps.
 *
 * Default: l1 node shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_guaranteed_rate_set(bf_dev_id_t dev,
                                               bf_dev_port_t port,
                                               bf_tm_l1_node_t l1_node,
                                               bool pps,
                                               uint32_t burst_size,
                                               uint32_t rate);

/**
 * @brief Set scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the l1 node for
 * scheduling.
 *
 * Default: l1 node scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_l1_remaining_bw_priority_get ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] priority        Scheduling priority of l1 node.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_remaining_bw_priority_set(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_l1_node_t l1_node,
    bf_tm_sched_prio_t priority);

/**
 * @brief Enable token bucket that assures l1 node guaranteed rate (pps or bps)
 *
 * Default: l1 node guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_guaranteed_rate_enable(bf_dev_id_t dev,
                                                  bf_dev_port_t port,
                                                  bf_tm_l1_node_t l1_node);

/**
 * @brief Disable token bucket that assures l1 node guaranteed rate
 * (pps or bps)
 *
 * Default: l1 node guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_guaranteed_rate_disable(bf_dev_id_t dev,
                                                   bf_dev_port_t port,
                                                   bf_tm_l1_node_t l1_node);

/**
 * @brief Enable l1 node scheduling.
 * If l1 is not associated with a port, the l1 node will be allocated
 * to the port. l1 node cannot already be associated with
 * another port.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_disable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_enable(bf_dev_id_t dev,
                                  bf_dev_port_t port,
                                  bf_tm_l1_node_t l1_node);

/**
 * @brief Disable l1 node scheduling.
 * If disabled, l1 node will not participate in scheduling.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_enable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_disable(bf_dev_id_t dev,
                                   bf_dev_port_t port,
                                   bf_tm_l1_node_t l1_node);

/**
 * @brief Disable l1 node and deallocate node from port.
 * All queues must have been dissocated from this l1 node before
 * calling this function.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_enable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_l1_free(bf_dev_id_t dev,
                                bf_dev_port_t port,
                                bf_tm_l1_node_t l1_node);

/**
 * @brief Enable token bucket that assures port shaping rate (pps or kbps)
 *
 * Default: Port shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_port_shaping_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_enable(bf_dev_id_t dev,
                                            bf_dev_port_t port);

/**
 * @brief Disable token bucket that assures port shaping rate (pps or kbps)
 *
 * Default: Port shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_port_shaping_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_disable(bf_dev_id_t dev,
                                             bf_dev_port_t port);

/**
 * @brief Enable port into scheduler.
 * When enabled, the port will participate
 * in scheduling and also port shaping is enabled.
 * The scheduling speed will be set on all channels (TM Ports)
 * starting from the port given depending on how many channels
 * are needed to participate.
 *
 * Default: By deafult, port is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_sched_port_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] speed           Speed (10G/25G/40G/50G/100G/200G/400G)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  BF_IN_USE if the port is not the first one in a group of channels
 *    needed for the given port_speed.
 */
bf_status_t bf_tm_sched_port_enable(bf_dev_id_t dev,
                                    bf_dev_port_t port,
                                    bf_port_speeds_t speed);

/**
 * @brief Disable port from scheduler.
 * If disabled, the port will not participate
 * in scheduling and also port shaping is disabled.
 *
 * Default: By deafult, port is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_sched_port_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_disable(bf_dev_id_t dev, bf_dev_port_t port);

/**
 * @brief Set queue scheduler advanced flow control mode.
 * Scheduler Advanced Flow Control Mechanism, 0 = Credit 1 = Xoff
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_q_adv_fc_mode_get()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[in] mode                  Scheduler Advanced Flow Control Mode
 *                                  0 = credit 1 = xoff.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_adv_fc_mode_set(bf_dev_id_t dev,
                                          bf_dev_port_t port,
                                          bf_tm_queue_t queue,
                                          bf_tm_sched_adv_fc_mode_t mode);

/**
 * @brief Sets scheduler advanced flow control mode enable/disable mode.
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_adv_fc_mode_enable_get()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] pipe                  Logical PipeId index
 * @param[in] enable                Scheduler Advanced Flow Control Mode
 *                                  Enable/Disable
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

bf_status_t bf_tm_sched_adv_fc_mode_enable_set(bf_dev_id_t dev,
                                               bf_dev_pipe_t pipe,
                                               bool enable);

/* @} */

/**
 * @addtogroup tm-get
 * @{
 */

/**
 * @brief Get queue scheduling priority.
 * Scheduling priority level when serving guaranteed bandwidth. Higher the
 * number, higher the  priority to select the queue for scheduling.
 *
 * Related APIs: bf_tm_sched_q_priority_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[out] priority             Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_priority_get(bf_dev_id_t dev,
                                       bf_dev_port_t port,
                                       bf_tm_queue_t queue,
                                       bf_tm_sched_prio_t *priority);

/**
 * @brief Default queue DWRR weights.
 * These weights are used when queues at same
 * priority level are scheduled during excess bandwidth sharing.
 *
 * Related APIs: bf_tm_sched_q_dwrr_weight_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] weight         Weight value. Supported range [ 0.. 1023 ]
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_dwrr_weight_get_default(bf_dev_id_t dev,
                                                  bf_dev_port_t port,
                                                  bf_tm_queue_t queue,
                                                  uint16_t *weight);
/**
 * @brief Get queue DWRR weights.
 * These weights are used when queues at same
 * priority level are scheduled during excess bandwidth sharing.
 *
 * Related APIs: bf_tm_sched_q_dwrr_weight_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] weight         Weight value. Supported range [ 0.. 1023 ]
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_dwrr_weight_get(bf_dev_id_t dev,
                                          bf_dev_port_t port,
                                          bf_tm_queue_t queue,
                                          uint16_t *weight);

/*
 * Get queue shaping guaranteed rate enable status.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_enable()
 *
 * @param[in] dev       ASIC device identifier.
 * @param[in] port      Port
 * @param[in] queue     queue
 * @param[out] enable   True if the queue guaranteed shaping rate
 *                      is enabled.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_get(bf_dev_id_t dev,
                                         bf_dev_port_t port,
                                         bf_tm_queue_t queue,
                                         bool *enable);

/*
 * Get queue scheduling enable status.
 *
 * Related APIs: bf_tm_sched_q_enable()
 *
 * @param[in] dev       ASIC device identifier.
 * @param[in] port      Port
 * @param[in] queue     queue
 * @param[out] enable   True if the queue scheduling
 *                      is enabled.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_enable_get(bf_dev_id_t dev,
                                     bf_dev_port_t port,
                                     bf_tm_queue_t queue,
                                     bool *enable);

/*
 * Get queue scheduling default enable status.
 *
 * Related APIs: bf_tm_sched_q_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port
 * @param[in] queue      queue
 * @param[out] enable    True if the queue scheduling
 *                       is enabled by default.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_enable_get_default(bf_dev_id_t dev,
                                             bf_dev_port_t port,
                                             bf_tm_queue_t queue,
                                             bool *enable);

/*
 * Get queue shaping max rate enable status.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_enable()
 *
 * @param[in] dev       ASIC device identifier.
 * @param[in] port      Port
 * @param[in] queue     queue
 * @param[out] enable   True if the queue max shaping rate
 *                      is enabled.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_enable_get(bf_dev_id_t dev,
                                             bf_dev_port_t port,
                                             bf_tm_queue_t queue,
                                             bool *enable);

/*
 * Get queue shaping max rate defaults.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port
 * @param[in] queue      queue
 * @param[out] enable    True if the queue max shaping rate
 *                       is enabled by default.
 * @param[out] priority  Default scheduling priority of the
 *                       queue.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_enable_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool *enable,
    bf_tm_sched_prio_t *priority);

/**
 * @brief Get queue shaping rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_rate_get(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           bf_tm_queue_t queue,
                                           bool *pps,
                                           uint32_t *burst_size,
                                           uint32_t *rate);

/**
 * @brief Get queue shaping rate.
 * Rate is in units of kbps or pps. Also get the rate provisioning type.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @param[out] prov_type      The rate provisioning type (OVER, UNDER,
 *                            MIN_ERROR)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_rate_get_provisioning(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool *pps,
    uint32_t *burst_size,
    uint32_t *rate,
    bf_tm_sched_shaper_prov_type_t *prov_type);

/**
 * @brief Gets queue default shaping rate.
 * Rate is in units of kbps or pps. Also get the rate provisioning type.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_get_provisioning()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port
 * @param[in] queue         Queue
 * @param[out] pps          If set to true, values are in terms of pps
 *                          and packets, else in terms of kbps and bytes.
 * @param[out] burst_size   Burst size packets or bytes.
 * @param[out] rate         Shaper value in pps or kbps.
 * @param[out] prov_type    The rate provisioning type (OVER, UNDER, MIN_ERROR)
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_shaping_rate_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool *pps,
    uint32_t *burst_size,
    uint32_t *rate,
    bf_tm_sched_shaper_prov_type_t *prov_type);

/**
 * @brief Get queue guaranteed rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_rate_get(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              bf_tm_queue_t queue,
                                              bool *pps,
                                              uint32_t *burst_size,
                                              uint32_t *rate);

/*
 * Get queue guaranteed min rate enable status.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port
 * @param[in] queue      queue
 * @param[out] enable    True if the queue guaranteed rate
 *                       is enabled.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_enable_get(bf_dev_id_t dev,
                                                bf_dev_port_t port,
                                                bf_tm_queue_t queue,
                                                bool *enable);

/*
 * Get queue guaranteed min rate defaults.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port
 * @param[in] queue      queue
 * @param[out] enable    True if the queue guaranteed rate
 *                       is enabled by default.
 * @param[out] priority  Default scheduling priority of queue.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_enable_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool *enable,
    bf_tm_sched_prio_t *priority);

/**
 * @brief Default queue guaranteed rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_get()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port
 * @param[in] queue         Queue
 * @param[out] pps          If set to true, values are in terms of pps
 *                          and packets, else in terms of kbps and bytes.
 * @param[out] burst_size   Burst size packets or bytes.
 * @param[out] rate         Shaper value in pps or kbps.
 * @param[out] prov_type    The rate provisioning type (OVER, UNDER, MIN_ERROR)
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_guaranteed_rate_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bool *pps,
    uint32_t *burst_size,
    uint32_t *rate,
    bf_tm_sched_shaper_prov_type_t *prov_type);

/**
 * @brief Get scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the queue for scheduling.
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] priority       Scheduling priority of queue.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_remaining_bw_priority_get(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bf_tm_sched_prio_t *priority);

/*
 * Get Port shaping max rate enable default status.
 *
 * Related APIs: bf_tm_sched_port_shaping_enable()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port
 * @param[out] enable    True if the port max shaping rate
 *                       is enabled by default.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_enable_get_default(bf_dev_id_t dev,
                                                        bf_dev_port_t port,
                                                        bool *enable);

/*
 * Get Port shaping max rate enable status.
 *
 * Related APIs: bf_tm_sched_port_shaping_enable()
 *
 * @param[in] dev       ASIC device identifier.
 * @param[in] port      Port
 * @param[out] enable   True if the port max shaping rate
 *                      is enabled.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_enable_get(bf_dev_id_t dev,
                                                bf_dev_port_t port,
                                                bool *enable);

/**
 * @brief Get port shaping rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_rate_get(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              bool *pps,
                                              uint32_t *burst_size,
                                              uint32_t *rate);

/**
 * @brief Default port shaping rate.
 * Also gets the rate provisioning type.
 * The default rate depends on the current port scheduling speed,
 * and if it is disabled, then the maximum port speed is used.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets else in terms of kbps and bytes.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @param[out] prov_type      The rate provisioning type (OVER, UNDER,
 *                            MIN_ERROR)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_rate_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bool *pps,
    uint32_t *burst_size,
    uint32_t *rate,
    bf_tm_sched_shaper_prov_type_t *prov_type);

/**
 * @brief Get port shaping rate. Rate is in units of kbps or pps.
 * Also get the rate provisioning type.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[out] pps            If set to true, values are in terms of pps
 *                            else in terms of kbps.
 * @param[out] burst_size     Burst size packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @param[out] prov_type      The rate provisioning type (OVER, UNDER,
 *                            MIN_ERROR)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_shaping_rate_get_provisioning(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bool *pps,
    uint32_t *burst_size,
    uint32_t *rate,
    bf_tm_sched_shaper_prov_type_t *prov_type);

/*
 * Get port current scheduling speed.
 *
 * Related APIs: bf_tm_sched_port_enable()
 *
 * @param[in] dev      ASIC device identifier.
 * @param[in] port     Port
 * @param[out] speed   Current scheduling speed on the port.
 * @return             Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_speed_get(bf_dev_id_t dev,
                                       bf_dev_port_t port,
                                       bf_port_speeds_t *speed);

/*
 * Get port scheduling speed reset value.
 *
 * Related APIs: bf_tm_sched_port_speed_get()
 *
 * @param[in] dev      ASIC device identifier.
 * @param[in] port     Port
 * @param[out] speed   Port scheduling speed when it was added.
 * @return             Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_port_speed_get_reset(bf_dev_id_t dev,
                                             bf_dev_port_t port,
                                             bf_port_speeds_t *speed);

/**
 * @brief Get per packet byte adjustment value
 *
 * Related APIs: bf_tm_sched_pkt_ifg_compensation_set ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pipe            Pipe identifier.
 * @param[out] adjustment     Byte adjustment done on every packet.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_pkt_ifg_compensation_get(bf_dev_id_t dev,
                                                 bf_dev_pipe_t pipe,
                                                 uint8_t *adjustment);

/**
 * @brief Get per packet byte adjustment default value
 *
 * Related APIs: bf_tm_sched_pkt_ifg_compensation_set ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pipe            Pipe identifier.
 * @param[out] adjustment     Default byte adjustment done on every packet.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_pkt_ifg_compensation_get_default(bf_dev_id_t dev,
                                                         bf_dev_pipe_t pipe,
                                                         uint8_t *adjustment);

/**
 * @brief Get queue scheduler advanced flow control mode.
 * Scheduler Advanced Flow Control Mechanism, 0 = Credit 1 = Xoff
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_q_adv_fc_mode_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[out] mode                 Scheduler Advanced Flow Control Mode
 *                                  0 = credit 1 = xoff.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_adv_fc_mode_get(bf_dev_id_t dev,
                                          bf_dev_port_t port,
                                          bf_tm_queue_t queue,
                                          bf_tm_sched_adv_fc_mode_t *mode);

/**
 * @brief Default queue scheduler advanced flow control mode.
 * Scheduler Advanced Flow Control Mechanism, 0 = Credit 1 = Xoff
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_q_adv_fc_mode_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[out] mode                 Scheduler Advanced Flow Control Mode
 *                                  0 = credit 1 = xoff.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_q_adv_fc_mode_get_default(
    bf_dev_id_t dev,
    bf_dev_port_t port,
    bf_tm_queue_t queue,
    bf_tm_sched_adv_fc_mode_t *mode);

/**
 * @brief Get scheduler advanced flow control mode enable/disable mode.
 * Used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_adv_fc_mode_enable_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] pipe                  Logical PipeId index
 * @param[out] enable               Scheduler Advanced Flow Control Mode
 *                                  Enable/Disable
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

bf_status_t bf_tm_sched_adv_fc_mode_enable_get(bf_dev_id_t dev,
                                               bf_dev_pipe_t pipe,
                                               bool *enable);

/**
 * @brief Get scheduler advanced flow control default mode.
 * Used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_adv_fc_mode_enable_get()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] pipe                  Logical PipeId index
 * @param[out] enable               Scheduler Advanced Flow Control
 *                                  default mode.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_sched_adv_fc_mode_enable_get_default(bf_dev_id_t dev,
                                                       bf_dev_pipe_t pipe,
                                                       bool *enable);

/* @} */

#endif
