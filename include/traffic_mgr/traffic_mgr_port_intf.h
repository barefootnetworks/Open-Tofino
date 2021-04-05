/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef __TRAFFIC_MGR_PORT_INTF_H__
#define __TRAFFIC_MGR_PORT_INTF_H__

/**
 * @file traffic_mgr_port_intf.h
 * @brief This file contains APIs for Traffic Manager application to
 *        program Port meant to implement part of QoS behaviour based on
 *        traffic properties.
 */

#include <traffic_mgr/traffic_mgr_types.h>
#include <bf_types/bf_types.h>

/**
 * @addtogroup tm-ingress
 * @{
 */

/**
 * @brief Set ingress port limit.
 * When buffer usage accounted on port basis crosses the
 * limit, traffic is not admitted into traffic manager.
 *
 * Default: Set to 100% buffer usage.
 *
 * Related APIs: bf_tm_port_ingress_drop_limit_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Limits in terms of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_ingress_drop_limit_set(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              uint32_t cells);

/**
 * @brief Set port hysteresis limits.
 * When usage of cells goes below hysteresis value port pause or
 * drop condition will be cleared.
 *
 * Default : No hysteresis.
 *
 * Related APIs: bf_tm_port_ingress_drop_limit_set(),
 *               bf_tm_port_ingress_hysteresis_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Offset Limit
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_ingress_hysteresis_set(bf_dev_id_t dev,
                                              bf_dev_port_t port,
                                              uint32_t cells);

/* @} */

/**
 * @addtogroup tm-egress
 * @{
 */

/**
 * @brief Set Port Unicast Cut Through Limit
 * This API can be used to set cut through buffer size on per port basis.
 * The specified size is set aside for unicast traffic in cut through mode.
 *
 * Default : Set according to absorb TM processing cycle time.
 *
 * Related APIs: bf_tm_port_uc_cut_through_limit_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] cells      Size in terms of cells (upto 16). Valid value [1..15]
 *                       If size is set to zero, then cut through is disabled.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_uc_cut_through_limit_set(bf_dev_id_t dev,
                                                bf_dev_port_t port,
                                                uint8_t cells);

/* @} */

/**
 * @addtogroup tm-flowcontrol
 * @{
 * Description of APIs for Traffic Manager application to
 * enable, disable, port level pause, PFC.
 */

/**
 * @brief Set the Flow Control Mode (Port Pause | PFC Pause)
 * This API can be used to set type of Pause desired. When pause type is
 * BF_TM_PAUSE_PFC, then pfc is asserted. If type set to BF_TM_PAUSE_PORT
 * port pause is asserted. When pause is set to BF_TM_PAUSE_NONE, then
 * no pause is asserted.
 *
 * Related APIs: bf_tm_port_pfc_cos_mapping_set()
 *               bf_tm_port_flowcontrol_rx_set()
 *               bf_tm_port_flowcontrol_mode_get()
 *
 * Default : No Pause or flow control.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] type       Pause type.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */
bf_status_t bf_tm_port_flowcontrol_mode_set(bf_dev_id_t dev,
                                            bf_dev_port_t port,
                                            bf_tm_flow_ctrl_type_t type);

/**
 * @brief bf_tm_port_flowcontrol_rx_set
 * This API can be used to set how port should react to pause assertion
 * from peer port. When pause type is BF_TM_PAUSE_PFC, then pfc is
 * honored. If type set to BF_TM_PAUSE_PORT port will react to pause.
 * When pause is set to BF_TM_PAUSE_NONE, then pause or pfc is ignored
 * and traffic is still pushed to epipe and egress port.
 *
 * Related APIs: bf_tm_port_flowcontrol_mode_set()
 *               bf_tm_port_flowcontrol_rx_get()
 *
 * Default : Ignore pause and pfc.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] type       Pause type.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */
bf_status_t bf_tm_port_flowcontrol_rx_set(bf_dev_id_t dev,
                                          bf_dev_port_t port,
                                          bf_tm_flow_ctrl_type_t type);
/**
 * @brief Set packet CoS to internal CoS
 * When PFC level pause is desired, it is required to map internal
 * iCoS(iCoS = ig_intr_md.ingress_cos) to packet CoS. This API can
 * be used to set up the CoS mapping.
 *
 * Default: No PFC
 *
 * Related APIs: bf_tm_q_pfc_cos_mapping_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @param[in] cos_to_icos Array of 8 uint8_t values.
 *                        Array index is CoS and array value is iCoS.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */
bf_status_t bf_tm_port_pfc_cos_mapping_set(bf_dev_id_t dev,
                                           bf_dev_port_t port,
                                           uint8_t *cos_to_icos);

/**
 * @brief Sets CPU port.
 * This API can be used to specify which one of the ports on a pipe is connected
 * to CPU. Copy to CPU packet fucntionality uses the specified port.
 * This function overrides if a cpu port was already set.
 *
 * Default: No CPU port
 *
 * Related APIs: bf_tm_port_cpuport_reset()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */
bf_status_t bf_tm_port_cpuport_set(bf_dev_id_t dev, bf_dev_port_t port);

/**
 * @brief Clear CPU port binding
 * This API can be used to specify no CPU port. This API can be used
 * to indicate no cpu port or clear any previously set cpu port.
 *
 * Default: No CPU port
 *
 * Related APIs: bf_tm_port_cpuport_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */
bf_status_t bf_tm_port_cpuport_reset(bf_dev_id_t dev);

/**
 * @brief bf_tm_port_cut_through_enable
 * Enable cut-through switching for a port in TM
 *
 * Default : Cut-through is disabled.
 *
 *NOTE:
 *    This API shouldn't be called directly as enabling/disabling
 *    cut-through is a two-step process (enable/disable in TM and
 *    enable/disable in ParDe EBUF register).
 *    This should be called through BF_PAL API only.
 *
 * Related APIs: bf_tm_port_cut_through_disable()
 *               bf_tm_port_cut_through_enable_status_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_cut_through_enable(bf_dev_id_t dev, bf_dev_port_t port);

/**
 * @brief bf_tm_port_cut_through_disable
 * Disable cut-through switching for a port in TM
 *
 * Default : Cut-through is disabled.
 *
 *NOTE:
 *    This API shouldn't be called directly as enabling/disabling
 *    cut-through is a two-step process (enable/disable in TM and
 *    enable/disable in ParDe EBUF register).
 *    This should be called through BF_PAL API only.
 *
 * Related APIs: bf_tm_port_cut_through_enable()
 *               bf_tm_port_cut_through_enable_status_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_cut_through_disable(bf_dev_id_t dev, bf_dev_port_t port);

/**
 * @brief bf_tm_port_all_queues_flush
 * Use this API to flush all queues assigned to a port.
 * Only available on tofino2. Otherwirse no-op, returns success.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

bf_status_t bf_tm_port_all_queues_flush(bf_dev_id_t dev, bf_dev_port_t port);

/**
 * @brief Set port skid limits.
 * Cannot be increased beyond the size of skid pool. If set to zero,
 * in transit  traffic from all lossless PPGs of the port will be dropped
 * once pause/pfc is asserted.
 *
 * Default : Skid limits are set to zero.
 *
 * Related APIs: bf_tm_ppg_skid_limit_get()
 *
 * @param[in] dev               ASIC device identifier.
 * @param[in] port              Port Identifier
 * @param[in] cells             Limits in terms of number of cells.
 * @return                      Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_skid_limit_set(bf_dev_id_t dev,
                                      bf_dev_port_t port,
                                      uint32_t cells);

/**
 * @brief bf_tm_port_set_qac_rx
 * Set the qac rx state (enable or disable) for a port in TM
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] state      Enable (true) or disable (false)
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
bf_status_t bf_tm_port_set_qac_rx(bf_dev_id_t dev,
                                  bf_dev_port_t port,
                                  bool state);

/* @} */

#endif
