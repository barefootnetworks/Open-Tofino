/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _BF_PAL_TYPES_H
#define _BF_PAL_TYPES_H

// File includes
#include <bf_types/bf_types.h>

// Defined Macros
#define MAX_PORT_HDL_STRING_LEN 100
#define MAX_CHAN_PER_PORT 8
#define MAX_LANES_PER_PORT MAX_CHAN_PER_PORT
#define MAX_TF3_SERDES_LANES_PER_CHAN 2
#define MAX_TF_SERDES_LANES_PER_CHAN MAX_CHAN_PER_PORT

/**
 * Identifies the admin state of a port in the system
 */
typedef enum bf_pm_port_admin_state_e {
  PM_PORT_DISABLED = 0,
  PM_PORT_ENABLED = 1
} bf_pm_port_admin_state_e;

/**
 * Identifies the oper status of a port in the system
 */
typedef enum bf_pm_port_oper_status_e {
  PM_PORT_DOWN = 0,
  PM_PORT_UP = 1
} bf_pm_port_oper_status_e;

/**
 * Identifies if a port should autonegotiate or not
 */
typedef enum bf_pm_port_autoneg_policy_e {
  /** PM_AN_DEFAULT meant to be used by BSP. Applications should NOT set this
   * value.
   * By default, AN will be enabled/disabled based on Module plugged-in.
   * For copper it will be enabled.
   */
  PM_AN_DEFAULT = 0,

  /** Applications can use AN_FORCE to enable/disable the auto-neg
   */
  PM_AN_FORCE_ENABLE = 1,
  PM_AN_FORCE_DISABLE = 2,
  PM_AN_MAX = 3
} bf_pm_port_autoneg_policy_e;

/**
 * Identifies if a port should autonegotiate KR mode or not (CR_MODE)
 */
typedef enum bf_pm_port_kr_mode_policy_e {
  /** PM_KR_DEFAULT meant to be used by BSP. Applications should NOT set this
   * value.
   * By default, KR will be enabled/disabled based on port, CPU port=KR,
   * others not
   */
  PM_KR_DEFAULT = 0,

  /** Applications can use KR_FORCE to enable/disable the auto-neg
   */
  PM_KR_FORCE_ENABLE = 1,
  PM_KR_FORCE_DISABLE = 2,
  PM_KR_MAX = 3
} bf_pm_port_kr_mode_policy_e;

/**
 * Identifies if a ports serdes are AC coupled or DC coupled on the board
 */
typedef enum bf_pm_port_term_mode_e {
  PM_TERM_MODE_DEFAULT = 0,

  // value is determined by board layout and must be set appropriately
  // by the application.
  // TOF1
  PM_TERM_MODE_GND = 1,
  PM_TERM_MODE_AVDD = 2,
  PM_TERM_MODE_FLOAT = 3,
  // TOF2
  PM_TERM_MODE_AC = 4,
  PM_TERM_MODE_DC = 5,
  PM_TERM_MODE_MAX,
} bf_pm_port_term_mode_e;

/**
 * Identifies if a port/lane should use 1/(1+D) precoding or not
 */
typedef enum bf_pm_port_precoding_policy_e {
  /** Applications can specify whether to use precoding for PAM4 links or not.
   * Enable precoding for all PAM4 links with Copper modules.
   *
   * The precoding setting applies to individual lanes and spearately
   * in each direction (Tx or Rx).
   */
  PM_PRECODING_DISABLE = 0,
  PM_PRECODING_ENABLE = 1,

  /** NOTE: For internal use only. Applications should not use this.
   * Forced via ucli to overide values set by applications for debug
   * purposes.
   *
   * To overide application settings, do -
   *    - port-dis <port_str>
   *    - use pc-tx-set or pc-rx-set ucli command
   *    - port-enb <port_str>
   *
   *  To clear overide, do -
   *    - port-dis <port_str>
   *    - use pc-tx-clear or pc-rx-clear ucli command
   *    - port-enb <port_str>
   */
  PM_PRECODING_FORCE_DISABLE = 3,
  PM_PRECODING_FORCE_ENABLE = 4,

  /** Initial state */
  PM_PRECODING_INVALID = 0xFFFF,
} bf_pm_port_precoding_policy_e;

/**
 * Identifies if a port-directions
 */
typedef enum bf_pm_port_dir_e {
  PM_PORT_DIR_DEFAULT = 0,  // both rx and tx
  PM_PORT_DIR_TX_ONLY = 1,
  PM_PORT_DIR_RX_ONLY = 2,
  PM_PORT_DIR_DECOUPLED = 3,
  PM_PORT_DIR_MAX
} bf_pm_port_dir_e;

/**
 * Identifies NRZ or PAM4
 */
typedef enum bf_pm_encoding_type_ {
  BF_ENCODING_NRZ = 0,
  BF_ENCODING_PAM4,
} bf_pm_encoding_type_t;

/**
 * Identifies a front panel port on the system
 */
typedef struct bf_pal_front_port_handle_t {
  uint32_t conn_id;
  uint32_t chnl_id;
} bf_pal_front_port_handle_t;

/**
 * Identifies all the information about a specific front panel port. This
 * information is derived from the platforms modules
 */
typedef struct bf_pal_front_port_info_t {
  bf_dev_id_t dev_id;
  bf_pal_front_port_handle_t port_hdl;
  char port_str[MAX_PORT_HDL_STRING_LEN];
  uint32_t log_mac_id;
  uint32_t log_mac_lane;
} bf_pal_front_port_info_t;

/**
 * Identifies all the information to configure the serdes. This information
 * is derived from the platforms modules
 */
typedef struct bf_pal_serdes_info_t {
  bool tx_inv;
  bool rx_inv;
  int tx_attn;
  int tx_pre;
  int tx_post;
} bf_pal_serdes_info_t;

/**
 * Identifies the MAC lane to Serdes lane mappings based on the board layout.
 * This information is derived from the platforms modules
 */
typedef struct bf_pal_mac_to_serdes_lane_map_t {
  uint32_t tx_lane;  // Indicates which TX serdes slice this MAC lane is
                     // connected to
  uint32_t rx_lane;  // Indicates which RX serdes slice this MAC lane is
                     // connected to
} bf_pal_mac_to_serdes_lane_map_t;

/**
 * Indentifies the information configured for the port. This information
 * is passed to the platforms module during the registered callbacks.
 * Information is valid only AFTER the port has been added successfully
 */
typedef struct bf_pal_front_port_cb_cfg_t {
  bf_port_speed_t speed_cfg;  // Speed configured for the port
  int num_lanes;              // Number of MAC lanes consumed by the port
  bool is_an_on;              // Is port Auto-Negotiating?
  bf_pm_port_dir_e port_dir;  // Port direction configured
  bf_pm_encoding_type_t enc_type;
} bf_pal_front_port_cb_cfg_t;

/**
 * Identifies information required to configure the serdes coefficients.
 * Needs to be set before port-enable.
 * Any settings done via Json confiuration file will be overwritten.
 */
typedef struct bf_pal_serdes_params_t {
  /* tx coefficients */
  int tx_attn;
  int tx_pre;
  int tx_post;
} bf_pal_serdes_params_t;

/**
 * Serdes polarity per logical lane
 */
typedef struct bf_pal_serdes_polarity_ {
  bool rx_inv;
  bool tx_inv;
} bf_pal_serdes_polarity_t;

/**
 * Serdes tx coefficients for tof2
 */
typedef struct bf_pal_serdes_tx_eq_params_tof2_ {
  int32_t tx_main;
  int32_t tx_pre1;
  int32_t tx_pre2;
  int32_t tx_post1;
  int32_t tx_post2;
} bf_pal_serdes_tx_eq_params_tof2_t;

/**
 * Serdes tx coefficients for all chips, excluding tofino
 */
typedef struct bf_pal_serdes_tx_eq_params_ {
  union tx_eq_union_ {
    bf_pal_serdes_tx_eq_params_tof2_t tof2[MAX_CHAN_PER_PORT];
  } tx_eq;
} bf_pal_serdes_tx_eq_params_t;

#endif
