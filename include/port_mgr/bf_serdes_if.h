/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef BF_SERDES_IF_H_INCLUDED
#define BF_SERDES_IF_H_INCLUDED

/**
 * @file bf_serdes_if.h
 *
 * @brief BF Drivers APIs for Tofino SerDes management.
 *
 */

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
//  Enumerated Types
//----------------------------------------------------------------------------

/** @brief   Enumeration of supported Tofino serdes access modes.
 */
typedef enum {
  BF_SDS_ACCESS_SBUS = 0,
  BF_SDS_ACCESS_CORE,
} bf_serdes_access_method_e;

/** @brief SerDes Line Rates
 */
typedef enum {
  BF_SDS_LINE_RATE_1p25G = 0, /**<  1.25Gbps     */
  BF_SDS_LINE_RATE_2p5G = 1,  /**<  2.5Gbps      */
  BF_SDS_LINE_RATE_10G = 2,   /**< 10.3125Gbps   */
  BF_SDS_LINE_RATE_25G = 3,   /**< 25.78Gbps     */
} bf_sds_line_rate_mode_t;

/** @brief SerDes Loopback Modes
 */
typedef enum {
  BF_SDS_LB_OFF = 0,          /**< Normal Operation, no loopback */
  BF_SDS_LB_SER_TX_TO_RX = 1, /**< Serial TX to RX near end loopback */
  BF_SDS_LB_PAR_RX_TO_TX = 2, /**< Parallel RX to TX far end loopback */
} bf_sds_loopback_t;

/** @brief TX PLL REFCLK Source
 */
typedef enum {
  BF_SDS_TX_PLL_ETH_REFCLK = 0, /**< Source from external ETH_REFCLK */
  BF_SDS_TX_PLL_ALT_REFCLK = 1, /**< Source from external ALT_REFCLK */
  BF_SDS_TX_PLL_RXCLK = 2,      /**< Source from RX Recovered clock */
  BF_SDS_TX_PLL_OFF = 3,        /**< Debug Only. Tie clock input to GND */
  BF_SDS_TX_PLL_PCIECLK = 4,    /**< Debug Only. Source from PCIe clock */
} bf_sds_tx_pll_clksel_t;

/** @brief Management Logic Clock Source
 */
typedef enum {
  BF_SDS_MGMT_CLK_REFCLK = 0,       /**< Source from external REFCLK */
  BF_SDS_MGMT_CLK_REFCLK_DIV2 = 1,  /**< Debug Only. REFCLK/2 */
  BF_SDS_MGMT_CLK_PCIECLK = 2,      /**< Source from external PCIE CLK */
  BF_SDS_MGMT_CLK_PCIECLK_DIV2 = 3, /**< Debug Only. PCIE CLK/2 */
} bf_sds_mgmt_clk_clksel_t;

/** @brief RX Termination Mode
 */
typedef enum {
  BF_SDS_RX_TERM_GND = 0,   /**< Terminate 50 Ohm to GND */
  BF_SDS_RX_TERM_AVDD = 1,  /**< Terminate 50 Ohm to AVDD */
  BF_SDS_RX_TERM_FLOAT = 2, /**< Terminate 100 Ohm between P/N */
} bf_sds_rx_term_t;

/** @brief RX EQ Calibration Command
 */
typedef enum {
  BF_SDS_RX_ICAL_PCAL = 0,       /**< Run iCal follow by pCal_once */
  BF_SDS_RX_ICAL_NO_PCAL = 1,    /**< (Debug) Run iCal without pCal */
  BF_SDS_RX_PCAL_ONCE = 2,       /**< Run pCal once */
  BF_SDS_RX_PCAL_CONT_START = 3, /**< Start pCal continuous mode */
  BF_SDS_RX_PCAL_CONT_STOP = 4,  /**< Stop  pCal */
  BF_SDS_RX_PCAL_RR_DISABLE = 5, /**< Disable Round Robin participation */
  BF_SDS_RX_PCAL_RR_ENABLE = 6,  /**< Enable  Round robin participation */
  BF_SDS_RX_CAL_SLICER_ONLY = 7, /**< Run slicer cal only */
} bf_sds_rx_cal_mode_t;

/** @brief RX Eye Measurement Mode
 */
typedef enum {
  BF_SDS_RX_EYE_MEAS_HEIGHT = 0, /**< Measure Eye Height */
  BF_SDS_RX_EYE_MEAS_WIDTH = 1,  /**< Measure Eye Width */
} bf_sds_rx_eye_meas_mode_t;

/** @brief RX Eye Measurement BER
 */
typedef enum {
  BF_SDS_RX_EYE_BER_1E6 = 0, /**< Receive 3e6 bits with up to 1 error */
  BF_SDS_RX_EYE_BER_1E9 = 1, /**< Receive 3e9 bits with up to 1 error */
} bf_sds_rx_eye_meas_ber_t;

/** @brief Pattern Generator/Checker Pattern Select
 */
typedef enum {
  BF_SDS_PAT_PATSEL_OFF = 0,    /**< Select Core Data */
  BF_SDS_PAT_PATSEL_PRBS7 = 1,  /**< Select PRBS-7   Pattern */
  BF_SDS_PAT_PATSEL_PRBS9 = 2,  /**< Select PRBS-9   Pattern */
  BF_SDS_PAT_PATSEL_PRBS11 = 3, /**< Select PRBS-11  Pattern */
  BF_SDS_PAT_PATSEL_PRBS15 = 4, /**< Select PRBS-15  Pattern */
  BF_SDS_PAT_PATSEL_PRBS23 = 5, /**< Select PRBS-23  Pattern */
  BF_SDS_PAT_PATSEL_PRBS31 = 6, /**< Select PRBS-31  Pattern */
  BF_SDS_PAT_PATSEL_FIXED = 7,  /**< Select 80b Fixed Pattern */
} bf_sds_pat_patsel_t;

/** @brief INT 0x2B bit 15 defs
 */
typedef enum {
  BF_SDS_RR_PCAL_DISABLE = 0,
  BF_SDS_RR_PCAL_ENABLE = 1,
} bf_sds_rr_pcal_mode_t;

/** @brief PMRO temperature sensor channel defs
 */
typedef enum {
  BF_SDS_MAIN_TEMP_SENSOR_CH = 0,
  BF_SDS_REMOTE_TEMP_SENSOR_0_CH,
} bf_sds_temp_sensor_channel_t;

/** @brief PMRO voltage sensor channel defs
 */
typedef enum {
  BF_SDS_MAIN_VOLT_SENSOR_CH = 0,
  BF_SDS_REMOTE_VOLT_SENSOR_0_CH,
} bf_sds_voltage_sensor_channel_t;

/** @brief Serdes reset types
 */
typedef enum {
  BF_SDS_RESET_NODE = 0,
  BF_SDS_RESET_MICROPROCESSOR,
  BF_SDS_RESET_NODE_AND_MICROPROCESSOR,
} bf_sds_reset_type_t;

/** @brief Tofino-specific Serdes DFE control
 * Used to customize DFE step of bring-up FSM
 */
typedef enum {
  BF_SDS_TOF_DFE_CTRL_DEFAULT = 0,
  BF_SDS_TOF_DFE_CTRL_ICAL = BF_SDS_TOF_DFE_CTRL_DEFAULT,
  BF_SDS_TOF_DFE_CTRL_PCAL = 0x0001,
  BF_SDS_TOF_DFE_CTRL_SEEDED_HF = 0x0002,
  BF_SDS_TOF_DFE_CTRL_SEEDED_LF = 0x0004,
  BF_SDS_TOF_DFE_CTRL_SEEDED_DC = 0x0008,
  BF_SDS_TOF_DFE_CTRL_FIXED_HF = 0x0010,
  BF_SDS_TOF_DFE_CTRL_FIXED_LF = 0x0020,
  BF_SDS_TOF_DFE_CTRL_FIXED_DC = 0x0040,
} bf_sds_tof_dfe_ctrl_t;

/** @brief Tofino-specific loopbandwidth settings
 * Only the below values have been tested by Barefoot
 * Any other setting may be used but Barefoot cannot
 * guarantee proper operation.
 */
typedef enum {
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_DEFAULT = 0,
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_1MHZ = 0x503,
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_3MHZ = 0x511,
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_9MHZ = 0x51b,
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_A0_DEFAULT = BF_SDS_TOF_TX_LOOP_BANDWIDTH_3MHZ,
  BF_SDS_TOF_TX_LOOP_BANDWIDTH_B0_DEFAULT = BF_SDS_TOF_TX_LOOP_BANDWIDTH_1MHZ,
} bf_sds_tof_tx_loop_bandwidth_t;

//----------------------------------------------------------------------------
//  Structure Types
//----------------------------------------------------------------------------

/** @brief Lane Status
 */
typedef struct {
  bf_sds_line_rate_mode_t line_rate; /**< Configured line rate mode */
  bool tx_en;         /**< 1: TX path enabled.                0: Disabled */
  bool tx_drv_en;     /**< 1: TX output enabled.              0: Disabled */
  bool tx_pll_lock;   /**< 1: TX PLL locked.                  0: Unlocked */
  bool rx_pll_lock;   /**< 1: RX PLL locked.                  0: Unlocked */
  bool rx_sigdet;     /**< 1: RX signal detected              0: No signal */
  bool kr_en;         /**< 1: KR mode enabled.                0: Disabled */
  bool kr_lt_success; /**< 1: KR link training successful.    0: Failed */
  bool kr_an_success; /**< 1: KR auto negotiation successful. 0: Failed */
  int rx_eye_adp;     /**< Eye height reported by adaptation engine */
} bf_sds_lane_status_t;

/** @brief TX Driver Status
 */
typedef struct {
  bool tx_en;     /**< 1: TX path enabled.            0: Disabled */
  bool tx_drv_en; /**< 1: TX output enabled.          0: Disabled */
  bool tx_inv;    /**< 1: invert P/N.                 0: no inversion */
  int amp_main;   /**< Main-cursor amplitude  */
  int amp_post;   /**< Post-cursor amplitude  */
  int amp_pre;    /**< Pre-cursor  amplitude  */
} bf_sds_tx_drv_status_t;

/** @brief RX EQ Status
 */
typedef struct {
  bool cal_done;    /**< 1: Completed. 0: In progress                   */
  bool cal_good;    /**< 1: Good Eye. 0: Poor Eye. -1: Bad Eye          */
  int cal_eye;      /**< (mVppd) Eye height observed by cal algorithm   */
  int ctle_dc;      /**< DC boost (0..255) */
  int ctle_lf;      /**< Peaking filter low frequency gain (0..15) */
  int ctle_hf;      /**< Peaking filter high frequency gain (0..15) */
  int ctle_bw;      /**< Peaking filter peakign frequency (0..15) */
  int dfe_taps[16]; /**< Array of DFE Taps (each -127..127) */
} bf_sds_rx_eq_status_t;

typedef struct {
  uint32_t tx_phy_lane_id;
  uint32_t rx_phy_lane_id;
  bool tx_phy_pn_swap;
  bool rx_phy_pn_swap;
  uint32_t tx_attn;
  uint32_t tx_pre;
  uint32_t tx_post;
} bf_sds_lane_info_t;

//----------------------------------------------------------------------------
//   Function Declaration
//----------------------------------------------------------------------------

/** @brief Set lane map and polarity (based on board configuration)
 *
 * @param[in] dev_id: Device identifier
 * @param[in] mac_block: Logical MAC block index (quad)
 * @param[in] tx_chnl  : Tx MAC channel, 0-3
 * @param[in] rx_chnl  : Rx MAC channel, 0-3
 * @param[in] lane_info: Pointer to board config for this quad/chnl
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_lane_info_set(bf_dev_id_t dev_id,
                                    bf_mac_block_id_t mac_block,
                                    uint32_t tx_chnl,
                                    uint32_t rx_chnl,
                                    bf_sds_lane_info_t *lane_info);

/** @brief Set SerDes Management Interface Clock Source
 *
 * Set the clock source for the management interface.
 *
 * The clock source can be either the 156.25MHz ETH_REFCLK or the
 * 100MHz PCIE_REFCLK.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  clk_src  : Clock source selector
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_clksel_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bf_sds_mgmt_clk_clksel_t clk_src);

/** @brief Get SerDes Management Interface Clock Source
 *
 * Get the clock source for the management interface.
 *
 * The clock source can be either the 156.25MHz ETH_REFCLK or the
 * 100MHz PCIE_REFCLK.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out]  clk_src  : Clock source selector
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_clksel_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bf_sds_mgmt_clk_clksel_t *clk_src);

/** @brief Set SerDes Management Bus Acess Method
 *
 * SerDes can be accessed either through SerDes Bus (SBUS) or through the
 * switch core.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  method   : BF_SDS_ACCESS_SBUS or BF_SDS_ACCESS_CORE
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_access_method_set(bf_dev_id_t dev_id,
                                             bf_serdes_access_method_e method);

/** @brief Get SerDes Management Bus Acess Method
 *
 * SerDes can be accessed either through SerDes Bus (SBUS) or through the
 * switch core.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[out] method  : BF_SDS_ACCESS_SBUS or BF_SDS_ACCESS_CORE
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_access_method_get(bf_dev_id_t dev_id,
                                             bf_serdes_access_method_e *method);

/** @brief Set SerDes Management Interface Broadcast Mode
 *
 * Set SerDes management interface broadcast mode.  If enabled, the specified
 * SerDes lane will respond to broadcast write commands.
 *
 * Broadcast mode is only availabe if bus access is set to SerDes Bus
 * (BF_SDS_ACCESS_SBUS)
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  bcast_en : 1: Broadcast Enable. 0: Broadcast Disable
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_bcast_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool tx_dir,
                                     bool bcast_en);

/** @brief Get SerDes Management Interface Broadcast Mode
 *
 * Get SerDes management interface broadcast mode.  If enabled, the specified
 * SerDes lane will respond to broadcast write commands.
 *
 * Broadcast mode is only availabe if bus access is set to SerDes Bus
 * (BF_SDS_ACCESS_SBUS)
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] bcast_en : 1: Broadcast Enable. 0: Broadcast Disable
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_mgmt_bcast_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool tx_dir,
                                     bool *bcast_en);

/* This is an internal / debug only function. */
bf_status_t bf_serdes_mgmt_reg_set(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   int lane,
                                   bool tx_dir,
                                   int reg,
                                   uint32_t data);

/* This is an internal / debug only function. */
bf_status_t bf_serdes_mgmt_reg_get(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   int lane,
                                   bool tx_dir,
                                   int reg,
                                   uint32_t *data);

/* This is an internal / debug only function. */
bf_status_t bf_serdes_mgmt_uc_int(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  int lane,
                                  bool tx_dir,
                                  int interrupt,
                                  uint32_t int_data,
                                  uint32_t *rtn_data);

/** @brief Set Transmit and Receive lane mapping in a port
 *
 * Configures the port (quad lane) based lane mapper to map physical lanes
 * to logical lanes.
 *
 * @param[in]  dev_id    : Device identifier
 * @param[in]  mac_block : MAC block number
 * @param[in]  lane_map  : Pointer to the lane map
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_port_tof1_lane_map_set(bf_dev_id_t dev_id,
                                      bf_mac_block_id_t mac_block,
                                      bf_mac_block_lane_map_t *lane_map);

/** @brief Get Transmit and Receive lane mapping in a port
 *
 * Configures the port (quad lane) based lane mapper to map physical lanes
 * to logical lanes.
 *
 * @param[in]  dev_id    : Device identifier
 * @param[in]  mac_block : MAC block number
 * @param[in]  lane_map  : Pointer to return the lane map
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_port_lane_map_get(bf_dev_id_t dev_id,
                                 bf_mac_block_id_t mac_block,
                                 bf_mac_block_lane_map_t *lane_map);

/** @brief Set RX EQ Periodic Calibration Round Robin Limit
 *
 * Sets the number of lanes across the entire device that will run RX EQ
 * fine tuning concurrently.
 *
 * @param[in]  dev_id   : Device identifier
 *
 * @param[in]  fine_tune_lane_cnt : Number of lanes running fine tuning (2..32)
 * for the whole device. Use even numbers only.
 * If odd number, -1 will be applied first.
 * If out of range, default of 8 will be used.
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_dev_rx_eq_cal_rr_set(bf_dev_id_t dev_id, int fine_tune_lane_cnt);

/** @brief Get RX EQ Periodic Calibration Round Robin Limit
 *
 * Gets the number of lanes across the entire device that will run RX EQ
 * fine tuning concurrently.
 *
 * @param[in]  dev_id   : Device identifier
 *
 * @param[out] fine_tune_lane_cnt : Number of lanes running fine tuning (2..32)
 * for the whole device.
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_dev_rx_eq_cal_rr_get(bf_dev_id_t dev_id,
                                    int *fine_tune_lane_cnt);

/** @brief Set TX PLL REFCLK Source
 *
 *  Sets the reference clock source for the TX PLL.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  clk_source   : TX PLL clock source
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_pll_clksel_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_tx_pll_clksel_t clk_source);

/** @brief Get TX PLL REFCLK Source
 *
 *  Gets the reference clock source for the TX PLL.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] clk_source   : TX PLL clock source
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_pll_clksel_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_tx_pll_clksel_t *clk_source);

/** @brief Run SerDes Initialization
 *
 * Configures TX and RX sub-blocks inside the SerDes lane to bring the SerDes
 * to normal operation state.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  line_rate    : Line rate for this lane
 * @param[in]  init_rx      : if 1, initialize RX blocks
 * @param[in]  init_tx      : If 1, initialize TX blocks
 * @param[in]  tx_drv_en    : Enable TX driver after initialization
 * @param[in]  tx_phase_cal : TX data clock domain crossing clock phase cal
 * Needs to run once at start up
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_lane_init_run(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_line_rate_mode_t line_rate,
                                    bool init_rx,
                                    bool init_tx,
                                    bool tx_drv_en,
                                    bool phase_cal);

/** @brief Get TX PLL Locked
 *
 * Checks if TX PLL has successfully calibrated and frequency locked
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] locked       : 1 if PLL locked.  0 if not locked
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_pll_lock_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bool *locked);

/** @brief Get RX CDR Locked
 *
 * Checks if RX CDR has successfully calibrated and frequency locked
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] locked       : 1 if CDR locked.  0 if not locked
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_cdr_lock_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bool *locked);

/** @brief Get RX and Tx PLL Locked status
 *
 * Checks if RX CDR has successfully calibrated and frequency locked
 * Checks if TX PLL has successfully calibrated and frequency locked
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] rx_locked       : 1 if RX CDR locked.  0 if not locked
 * @param[out] tx_locked       : 1 if TX PLL locked.  0 if not locked
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_and_tx_lock_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane,
                                         bool *rx_locked,
                                         bool *tx_locked);

/** @brief Get TX PLL Status
 *
 * Get detailed TX PLL status information including PLL lock, line rate
 * divider and PLL frequency
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] locked       : 1 if PLL locked.  0 if not locked
 * @param[out] div          : PLL feedback divider
 * @param[out] freq         : Frequency in MHz
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_pll_status_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bool *locked,
                                        int *div,
                                        int *freq);

/** @brief Get RX CDR Status
 *
 * Get detailed RX CDR status information including PLL lock, line rate
 * divider and CDR frequency
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] locked       : 1 if CDR locked.  0 if not locked
 * @param[out] div          : CDR feedback divider
 * @param[out] freq         : Frequency in MHz
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_cdr_status_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bool *locked,
                                        int *div,
                                        int *freq);

/** @brief Set SerDes Loopback Mode
 *
 * Selects between normal mode (no loopback), TX-to-RX serial (near end)
 * loopback and RX-to-TX parallel (far end) loopback modes.
 *
 * For RX-to-TX parallel loopback, this function will call
 * bf_serdes_tx_pll_clksel_set() to select RX recovered clock (RXCLK)
 * as reference clock.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  loopback_mode : SerDes loopback mode
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_pll_clksel_set(), bf_serdes_tx_pll_clksel_get()
 *
 */
bf_status_t bf_serdes_lane_loopback_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_loopback_t loopback_mode);

/** @brief Get SerDes Loopback Mode
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] loopback_mode : SerDes loopback mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_lane_loopback_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_loopback_t *loopback_mode);

/** @brief Get Lane Status
 *
 * Quick overview of key lane health status
 *
 * @param[in]  dev_id       : Device identifier
 * @param[in]  dev_port     : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] lane_status  : Lane Status Info
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_lane_status_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bf_sds_lane_status_t *lane_status);

/** @brief Set Transmit Path Enable
 *
 * This function sets the TX path enable which also functions as a reset.
 * Some TX settings become effect when TX state goes from Disable to Enable
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_en    : 1: TX path enabled. 0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_en_set(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                bool tx_en);

/** @brief Get Transmit Path Enable
 *
 * This function gets the TX path enable which also functions as a reset.
 * Some TX settings become effect when TX state goes from Disable to Enable
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] en       : 1: TX path enabled. 0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_en_get(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                bool *tx_en);

/** @brief Set Transmit Output Enable
 *
 * This function sets the TX Output enable.  When disabled, output pins
 * P/N will drive to AVDD
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_drv_en    : 1: TX Output enabled.  0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_en_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool tx_drv_en);

/** @brief Get Transmit Output Enable
 *
 * This function gets the TX Output enable.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] en       : 1: TX Output enabled.  0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_en_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool *tx_drv_en);

/** @brief Set Transmit Output Polarity Inversion
 *
 * This function sets the TX output polarity inversion.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_inv   : 1: invert P/N. 0: no inversion
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_inv_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool tx_inv);

/** @brief Set Transmit Output Polarity Inversion
 *
 * This function sets the TX output polarity inversion.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_inv   : 1: invert P/N. 0: no inversion
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_inv_set_allow_unassigned(bf_dev_id_t dev_id,
                                                      bf_dev_port_t dev_port,
                                                      int lane,
                                                      bool tx_inv);

/** @brief Get Transmit Output Polarity Inversion
 *
 * This function gets the TX output polarity inversion.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] inv      : 1: invert P/N. 0: no inversion
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_inv_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool *tx_inv);

/** @brief Valid Check for Transmit EQ attenuation setting
 *
 * This funciton checks to see if the specified TX EQ attenuation settings
 * are valid.
 *
 * @param[in]  attn_main    : Main cursor setting (  0 to 23)
 * @param[in]  attn_post    : Post cursor setting (-31 to 31)
 * @param[in]  attn_pre     : Pre  cursor setting (-31 to 31)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_attn_is_valid(int attn_main,
                                           int attn_post,
                                           int attn_pre);

/** @brief Set Transmit EQ based on Attenuation
 *
 * This function allows users to set transmit EQ attenuation settings directly.
 *
 * Range limits:
 *
 *  - attn_main
 *      - <= 23 for general applications
 *      - <= 16 for KR applications
 *  - attn_pre + attn_main + attn_post <= 32
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  attn_main    : Main cursor setting (  0 to 23)
 * @param[in]  attn_post    : Post cursor setting (-31 to 31)
 * @param[in]  attn_pre     : Pre  cursor setting (-31 to 31)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_amp_set()
 */
bf_status_t bf_serdes_tx_drv_attn_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int attn_main,
                                      int attn_post,
                                      int attn_pre);

/** @brief Set Transmit EQ based on Attenuation
 *
 * This function allows users to set transmit EQ attenuation settings directly.
 *
 * Range limits:
 *
 *  - attn_main
 *      - <= 23 for general applications
 *      - <= 16 for KR applications
 *  - attn_pre + attn_main + attn_post <= 32
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  attn_main    : Main cursor setting (  0 to 23)
 * @param[in]  attn_post    : Post cursor setting (-31 to 31)
 * @param[in]  attn_pre     : Pre  cursor setting (-31 to 31)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_amp_set()
 */
bf_status_t bf_serdes_tx_drv_attn_set_allow_unassigned(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int lane,
                                                       int attn_main,
                                                       int attn_post,
                                                       int attn_pre);

/** @brief Set Transmit EQ based on Attenuation
 *
 * This function allows users to set transmit EQ attenuation settings directly.
 *
 * Range limits:
 *
 *  - attn_main
 *      - <= 23 for general applications
 *      - <= 16 for KR applications
 *  - attn_pre + attn_main + attn_post <= 32
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  attn_main    : Main cursor setting (  0 to 23)
 * @param[in]  attn_post    : Post cursor setting (-31 to 31)
 * @param[in]  attn_pre     : Pre  cursor setting (-31 to 31)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_amp_set()
 */
bf_status_t bf_serdes_tx_drv_attn_set_allow_unassigned(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int lane,
                                                       int attn_main,
                                                       int attn_post,
                                                       int attn_pre);

/** @brief Get Transmit EQ based on Attenuation
 *
 * This function allows users to get transmit EQ attenuation settings directly.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out]  attn_main    : Main cursor settoutg (  0 to 23)
 * @param[out]  attn_post    : Post cursor settoutg (-31 to 31)
 * @param[out]  attn_pre     : Pre  cursor settoutg (-31 to 31)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_amp_get()
 *
 */
bf_status_t bf_serdes_tx_drv_attn_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int *attn_main,
                                      int *attn_post,
                                      int *attn_pre);

/** @brief Get Transmit EQ based on Attenuation
 *
 * This function allows users to get transmit EQ attenuation settings directly.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane         : Logical lane within port (0..3, mode dependent)
 *
 * @param[out]  attn_main    : Main cursor settoutg (  0 to 23)
 * @param[out]  attn_post    : Post cursor settoutg (-31 to 31)
 * @param[out]  attn_pre     : Pre  cursor settoutg (-31 to 31)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_amp_get()
 *
 */
bf_status_t bf_serdes_tx_drv_attn_get_allow_unassigned(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int lane,
                                                       int *attn_main,
                                                       int *attn_post,
                                                       int *attn_pre);

/** @brief Set Transmit EQ based on Output Amplitude
 *
 * This function allows the user to configure output waveform based on
 * amplitude at various UI location relative to the edge transition.
 *
 *  - amp_pre  (amplitude 1  UI before edge transition, 962 to 156 mVppd)
 *  - amp_main (amplitude 1  UI after  edge transition, 962 to 364 mVppd)
 *  - amp_post (amplitude 2+ UI after  edge transition, 962 to 156 mVppd)
 *
 * This function translates amplitude setting into attenuation setting used by
 * the hardware.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  amp_main : Main-cursor amplitude
 * @param[in]  amp_post : Post-cursor amplitude
 * @param[in]  amp_pre  : Pre-cursor amplitude
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_attn_set()
 *
 */
bf_status_t bf_serdes_tx_drv_amp_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int amp_main,
                                     int amp_post,
                                     int amp_pre);

/** @brief Get Transmit EQ based on Output Amplitude
 *
 * This functions gets the hardware setting (TX EQ attenuation) and converts
 * them to TX amplitude setting

 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] amp_main : Main-cursor amplitude
 * @param[out] amp_post : Post-cursor amplitude
 * @param[out] amp_pre  : Pre-cursor amplitude
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_drv_attn_get()
 *
 */
bf_status_t bf_serdes_tx_drv_amp_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int *amp_main,
                                     int *amp_post,
                                     int *amp_pre);

/** @brief Get Transmit Driver Status
 *
 * Get TX Driver status
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] tx_status : TX status
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_drv_status_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_tx_drv_status_t *tx_status);

/** @brief Set Receive Path Enable
 *
 * This function sets the RX path enable which also functions as a reset.
 * Some RX settings become effect when RX state goes from Disable to Enable
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  rx_en    : 1: RX path enabled. 0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_en_set(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                bool rx_en);

/** @brief Get Receive Path Enable
 *
 * This function gets the RX path enable which also functions as a reset.
 * Some RX settings become effect when TX state goes from Disable to Enable
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] en       : 1: RX path enabled. 0: disabled
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_en_get(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                bool *rx_en);

/** @brief Set Receive Input Polarity Inversion
 *
 * This function sets the RX input polarity inversion.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  rx_inv   : 1: invert P/N. 0: no inversion
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_inv_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool rx_inv);

/** @brief Get Receive Input Polarity Inversion
 *
 * This function gets the RX input polarity inversion.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] inv      : 1: invert P/N. 0: no inversion
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_inv_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool *rx_inv);

/** @brief Set Receive Input Termination
 *
 * Set RX AFE (analog front end) serial input buffer termination option.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  rx_term  : Termination option
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_term_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bf_sds_rx_term_t rx_term);

/** @brief Get Receive Input Termination
 *
 * Get RX AFE (analog front end) serial input buffer termination option.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] rx_term  : Termination option
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_term_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bf_sds_rx_term_t *rx_term);

/** @brief Set Receive Loss of Signal Threshold
 *
 * Set RX input LOS threshold in DAC steps.  This function uses PCIe signal
 * detect circuit to detect LOS condition.
 *
 * Threshold range is
 * For A0, 0-15
 * For B0, 0-255
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 * @param[in]  rx_los_en    : LOS enable
 * @param[in]  rx_los_thres : LOS threshold DAC setting
 *
 * @return Status of the API call
 *
 */

bf_status_t bf_serdes_rx_afe_los_thres_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           bool rx_los_en,
                                           int rx_los_thres);

/** @brief Get Receive Loss of Signal Threshold
 *
 * Get RX input LOS threshold in DAC steps
 *
 * Threshold range is (for A0):
 *    0-15
 * Threshold range is (for B0):
 *    0-255
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 * @param[out] rx_los_en    : LOS enable
 * @param[out] rx_los_thres : LOS threshold in DAC steps
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_los_thres_get(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           bool *rx_los_en,
                                           int *rx_los_thres);

/** @brief Get Signal Detect Status
 *
 * If signal detect circuit is enabled, this function will report if a signal
 * loss condition (average amplitude below threshold) was recorded since the
 * last time this function was called.
 *
 * Upon calling this function, the sticky LOS signal will be reset.
 *
 * *Warning: This is intended as a debug feature as signal detect circuit is
 * not guaranted for non-PCIe Gen1/2 line rates.*
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] rx_los   : Loss of signal indicator
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_afe_los_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool *rx_los);

/** @brief Set Receive Equalizer Parameters (Advanced)
 *
 * Sets specific RX EQ parameters.
 *
 * This is an advanced function reserved for internal/debug use.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  row      : row of INT 0x26, table 9.
 * @param[in]  col      : column of INT 0x26, table 9.
 * @param[in]  value    : Parameter value
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_param_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int row,
                                      int col,
                                      int value);

/** @brief get Receive Equalizer Parameters (Advanced)
 *
 * Gets specific RX EQ parameters.
 *
 * This is an advanced function reserved for internal/debug use.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  row      : row of INT 0x26, table 9.
 * @param[in]  col      : column of INT 0x26, table 9.
 * @param[out]  value    : Parameter value
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_param_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int row,
                                      int col,
                                      int *value);

/** @brief RX EQ Calibration in Progress
 *
 * Check to see if PHY microcontroller (uC) is busy running RX EQ calibration.
 *
 * This function will check up to chk_cnt times with chk_wait (ms) delays in
 * between each check.  If uC busy is detected, the function will return
 * right away without finishing all the chk_cnt.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  chk_cnt  : Number of times to check
 * @param[in]  chk_wait : (ms) Time to wait between checks
 * @param[out] uc_busy  : 1: microcontroller is busy. 0: not busy
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_cal_busy_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane,
                                         int chk_cnt,
                                         int chk_wait,
                                         bool *uc_busy);

/** @brief Set RX EQ CTLE Fixed Settings
 *
 * Sets fixed RX EQ CTLE settings.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  ctle_dc : DC boost (0..255)
 * @param[in]  ctle_lf : Peaking filter low frequency gain (0..15)
 * @param[in]  ctle_hf : Peaking filter high frequency gain (0..15)
 * @param[in]  ctle_bw : Peaking filter peaking frequency (0..7)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_mode_set()
 *
 */
bf_status_t bf_serdes_rx_eq_ctle_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int ctle_dc,
                                     int ctle_lf,
                                     int ctle_hf,
                                     int ctle_bw);

/** @brief Get RX EQ CTLE Fixed Settings
 *
 * Gets live RX EQ CTLE settings.
 *
 * If this function is called after iCal, the calibrated CTLE results will
 * be returned.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] ctle_dc : DC boost (0..255)
 * @param[out] ctle_lf : Peaking filter low frequency gain (0..15)
 * @param[out] ctle_hf : Peaking filter high frequency gain (0..15)
 * @param[out] ctle_bw : Peaking filter peaking frequency (0..7)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_mode_set()
 *
 */
bf_status_t bf_serdes_rx_eq_ctle_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int *ctle_dc,
                                     int *ctle_lf,
                                     int *ctle_hf,
                                     int *ctle_bw);

/** @brief Set Specific RX EQ DFE Tap (Advanced)
 *
 * Set DFE Tap value for a specific tap.
 *
 * This is an advanced function for internal and debug use only.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  dfe_tap_num : DFE Tap Number (1 or higher)
 * @param[in]  dfe_tap_val : DFE Tap Value (-15..15)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_dfe_set()
 *
 */
bf_status_t bf_serdes_rx_eq_dfe_adv_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        int dfe_tap_num,
                                        int dfe_tap_val);

/** @brief Get Specific RX EQ DFE Tap (Advanced)
 *
 * Get DFE Tap value for a specific tap (tap 2 and beyond).
 * This is an advanced function for debug only.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tap_num : DFE Tap Number (2 or higher)
 * @param[out] tap_val : DFE Tap Value (-15..15)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_dfe_get()
 *
 */
bf_status_t bf_serdes_rx_eq_dfe_adv_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        int dfe_tap_num,
                                        int *dfe_tap_val);

/** @brief Set RX EQ DFE Tap
 *
 * Set DFE Tap values if RX EQ mode is one of FIX_DFE modes.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  dfe_gain   : DFE Gain (0..15)
 * @param[in]  tap1..4    : DFE Taps 1 to 4 (-127..127). dfe_tap[0]=Tap 1
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_mode_set()
 *
 */
bf_status_t bf_serdes_rx_eq_dfe_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    int dfe_gain,
                                    int dfe_tap[4]);

/** @brief Get RX EQ DFE Tap
 *
 * Get DFE Tap values.
 *
 * If rx_eq_mode is ADP_DFE, this will retrieve adapted value.  If rx_eq_mode
 * is FIX_DFE, this will retrieve the fixed value.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] dfe_gain   : DFE Gain (0..15)
 * @param[out] tap1..4    : DFE Taps 1 to 4 (-127..127). dfe_tap[0]=Tap 1
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_mode_set()
 *
 */
bf_status_t bf_serdes_rx_eq_dfe_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    int *dfe_gain,
                                    int dfe_taps[4]);

/** @brief Set RX EQ Calibration Parameters (Advanced)
 *
 * Set RX EQ calibration parameters if the default settings do not work well
 * for a given type of channel.
 *
 * This is a debug feature not recommended for general usage.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  ctle_dc_hint     : Hint for DC boost (0..255)
 * @param[in]  dfe_gain_range   : DFE gain range (bits 7:0)
 * @param[in]  pcal_loop_cnt    : Number of fine adjustment loops to run
 *                                  for each pCal run (1..15)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_cal_param_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          int lane,
                                          int ctle_dc_hint,
                                          int dfe_gain_range,
                                          int pcal_loop_cnt);

/** @brief Get RX EQ Calibration Parameters (Advanced)
 *
 * Get RX EQ calibration parameters
 *
 * This is a debug feature not recommended for general usage.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] ctle_dc_hint     : Hint for DC boost (0..255)
 * @param[out] dfe_gain_range   : DFE gain range (bits 7:0)
 * @param[out] pcal_loop_cnt    : Number of fine adjustment loops to run
 *                                  for each pCal run (1..15)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_cal_param_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          int lane,
                                          int *ctle_dc_hint,
                                          int *dfe_gain_range,
                                          int *pcal_loop_cnt);

/** @brief Run RX EQ Calibration (Advanced)
 *
 * Run RX EQ Calibration.  This is the advanced version of
 * bf_serdes_rx_ical_run() and bf_serdes_rx_eq_pcal_run() with more control
 * over the adaptation behavior.
 *
 * This function is intended for internal use.  User may call this function
 * if a standard calibration setting does not work well for user's channel.
 *
 * If CTLE or DFE calibration is bypassed, fixed settings need to be specified
 * prior to calling this function via bf_serdes_rx_eq_ctle_set() and
 * bf_serdes_rx_eq_dfe_set().
 *
 * If special EQ tuning parameters are needed, set up special params using
 * bf_serdes_rx_eq_cal_param_set() prior to calling this function.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  cal_cmd  : Command to control the calibration routine
 * @param[in]  ctle_cal_cfg : 5 bits [4:0]. Default is 0x00
 *                          bit[0]: 1: Do not tune DC.    0: Tune DC
 *                          bit[1]: 1: Do not tune LF.    0: Tune LF
 *                          bit[2]: 1: Do not tune HF.    0: Tune HF
 *                          bit[3]: 1: Do not tune BW.    0: Tune BW
 *                          bit[4]: 1: Use DC tune hint.  0: Do not use Hint
 * @param[in]  dfe_fixed  : 1: Do not tune DFE.  0: tune DFE
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_ctle_set(), bf_serdes_rx_eq_dfe_set()
 * @see bf_serdes_rxeq_cal_param_set()
 * @see bf_serdes_rx_eq_ical_run(), bf_serdes_rx_eq_pcal_run()
 *
 */
bf_status_t bf_serdes_rx_eq_cal_adv_run(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bf_sds_rx_cal_mode_t cal_cmd,
                                        int ctle_cal_cfg,
                                        int dfe_fixed);

/** @brief Run RX EQ Initial Calibration
 *
 * Start RX EQ initial calibration.  This function performs the following
 * calibrations:
 *  - Slicer offset calibration
 *  - CTLE Calibration
 *  - DFE Calibration
 *
 * bf_serdes_rx_eq_ical_eye_get() can be called to check for completion and
 * if calibration was successful.
 *
 * If the user's channel cannot be calibrated well with standard ical settings,
 * bf_serdes_rx_eq_cal_adv_run() may be used for finer RX EQ cal control.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_ical_eye_get()
 * @see bf_serdes_rx_eq_cal_adv_run()
 *
 */
bf_status_t bf_serdes_rx_eq_ical_run(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane);

/** @brief Get RX EQ Cal Observed Eye Height (Advanced)
 *
 * Gets the eye height observed by the EQ calibration routine following
 * bf_serdes_rx_eq_ical_run().
 *
 * This is an advanced function since the eye reported by adaptation will
 * typically be larger than that reported by 2D eye scan which spends more
 * time accumulating errors at the eye boundary.  So adp_eye cannot be
 * evaluated quantatively.  It is a rough indicator.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] cal_eye  : (mVppd) Eye height observed by cal algorithm
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_ical_run()
 *
 */
bf_status_t bf_serdes_rx_eq_cal_eye_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        int *cal_eye);

/** @brief RX Equalizer Calibration is Done
 *
 * Check to see if RX EQ iCal has completed and if calibration
 * is successful.  cal_good is only relavent if cal_done = 1.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  cal_good_thres : (mVppd) cal_good=1 if cal_eye is above
 *                              threshold.  125mVppd is recommended.
 * @param[out] cal_done :  1: Completed. 0: In progress
 * @param[out] cal_good :  1: cal_eye above threshold.
 *                         0: cal_eye below threshold.
 *                        -1: cal_eye is < 50mVppd
 * @param[out] cal_eye  : (mVppd) Eye height observed by cal algorithm
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_ical_run()
 *
 */
bf_status_t bf_serdes_rx_eq_ical_eye_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane,
                                         int cal_good_thres,
                                         bool *cal_done,
                                         bool *cal_good,
                                         int *cal_eye);

/** @brief Start/Stop RX EQ Periodic Calibration
 *
 * Start periodic cal (pCal) or doing a one time pCal, which is equivalent to
 * stopping the pCal.
 *
 * pCal is fine DFE adjustment to adjust for slow changing temperature and
 * voltage conditions.  It will not disrupt traffic.  Prior to running pCal,
 * RX EQ initial calibration (iCal) must first be run to make sure the EQ
 * is tuned for a given channel.
 *
 * There is a device wide round robin mechanism to run pCal for each lane.
 * The round robin mechanism is controlled by bf_dev_rx_eq_cal_rr_set()
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  cal_cont : 1: Continuous pCal  0: One time pCal
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_ical_run()
 * @see bf_dev_rx_eq_cal_rr_set()
 *
 */
bf_status_t bf_serdes_rx_eq_pcal_run(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int cal_cont);

/** @brief Get RX Equalizer Status
 *
 * Get RX EQ CTLE/DFE settings for a port including:
 *      - RXEQ Mode
 *      - adp_done
 *      - adp_success
 *      - adp_eye
 *      - ctle settings (ctle_dc/lf/hf/bw)
 *      - dfe_taps
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] st       : RX EQ status
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eq_status_get(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane,
                                       bf_sds_rx_eq_status_t *rx_status);

/** @brief Set RX Comparison Slicer Position (Advanced)
 *
 * This is an advanced debug function to check eye margin.
 * This function sets the RX comparison slicer's x, y position (phase, vertical
 * offset) and enables comparison between data slicer and comparison slicer.
 *
 * To observe error count, call bf_serdes_pat_rx_err_cnt_get() or
 *
 * @note
 * Continuous adaptation for this channel will be disabled to access the
 * comparison slicer while this function is enabled.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  offset_en : 1: Enable comparison slicer offset.
 *                         0: Disable (Normal Traffic).
 * @param[in]  pos_x     : Horizontal Position ( -32.. 31 phase setting)
 * @param[in]  pos_y     : Vertical Position   (-500..500 mV)
 *
 *
 * @return Status of the API call
 * @see bf_serdes_pat_rx_err_cnt_get()
 */
bf_status_t bf_serdes_rx_eye_offset_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        int offset_en,
                                        int pos_x,
                                        int pos_y);

/** @brief Measure Vertical or Horizontal Eye Opening
 *
 * Perform Vertical or Horizontal eye scan to a particular BER.
 * Eye opening in mV or mUI is returned.
 *
 * The eye opening qualifying condition is that there can be up to
 * 1 bit error in 3/BER received bits.  E.g. for BER=1e-6, 1 bit error in
 * 3e6 received bits.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  meas_mode : Specify vertical or horizontal eye scan
 * @param[in]  meas_ber  : Specify 1e-6 or 1e-9 BER target
 * @param[out] meas_eye  : Measured eye height (mV) or eye width (mUI)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eye_get(bf_dev_id_t dev_id,
                                 bf_dev_port_t dev_port,
                                 int lane,
                                 bf_sds_rx_eye_meas_mode_t meas_mode,
                                 bf_sds_rx_eye_meas_ber_t meas_ber,
                                 int *meas_eye);

/** @brief 3D Eye Scan
 *
 * Perform 3D eye scan for link debug.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  meas_ber  : Specify 1e-6 or 1e-9 BER target
 * @param[out] meas_eye : Memory allocated by user to store measured data
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_eye_3d_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_rx_eye_meas_ber_t meas_ber,
                                    char *eye_plot_data,
                                    int max_eye_plot_data);

/** @brief Save 3D Eye Scan Data
 *
 * Saves 3D eye scan data to a file
 *
 * @param[in]  meas_data : Memory allocated by user to store measured data
 * @param[in]  file_loc  : Save file location
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eye_3d_get()
 */
bf_status_t bf_serdes_rx_eye_3d_save(int *meas_data, int *file_loc);

/** @brief Force inject Tx bit errors on a serdes slice
 *
 * Insert multiple single bit errors (as specified by num_bits) on the
 * transmit data output.  This is a debug feature to intentionally corrupts
 * transmit serial data.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  num_bits : Number or error bits to inject (0..65535)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_err_inj_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int num_bits);

/** @brief Force inject Rx bit errors on a serdes slice
 *
 * Insert multiple single bit errors (as specified by num_bits) on the
 * receive data input.  This is a debug feture to intentionally corrupts
 * receive serial data.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  num_bits : Number or error bits to inject (0..65535)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_err_inj_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int num_bits);

/** @brief Set Transmit Pattern Generator
 *
 * Sets TX Pattern Generator.  Supported modes are:
 *      - Normal Traffic:   Core data, PRBS off
 *      - PRBS Pattern:     PRBS-7/9/11/15/23/31
 *      - Fixed Pattern:    80b User defined fixed pattern
 *
 * To set 80b TX fixed pattern, call bf_serdes_tx_fixed_pat_set()
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_patsel : Select transmit data / PRBS pattern
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_userpat_set()
 *
 */
bf_status_t bf_serdes_tx_patsel_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_pat_patsel_t tx_patsel);

/** @brief Get Transmit Pattern Generator
 *
 * Gets TX Pattern Generator.  Supported modes are:
 *      - Normal Traffic:   Core data, PRBS off
 *      - PRBS Pattern:     PRBS-7/9/11/15/23/31
 *      - Fixed Pattern:    80b User defined fixed pattern
 *
 * To get 80b TX fixed pattern, call bf_serdes_tx_fixed_pat_get()
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] tx_patsel : Select transmit data / PRBS pattern
 *
 * @return Status of the API call
 *
 * @see bf_serdes_tx_userpat_get()
 *
 */
bf_status_t bf_serdes_tx_patsel_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_pat_patsel_t *tx_patsel);

/** @brief Set Transmit Fixed Pattern
 *
 * Sets 80b fixed transmit user pattern.  This is useful for PLL output jitter
 * measurement where a clock pattern is needed, or for debugs where a specific
 * data pattern (pulses or walking 1's pattern) is needed.
 *
 * The 80b fixed pattern is configured via four 32b words.  Only bits[19:0] of
 * each word is used.  Transmit order is LSBit first.  First bit transmitted
 * out serially is tx_fixed_pat[0][0].
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  tx_fixed_pat[4] : 80b fixed pattern in 4 32b words. Only
 *                               bits[19:0] are used.  LSBit transmitted first.
 *
 * @return Status of the API call
 *
 */

bf_status_t bf_serdes_tx_fixed_pat_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane,
                                       int tx_fixed_pat[4]);

/** @brief Get Transmit Fixed Pattern
 *
 * Gets 80b fixed transmit pattern.  This is useful for PLL output jitter
 * measurement where a clock pattern is needed, or for debugs where a specific
 * data pattern (pulses or walking 1's pattern) is needed.
 *
 * The 80b fixed pattern is stored in four 32b words.  Only bits[19:0] of
 * each word is used.  Transmit order is LSBit first.  First bit transmitted
 * out serially is tx_fixed_pat[0][0].
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] tx_fixed_pat0..3 : 80b fixed pattern in 4 32b words. Only
 *                                bits[19:0] are used.  LSBit transmitted first.
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_fixed_pat_get(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane,
                                       int tx_fixed_pat[4]);

/** @brief Set Receive Pattern Checker
 *
 * Sets RX Pattern Checker.  Supported modes are:
 *      - Normal Traffic:       Core data, PRBS off
 *      - PRBS Pattern:         PRBS-7/9/11/15/23/31
 *      - Fixed Pattern:        80b Fixed Pattern
 *
 * The PRBS and Fixed pattern checker performs auto reseeding.  If errors are
 * detected for two consecutive words, a new seed (main data) will be used
 * to reseed the PRBS and fixed pattern checker.
 *
 * Fixed pattern uses incoming data to seed a reference, then compares
 * subsequent incoming data against it.
 *
 * @note    Continuous pCal needs to be disabled when RX pattern checker is
 *          enabled.  User needs to re-enable pCal by calling
 *          bf_serdes_rx_eq_pcal_run()
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[in]  rx_patsel : Receive data / PRBS pattern
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_eq_pcal_run()
 *
 */
bf_status_t bf_serdes_rx_patsel_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_pat_patsel_t rx_patsel);

/** @brief Get Receive Pattern Checker
 *
 * Gets RX Pattern Checker.  Supported modes are:
 *      - Normal Traffic:       Core data, PRBS off
 *      - PRBS Pattern:         PRBS-7/9/11/15/23/31
 *      - Fixed Pattern:        80b Fixed Pattern
 *
 * The PRBS and Fixed pattern checker performs auto reseeding.  If errors are
 * detected for two consecutive words, a new seed (main data) will be used
 * to reseed the PRBS and fixed pattern checker.
 *
 * Fixed pattern uses incoming data to seed a reference, then compares
 * subsequent incoming data against it.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] rx_patsel : Receive data / PRBS pattern
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_patsel_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bf_sds_pat_patsel_t *rx_patsel);

/** @brief Captured 80b of Received Data
 *
 * Capture 80b of received data pattern.  This is a debug function to see what
 * data is being received (no data, random or known pattern).  This function
 * is most useful if a known repeated pattern < 80b is sent.
 *
 * The 80b captured pattern is stored in four 32b words.  Only bits[19:0] of
 * each word is used.  Receive order is LSBit first.  First bit received
 * serially is rx_cap_pat[0][0].
 *
 * @note    Continuous pCal needs to be disabled when RX pattern checker is
 *          enabled.  User needs to re-enable pCal by calling
 *          bf_serdes_rx_eq_pcal_run()
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] rx_cap_pat0..3 : 80b captured pattern in 4 32b words. Only
 *                              bits[19:0] are used.  LSBit transmitted first.
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_rx_data_cap_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int rx_cap_pat[4]);

/** @brief Get Receive Comparator Error Count
 *
 * Get RX comparator error count.
 *
 * For the error counter to activate, bf_serdes_rx_patsel_set() needs to be
 * called first to put the receive in PRBS or Fixed pattern checking mode.
 *
 * The error counter self-clears on read and saturates at 0xFFFF_FFFF.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane     : Logical lane within port (0..3, mode dependent)
 *
 * @param[out] err_cnt : Accumulated error count, pegs at 0xffffffff
 *
 * @return Status of the API call
 *
 * @see bf_serdes_rx_patsel_set()
 *
 */
bf_status_t bf_serdes_rx_err_cnt_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     uint32_t *rx_err_cnt);

/** @brief Get the Tx equalization settings on a Tofino serdes lane
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[out] pre      : pre-cursor
 * @param[out] atten    : attenuation
 * @param[out] post     : post-cursor
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_tx_eq(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                int *pre,
                                int *atten,
                                int *post);

/** @brief Set the Tx equalization parameters on a Tofino serdes lane
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] pre       : pre-cursor
 * @param[in] atten     : attenuation
 * @param[in] post      : post-cursor
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_eq(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                int pre,
                                int atten,
                                int post);

/** @brief Check that the PLLs match the expected divider
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] divider   : expected Tx/Rx divider (assumed to be the same)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_pll_state(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    int expected_divider);

/** @brief Get current state of tx_output_en from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[out] en       : returned state of tx_output_en
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_tx_output_en(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane,
                                       bool *en);

/** @brief Set tx_output_en for a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] en       : Value for tx_output_en
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_output_en(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane,
                                       bool en);

/** @brief Set tx_output_en, tx_en, rx_en for a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] rx_en       : Value for rx_en
 * @param[in] tx_en       : Value for tx_en
 * @param[in] tx_output_en       : Value for tx_output_en
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_tx_and_tx_output_en(bf_dev_id_t dev_id,
                                                 bf_dev_port_t dev_port,
                                                 int lane,
                                                 bool rx_en,
                                                 bool tx_en,
                                                 bool tx_output_en);

/** @brief Set tx_output_en, tx_en, rx_en for a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in] lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] rx_en       : Value for rx_en
 * @param[in] tx_en       : Value for tx_en
 * @param[in] tx_output_en       : Value for tx_output_en
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_tx_and_tx_output_en_allow_unassigned(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    int lane,
    bool rx_en,
    bool tx_en,
    bool tx_output_en);

/** @brief Get signal_ok threshold from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] thresh: returned signal ok threshold
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_signal_ok_thresh(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           int *thresh);

/** @brief Set signal_ok threshold n a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  thresh: threshold to set (0-15)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_signal_ok_thresh(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           int thresh);

/** @brief Get signal_ok status from a serdes slice. Note this API
 *         uses avago_serdes_get_signal_ok() which returns a latched
 *         status of LOS. NOT_LOS is defined as "signal_ok". To get
 *         the current status the API is called twice. This discards
 *         the latched LOS indication and returns the current status.
 *         If the latched LOS status is important then users should
 *         call port_mgr_port_check_los() prior to calling this API.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] sig_ok: returned signal ok, 1=ok, 0=not ok (los)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_signal_ok(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool *sig_ok);

/** @brief Get LOS status from a serdes slice. Note this API
 *         uses avago_serdes_get_signal_ok() which returns a latched
 *         status of LOS. To get the current status call this API
 *         twice (or use the port_mgr_port_check_signal_ok API).
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] los   : returned LOS indication, 1=LOS, 0=no los
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_los(bf_dev_id_t dev_id,
                              bf_dev_port_t dev_port,
                              int lane,
                              bool *los);

/** @brief Start DFE ICAL (coarse tuning) on a serdes slice.
 *
 * [ POST_ENABLE ]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_start_dfe_ical(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane);

/** @brief Start DFE ICAL (coarse tuning) on a serdes slice.
 *
 * [ POST_ENABLE ]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_start_dfe_ical_allow_unassigned(bf_dev_id_t dev_id,
                                                      bf_dev_port_t dev_port,
                                                      int lane);

/** @brief Start DFE PCAL (fine tuning) on a serdes slice.
 *
 * [ POST_ENABLE ]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_start_dfe_pcal(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane);

/** @brief Start DFE PI CAL (phase interpolator cal) on a serdes slice.
 *
 * [ POST_ENABLE ]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_start_dfe_pi_cal(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       int lane);

/** @brief Start DFE adaptive (continuous tuning) on a serdes slice.
 *
 * [ POST_ENABLE ]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_start_dfe_adaptive(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane);

/** @brief Stop DFE adaptive (continuous tuning) on a serdes slice.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_stop_dfe_adaptive(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane);

/** @brief Stop DFE (ICAL or PCAL) on a serdes slice.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_stop_dfe(bf_dev_id_t dev_id,
                               bf_dev_port_t dev_port,
                               int lane);

/** @brief Check if DFE is running on a serdes slice.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 * @param[out] dfe_running: 0=not running, 1=running
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_dfe_running(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      bool *dfe_running);

/** @brief Check if DFE is running on a serdes slice.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 * @param[out] dfe_running: 0=not running, 1=running
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_dfe_running_allow_unassigned(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int lane,
                                                       bool *dfe_running);

/** @brief Get Tx polarity from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] inv   : returned, 1=inverted, 0=not inverted
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_tx_invert(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool *inv);

/** @brief Set Tx polarity on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  inv   : 1=invert, 0=dont invert
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_invert(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool inv);

/** @brief Get Tx polarity from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] inv   : returned, 1=inverted, 0=not inverted
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_rx_invert(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool *inv);

/** @brief Set Tx polarity on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in] inv   : 1=invert, 0=dont invert
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_invert(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool inv);

/** @brief Force inject Tx bit errors on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in] num_bits: number of errorer bits to inject (0-65535)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_inject_error(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          int lane,
                                          int num_bits);

/** @brief Force inject Rx bit errors on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  num_bits: number of errorer bits to inject (0-65535)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_inject_error(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          int lane,
                                          int num_bits);

/** @brief Get Tx PLL clk_src mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  clk     : returned,
 *                : AVAGO_SERDES_TX_PLL_REFCLK
 *                : AVAGO_SERDES_TX_PLL_RX_DIVX
 *                : AVAGO_SERDES_TX_PLL_OFF
 *                : AVAGO_SERDES_TX_PLL_PCIE_CORE_CLK
 *                : AVAGO_SERDES_TX_PLL_PCIE_CORE_CLK_DIV2
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_tx_pll_clk_source(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            int lane,
                                            int *clk);

/** @brief Set Tx PLL clk_src mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  clk     : AVAGO_SERDES_TX_PLL_REFCLK
 *                : AVAGO_SERDES_TX_PLL_RX_DIVX
 *                : AVAGO_SERDES_TX_PLL_OFF
 *                : AVAGO_SERDES_TX_PLL_PCIE_CORE_CLK
 *                : AVAGO_SERDES_TX_PLL_PCIE_CORE_CLK_DIV2
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_pll_clk_source(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            int lane,
                                            int clk);

/** @brief Get Spico clk_src mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] clk : returned,
 *                  AVAGO_SERDES_SPICO_REFCLK
 *                  AVAGO_SERDES_SPICO_PCIE_CORE_CLK
 *                  AVAGO_SERDES_SPICO_TX_F10_CLK_FIXED
 *                  AVAGO_SERDES_SPICO_TX_F40_CLK_FIXED
 *                  AVAGO_SERDES_SPICO_REFCLK_DIV2
 *                  AVAGO_SERDES_SPICO_PCIE_CORE_CLK_DIV2
 *                  AVAGO_SERDES_SPICO_TX_F10_CLK_FIXED_DIV2
 *                  AVAGO_SERDES_SPICO_TX_F40_CLK_FIXED_DIV2
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_spico_clk_source(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           int *clk);

/** @brief Set Spico clk_src mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  clk     : AVAGO_SERDES_SPICO_REFCLK
 *                : AVAGO_SERDES_SPICO_PCIE_CORE_CLK
 *                : AVAGO_SERDES_SPICO_TX_F10_CLK_FIXED
 *                : AVAGO_SERDES_SPICO_TX_F40_CLK_FIXED
 *                : AVAGO_SERDES_SPICO_REFCLK_DIV2
 *                : AVAGO_SERDES_SPICO_PCIE_CORE_CLK_DIV2
 *                : AVAGO_SERDES_SPICO_TX_F10_CLK_FIXED_DIV2
 *                : AVAGO_SERDES_SPICO_TX_F40_CLK_FIXED_DIV2
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_spico_clk_source(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           int clk);

/** @brief Set any one of several DFE parameters on a Tofino serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  row     : row from below table
 * @param[in]  col     : col from below table
 * @param[in]  value   : value to set parameter to
 * <pre>
 *                    0x0       0x01  0x2   0x3     0x4
 *            0x0 - thresh_HF    d0e   HF  GAIN  dvos_d0e_lo*
 *            0x1 - thresh_LF    d0o   LF    2*  dvos_d0e_hi*
 *            0x2 - thresh_AGC   d1e   DC    3*  dvos_d0o_lo*
 *            0x3 - err_cnt_lo   d1o   BW    4*  dvos_d0o_hi*
 *            0x4 - err_cnt_hi   t1e   LB    5*  dvos_d1e_lo*
 *            0x5 - gainDFE_lo   t1o         6*  dvos_d1e_hi*
 *            0x6 - gainDFE_hi   t0e         7*  dvos_d1o_lo*
 *            0x7 - agc_gain_bnd t0o         8*  dvos_d1o_hi*
 *            0x8 - agc_eq_bnd   alpha       9*  tvos_d0e_lo*
 *            0x9 - thresh_lev               A** tvos_d0e_hi*
 *            0xA - dfe_state                B** tvos_d0o_lo*
 *            0xB - dfe_status               C** tvos_d0o_hi*
 *            0xC - d6_vos_only              D** tvos_d1e_lo*
 *            0xD - ctle_only                    tvos_d1e_hi*
 *            0xE - enable_dlev                  tvos_d1o_lo*
 *            0xF - run_coarse                   tvos_d1o_hi*
 * </pre>
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_dfe_param(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    int row,
                                    int col,
                                    int value);

/** @brief Initialize a serdes slice. Configures and calibrates a single
 *         serdes slice with most basic parameters.
 *
 * Note: This API can ONLY be used if tx and rx serdes lanes are mapped
 * identically!
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane      : Logical lane (within port) 0-3, depending upon mode
 * @param[in] reset     : 1=reset serdes first, 0=no reset
 * @param[in] init_mode : Init mode
 * @param[in] divider   : Tx/Rx divider (assumed to be the same)
 * @param[in] data_width: 10/20/40
 * @param[in] phase_cal : 1=perform phase calibration, 0=no phase calibration
 * @param[in] output_en : 1=set tx_output_en, allowing signal to be seen
 *externally
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_init(bf_dev_id_t dev_id,
                           bf_dev_port_t dev_port,
                           int lane,
                           int reset,
                           int init_mode,
                           int divider,
                           int data_width,
                           int phase_cal,
                           int output_en);

/** @brief Set basic Tx/Rx serdes parameters for Autonegotiation
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane:   : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_params_an_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool do_tx_eq);

/** @brief Set basic Tx/Rx serdes parameters for non-AN ports
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane:   : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_params_set(bf_dev_id_t dev_id,
                                 bf_dev_port_t dev_port,
                                 int lane);

/** @brief Get Tx data_sel mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] data_sel: returned,
 *                : AVAGO_SERDES_TX_DATA_SEL_CORE
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS7
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS9
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS11
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS15
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS23
 *                : AVAGO_SERDES_TX_DATA_SEL_PRBS31
 *                : AVAGO_SERDES_TX_DATA_SEL_USER
 *                : AVAGO_SERDES_TX_DATA_SEL_LOOPBACK
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_tx_data_sel(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int *data_sel);

/** @brief Set Tx data_sel mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in] data_sel:
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS7    = 0, < PRBS7 (x^7+x^6+1) generator
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS9    = 1, < PRBS9 (x^9+x^5+1)
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS11   = 2, < PRBS11 (x^11+x^9+1)
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS15   = 3, < PRBS15 (x^15+x^14+1)
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS23   = 4, < PRBS23 (x^23+x^18+1)
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS31   = 5, < PRBS31 (x^31+x^28+1)
 *     AVAGO_SERDES_TX_DATA_SEL_PRBS13   = 6, < PRBS13 (x^13+x^12+x^2+x^1+1)
 *     AVAGO_SERDES_TX_DATA_SEL_USER     = 7, < User pattern generator
 *     AVAGO_SERDES_TX_DATA_SEL_CORE     = 8, < External data
 *     AVAGO_SERDES_TX_DATA_SEL_LOOPBACK = 9, < Parallel loopback from receiver
 *     AVAGO_SERDES_TX_DATA_SEL_PMD      = 10,< PMD training data
 *     AVAGO_SERDES_TX_DATA_SEL_AN       = 11 < Auto-negotiation data
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_tx_data_sel(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int data_sel);

/** @brief Get Rx cmp_sel mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] cmp_sel : returned,
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS7
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS9
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS11
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS15
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS23
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS31
 *                : AVAGO_SERDES_RX_CMP_DATA_SELF_SEED
 *                : AVAGO_SERDES_RX_CMP_DATA_OFF
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_rx_cmp_sel(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int *cmp_sel);

/** @brief bf_port_rx_cmp_sel_set
 *         Set Rx cmp_sel mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in] cmp_sel : AVAGO_SERDES_RX_CMP_DATA_PRBS7
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS9
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS11
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS15
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS23
 *                : AVAGO_SERDES_RX_CMP_DATA_PRBS31
 *                : AVAGO_SERDES_RX_CMP_DATA_SELF_SEED
 *                : AVAGO_SERDES_RX_CMP_DATA_OFF
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_cmp_sel(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int cmp_sel);

/** @brief Get Rx cmp_mode mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] cmp_mode: returned,
 *                : AVAGO_SERDES_RX_CMP_MODE_OFF
 *                : AVAGO_SERDES_RX_CMP_MODE_XOR
 *                : AVAGO_SERDES_RX_CMP_MODE_TEST_PATGEN
 *                : AVAGO_SERDES_RX_CMP_MODE_MAIN_PATGEN
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_rx_cmp_mode(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int *cmp_mode);

/** @brief Set Rx cmp_mode mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in] cmp_mode: AVAGO_SERDES_RX_CMP_MODE_OFF
 *                : AVAGO_SERDES_RX_CMP_MODE_XOR
 *                : AVAGO_SERDES_RX_CMP_MODE_TEST_PATGEN
 *                : AVAGO_SERDES_RX_CMP_MODE_MAIN_PATGEN
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_cmp_mode(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int cmp_mode);

/** @brief Get Rx term mode from a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] term  : returned,
 *              : AVAGO_SERDES_RX_TERM_AGND
 *              : AVAGO_SERDES_RX_TERM_AVDD
 *              : AVAGO_SERDES_RX_TERM_FLOAT
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_get_rx_term(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  int lane,
                                  int *term);

/** @brief Set Rx term mode on a serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  term  : AVAGO_SERDES_RX_TERM_AGND
 *              : AVAGO_SERDES_RX_TERM_AVDD
 *              : AVAGO_SERDES_RX_TERM_FLOAT
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_set_rx_term(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  int lane,
                                  int term);

/** @brief Set or clear AN enable
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  en      : state to set an_en
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_en_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bool en);

/** @brief Set AN advertisement
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  base_pg : lower 48b contain base page advertsement
 * @param[in]  num_next_pg: # of next pages to be loaded
 * @param[in]  next_pg:  ptr to array of uint64_t values representing the
 *                  next page advertiements (in the lower 48b of each)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_advert_set(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane,
                                         uint64_t base_pg,
                                         int num_next_pg,
                                         uint64_t *next_pg);

/** @brief Start autoneg
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  base_pg : lower 48b contain base page advertsement
 * @param[in]  num_next_pg: Number of next pages to be loaded
 * @param[in]  disable_nonce_match: debug flag, enables AN on loopback modules
 * @param[in]  disable_link_inhibit_timer: debug flag, LT can extend beyond
 * 510ms
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_start(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    uint64_t base_pg,
                                    int num_next_pg,
                                    bool disable_nonce_match,
                                    bool disable_link_inhibit_timer);

/** @brief Stop autoneg
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_stop(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

/** @brief Get AN_GOOD state
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane: Logical lane (within port) 0-3, depending upon mode
 * @param[out] an_good: Autoneg good state
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_an_good_get(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  bool *an_good);

/** @brief Get AN_COMPLETE state
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane: Logical lane (within port) 0-3, depending upon mode
 * @param[out] an_cmplt : Autoneg complete state
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_an_complete_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bool *an_cmplt);

/** @brief Get autoneg state
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane : Logical lane (within port) 0-3, depending upon mode
 * @param[out] st : Autoneg state
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_st_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     bf_an_state_e *st);

/** @brief Get autoneg hcd and fec resolution
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_hcd_fec_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_port_speed_t *hcd,
                                          bf_fec_type_t *fec,
                                          uint32_t *av_hcd);

/** @brief Get link-training state
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] lt_st   : Link training state
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_link_training_st_get(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           int lane,
                                           bf_lt_state_e *lt_st);

/** @brief Get link-training state (extended)
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_link_training_st_extended_get(bf_dev_id_t dev_id,
                                                    bf_dev_port_t dev_port,
                                                    int lane,
                                                    int *failed,
                                                    int *in_prg,
                                                    int *rx_trnd,
                                                    int *frm_lk,
                                                    int *rmt_rq,
                                                    int *lcl_rq,
                                                    int *rmt_rcvr_rdy);

/** @brief Get all AN state variables
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_all_state(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bool *lp_base_pg_rdy,
                                        bool *lp_next_pg_rdy,
                                        bool *an_good,
                                        bool *an_complete,
                                        bool *an_failed);

/** @brief Get link-partners Base Page
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] lp_base_pg: Base page
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_lp_base_pg_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int lane,
                                             uint64_t *lp_base_pg);

/** @brief Get link-partners Next Page
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[out] lp_next_pg: Next page
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_lp_next_pg_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int lane,
                                             uint64_t *lp_next_pg);

/** @brief Set link-partners Next Page
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  next_pg : Next page
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_autoneg_next_pg_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          int lane,
                                          uint64_t next_pg);

/* This is an internal function */
int port_mgr_av_sd_rx_patsel_set(bf_dev_id_t dev_id,
                                 int ring,
                                 int sd,
                                 int rx_patsel);

/** @brief Initiate a temperature reading on the PMRO node
 *         A sensor reading can take several milliseconds to complete
 *         so the interface is broken into "start" adn "get" APIs.
 *         Call "start" to initiate a sensor measurement, then poll
 *         the "get" API until it returns something other than
 *         BF_NOT_READY.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in] channel: Which sensor to read
 *                 0 : BF_SDS_MAIN_TEMP_SENSOR_CH
 *                 1 : BF_SDS_REMOTE_TEMP_SENSOR_0_CH
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_temperature_read_start(
    bf_dev_id_t dev_id, int sensor, bf_sds_temp_sensor_channel_t channel);

/** @brief Read a temperature sensor value from the PMRO node
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in] channel: Which sensor to read
 *                 0 : BF_SDS_MAIN_TEMP_SENSOR_CH
 *                 1 : BF_SDS_REMOTE_TEMP_SENSOR_0_CH
 * @param[out] temp_mC: sensor value, in degrees mC
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_temperature_read_get(bf_dev_id_t dev_id,
                                           int sensor,
                                           bf_sds_temp_sensor_channel_t channel,
                                           uint32_t *temp_mC);

/** @brief Initiate a voltage reading on the PMRO node
 *         A sensor reading can take several milliseconds to complete
 *         so the interface is broken into "start" adn "get" APIs.
 *         Call "start" to initiate a sensor measurement, then poll
 *         the "get" API until it returns something other than
 *         BF_NOT_READY.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in] channel: Which sensor to read
 *                 0 : BF_SDS_MAIN_VOLT_SENSOR_CH
 *                 1 : BF_SDS_REMOTE_VOLT_SENSOR_0_CH
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_voltage_read_start(
    bf_dev_id_t dev_id, int sensor, bf_sds_voltage_sensor_channel_t channel);

/** @brief Read a voltage sensor value from the PMRO node
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in] channel: Which sensor to read
 *                 0 : BF_SDS_MAIN_VOLTAGE_SENSOR_CH
 *                 1 : BF_SDS_REMOTE_VOLTAGE_SENSOR_0_CH
 * @param[out] voltage_mV : sensor value, in mV
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_voltage_read_get(bf_dev_id_t dev_id,
                                       int sensor,
                                       bf_sds_voltage_sensor_channel_t channel,
                                       uint32_t *voltage_mV);

/** @brief Reset a serdes slice
 *
 * Caution: If lane uses asymmetric serdes (tx != rx) this
 *          API resets BOTH involved slices.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  reset_type : Type of the reset
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_reset(bf_dev_id_t dev_id,
                            bf_dev_port_t dev_port,
                            int lane,
                            bf_sds_reset_type_t reset_type);

/** @brief load the firmware version to all serdes slices.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  fw_ver: version associated with firmware, for verification
 * @param[in]  fw_path: path to file containing firmware (if NULL use default)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_bcast_firmware_load(bf_dev_id_t dev_id,
                                          uint32_t fw_ver,
                                          char *fw_path);

/** @brief load the firmware version to the sbus master
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  fw_ver: version associated with firmware, for verification
 * @param[in]  fw_path: path to file containing firmware (if NULL use default)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_sbm_firmware_load(bf_dev_id_t dev_id,
                                        uint32_t fw_ver,
                                        char *fw_path);

/** @brief load the firmware version to a given serdes slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  fw_ver: version associated with firmware, for verification
 * @param[in]  fw_path: path to file containing firmware (if NULL use default)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_ucast_firmware_load(
    bf_dev_id_t dev_id, int ring, int sd, uint32_t fw_ver, char *fw_path);

/** @brief Start/Stop transmission/reception of link-training frames
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  en    : false=disable LT, true=enable LT
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_link_training_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane,
                                        bool en);

/** @brief Assert restart_training
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_link_training_restart(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            int lane);

/** @brief Set up for either clause 92 or clause 72 link-training
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  mode  : BF_CLAUSE_72_LINK_TRAINING
 *                BF_CLAUSE_92_LINK_TRAINING
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_link_training_mode_set(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int lane,
                                             bf_link_training_mode_t mode);

/** @brief Assert HCD link-status
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  hcd   : Negotiated (avago) hcd value
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_assert_hcd_link_status(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int lane,
                                             uint32_t hcd);

/** @brief Get rough estimate of eye quality
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out]  metric: rough metric of eye quality
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_metric_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     uint32_t *metric);

/** @brief Set "delay cal"
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_delay_cal_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane);

/** @brief Get Eye Heights from vertical bathtub curve (VBTC)
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_height_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     int *eye_ht_1e06,
                                     int *eye_ht_1e10,
                                     int *eye_ht_1e12,
                                     int *eye_ht_1e15,
                                     int *eye_ht_1e17);

/** @brief Returns user-specified minimum eye heights at 1e06, 1e10, and 1e12
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_quality_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int *qualifying_eye_ht_1e06,
                                      int *qualifying_eye_ht_1e10,
                                      int *qualifying_eye_ht_1e12);

/** @brief Sets user-specified minimum eye heights at 1e06, 1e10, and 1e12
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_quality_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      int qualifying_eye_ht_1e06,
                                      int qualifying_eye_ht_1e10,
                                      int qualifying_eye_ht_1e12);

/** @brief Sets user-specified minimum eye heights at 1e06, 1e10, and 1e12
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_quality_configured_set(bf_dev_id_t dev_id,
                                                 bf_dev_port_t dev_port,
                                                 int lane,
                                                 int qualifying_eye_ht_1e06,
                                                 int qualifying_eye_ht_1e10,
                                                 int qualifying_eye_ht_1e12);

/** @brief Resets to user-specified minimum eye heights at 1e06, 1e10, and 1e12
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_eye_quality_reset(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane);

/** @brief Setup PRBS on a serdes slice.
 *
 * [ POST_ENABLE ] / [PRE_ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  prbs_speed : Speed in which PRBS test is to be set (25G or 10G)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_prbs_init(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                int lane,
                                bf_port_prbs_speed_t prbs_speed);

/** @brief Set PRBS mode on a serdes slice.
 *
 * [ POST_ENABLE ] / [PRE_ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 * @param[in] prbs_mode  : Mode of the PRBS test (31, 23, 15, 13, 11, 9, 7)
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_prbs_mode_set(bf_dev_id_t dev_id,
                                    bf_port_prbs_mode_t prbs_mode);

/** @brief Set PRBS Error compare mode on a serdes slice.
 *
 * [ POST_ENABLE ] / [PRE_ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_prbs_cmp_mode_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        int lane);

/** @brief Turn off PRBS on a serdes slice.
 *
 * [ POST_ENABLE ] / [PRE_ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_prbs_diag_off(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane);

/** @brief Display stats banner .
 *
 * @param[in] display_ucli_cookie: ucli context (typecasted as (void *)) in
 * which to display
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_prbs_stats_banner_display(void *display_ucli_cookie);

/** @brief Get the stats like errors and eye_metric for a serdes slice.
 *
 * [ POST_ENABLE ] / [PRE ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 *
 * @param[out]  errors     : Errors seen on the serdes slice
 * @param[out]  eye_metric : Eye calculated for a serdes slice

 * @return Status of the API call
 *
 *
 */
bf_status_t bf_serdes_debug_stats_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int lane,
                                      uint32_t *errors,
                                      uint32_t *eye_metric);

/** @brief Return a quick estimate of eye height
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_quick_eye_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    int *eye_ht);

/** @brief Return hardware address associated with dev_port/lane
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_hw_addr_get(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  int lane,
                                  bool tx_dir,
                                  uint32_t *hw_addr1, /*ring*/
                                  uint32_t *hw_addr2 /*sd*/);

/** @brief Return hardware address associated with dev_port/lane
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane    : Logical lane (within port) 0-3, depending upon mode
 * @param[in] dfe_ctrl: BF_SDS_TOF_DFE_CTRL_DEFAULT
 *                  BF_SDS_TOF_DFE_CTRL_ICAL
 *                  BF_SDS_TOF_DFE_CTRL_PCAL
 *                  BF_SDS_TOF_DFE_CTRL_SEEDED_HF
 *                  BF_SDS_TOF_DFE_CTRL_SEEDED_LF
 *                  BF_SDS_TOF_DFE_CTRL_SEEDED_DC
 *                  BF_SDS_TOF_DFE_CTRL_FIXED_HF
 *                  BF_SDS_TOF_DFE_CTRL_FIXED_LF
 *                  BF_SDS_TOF_DFE_CTRL_FIXED_DC
 * @param[in] hf_val  : used only for fixed or seeded hf
 * @param[in] lf_val  : used only for fixed or seeded lf
 * @param[in] dc_val  : used only for fixed or seeded dc
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_dfe_config_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     int lane,
                                     bf_sds_tof_dfe_ctrl_t dfe_ctrl,
                                     uint32_t hf_val,
                                     uint32_t lf_val,
                                     uint32_t dc_val);

/** @brief Get idle status from a serdes slice.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  ei    : returned idle status
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_elec_idle_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int lane,
                                    bool *ei);

/** @brief Set loop bandwidth (bbgain)
 *
 * [ POST_ENABLE ] / [PRE_ENABLE]
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane       : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  setting    : only tested values are 503, 511, 51b. 0=default,
 * -1=default
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_tx_loop_bandwidth_set(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    int lane,
    bf_sds_tof_tx_loop_bandwidth_t setting);

/** @brief Get frequency lock status for a slice
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out] flock : returned: 1=locked, 0=not locked
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_frequency_lock_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int lane,
                                         bool *flock);

/** @brief Enable or disable the sig_ok threshold
 *
 * Note: The sig_ok threshold MUST BE DISABLED for DFE to work.
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[in]  en    : enable or disable sig ok threshold
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_signal_ok_thresh_enable_set(bf_dev_id_t dev_id,
                                                  bf_dev_port_t dev_port,
                                                  int lane,
                                                  bool en);

/** @brief Disable calibration of signal ok detection threshold for the device
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  true_or_false: Disable/Enable signal ok threshold calibration
 *                            for the whole device
 */
bf_status_t bf_serdes_signal_ok_thresh_calibration_disable(bf_dev_id_t dev_id,
                                                           bool true_or_false);

/** @brief Get LOS status from a serdes slice. Note this API
 *         uses avago_serdes_get_signal_ok() which returns a latched
 *         status of LOS. To get the current status call this API
 *         twice (or use the port_mgr_port_check_signal_ok API).
 *
 * @param[in]  dev_id   : Device identifier
 * @param[in]  dev_port : Port identifier
 * @param[in]  lane  : Logical lane (within port) 0-3, depending upon mode
 * @param[out]  failed  : returned LOS indication, 1=LOS, 0=no los
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_serdes_calibration_status_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             int lane,
                                             bool *failed);

#ifdef __cplusplus
}
#endif /* C++ */

#endif  // BF_SERDES_IF_H_INCLUDED
