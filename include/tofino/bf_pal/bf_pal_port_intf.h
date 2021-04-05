/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _BF_PAL_PORT_INTF_H
#define _BF_PAL_PORT_INTF_H

#include <bf_types/bf_types.h>
#include <port_mgr/bf_port_if.h>
#include <tofino/bf_pal/bf_pal_types.h>

/**
 * @brief Callback function pointer to be registered for port status
 * notifications
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param up Port Status
 * @param cookie Registered data returned back
 * @return Status of the API call
 */
typedef bf_status_t (*port_status_chg_cb)(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bool up,
                                          void *cookie);

/**
 * @brief Callback function pointer to be registered for port mode change
 * notifications
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param num_pkt Number of packets to be sent
 * @param pkt_size Size of the packets to be sent
 * @return Status of the API call
 */
typedef bf_status_t (*port_mode_chg_cb)(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        uint32_t num_pkt,
                                        uint32_t pkt_size);

/**
 * @brief Get the max number of ports on the target
 * @param dev_id Device id
 * @param ports Max number of ports on the target
 * @return Status of the API call
 */
bf_status_t bf_pal_max_ports_get(bf_dev_id_t dev_id, uint32_t *ports);

/**
 * @brief Get the number of front ports on the target
 * @param dev_id Device id
 * @param ports Number of front ports on the target
 * @return Status of the API call
 */
bf_status_t bf_pal_num_front_ports_get(bf_dev_id_t dev_id, uint32_t *ports);

/**
 * @brief Map the front port index to the dev port on the target
 * @param dev_id Device id
 * @param fp_idx Front port index
 * @param dev_port Dev port corresponding to the front port index
 * @return Status of the API call
 */
bf_status_t bf_pal_fp_idx_to_dev_port_map(bf_dev_id_t dev_id,
                                          uint32_t fp_idx,
                                          bf_dev_port_t *dev_port);

/**
 * @brief Get the first (MAC) port on a given device
 * @param dev_id Device id
 * @param dev_port First dev port on the device
 * @return Status of the API call
 */
bf_status_t bf_pal_port_get_first(bf_dev_id_t dev_id, bf_dev_port_t *dev_port);

/**
 * @brief Get the first (MAC) port on a given device that has been
 * 	  already added in the system
 * @param[in] dev_id Device id
 * @param[out] dev_port First dev port on the device
 * @return Status of the API call
 */
bf_status_t bf_pal_port_get_first_added(bf_dev_id_t dev_id,
                                        bf_dev_port_t *dev_port);

/**
 * @brief Get the next (MAC) port on a given device
 * @param dev_id Device id
 * @param curr_dev_port Current dev port on the device
 * @param next_dev_port Next dev port on the device
 * next_dev_port == -1, current dev_port is the last port on the device
 * @return Status of the API call
 */
bf_status_t bf_pal_port_get_next(bf_dev_id_t dev_id,
                                 bf_dev_port_t curr_dev_port,
                                 bf_dev_port_t *next_dev_port);

/**
 * @brief Get the next (MAC) port on a given device that has been already
 * 	  added in the system
 * @param[in] dev_id Device id
 * @param[out] curr_dev_port Current dev port on the device
 * @param[in] next_dev_port Next dev port on the device
 * next_dev_port == -1, current dev_port is the last port on the device
 * @return Status of the API call
 */
bf_status_t bf_pal_port_get_next_added(bf_dev_id_t dev_id,
                                       bf_dev_port_t curr_dev_port,
                                       bf_dev_port_t *next_dev_port);

/**
 * @brief Get the range of the recirculation ports on the target
 * @param dev_id Device id
 * @param start_recirc_port Start of the range of the recirculation ports on the
 * target
 * @param end_recirc_port End of the range of the recirculation ports on the
 * target
 * @return Status of the API call
 */
bf_status_t bf_pal_recirc_port_range_get(bf_dev_id_t dev_id,
                                         uint32_t *start_recirc_port,
                                         uint32_t *end_recirc_port);

/**
 * @brief Map a given recirculation port to dev port on the target
 * @param dev_id Device id
 * @param recirc_port Recirculation port id
 * @param dev_port Dev port corresponding to the front port index
 * @return Status of the API call
 */
bf_status_t bf_pal_recirc_port_to_dev_port_map(bf_dev_id_t dev_id,
                                               uint32_t recirc_port,
                                               bf_dev_port_t *dev_port);

/**
 * @brief Port add function
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param speed Enum type describing the speed of the port
 * @param fec_type Enum type describing the FEC type of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_add(bf_dev_id_t dev_id,
                            bf_dev_port_t dev_port,
                            bf_port_speed_t speed,
                            bf_fec_type_t fec_type);

/**
 * @brief Port add function, specify lane number
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param speed Enum type describing the speed of the port
 * @param n_lanes describing lane number of the port
 * @param fec_type Enum type describing the FEC type of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_add_with_lanes(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       bf_port_speed_t speed,
                                       uint32_t n_lanes,
                                       bf_fec_type_t fec_type);

/**
 * @brief Port add function for all ports
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param speed Enum type describing the speed of the port
 * @param fec_type Enum type describing the FEC type of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_add_all(bf_dev_id_t dev_id,
                                bf_port_speed_t speed,
                                bf_fec_type_t fec_type);

/**
 * @brief Port delete function
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_del(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

/**
 * @brief All ports delete function
 * @param dev_id Device id
 * @return Status of the API call
 */
bf_status_t bf_pal_port_del_all(bf_dev_id_t dev_id);

/**
 * @brief Port Enable function
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_enable(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

/**
 * @brief Port Enable function for all ports
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_enable_all(bf_dev_id_t dev_id);

/**
 * @brief Port Disable function
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_disable(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

/**
 * @brief Get all the stats of a port (User must ensure that that sufficient
 * space fot stats array has been allocated
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param stats Array to hold all the stats read from hardware
 * @return Status of the API call
 */
bf_status_t bf_pal_port_all_stats_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      uint64_t stats[BF_NUM_RMON_COUNTERS]);

/**
 * @brief Get a particular stat of a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param ctr_type Enum type to hold the id of the stats counter
 * @param stat_val Counter value
 * @return Status of the API call
 */
bf_status_t bf_pal_port_this_stat_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_rmon_counter_t ctr_type,
                                      uint64_t *stat_val);

/**
 * @brief Map the Port Stats id to its corresponding description
 * @param ctr_type Enum type to hold the id of the stats counter
 * @param str Corresponding description of the counter type
 * @return Status of the API call
 */
bf_status_t bf_pal_port_this_stat_id_to_str(bf_rmon_counter_t ctr_type,
                                            char **str);

/**
 * @brief Clear a particular stats counter for a particular port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param ctr_type Enum type to hold the id of the stats counter
 * @return Status of the API call
 */
bf_status_t bf_pal_port_this_stat_clear(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bf_rmon_counter_t ctr_type);

/**
 * @brief Clear all the stats of a particular port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_all_stats_clear(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port);

/**
 * @brief Set polling interval for collecting Port stats
 * @param dev_id Device id
 * @param poll_intv_ms Polling interval in milliseconds
 * @return Status of the API call
 */
bf_status_t bf_pal_port_stats_poll_intvl_set(bf_dev_id_t dev_id,
                                             uint32_t poll_intvl_ms);
/**
 * @brief Get polling interval for collecting Port stats
 * @param dev_id Device id
 * @param poll_intv_ms Polling interval in milliseconds
 * @return Status of the API call
 */
bf_status_t bf_pal_port_stats_poll_intvl_get(bf_dev_id_t dev_id,
                                             uint32_t *poll_intvl_ms);

/**
 * @brief Stop the timer for collecting Port stats
 * @param dev_id dev_id
 * @return Status of the API call
 */
bf_status_t bf_pal_port_stats_poll_stop(bf_dev_id_t dev_id);

/**
 * @brief Register for a port status change callback
 * @param cb_fn Callback function which is called when the port status changes
 * @param cookie Data to be given back in callback fn
 * @return Status of the API call
 */
bf_status_t bf_pal_port_status_notif_reg(port_status_chg_cb cb_fn,
                                         void *cookie);

/**
 * @brief Register for a port mode change callback
 * @param dev_id Device id
 * @param cb_fn Callback function which is called when the port mode changes
 * @param cookie Data to be given back in callback fn
 * @return Status of the API call
 */
bf_status_t bf_pal_port_mode_change_notif_reg(bf_dev_id_t dev_id,
                                              port_mode_chg_cb cb_fn,
                                              void *cookie);

/**
 * @brief Set a port in loopback mode
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param mode Loopback mode to be set on the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_loopback_mode_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_loopback_mode_e mode);

/**
 * @brief Get loopback mode of a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param mode Loopback mode on the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_loopback_mode_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_loopback_mode_e *mode);
/**
 * @brief Set the Autoneg policy for the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param an_policy Autoneg policy for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_autoneg_policy_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int an_policy);

/**
 * @brief Get the Autoneg policy of the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param an_policy Autoneg policy of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_autoneg_policy_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pm_port_autoneg_policy_e *an_policy);

/**
 * @brief Set the Autoneg policy for all ports
 * @param dev_id Device id
 * @param an_policy Autoneg policy for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_autoneg_policy_set_all(bf_dev_id_t dev_id,
                                               int an_policy);

/**
 * @brief Set the KR mode policy for the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param kr_policy KR mode policy for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_kr_mode_policy_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int kr_policy);

/**
 * @brief Get the KR mode policy of the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param kr_policy KR mode policy of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_kr_mode_policy_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pm_port_kr_mode_policy_e *kr_policy);

/**
 * @brief Set the KR mode policy for all ports
 * @param dev_id Device id
 * @param kr_policy KR mode policy for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_kr_mode_policy_set_all(bf_dev_id_t dev_id,
                                               int kr_policy);

/**
 * @brief Set the termination mode for the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param term_mode for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_term_mode_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int term_mode);

/**
 * @brief Get the termination mode of the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param term_mode of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_term_mode_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_pm_port_term_mode_e *term_mode);

/**
 * @brief Set the termination mode for all ports
 * @param dev_id Device id
 * @param term_mode for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_term_mode_set_all(bf_dev_id_t dev_id, int term_mode);

/**
 * @brief Set the Rx Precoding policy for the port and lane
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param pc_policy Rx Precoding policy for the port and lane
 * @return Status of the API call
 */
bf_status_t bf_pal_port_precoding_rx_set(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    uint32_t ln,
    bf_pm_port_precoding_policy_e pc_policy);
/**
 * @brief Set the Tx Precoding policy for the port and lane
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param pc_policy Tx Precoding policy for the port and lane
 * @return Status of the API call
 */
bf_status_t bf_pal_port_precoding_tx_set(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    uint32_t ln,
    bf_pm_port_precoding_policy_e pc_policy);

/**
 * @brief Get the Rx Precoding policy of the port and lane
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param pc_policy Rx Precoding policy of the port and lane
 * @return Status of the API call
 */
bf_status_t bf_pal_port_precoding_rx_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pm_port_precoding_policy_e *pc_policy);

/**
 * @brief Get the Tx Precoding policy of the port and lane
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param pc_policy Tx Precoding policy of the port and lane
 * @return Status of the API call
 */
bf_status_t bf_pal_port_precoding_tx_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pm_port_precoding_policy_e *pc_policy);

/**
 * @brief Set the Precoding policy for all ports
 * @param dev_id Device id
 * @param an_policy Precoding policy for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_precoding_policy_set_all(
    bf_dev_id_t dev_id, bf_pm_port_precoding_policy_e pc_policy);

/**
 * @brief Read and update all the stats for a port from hardware
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_all_stats_update(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port);

/**
 * @brief Get all the stats of a port (User must ensure that that sufficient
 * space for stats array has been allocated
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param stats Array to hold all the stats read from hardware
 * @param timestamp Time stamp of the stats update, sec and nsec
 * @return Status of the API call
 */
bf_status_t bf_pal_port_all_pure_stats_get_with_timestamp(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    uint64_t stats[BF_NUM_RMON_COUNTERS],
    int64_t *timestamp_s,
    int64_t *timestamp_ns);

/**
 * @brief Get the number of lanes consumed by a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param num_lanes Number of lanes used up by the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_num_lanes_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int *num_lanes);

/**
 * @brief Set MTU on a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_mtu Tx Maximum transfer unit length (0-65535)
 * @param rx_mtu Rx Maximum transfer unit length (0-65535)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_mtu_set(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                uint32_t tx_mtu,
                                uint32_t rx_mtu);

/**
 * @brief Get MTU of a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_mtu Tx Maximum transfer unit length (0-65535)
 * @param rx_mtu Rx Maximum transfer unit length (0-65535)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_mtu_get(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                uint32_t *tx_mtu,
                                uint32_t *rx_mtu);

/**
 * @brief Enable or Disable Per-COS Tx/Rx pause on a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_en_map per-COS bitmap of state to set Tx (0=disable, 1=enable)
 * @param rx_en_map per-COS bitmap of state to set Rx (0=disable, 1=enable)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_flow_control_pfc_set(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             uint32_t tx_en_map,
                                             uint32_t rx_en_map);
/**
 * @brief Get Enable or Disable Per-COS Tx/Rx pause configured on the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_en_map per-COS bitmap of state to set Tx (0=disable, 1=enable)
 * @param rx_en_map per-COS bitmap of state to set Rx (0=disable, 1=enable)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_flow_control_pfc_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             uint32_t *tx_en_map,
                                             uint32_t *rx_en_map);
/**
 * @brief Enable or Disable link Tx and Rx link pause on a Tofino port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_en state to set (False=disable, True=enable)
 * @param rx_en state to set (False=disable, True=enable)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_flow_control_link_pause_set(bf_dev_id_t dev_id,
                                                    bf_dev_port_t dev_port,
                                                    bool tx_en,
                                                    bool rx_en);
/**
 * @brief Get Enable or Disable link Tx and Rx link pause configured on the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param tx_en state to set (False=disable, True=enable)
 * @param rx_en state to set (False=disable, True=enable)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_flow_control_link_pause_get(bf_dev_id_t dev_id,
                                                    bf_dev_port_t dev_port,
                                                    bool *tx_en,
                                                    bool *rx_en);

/**
 * @brief Get the oper state of a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param state Oper state of the port (0=Down, 1=Up)
 * @return Status of the API call
 */
bf_status_t bf_pal_port_oper_state_get(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int *state);

/**
 * @brief Check if port is valid
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_is_valid(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
/**
 * @brief Set the FEC type of an already existing port
 * Note: This API will cause the port to be deleted and re-added if the new
 * fec type is not the same as the existing fec type for the port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param fec_type Fec type to be set for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_fec_set(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                bf_fec_type_t fec_type);

/**
 * @brief Get the FEC type of an already existing port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param fec_type Fec type be set on the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_fec_get(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                bf_fec_type_t *fec_type);

/**
 * @brief Get the media type detected for a port
 * Note: If no QSFP is inserted on a port, then the media type reported is
 * UNKNOWN
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param media_type Media type detected on the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_media_type_get(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       bf_media_type_t *media_type);

/**
 * @brief Enable cut-through switching for a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_cut_through_enable(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port);

/**
 * @brief Disable cut-through switching for a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @return Status of the API call
 */
bf_status_t bf_pal_port_cut_through_disable(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port);

/**
 * @brief Get cut-through enable status for a port
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param ct_enabled Pointer to current cut-through enable status
 * @return Status of the API call
 */
bf_status_t bf_pal_port_cut_through_enable_status_get(bf_dev_id_t dev_id,
                                                      bf_dev_port_t dev_port,
                                                      bool *ct_enabled);

/**
 * @brief Get the number of pipes on a target
 * @param dev_id Device id
 * @param num_pipes Number of active pipes on the target
 * @return Status of the API call
 */
bf_status_t bf_pal_num_pipes_get(bf_dev_id_t dev_id, uint32_t *num_pipes);

/**
 * @brief Get the dev port number
 * @param dev_id Device id
 * @param port_hdl Port handle for the front port
 * @param dev_port Corresponding dev port
 * @return Status of the API call
 */
bf_status_t bf_pal_front_port_to_dev_port_get(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    bf_dev_port_t *dev_port);

/**
 * @brief Get the front panel port number
 * @param dev_id Device id
 * @param dev_port Corresponding dev port
 * @param port_hdl Port handle for the front port
 * @return Status of the API call
 */
bf_status_t bf_pal_dev_port_to_front_port_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Get the dev port number
 * @param dev_id Device id
 * @param port_str Port str, length max:MAX_PORT_HDL_STRING_LEN
 * @param dev_port Corresponding dev port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_str_to_dev_port_map(bf_dev_id_t dev_id,
                                            char *port_str,
                                            bf_dev_port_t *dev_port);

/**
 * @brief Get the dev port number
 * @param dev_id Device id
 * @param dev_port Corresponding dev port
 * @param port_str Port str, length required:MAX_PORT_HDL_STRING_LEN
 * @return Status of the API call
 */
bf_status_t bf_pal_dev_port_to_port_str_map(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            char *port_str);
/**
 * @brief Get a particular stat of a port via register access
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param ctr_type_array Pointer to array holding stats counter id(s)
 * @param stat_val Counter value
 * @param num_of_ctr to read number of ctr_type
 * @return Status of the API call
 */
bf_status_t bf_pal_port_stat_direct_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bf_rmon_counter_t *ctr_type_array,
                                        uint64_t *stat_val,
                                        uint32_t num_of_ctr);

/**
 * @brief Check if the port is an internal port
 * @param dev_id Device id
 * @param dev_port Corresponding dev port
 * @param is_internal Internal port
 * @return Status of the API call
 */
bf_status_t bf_pal_is_port_internal(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    bool *is_internal);

/**
 * @brief Sets the port direction
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param port_dir Port Direction 0->default 1->TX only, 2->RX only
 * @return Status of the API call
 */
bf_status_t bf_pal_port_direction_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_pm_port_dir_e port_dir);

/**
 * @brief Set the media type for a port.
 *        Initial configuration via in Json file will be ignored, if any.
 *        Need to be set before port-enable.
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param media_type Media type configured on the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_media_type_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       bf_media_type_t media_type);

/**
 * @brief Set the serdes parameters for a port per channel.
 *        Initial configuration via in Json file will be ignored, if any.
 *        Need to be set before port-enable.
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param serdes_param Serdes param
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_params_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_pal_serdes_params_t *serdes_param);

/**
 * @brief Get the phy pipe, log pipe
 * @param dev_id dev id
 * @param dev_port Corresponding dev port
 * @param phy_pipe log_pipe Physical pipe Logical pipe
 * @return Status of the API call
 */
bf_status_t bf_pal_get_pipe_from_dev_port(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_dev_pipe_t *phy_pipe,
                                          bf_dev_pipe_t *log_pipe);

/**
 * @brief Get the configured speed for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured speed for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_speed_get(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  bf_port_speed_t *speed);

/**
 * @brief Get the configured admin status for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @parom[out] is_enabled Admin status of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_is_enabled(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   bool *is_enabled);

/**
 * @brief Get the configured direction for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] port_dir Configured direction of the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_direction_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_pm_port_dir_e *port_dir);

/**
 * @brief Get the configured serdes tx eq pre param for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured serdes tx eq pre param for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_tx_eq_pre_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int *tx_pre);

/**
 * @brief Get the configured serdes tx eq post param for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured serdes tx eq post param for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_tx_eq_post_get(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port,
                                              int *tx_post);

/**
 * @brief Get the configured serdes tx eq pre2 param for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured serdes tx eq pre2 param for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_tx_eq_pre2_get(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port,
                                              int *tx_pre2);

/**
 * @brief Get the configured serdes tx eq post2 param for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured serdes tx eq post2 param for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_tx_eq_post2_get(bf_dev_id_t dev_id,
                                               bf_dev_port_t dev_port,
                                               int *tx_post2);

/**
 * @brief Get the configured serdes tx eq main param for the port
 * @param[in] dev_id Device id
 * @param[in] dev_port Device port number
 * @param[out] speed Configured serdes tx eq main param for the port
 * @return Status of the API call
 */
bf_status_t bf_pal_port_serdes_tx_eq_attn_get(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port,
                                              int *tx_attn);

/**
 * @brief Get the first front panel port index on the system
 * @param[in] dev_id Device id
 * @param[out] fp_idx front panel port index
 * @return Status of the API call
 */
bf_status_t bf_pal_fp_idx_get_first(bf_dev_id_t dev_id, uint32_t *fp_idx);

/**
 * @brief Get the next front panel port index on the system
 * @param[in] dev_id Device id
 * @param[in] curr_idx current front panel port index
 * @param[out] next_idx next front panel port index
 * @return Status of the API call
 */
bf_status_t bf_pal_fp_idx_get_next(bf_dev_id_t dev_id,
                                   uint32_t curr_idx,
                                   uint32_t *next_idx);
/**
 * @brief Get all historical stats of a port from mac-stats software cache
 * User must ensure that that sufficient space for stats array has been
 * allocated
 * @param dev_id device
 * @param dev_port device port number
 * @param stats Array that can hold all the stats from  mac-stats cache
 * @return Status of the API call
 */
bf_status_t bf_pal_port_mac_stats_historical_get(bf_dev_id_t dev_id,
                                                 bf_dev_port_t dev_port,
                                                 bf_rmon_counter_array_t *data);
/**
 * @brief Set all historical stats of a port to mac-stats software cache
 * User must ensure that that sufficient space for stats array has been
 * allocated
 * @param dev_id device
 * @param dev_port device port number
 * @param stats Array that can hold all the stats to be written to mac-stats
 * cache
 * @return Status of the API call
 */
bf_status_t bf_pal_port_mac_stats_historical_set(bf_dev_id_t dev_id,
                                                 bf_dev_port_t dev_port,
                                                 bf_rmon_counter_array_t *data);

/**
 * @brief Get the interrupt based link monitoring information
 * @param dev_id device
 * @param en Admin status of interrupt based link monitoring info
 * @return Status of the API call
 */
bf_status_t bf_pal_interrupt_based_link_monitoring_get(bf_dev_id_t dev_id,
                                                       bool *en);

#endif
