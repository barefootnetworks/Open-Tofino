/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _BF_PAL_PM_INTF_H
#define _BF_PAL_PM_INTF_H

// File includes
#include <tofino/bf_pal/bf_pal_types.h>
#include <port_mgr/bf_port_if.h>

// Public Functions

/**
 * @brief Mark a port ready for bringup
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param is_ready Flag to indicate if a port is ready or not
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_pltfm_front_port_ready_for_bringup(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, bool is_ready);

/**
 * @brief Mark a port eligible for autonegotiation
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param is_eligible Flag to indicate if a port can autoneg while bring up or
 * not
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_pltfm_front_port_eligible_for_autoneg(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, bool is_eligible);

/**
 * @brief Get the number of MAC lanes consumed by an existing port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param num_lanes Number of lanes consumed
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_pltfm_front_port_num_lanes_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *num_lanes);

/**
 * @brief Given a dev port, get the corresponding front panel port number
 *
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_dev_port_to_front_panel_port_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Given a front panel port number, get the corresponding dev port
 *
 * @param port_hdl Front panel port number
 * @param dev_id Device id containing port_hdl
 * @param dev_port Device port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_front_panel_port_to_dev_port_get(
    bf_pal_front_port_handle_t *port_hdl,
    bf_dev_id_t *dev_id_of_port,
    bf_dev_port_t *dev_port);

/**
 * @brief Get the first front panel port number
 *
 * @param dev_id Device id
 * @param first_port_hdl First front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_front_panel_port_get_first(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *first_port_hdl);

/**
 * @brief Get the first front panel port number that has already been
 * 	  successfully added in the system
 *
 * @param[in] dev_id Device id
 * @param[out] first_port_hdl First front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_front_panel_port_get_first_added(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *first_port_hdl);

/**
 * @brief Given a front panel port number, get the next front panel port number
 * curr_port_hdl == NULL -> next_port_hdl is the first port
 * return == BF_OBJECT_NOT_FOUND -> curr_port_hdl is the last port
 *
 * @param dev_id Device id
 * @param curr_port_hdl Current front panel port number
 * @param next_port_hdl Next front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_front_panel_port_get_next(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *curr_port_hdl,
    bf_pal_front_port_handle_t *next_port_hdl);

/**
 * @brief Given a front panel port number, get the next front panel port number
 *        that has already been successfully added in the system
 * curr_port_hdl == NULL -> next_port_hdl is the first port
 * return == BF_OBJECT_NOT_FOUND -> curr_port_hdl is the last port
 *
 * @param dev_id Device id
 * @param[in] curr_port_hdl Current front panel port number
 * @param[out] next_port_hdl Next front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_front_panel_port_get_next_added(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *curr_port_hdl,
    bf_pal_front_port_handle_t *next_port_hdl);

/**
 * @brief Get the port handle corresponding to the string
 *
 * @param dev_id Device id
 * @param port_str String describing the port
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_str_to_hdl_get(bf_dev_id_t dev_id,
                                      const char *port_str,
                                      bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Validate a port speed and channel
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 * @param speed Speed in which the port is to be added
 * @param n_lanes Lane number in which the port is to be added
 * @param fec FEC in which the port is to be added
 *
 * @return True if the validation successful
 */
bool bf_pm_port_valid_speed_and_channel(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bf_port_speed_t speed,
                                        uint32_t n_lanes,
                                        bf_fec_type_t fec);

/**
 * @brief Add a port with a given speed and fec
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param speed Speed in which the port is to be added
 * @param fec FEC in which the port is to be added
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_add(bf_dev_id_t dev_id,
                           bf_pal_front_port_handle_t *port_hdl,
                           bf_port_speed_t speed,
                           bf_fec_type_t fec);

/**
 * @brief Add a port with a given speed, lane number, and fec
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param speed Speed in which the port is to be added
 * @param n_lanes Lane number in which the port is to be added
 * @param fec FEC in which the port is to be added
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_add_with_lanes(bf_dev_id_t dev_id,
                                      bf_pal_front_port_handle_t *port_hdl,
                                      bf_port_speed_t speed,
                                      uint32_t n_lanes,
                                      bf_fec_type_t fec);

/**
 * @brief Add all ports with a given speed and fec
 *
 * @param dev_id Device id
 * @param speed Speed in which the port is to be added
 * @param fec FEC in which the port is to be added
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_add_all(bf_dev_id_t dev_id,
                               bf_port_speed_t speed,
                               bf_fec_type_t fec);

/**
 * @brief Add all ports with a given speed, lane number,and fec
 *
 * @param dev_id Device id
 * @param speed Speed in which the port is to be added
 * @param n_lanes Lane number in which the port is to be added
 * @param fec FEC in which the port is to be added
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_add_all_with_lanes(bf_dev_id_t dev_id,
                                          bf_port_speed_t speed,
                                          uint32_t n_lanes,
                                          bf_fec_type_t fec);
/**
 * @brief Delete a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_delete(bf_dev_id_t dev_id,
                              bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Delete all ports
 *
 * @param dev_id Device id
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_delete_all(bf_dev_id_t dev_id);

/**
 * @brief Enable a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_enable(bf_dev_id_t dev_id,
                              bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Enable all ports
 *
 * @param dev_id Device id
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_enable_all(bf_dev_id_t dev_id);

/**
 * @brief Disable a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_disable(bf_dev_id_t dev_id,
                               bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Enable/Disable Auto-negotiation for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param an_policy AN policy for the port
 *        (PM_AN_DEFAULT, PM_AN_FORCE_ENABLE, PM_AN_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_autoneg_set(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bf_pm_port_autoneg_policy_e an_policy);

/**
 * @brief Get Enable/Disable of Auto-negotiation for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param an_policy AN policy for the port
 *        (PM_AN_DEFAULT, PM_AN_FORCE_ENABLE, PM_AN_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_autoneg_get(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bf_pm_port_autoneg_policy_e *an_policy);
/**
 * @brief Enable/Disable Auto-negotiation for all ports
 *
 * @param dev_id Device id
 * @param an_policy AN policy for the port
 *        (PM_AN_DEFAULT, PM_AN_FORCE_ENABLE, PM_AN_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_autoneg_set_all(bf_dev_id_t dev_id,
                                       bf_pm_port_autoneg_policy_e an_policy);

/**
 * @brief Set array of the advertised speeds for a port
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[in] adv_speed_arr Array of advertised speeds for the port
 * @param[in] adv_speed_cnt Number of advertised speeds in array
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_adv_speed_set(const bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     const bf_port_speed_t *adv_speed_arr,
                                     const uint32_t adv_speed_cnt);

/**
 * @brief Enable/Disable KR Mode for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param kr_policy KR policy for the port
 *        (PM_KR_DEFAULT, PM_KR_FORCE_ENABLE, PM_KR_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_kr_mode_set(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bf_pm_port_kr_mode_policy_e kr_policy);

/**
 * @brief Get Enable/Disable of KR Mode for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param kr_policy KR policy for the port
 *        (PM_KR_DEFAULT, PM_KR_FORCE_ENABLE, PM_KR_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_kr_mode_get(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bf_pm_port_kr_mode_policy_e *kr_policy);

/**
 * @brief Enable/Disable KR Mode for all ports
 *
 * @param dev_id Device id
 * @param kr_policy KR policy for the port
 *        (PM_KR_DEFAULT, PM_KR_FORCE_ENABLE, PM_KR_FORCE_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_kr_mode_set_all(bf_dev_id_t dev_id,
                                       bf_pm_port_kr_mode_policy_e kr_policy);

/**
 * @brief Set AC coupled (default) or DC coupled termination for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param term_mode
 *        (PM_TERM_DEFAULT, PM_TERM_AC, PM_TERM_DC)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_term_mode_set(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     int ln,
                                     bf_pm_port_term_mode_e term_mode);

/**
 * @brief Get AC coupled (default) or DC coupled termination for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param term_mode
 *        (PM_TERM_DEFAULT, PM_TERM_AC, PM_TERM_DC)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_term_mode_get(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     int ln,
                                     bf_pm_port_term_mode_e *term_mode);
/**

 * @brief Set AC coupled (default) or DC coupled termination for all ports
 *
 * @param dev_id Device id
 * @param term_mode
 *        (PM_TERM_DEFAULT, PM_TERM_AC, PM_TERM_DC)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_term_mode_set_all(bf_dev_id_t dev_id,
                                         bf_pm_port_term_mode_e term_mode);

/**
 * @brief Enable/Disable Tx Precoding for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param pc_policy Tx PRECODING policy for the port
 *        (PM_PRECODING_DEFAULT, PM_PRECODING_ENABLE, PM_PRECODING_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_precoding_tx_set(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t ln,
    bf_pm_port_precoding_policy_e pc_policy);

/**
 * @brief Get Enable/Disable of Tx Precoding for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param pc_policy Tx PRECODING policy for the port
 *        (PM_PRECODING_DEFAULT, PM_PRECODING_ENABLE, PM_PRECODING_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_precoding_tx_get(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t ln,
    bf_pm_port_precoding_policy_e *pc_policy);

/**
 * @brief Enable/Disable Rx Precoding for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param pc_policy Rx PRECODING policy for the port
 *        (PM_PRECODING_DEFAULT, PM_PRECODING_ENABLE, PM_PRECODING_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_precoding_rx_set(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t ln,
    bf_pm_port_precoding_policy_e pc_policy);

/**
 * @brief Get Enable/Disable of Rx Precoding for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param pc_policy Rx PRECODING policy for the port
 *        (PM_PRECODING_DEFAULT, PM_PRECODING_ENABLE, PM_PRECODING_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_precoding_rx_get(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t ln,
    bf_pm_port_precoding_policy_e *pc_policy);
/**
 * @brief Enable/Disable Precoding for all ports, all lanes, both directions
 *
 * @param dev_id Device id
 * @param an_policy PRECODING policy for the port
 *        (PM_PRECODING_DEFAULT, PM_PRECODING_ENABLE, PM_PRECODING_DISABLE)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_precoding_set_all(
    bf_dev_id_t dev_id, bf_pm_port_precoding_policy_e pc_policy);

/**
 * @brief Setup PRBS on multiple ports. This sets the PRBS on all the ports
 * in the list
 *
 * @param dev_id Device id
 * @param port_hdl_list Front panel port number list
 * @param len Number of ports in the list
 * @param prbs_speed Speed of the PRBS test
 * @param prbs_mode Mode of the PRBS test
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_set(bf_dev_id_t dev_id,
                                bf_pal_front_port_handle_t *port_hdl_list,
                                uint32_t len,
                                bf_port_prbs_speed_t prbs_speed,
                                bf_port_prbs_mode_t prbs_mode);

/**
 * @brief Cleanup PRBS on multiple ports. This cleans up the PRBS only on all
 * the ports in the list
 *
 * @param dev_id Device id
 * @param port_hdl_list Front panel port number list
 * @param len Number of ports in the list
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_cleanup(bf_dev_id_t dev_id,
                                    bf_pal_front_port_handle_t *port_hdl_list,
                                    uint32_t len);

/**
 * @brief Get the PRBS stats on multiple ports
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param q_stats prbs stats
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_stats_get(bf_dev_id_t dev_id,
                                      bf_pal_front_port_handle_t *port_hdl,
                                      bf_sds_debug_stats_t *stats);

/**
 * @brief Get the oper status of a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param oper_status Operational status of the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_oper_status_get(bf_dev_id_t dev_id,
                                       bf_pal_front_port_handle_t *port_hdl,
                                       bool *oper_status);

/**
 * @brief Get the speed of a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param speed Speed of the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_speed_get(bf_dev_id_t dev_id,
                                 bf_pal_front_port_handle_t *port_hdl,
                                 bf_port_speed_t *speed);

/**
 * @brief Set the speed of a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param speed Speed of the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_speed_set(bf_dev_id_t dev_id,
                                 bf_pal_front_port_handle_t *port_hdl,
                                 bf_port_speed_t speed);

/**
 * @brief Set the speed and lane number of a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param speed Speed of the port
 * @param n_lanes Lane number of the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_speed_set_with_lanes(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    bf_port_speed_t speed,
    uint32_t n_lanes);

/**
 * @brief Get the FEC type for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param fec_type FEC type set for the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_fec_get(bf_dev_id_t dev_id,
                               bf_pal_front_port_handle_t *port_hdl,
                               bf_fec_type_t *fec_type);

/**
 * @brief Set the FEC type for a port. If the fec type being set is different
 * than the existing one, then the port will be deleted and re-added
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param fec_type FEC type to be set
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_fec_set(bf_dev_id_t dev_id,
                               bf_pal_front_port_handle_t *port_hdl,
                               bf_fec_type_t fec_type);

/**
 * @brief Get the loopback mode of a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param mode Loopback mode
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_loopback_mode_get(bf_dev_id_t dev_id,
                                         bf_pal_front_port_handle_t *port_hdl,
                                         bf_loopback_mode_e *mode);

/**
 * @brief Set a port in loopback
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param mode Loopback mode
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_loopback_mode_set(bf_dev_id_t dev_id,
                                         bf_pal_front_port_handle_t *port_hdl,
                                         bf_loopback_mode_e mode);

/**
 * @brief Start the timer to periodically update stats for all the ports
 * after the given polling interval
 *
 * @param dev_id Device id
 * @param poll_intv_ms Polling interval
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_poll_start(bf_dev_id_t dev_id,
                                        uint32_t poll_intv_ms);

/**
 * @brief Stop the timer to periodically update stats for all the ports
 *
 * @param dev_id dev id
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_poll_stop(bf_dev_id_t dev_id);

/**
 * @brief Get a particular stat counter for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param ctr_type Counter id
 * @param stat_val Value of the counter
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_this_stat_get(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     bf_rmon_counter_t ctr_type,
                                     uint64_t *stat_val);

/**
 * @brief Get all stat counters for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param stats Array of all the counter values
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_all_stats_get(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     uint64_t stats[BF_NUM_RMON_COUNTERS]);

/**
 * @brief Clear a particular stat counter for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param ctr_type Counter id
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_this_stat_clear(bf_dev_id_t dev_id,
                                       bf_pal_front_port_handle_t *port_hdl,
                                       bf_rmon_counter_t ctr_type);

/**
 * @brief Clear all stat counters for a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_all_stats_clear(bf_dev_id_t dev_id,
                                       bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Read all the stats asynchronously from the hardware and update the
 * local cache
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_all_stats_update(bf_dev_id_t dev_id,
                                        bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Read all the stats asynchronously from the hardware and update the
 * local cache
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param stats Array of all the counter values
 * @param timestamp Time stamp of the last stats update, struct timespec:sec and
 *nsec
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_all_pure_stats_get_with_timestamp(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint64_t stats[BF_NUM_RMON_COUNTERS],
    int64_t *timestamp_s,
    int64_t *timestamp_ns);

/**
 * @brief Read all the stats synchronously from the hardware and update the
 * cache
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_all_stats_update_sync(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Get the media type connected to the port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param media_type Media type connected to the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_media_type_get(bf_dev_id_t dev_id,
                                      bf_pal_front_port_handle_t *port_hdl,
                                      bf_media_type_t *media_type);

/**
 * @brief Get the number of ports on the chip depending on the efuse.
 *        Includes front and internal ports on the chip.
 * @param dev_id Device id
 * @param num_ports Number of ports on the chip
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_num_max_ports_get(bf_dev_id_t dev_id, uint32_t *num_ports);

/**
 * @brief Get the number of ports on the chip depending on the efuse,
 *        excluding the internal ports.
 *
 * @param dev_id Device id
 * @param num_ports Number of ports on the chip
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_num_front_ports_get(bf_dev_id_t dev_id, uint32_t *num_ports);

/**
 * @brief Get the dev port corresponding to the front port index
 *
 * @param dev_id Device id
 * @param fp_idx Front port index
 * @param dev_port Device port number corresponding to the front port index
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_front_port_index_to_dev_port_get(bf_dev_id_t dev_id,
                                                   uint32_t fp_idx,
                                                   bf_dev_port_t *dev_port);

/**
 * @brief Get the range of the available recirculation ports on the system
 *
 * @param dev_id Device id
 * @param start_recirc_port First recirc port index
 * @param end_recirc_port Last recirc port index
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_recirc_port_range_get(bf_dev_id_t dev_id,
                                        uint32_t *start_recirc_port,
                                        uint32_t *end_recirc_port);

/**
 * @brief Get the dev port corresponding to the recirculation port index
 *
 * @param dev_id Device id
 * @param recirc_port Recirculation port index
 * @param dev_port Device port number corresponding to the recirc port index
 *
 * @return Status of the API call
 *
 *  @see Alternative API bf_pm_recirc_devports_get() to get recirc ports.
 */
bf_status_t bf_pm_recirc_port_to_dev_port_get(bf_dev_id_t dev_id,
                                              uint32_t recirc_port,
                                              bf_dev_port_t *dev_port);

/**
 * @brief Set the serdes Tx Equalization Pre parameter
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param tx_pre Serdes Tx Pre Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_pre_set(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int tx_pre);

/**
 * @brief Get the serdes Tx Equalization Pre parameter
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] tx_pre Serdes Tx Pre Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_pre_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *tx_pre);

/**
 * @brief Get the serdes Tx Equalization Pre2 parameter
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] tx_pre2 Serdes Tx Pre2 Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_pre2_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *tx_pre2);

/**
 * @brief Set the serdes Tx Equalization Post parameter
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param tx_post Serdes Tx Post Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_post_set(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int tx_post);

/**
 * @brief Get the serdes Tx Equalization Post parameter
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] tx_post Serdes Tx Post Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_post_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *tx_post);

/**
 * @brief Get the serdes Tx Equalization Post2 parameter
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] tx_post2 Serdes Tx Post2 Emphasis param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_post2_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *tx_post2);

/**
 * @brief Set the serdes Tx Equalization Main parameter
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param tx_attn Serdes Tx Attenuation param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_main_set(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int tx_attn);

/**
 * @brief Get the serdes Tx Equalization Main parameter
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] tx_attn Serdes Tx Attenuation param
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_main_get(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, int *tx_attn);

/**
 * @brief Add a port FSM as a scheduled tasklet to bring up the port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_fsm_init(bf_dev_id_t dev_id,
                                bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Add a port FSM as a scheduled tasklet to bring up the port and
 * initialize the the FSM at UP state.
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_fsm_init_in_up_state(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Run the FSM tasklet. This should be periodically called by the
 * application
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_tasklet_scheduler();

/**
 * @brief Initialize the bf PM module
 *
 * @param void
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_init(void);

/**
 * @brief Set device-wide flag indicating whether or not interrupts should be
 * used for link status monitoring.
 *
 * Note: This API is intended to be called only once at initialization of a
 * device. It is not intended to switch back and forth between modes.
 *
 * @param dev_id Device id
 * @param en Indicates if interrupts should be used for link status monitoring
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_interrupt_based_link_monitoring_set(bf_dev_id_t dev_id,
                                                      bool en);

/**
 * @brief Get device-wide flag indicating whether or not interrupts are used
 * for link status monitoring.
 *
 * @param dev_id Device id
 * @param en Indicates if interrupts are used for link status monitoring
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_interrupt_based_link_monitoring_get(bf_dev_id_t dev_id,
                                                      bool *en);

/**
 * @brief Add and set all the internal ports in MAC lpbk for eligible SKU parts
 *
 * @param dev_id Device id
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_internal_ports_init(bf_dev_id_t dev_id);

/**
 * @brief Return if a particular port is internal (doesn't have serdes)
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param is_internal Indicates if the port internal or not
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_is_port_internal(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bool *is_internal);

/**
 * @brief Set default configuration (passed in by the platforms module) for a
 * serdes slice correponding to a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_cfg_set(bf_dev_id_t dev_id,
                                      bf_pal_front_port_handle_t *port_hdl);

bf_status_t bf_pm_port_traffic_status_get(bf_dev_id_t dev_id,
                                          bf_pal_front_port_handle_t *port_hdl,
                                          bool *there_is_traffic);
/**@set up the rate function
 *
 */
bf_status_t bf_pm_init_rate(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_pm_rate_timer_creat(bf_dev_id_t dev_id, uint32_t period_msecs);
bf_status_t bf_pm_rate_timer_start(bf_dev_id_t dev_id);
bf_status_t bf_pm_rate_timer_check_del(bf_dev_id_t dev_id);

/**
 * @brief Sets persistent port stats control flag.
 *
 * @param dev_id Device id
 * @param enable makes port stats persistent (true) of not (false). Default
 *               value is false.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_persistent_set(bf_dev_id_t dev_id, bool enable);

/**
 * @brief Gets persistent port stats control flag.
 *
 * @param dev_id Device id
 * @param enable returns the current value of the persistent port stats flag.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_persistent_get(bf_dev_id_t dev_id, bool *enable);

/**
 * @brief Clear the FEC counters
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_clear_the_fec_counters(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port);

/**
 * @brief Gets cumulative RS fec corrected and uncorreted block counters
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 * @param hi_ser High symbol error flag (1 = symbols errors exceeds the thresh.)
 * @param align_status FEC align status (1 = all lanes synchronized and aligned)
 * @param fec_corr_cnt Corrected blocks counter
 * @param fec_uncorr_cnt Uncorrected blocks counter
 * @param fec_ser_lane_0 Counter of FEC symbols errors on lane 0.
 * @param fec_ser_lane_1 Counter of FEC symbols errors on lane 1.
 * @param fec_ser_lane_2 Counter of FEC symbols errors on lane 2.
 * @param fec_ser_lane_3 Counter of FEC symbols errors on lane 3.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_get_rs_fec_counters(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           bool *hi_ser,
                                           bool *align_status,
                                           uint32_t *fec_corr_cnt,
                                           uint32_t *fec_uncorr_cnt);
/**
 * @brief Gets cumulative RS fec error counters per lane for Tofino 1 only
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 * @param lane_id lane id
 * @param fec_ser_lane Counter of FEC symbols errors for specified lane.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_get_rs_fec_ser_lane_cnt_tof1(bf_dev_id_t dev_id,
                                                    bf_dev_port_t dev_port,
                                                    uint32_t lane_id,
                                                    uint32_t *fec_ser_lane);
/**
 * @brief Gets cumulative FC fec counters
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 * @param fc_block_lock_status FC FEC lock status flag (1 = locked).
 * @param fc_fec_corr_blk_cnt Corrected blocks counter
 * @param fc_fec_uncorr_blk_cnt Uncorrected blocks counter
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_get_fc_fec_counters(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           bool *fc_block_lock_status,
                                           uint32_t *fc_fec_corr_blk_cnt,
                                           uint32_t *fc_fec_uncorr_blk_cnt);
/**
 * @brief Gets cumulative PCS error counters
 *
 * @param dev_id Device id
 * @param dev_port Device port number corresponding to the front port index
 * @param ber_cnt PCS bad sync header counter
 * @param errored_blk_cnt PCS errored block counter
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_get_pcs_counters(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        uint32_t *ber_cnt,
                                        uint32_t *errored_blk_cnt);
/**
 * @brief Get a particular stat counter for a port into user buffer
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param ctr_type_array Pointer to array holding Counter ids
 * @param ctr_data buffer for the counter
 * @param num_of_ctr number of counter id
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stat_direct_get(bf_dev_id_t dev_id,
                                       bf_pal_front_port_handle_t *port_hdl,
                                       bf_rmon_counter_t *ctr_type_array,
                                       uint64_t *ctr_data,
                                       uint32_t num_of_ctr);
/**
 * @brief Update stats poll interval
 * @param dev_id Device id
 * @param poll_intv_ms Stats polling interval in millisec - minimum 500 msec
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_poll_period_update(bf_dev_id_t dev_id,
                                                uint32_t poll_intv_ms);
/**
 * @brief get stats poll interval
 * @param dev_id Device id
 * @param poll_intv_ms Stats polling interval in millisec - minimum 500 msec
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_stats_poll_period_get(bf_dev_id_t dev_id,
                                             uint32_t *poll_intv_ms);
/**
 * @brief Set the port direction
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param port_dir Direction for the port
 *        (PM_PORT_DIR_DEFAULT, PM_PORT_DIR_TX_ONLY, PM_PORT_DIR_RX_ONLY)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_direction_set(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     bf_pm_port_dir_e port_dir);
/**
 * @brief Get the port direction
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param port_dir Buffer for the port direction
 *        (PM_PORT_DIR_DEFAULT, PM_PORT_DIR_TX_ONLY, PM_PORT_DIR_RX_ONLY)
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_direction_get(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl,
                                     bf_pm_port_dir_e *port_dir);

/**
 * @brief Set the media type connected to the port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param media_type Media type connected to the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_media_type_set(bf_dev_id_t dev_id,
                                      bf_pal_front_port_handle_t *port_hdl,
                                      bf_media_type_t media_type);

/**
 * @brief Set the serdes param (coefficients)
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param serdes_param Serdes params to be configured
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_param_set(bf_dev_id_t dev_id,
                                        bf_pal_front_port_handle_t *port_hdl,
                                        bf_pal_serdes_params_t *serdes_param);

/**
 * @brief Add a port FSM as a scheduled tasklet to wait for down event
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_fsm_set_down_event_state(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl);

/**
 * @brief Get the number of internal ports on the chip depending on the efuse,
 *        excluding the front panel ports.
 * @param dev_id Device id
 * @param num_ports Number of ports on the chip
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_num_internal_ports_get(bf_dev_id_t dev_id,
                                         uint32_t *num_ports);

/**
 * @brief Given a port name, get the corresponding dev_port
 *
 * @param dev_id Device id
 * @param port_str Port Name
 * @param dev_port Device port number
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_str_to_dev_port_get(bf_dev_id_t dev_id,
                                           char *port_str,
                                           bf_dev_port_t *dev_port);
/**
 * @brief Given a dev port, get the corresponding port name
 *
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param port_str Port Name
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_dev_port_to_port_str_get(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           char *port_str);
/**
 * @brief Given a dev id, return true if it tofino2
 *
 * @param dev_id Device id
 *
 * @return True if device-family is tofino2
 */
bool bf_pm_intf_is_device_family_tofino2(bf_dev_id_t dev_id);

/**
 * @brief Given a dev id, return true if it tofino
 *
 * @param dev_id Device id
 *
 * @return True if device-family is tofino
 */
bool bf_pm_intf_is_device_family_tofino(bf_dev_id_t dev_id);

/**
 * @brief Given a dev id, return true if it tofino3
 *
 * @param dev_id Device id
 *
 * @return True if device-family is tofino3
 */
bool bf_pm_intf_is_device_family_tofino3(bf_dev_id_t dev_id);

/**
 * @brief Sets the serdes rx and tx polarity
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param num_entries Sizeof serdes_pol
 * @param serdes_tx_eq Pointer to bf_pal_serdes_polarity_t array
 *
 * @note  API not valid for Tofino
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_polarity_set(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t num_entries,
    bf_pal_serdes_polarity_t *serdes_pol);

/**
 * @brief Sets the serdes TX params
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param num_entries Sizeof serdes_tx_eq
 * @param serdes_tx_eq Pointer to bf_pal_serdes_tx_params_t array
 *
 * @note  bf_pal_serdes_tx_params_t is a Union and is chip dependant
 *        See bf_pal_serdes_tx_eq_params_t for chips supported.
 *
 * @note  API not valid for Tofino
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_eq_params_set(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl,
    uint32_t num_entries,
    bf_pal_serdes_tx_eq_params_t *serdes_tx_eq);

/**
 * @brief Gets the port-level serdes TX status
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param tx_ready Pointer to tx_ready flag to be filled
 *
 * @note Ensures all TX serdes lanes within a port are in ready state
 *
 * @note  API not valid for Tofino. Valid after port-enable
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_tx_ready_get(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           bool *tx_ready);

/**
 * @brief Gets the port-level serdes RX status
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param rx_ready Pointer to rx_ready flag to be filled
 *
 * @note Ensures all RX serdes lanes within a port are in ready state
 *
 * @note  API not valid for Tofino. Valid after port-enable
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_rx_ready_get(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           bool *rx_ready);

/**
 * @brief Sets the port-level serdes RX ready for bringup state
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param enable Enables 1: Enables the DFE tuning, 0 disables the DFE
 *
 * @note DFE is valid only when auto-neg is OFF.
 *
 * @note  API not valid for Tofino.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_serdes_rx_ready_for_bringup_set(
    bf_dev_id_t dev_id, bf_pal_front_port_handle_t *port_hdl, bool enable);

/**
 * @brief Setup PRBS on multiple ports on Tofino2. This sets the PRBS on all the
 *ports
 * in the list
 *
 * @param dev_id Device id
 * @param port_hdl_list Front panel port number list
 * @param len Number of ports in the list
 * @param prbs_mode Mode of the PRBS test
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_mode_set(bf_dev_id_t dev_id,
                                     bf_pal_front_port_handle_t *port_hdl_list,
                                     uint32_t len,
                                     bf_port_prbs_mode_t prbs_mode);

/**
 * @brief Cleanup PRBS on multiple ports. This cleans up the PRBS only on all
 * the ports in the list
 *
 * @param dev_id Device id
 * @param port_hdl_list Front panel port number list
 * @param len Number of ports in the list
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_mode_clear(
    bf_dev_id_t dev_id,
    bf_pal_front_port_handle_t *port_hdl_list,
    uint32_t len);

/**
 * @brief Get the PRBS stats on a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param stats prbs stats
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_prbs_mode_stats_get(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           bf_port_sds_prbs_stats_t *stats);

/**
 * @brief Get the eye-val on a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port numeye_val
 * @param eye_val eye-val count
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_eye_val_get(bf_dev_id_t dev_id,
                                   bf_pal_front_port_handle_t *port_hdl,
                                   bf_port_eye_val_t *eye);

/**
 * @brief Get the BER on a port
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param ber BER count
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_ber_get(bf_dev_id_t dev_id,
                               bf_pal_front_port_handle_t *port_hdl,
                               bf_port_ber_t *stats);

/**
 * @brief Gets the configured admin state of the port
 *
 * @param[in] dev_id Device id
 * @param[in] port_hdl Front panel port number
 * @param[out] is_enabled Configured admin state of the port
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_is_enabled(bf_dev_id_t dev_id,
                                  bf_pal_front_port_handle_t *port_hdl,
                                  bool *is_enabled);

/**
 * @brief Gets the first front panel port index on the system
 *
 * @param[in] dev_id Device id
 * @param[out] fp_idx First front panel port index on the system
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_front_port_index_get_first(bf_dev_id_t dev_id,
                                             uint32_t *fp_idx);

/**
 * @brief Gets the next front panel port index on the system
 *
 * @param[in] dev_id Device id
 * @param[in] curr_fp_idx Current front panel port index on the system
 * @param[out] next_fp_idx Next front panel port index on the system
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_front_port_index_get_next(bf_dev_id_t dev_id,
                                            uint32_t curr_fp_idx,
                                            uint32_t *next_fp_idx);
/**
 * @brief Gets the recirc dev-ports available on the device.
 *
 *  Given an pointer to an array[30] and dev-id, returns recirc dev-ports
 *  and maximum of recirc-port on that dev-id. Note max dev-ports returned
 *  will be 27 - upto tofino2.
 *
 *  @note: Both the Ethernet MAC CPU ports and PCIe CPU ports can be used for
 *  recirculation if CPU connectivity is not required. However, since they are
 *  configured for CPU connectivity by default, and there are dedicated APIs to
 *  retrieve them, the CPU ports are not returned by this API.
 *
 **  For Tofino-3: Recirc ports are
 *      Pipe == 0:
 *         port 6 is recirc
 *      Pipe != 0:
 *         0, 2, 4, 6 are recirc ports
 *
 *  For Tofino-2: Recirc ports are
 *      Pipe == 0:
 *         Chan 1, 6 and 7
 *      Pipe != 0:
 *         0-7 are recirc ports
 *
 *  For Tofino:
 *      Four Pipes system, recirc ports are
 *         Pipe-0 : 68, 69, 70, 71
 *         Pipe-1 : 192, 193, 194, 195, 196, 197, 198, 199
 *         Pipe-2 : 324, 325, 326, 327
 *         Pipe-3 : 448, 449, 450, 451, 452, 453, 454, 455
 *      Two Pipes system, recirc ports are
 *         Pipe-0 : 68, 69, 70, 71
 *         Pipe-1 : 196, 197, 198, 199
 *
 * @param[in] dev_id Device id
 * @param[out] recirc_devport_list Pointer to an array[30]
 *
 * @return  On success, returns maximum number of recirc ports
 * @return  On error, returns 0
 *
 * @see    Alternative API to get recirc-ports
 *         bf_pm_recirc_port_to_dev_port_get(),
 *         bf_pm_recirc_port_range_get()
 */
uint32_t bf_pm_recirc_devports_get(bf_dev_id_t dev_id,
                                   uint32_t *recirc_devport_list);

/**
 * @brief Set max number of block errors allowed to link up.
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param maximum nunber of block errors allowed to link up. Set to 0 to disable
 *        block error validation.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_link_up_max_err_set(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           uint32_t max_errors);

/**
 * @brief Get max number of block errors allowed to link up.
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param ref to a variable to return the maximum nunber of block errors allowed
 *            to link up.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_link_up_max_err_get(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           uint32_t *max_errors);
/**
 * @brief Set link up debounce for the specified port.
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param debounce_value link up debounce in FSM cycles.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_debounce_thresh_set(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           uint32_t debounce_value);

/**
 * @brief Get link up debounce for the specified port.
 *
 * @param dev_id Device id
 * @param port_hdl Front panel port number
 * @param debounce_value link up debounce in FSM cycles.
 *
 * @return Status of the API call
 */
bf_status_t bf_pm_port_debounce_thresh_get(bf_dev_id_t dev_id,
                                           bf_pal_front_port_handle_t *port_hdl,
                                           uint32_t *debounce_value);

bf_status_t bf_pm_init_platform(void);

/**
 * @brief Get the AN advertisement speed for the port
 *
 * @param dev_id Device id
 * @param dev_port Device port number
 * @param speed Speed of the port
 * @param n_lanes Lane number of the port
 *
 * @return AN advertisement speed
 */
bf_an_adv_speeds_t bf_pm_get_an_adv_speed(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_port_speed_t speed,
                                          uint32_t n_lanes);
#endif
