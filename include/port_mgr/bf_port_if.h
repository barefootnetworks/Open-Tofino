/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef BF_PORT_IF_H_INCLUDED
#define BF_PORT_IF_H_INCLUDED

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file bf_port_if.h
 * \brief Details Port-level APIs.
 *
 */

/**
 * @addtogroup lld-port-api
 * @{
 * This is a description of some APIs.
 */

/** \brief   Enumeration of supported Tofino serdes encoding modes
 */
typedef enum {
  BF_SERDES_ENC_MODE_NONE = 0,
  BF_SERDES_ENC_MODE_NRZ,
  BF_SERDES_ENC_MODE_PAM4,
} bf_serdes_encoding_mode_t;

/** \brief   Max number of Autoneg next-pages supported.
 *           Controls the size of an array of next page
 *           advertisements in port_mgr_port_t
 */
#define BF_MAX_AN_NEXT_PAGES 3

/** \brief   Enumeration of supported Tofino link-training modes.
 */
typedef enum {
  BF_CLAUSE_72_LINK_TRAINING = 0,
  BF_CLAUSE_92_LINK_TRAINING,
} bf_link_training_mode_t;

/** \brief   Enumeration of supported Tofino auto-negotiation statuses.
 */
typedef enum {
  BF_AN_ST_NONE = 0,
  BF_AN_ST_RUNNING,
  BF_AN_ST_GOOD,
  BF_AN_ST_FAILED,
  BF_AN_ST_RESTARTED,
  BF_AN_ST_COMPLETE,
} bf_an_state_e;

/** \brief   Enumeration of supported Tofino (clause 72) Link Training statuses.
 */
typedef enum {
  BF_LT_ST_NONE = 0,
  BF_LT_ST_RUNNING,
  BF_LT_ST_FAILED,
  BF_LT_ST_RESTARTED,
  BF_LT_ST_COMPLETE,
} bf_lt_state_e;

/** \brief   Enumeration of supported Tofino DFE modes.
 */
typedef enum {
  BF_DFE_TYP_NONE = 0,
  BF_DFE_TYP_COARSE,
  BF_DFE_TYP_FINE,
  BF_DFE_TYP_CONTINUOUS,
} bf_dfe_type_e;

/** \brief   Enumeration of supported Tofino port loopback modes.
 */
typedef enum {
  BF_LPBK_NONE = 0,
  BF_LPBK_MAC_NEAR,
  BF_LPBK_MAC_FAR,
  BF_LPBK_PCS_NEAR,
  BF_LPBK_SERDES_NEAR,
  BF_LPBK_SERDES_FAR,
  BF_LPBK_PIPE,  // note: tof2 only mode. epb -> ipb

  // tof3-specific loopback modes
  BF_LPBK_SERDES_NEAR_PARALLEL,
  BF_LPBK_SERDES_FAR_PARALLEL,
  BF_LPBK_SERDES_FAR_RMT,

} bf_loopback_mode_e;

/** \brief   Enumeration of supported port direction configuration modes.
 */
typedef enum {
  BF_PORT_DIR_DUPLEX = 0,
  BF_PORT_DIR_TX_ONLY,
  BF_PORT_DIR_RX_ONLY,
  BF_PORT_DIR_DECOUPLED,
  BF_PORT_DIR_MAX
} bf_port_dir_e;

typedef enum {
  // IEEE Tech Ability Field options
  BF_ADV_SPD_1000BASE_KX = (1 << 0),
  BF_ADV_SPD_10GBASE_KX4 = (1 << 1),  // not supported
  BF_ADV_SPD_10GBASE_KR = (1 << 2),
  BF_ADV_SPD_40GBASE_KR4 = (1 << 3),
  BF_ADV_SPD_40GBASE_CR4 = (1 << 4),
  BF_ADV_SPD_100GBASE_CR10 = (1 << 5),  // not supported
  BF_ADV_SPD_40GBASE_KP4 = (1 << 6),    // not supported
  BF_ADV_SPD_100GBASE_KR4 = (1 << 7),
  BF_ADV_SPD_100GBASE_CR4 = (1 << 8),
  BF_ADV_SPD_25GBASE_KRS_CRS = (1 << 9),  // note: no RS-FEC capability
  BF_ADV_SPD_25GBASE_KR_CR = (1 << 10),
  BF_ADV_SPD_2_5GBASE_KR = (1 << 11),
  BF_ADV_SPD_5GBASE_KR = (1 << 12),
  BF_ADV_SPD_50GBASE_KR1_CR1 = (1 << 13),
  BF_ADV_SPD_100GBASE_KR2_CR2 = (1 << 14),
  BF_ADV_SPD_200GBASE_KR4_CR4 = (1 << 15),
  BF_ADV_SPD_400GBASE_KR8_CR8 = (1 << 16),  // FIXME guess, not defined yet

  // Consortium Extended Tech Ability Field options
  BF_ADV_SPD_25GBASE_KR1_CONSORTIUM = (1 << 24),
  BF_ADV_SPD_25GBASE_CR1_CONSORTIUM = (1 << 25),
  BF_ADV_SPD_50GBASE_KR2_CONSORTIUM = (1 << 28),
  BF_ADV_SPD_50GBASE_CR2_CONSORTIUM = (1 << 29),
  BF_ADV_SPD_400GBASE_CR8_CONSORTIUM = (1 << 30),
} bf_an_adv_speeds_t;

typedef enum {
  // IEEE base-pg FEC bits (F0 and F1)
  BF_ADV_FEC_FC_10G_ABILITY_IEEE = (1 << 0),  // base-r FEC ability (10g only)
  BF_ADV_FEC_FC_10G_REQUEST_IEEE = (1 << 1),  // base-r FEC request (10g only)
  BF_ADV_FEC_RS_25G_REQUEST_IEEE = (1 << 2),  // 25g only
  BF_ADV_FEC_FC_25G_REQUEST_IEEE = (1 << 3),  // 25g only
  // Consortium Unformatted Next pg bits (F1-4)
  BF_ADV_FEC_RS_ABILITY_CONSORTIUM = (1 << 4),  // clause 91
  BF_ADV_FEC_FC_ABILITY_CONSORTIUM = (1 << 5),  // clause 74
  BF_ADV_FEC_RS_REQUEST_CONSORTIUM = (1 << 6),  // clause 91
  BF_ADV_FEC_FC_REQUEST_CONSORTIUM = (1 << 7),  // clause 74
} bf_an_fec_t;

typedef enum {
  // IEEE base-pg pause bits (C1 and C0)
  BF_ADV_PAUSE_RX = (1 << 0),  // C0 (?)
  BF_ADV_PAUSE_TX = (1 << 1),  // C1 (?)
} bf_an_pause_t;

typedef enum bf_port_prbs_speed_e {
  BF_PORT_PRBS_SPEED_10G = 0,
  BF_PORT_PRBS_SPEED_25G,
  BF_PORT_PRBS_SPEED_MAX
} bf_port_prbs_speed_t;

typedef enum bf_port_prbs_mode_e {
  BF_PORT_PRBS_MODE_31 = 0,
  BF_PORT_PRBS_MODE_23,
  BF_PORT_PRBS_MODE_15,
  BF_PORT_PRBS_MODE_13,
  BF_PORT_PRBS_MODE_11,
  BF_PORT_PRBS_MODE_9,
  BF_PORT_PRBS_MODE_7,
  BF_PORT_PRBS_MODE_NONE,  // "mission mode"
  BF_PORT_PRBS_MODE_MAX
} bf_port_prbs_mode_t;

/** \brief Link Fault Status enumeration
 */
typedef enum bf_port_link_fault_st_e {
  BF_PORT_LINK_FAULT_OK = 0,
  BF_PORT_LINK_FAULT_LOC_FAULT,
  BF_PORT_LINK_FAULT_REM_FAULT
} bf_port_link_fault_st_t;

typedef struct bf_sds_debug_stats_t {
  int attn_main;
  int attn_post;
  int attn_pre;
  uint32_t errors;
  uint32_t eye_metric;
} bf_sds_debug_stats_t;

typedef struct bf_sds_debug_stats_per_quad_t {
  bf_sds_debug_stats_t chnl_stats[8];
} bf_sds_debug_stats_per_quad_t;

typedef struct bf_tof2_sds_prbs_stats_t {
  uint32_t errors;
} bf_tof2_sds_prbs_stats_t;

typedef struct bf_port_sds_prbs_stats_t {
  union prbs_stats_union_ {
    bf_tof2_sds_prbs_stats_t tof2_channel[8];
  } prbs_stats;
} bf_port_sds_prbs_stats_t;

typedef struct bf_tof2_eye_val_t {
  float eyes_0;
  float eyes_1;
  float eyes_2;
} bf_tof2_eye_val_t;

typedef struct bf_port_eye_val_t {
  union eye_val_union_ {
    bf_tof2_eye_val_t tof2_channel[8];
  } eye_val;
} bf_port_eye_val_t;

typedef struct bf_tof2_ber_t {
  uint64_t sym_err_ctrs[16];
  float ber_per_lane[16];
  float ber_aggr;
} bf_tof2_ber_t;

typedef struct bf_port_ber_t {
  union ber_union_ {
    bf_tof2_ber_t tof2;
  } ber;
} bf_port_ber_t;

typedef struct bf_tof3_pcs_status_ {
  uint32_t rx_status_reg;
  uint32_t tx_rdy;
  uint32_t rx_rdy;
  uint32_t rx_status;
  uint32_t rs_rx_fault_remote;
  uint32_t rs_rx_fault_local;
  uint32_t rs_rx_link_interruption;
  uint32_t rs_tx_fault;
  uint32_t degraded_local;
  uint32_t degraded_remote;
  uint32_t block_lock_all;
  uint32_t align_status;
  uint32_t am_map_ok;
  uint32_t am_lock_all;
  uint32_t hi_ser;
  uint32_t hi_ber;
  bool up;
} bf_tof3_pcs_status_t;

/** \brief CLK-OBS Pad Clock Source
 */
typedef enum {
  BF_SDS_NONE_CLK =
      0, /**< select the clock coming from previous mac in the daisy chain */
  BF_SDS_RX_RECOVEREDCLK, /**< Source from Recovered Rx data */
  BF_SDS_TX_CLK,          /**< Source from TX CLK */
  BF_SDS_RX_PCS_FIFOCLK,  /**< Source from RX PCS_FIFO clock */
  BF_SDS_TX_PCS_FIFOCLK,  /**< Source from TX PCS_FIFO clock */
  BF_SDS_MACCLK,          /**< MAC clock */
  BF_SDS_HALF_MACCLK,     /**< Half MAC clock */
} bf_sds_clkobs_clksel_t;

/** \brief CLK-OBS Pad Clock Source
 */
typedef enum {
  BF_CLKOBS_PAD_0 = 0, /**< pad 0 */
  BF_CLKOBS_PAD_1 = 1, /**< pad 1 */
} bf_clkobs_pad_t;

// Current value of the OUI to set in next-pages
#define ETHERNET_CONSORTIUM_CID 0x6A737D

typedef enum {
  BF_SIGOVRD_PASS_THRU = 0,
  BF_SIGOVRD_PASS_THRU_INV,
  BF_SIGOVRD_FORCE_LO,
  BF_SIGOVRD_FORCE_HI,
} bf_sigovrd_fld_t;

/*****************************************************************
* bf_an_advertisement_set
*
* Construct the base page and (if necessary) next-page code words
* based on the specified ability/request bits.
*
* The 48b codewords are returned in the lower 48 bits of a
* user-specified array of uint64_t entries. The size of this
* array should be passed as the value of max_pgs
*
* Currently, AN for consortium requires a base page and 2 next
* pages, so the "pgs" array can be 3x64b entries and max_pgs
* set to "3".
*
* If no Consortium advertisements are requiested then num_pgs
* will be set to "1" and pgs[0] will contain the base page. No
* next pages need to be sent (NP=0)
*
* If any Consortium advertisements are requested then num_pgs
* will be set to "3". pgs[0] will contain the base page. NP=1.
* pgs[1] will contain an OUI tagged formatted next pagei with
* message code="5". OUI is set to "oui" (which, currently,
* should be the Ethernet Consortium CID=0x6A737D). NP=1

* pgs[2] will contain an OUI tagged unformatted next page with
* the specified consortium advertisements. OUI is set to "oui"
* (which, currently, should be the Ethernet Consortium CID
* (0x6A737D). NP=0.
*/
bf_status_t bf_an_advertisement_set(bf_an_adv_speeds_t speed_adv,
                                    bf_an_pause_t pause_adv,
                                    bf_an_fec_t fec_adv,
                                    uint32_t oui,
                                    uint32_t max_pgs,
                                    uint32_t *num_pgs,
                                    uint64_t *pgs);

bf_status_t bf_an_advertisement_get(uint32_t num_pgs,
                                    uint64_t *pgs,
                                    bf_an_adv_speeds_t *speed_adv,
                                    bf_an_pause_t *pause_adv,
                                    bf_an_fec_t *fec_adv,
                                    uint32_t *oui);

typedef uint32_t bf_mac_block_id_t;
bf_status_t bf_port_map_dev_port_to_mac(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bf_mac_block_id_t *mac_block,
                                        int *ch);
bf_status_t bf_port_map_mac_to_dev_port(bf_dev_id_t dev_id,
                                        bf_mac_block_id_t mac_block,
                                        int ch,
                                        bf_dev_port_t *dev_port);

/* # of channels within a MAC block on Tofino */
#define BF_TOF_MAC_BLOCK_CHANNELS 4
#define BF_TOF2_MAC_BLOCK_CHANNELS 8

/* # of channels within a MAC block (on any chip) */
#define BF_MAX_MAC_BLOCK_CHANNELS BF_TOF2_MAC_BLOCK_CHANNELS

/* MAC block lane map. Used to define the lane swizzling that can occur during
 *  board layout.
 *
 *  Each MAC block is connected to 4 serdes slices. Which serdes
 *  slice is used for tx or rx is determined by the traces from the device
 *  to the connector.
 *
 *  This structure is used to configure the internal muxes to direct the
 *  inputs/outputs of the serdes slices to the appropriate MAC channel
 *  such that MAC channel n (n=0-3) corresponds to channel n (n=0-3) on
 *  the connector.
 *
 *  Separate tx and rx lane maps allow for independently mapping the tx
 *  and rx directions. The index into each array represents the
 *  logical channel within the MAC block. The value of the entry defines
 *  the serdes slice (within the MAC blocks 4 slices) is wired to the
 *  corresponding connector lane (or channel).
 */
typedef struct bf_mac_block_lane_map_t {
  bf_dev_port_t dev_port;
  uint32_t tx_lane[BF_MAX_MAC_BLOCK_CHANNELS];
  uint32_t rx_lane[BF_MAX_MAC_BLOCK_CHANNELS];
  uint32_t fp_conn_id;
  uint32_t fp_chnl_id;
} bf_mac_block_lane_map_t;

/** \brief User-defined function (with the given signature) to be called
 *         whenever the operational state of a port has been detected to
 *         have changed state (from up->down or down->up).
 */
typedef void (*bf_port_callback_t)(bf_dev_id_t dev_id,
                                   bf_dev_port_t port,
                                   int up,
                                   void *userdata);

typedef void (*bf_port_int_callback_t)(bf_dev_id_t dev_id,
                                       bf_dev_port_t port,
                                       uint32_t offset,
                                       uint32_t set_int_bits,
                                       void *userdata);
/**
 * @}
 */

/** \brief Enum identifying each MAC stat counter.
 *         Also defines BF_NUM_RMON_COUNTERS giving the number
 *         of counters.
 */

typedef enum {
  bf_mac_stat_FramesReceivedOK = 0,
  bf_mac_stat_FramesReceivedAll,
  bf_mac_stat_FramesReceivedwithFCSError,
  bf_mac_stat_FrameswithanyError,
  bf_mac_stat_OctetsReceivedinGoodFrames,
  bf_mac_stat_OctetsReceived,
  bf_mac_stat_FramesReceivedwithUnicastAddresses,
  bf_mac_stat_FramesReceivedwithMulticastAddresses,
  bf_mac_stat_FramesReceivedwithBroadcastAddresses,
  bf_mac_stat_FramesReceivedoftypePAUSE,
  bf_mac_stat_FramesReceivedwithLengthError,
  bf_mac_stat_FramesReceivedUndersized,
  bf_mac_stat_FramesReceivedOversized,
  bf_mac_stat_FragmentsReceived,
  bf_mac_stat_JabberReceived,
  bf_mac_stat_PriorityPauseFrames,
  bf_mac_stat_CRCErrorStomped,
  bf_mac_stat_FrameTooLong,
  bf_mac_stat_RxVLANFramesGood,
  bf_mac_stat_FramesDroppedBufferFull,
  bf_mac_stat_FramesReceivedLength_lt_64,
  bf_mac_stat_FramesReceivedLength_eq_64,
  bf_mac_stat_FramesReceivedLength_65_127,
  bf_mac_stat_FramesReceivedLength_128_255,
  bf_mac_stat_FramesReceivedLength_256_511,
  bf_mac_stat_FramesReceivedLength_512_1023,
  bf_mac_stat_FramesReceivedLength_1024_1518,
  bf_mac_stat_FramesReceivedLength_1519_2047,
  bf_mac_stat_FramesReceivedLength_2048_4095,
  bf_mac_stat_FramesReceivedLength_4096_8191,
  bf_mac_stat_FramesReceivedLength_8192_9215,
  bf_mac_stat_FramesReceivedLength_9216,
  bf_mac_stat_FramesTransmittedOK,
  bf_mac_stat_FramesTransmittedAll,
  bf_mac_stat_FramesTransmittedwithError,
  bf_mac_stat_OctetsTransmittedwithouterror,
  bf_mac_stat_OctetsTransmittedTotal,
  bf_mac_stat_FramesTransmittedUnicast,
  bf_mac_stat_FramesTransmittedMulticast,
  bf_mac_stat_FramesTransmittedBroadcast,
  bf_mac_stat_FramesTransmittedPause,
  bf_mac_stat_FramesTransmittedPriPause,
  bf_mac_stat_FramesTransmittedVLAN,
  bf_mac_stat_FramesTransmittedLength_lt_64,
  bf_mac_stat_FramesTransmittedLength_eq_64,
  bf_mac_stat_FramesTransmittedLength_65_127,
  bf_mac_stat_FramesTransmittedLength_128_255,
  bf_mac_stat_FramesTransmittedLength_256_511,
  bf_mac_stat_FramesTransmittedLength_512_1023,
  bf_mac_stat_FramesTransmittedLength_1024_1518,
  bf_mac_stat_FramesTransmittedLength_1519_2047,
  bf_mac_stat_FramesTransmittedLength_2048_4095,
  bf_mac_stat_FramesTransmittedLength_4096_8191,
  bf_mac_stat_FramesTransmittedLength_8192_9215,
  bf_mac_stat_FramesTransmittedLength_9216,
  bf_mac_stat_Pri0FramesTransmitted,
  bf_mac_stat_Pri1FramesTransmitted,
  bf_mac_stat_Pri2FramesTransmitted,
  bf_mac_stat_Pri3FramesTransmitted,
  bf_mac_stat_Pri4FramesTransmitted,
  bf_mac_stat_Pri5FramesTransmitted,
  bf_mac_stat_Pri6FramesTransmitted,
  bf_mac_stat_Pri7FramesTransmitted,
  bf_mac_stat_Pri0FramesReceived,
  bf_mac_stat_Pri1FramesReceived,
  bf_mac_stat_Pri2FramesReceived,
  bf_mac_stat_Pri3FramesReceived,
  bf_mac_stat_Pri4FramesReceived,
  bf_mac_stat_Pri5FramesReceived,
  bf_mac_stat_Pri6FramesReceived,
  bf_mac_stat_Pri7FramesReceived,
  bf_mac_stat_TransmitPri0Pause1USCount,
  bf_mac_stat_TransmitPri1Pause1USCount,
  bf_mac_stat_TransmitPri2Pause1USCount,
  bf_mac_stat_TransmitPri3Pause1USCount,
  bf_mac_stat_TransmitPri4Pause1USCount,
  bf_mac_stat_TransmitPri5Pause1USCount,
  bf_mac_stat_TransmitPri6Pause1USCount,
  bf_mac_stat_TransmitPri7Pause1USCount,
  bf_mac_stat_ReceivePri0Pause1USCount,
  bf_mac_stat_ReceivePri1Pause1USCount,
  bf_mac_stat_ReceivePri2Pause1USCount,
  bf_mac_stat_ReceivePri3Pause1USCount,
  bf_mac_stat_ReceivePri4Pause1USCount,
  bf_mac_stat_ReceivePri5Pause1USCount,
  bf_mac_stat_ReceivePri6Pause1USCount,
  bf_mac_stat_ReceivePri7Pause1USCount,
  bf_mac_stat_ReceiveStandardPause1USCount,
  bf_mac_stat_FramesTruncated,
  BF_NUM_RMON_COUNTERS,
} bf_rmon_counter_t;

/** \brief Typedef that can be cast over the (64B aligned) MAC
 *         stat DMA buffer
 */
typedef struct bf_rmon_counter_array_t {
  union {
    uint64_t ctr_array_padded_for_64B_alignment[128];
    uint64_t ctr_array[BF_NUM_RMON_COUNTERS];
    struct ctr_ids {
      uint64_t FramesReceivedOK;
      uint64_t FramesReceivedAll;
      uint64_t FramesReceivedwithFCSError;
      uint64_t FrameswithanyError;
      uint64_t OctetsReceivedinGoodFrames;
      uint64_t OctetsReceived;
      uint64_t FramesReceivedwithUnicastAddresses;
      uint64_t FramesReceivedwithMulticastAddresses;
      uint64_t FramesReceivedwithBroadcastAddresses;
      uint64_t FramesReceivedoftypePAUSE;
      uint64_t FramesReceivedwithLengthError;
      uint64_t FramesReceivedUndersized;
      uint64_t FramesReceivedOversized;
      uint64_t FragmentsReceived;
      uint64_t JabberReceived;
      uint64_t PriorityPauseFrames;
      uint64_t CRCErrorStomped;
      uint64_t FrameTooLong;
      uint64_t RxVLANFramesGood;
      uint64_t FramesDroppedBufferFull;
      uint64_t FramesReceivedLength_lt_64;
      uint64_t FramesReceivedLength_eq_64;
      uint64_t FramesReceivedLength_65_127;
      uint64_t FramesReceivedLength_128_255;
      uint64_t FramesReceivedLength_256_511;
      uint64_t FramesReceivedLength_512_1023;
      uint64_t FramesReceivedLength_1024_1518;
      uint64_t FramesReceivedLength_1519_2047;
      uint64_t FramesReceivedLength_2048_4095;
      uint64_t FramesReceivedLength_4096_8191;
      uint64_t FramesReceivedLength_8192_9215;
      uint64_t FramesReceivedLength_9216;
      uint64_t FramesTransmittedOK;
      uint64_t FramesTransmittedAll;
      uint64_t FramesTransmittedwithError;
      uint64_t OctetsTransmittedwithouterror;
      uint64_t OctetsTransmittedTotal;
      uint64_t FramesTransmittedUnicast;
      uint64_t FramesTransmittedMulticast;
      uint64_t FramesTransmittedBroadcast;
      uint64_t FramesTransmittedPause;
      uint64_t FramesTransmittedPriPause;
      uint64_t FramesTransmittedVLAN;
      uint64_t FramesTransmittedLength_lt_64;
      uint64_t FramesTransmittedLength_eq_64;
      uint64_t FramesTransmittedLength_65_127;
      uint64_t FramesTransmittedLength_128_255;
      uint64_t FramesTransmittedLength_256_511;
      uint64_t FramesTransmittedLength_512_1023;
      uint64_t FramesTransmittedLength_1024_1518;
      uint64_t FramesTransmittedLength_1519_2047;
      uint64_t FramesTransmittedLength_2048_4095;
      uint64_t FramesTransmittedLength_4096_8191;
      uint64_t FramesTransmittedLength_8192_9215;
      uint64_t FramesTransmittedLength_9216;
      uint64_t Pri0FramesTransmitted;
      uint64_t Pri1FramesTransmitted;
      uint64_t Pri2FramesTransmitted;
      uint64_t Pri3FramesTransmitted;
      uint64_t Pri4FramesTransmitted;
      uint64_t Pri5FramesTransmitted;
      uint64_t Pri6FramesTransmitted;
      uint64_t Pri7FramesTransmitted;
      uint64_t Pri0FramesReceived;
      uint64_t Pri1FramesReceived;
      uint64_t Pri2FramesReceived;
      uint64_t Pri3FramesReceived;
      uint64_t Pri4FramesReceived;
      uint64_t Pri5FramesReceived;
      uint64_t Pri6FramesReceived;
      uint64_t Pri7FramesReceived;
      uint64_t TransmitPri0Pause1USCount;
      uint64_t TransmitPri1Pause1USCount;
      uint64_t TransmitPri2Pause1USCount;
      uint64_t TransmitPri3Pause1USCount;
      uint64_t TransmitPri4Pause1USCount;
      uint64_t TransmitPri5Pause1USCount;
      uint64_t TransmitPri6Pause1USCount;
      uint64_t TransmitPri7Pause1USCount;
      uint64_t ReceivePri0Pause1USCount;
      uint64_t ReceivePri1Pause1USCount;
      uint64_t ReceivePri2Pause1USCount;
      uint64_t ReceivePri3Pause1USCount;
      uint64_t ReceivePri4Pause1USCount;
      uint64_t ReceivePri5Pause1USCount;
      uint64_t ReceivePri6Pause1USCount;
      uint64_t ReceivePri7Pause1USCount;
      uint64_t ReceiveStandardPause1USCount;
      uint64_t FramesTruncated;
    } ctr_ids;
  } format;
} bf_rmon_counter_array_t;

typedef void (*bf_port_mac_stat_callback_t)(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            bf_rmon_counter_array_t *ctrs,
                                            void *userdata);

bf_status_t bf_port_mgr_init(void);
bf_status_t bf_port_rmon_counter_to_str(bf_rmon_counter_t ctr, char **str);

// callback for customer notifications
bf_status_t bf_port_bind_status_change_cb(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          bf_port_callback_t fn,
                                          void *userdata);
bf_status_t bf_port_bind_mac_interrupt_cb(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          bf_port_int_callback_t fn,
                                          void *userdata);
bf_status_t bf_port_has_mac(bf_dev_id_t dev_id,
                            bf_dev_port_t dev_port,
                            bool *has_mac);
bf_status_t bf_port_oper_state_get_extended(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            int *state,
                                            bool *pcs_rdy,
                                            bool *l_fault,
                                            bool *r_fault);
bf_status_t bf_port_oper_state_update(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      int state);
bf_status_t bf_port_oper_state_get_and_issue_callbacks(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int *state);
bf_status_t bf_port_oper_state_get(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   int *state);
bf_status_t bf_port_oper_state_callbacks_issue(bf_dev_id_t dev_id,
                                               bf_dev_port_t dev_port);
bf_status_t bf_port_is_oper_state_callback_pending(bf_dev_id_t dev_id,
                                                   bf_dev_port_t dev_port,
                                                   bool *pending);
bf_status_t bf_port_oper_state_get_no_side_effect(bf_dev_id_t dev_id,
                                                  bf_dev_port_t dev_port,
                                                  int *state);
bf_status_t bf_port_mac_stats_hw_async_get(bf_dev_id_t dev_id,
                                           bf_dev_port_t port,
                                           bf_port_mac_stat_callback_t user_cb,
                                           void *userdata);
bf_status_t bf_port_mac_stats_hw_only_sync_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_rmon_counter_array_t *ctr_data);
bf_status_t bf_port_mac_stats_hw_sync_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          bf_rmon_counter_array_t *ctr_data);
bf_status_t bf_port_mac_stats_cached_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t port,
                                         bf_rmon_counter_array_t *ctr_data);
bf_status_t bf_port_mac_stats_ctr_cached_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t port,
                                             bf_rmon_counter_t ctr_id,
                                             uint64_t *ctr_data);
bf_status_t bf_port_autoneg_advert_get(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       uint32_t *num_pgs,
                                       uint64_t *pgs);
bf_status_t bf_port_autoneg_advert_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       uint32_t num_pgs,
                                       uint64_t *pgs);
bf_status_t bf_port_autoneg_lp_advert_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          uint64_t *base_pg,
                                          uint32_t *num_np,
                                          uint64_t *next_pg);
bf_status_t bf_port_flow_control_link_pause_set(bf_dev_id_t dev_id,
                                                bf_dev_port_t dev_port,
                                                bool tx_en,
                                                bool rx_en);
bf_status_t bf_port_flow_control_link_pause_get(bf_dev_id_t dev_id,
                                                bf_dev_port_t dev_port,
                                                bool *tx_en,
                                                bool *rx_en);
bf_status_t bf_port_flow_control_pfc_set(bf_dev_id_t dev_id,
                                         bf_dev_port_t port,
                                         uint32_t tx_en_map,
                                         uint32_t rx_en_map);
bf_status_t bf_port_flow_control_pfc_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         uint32_t *tx_en_map,
                                         uint32_t *rx_en_map);
bf_status_t bf_port_autoneg_restart_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bool disable_nonce_match,
                                        bool disable_link_inhibit_timer);
bf_status_t bf_port_autoneg_config_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port);
bf_status_t bf_port_autoneg_state_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_an_state_e *an_st);
bf_status_t bf_port_autoneg_hcd_fec_resolve(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            uint64_t tx_base_page,
                                            uint64_t rx_base_page,
                                            bf_port_speed_t *hcd_speed,
                                            int *hcd_lanes,
                                            bf_fec_type_t *fec);
bf_status_t bf_port_autoneg_complete_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         bool *an_cmplt);
#if 0
bf_status_t bf_port_start_link_training_set(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            int ln);
bf_status_t bf_port_autoneg_lt_state_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         int ln,
                                         bf_lt_state_e *lt_state);
#endif
bf_status_t bf_port_autoneg_hcd_fec_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t port,
                                        bf_port_speed_t *hcd,
                                        bf_fec_type_t *fec);
bf_status_t bf_port_autoneg_hcd_fec_get_v2(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           bf_port_speed_t *hcd_speed,
                                           int *hcd_lanes,
                                           bf_fec_type_t *fec);
bf_status_t bf_port_autoneg_pause_resolution_get(bf_dev_id_t dev_id,
                                                 bf_dev_port_t port,
                                                 bool *tx_pause,
                                                 bool *rx_pause);
bf_status_t bf_port_mtu_set(bf_dev_id_t dev_id,
                            bf_dev_port_t port,
                            uint32_t tx_mtu,
                            uint32_t rx_mtu);
bf_status_t bf_port_mtu_get(bf_dev_id_t dev_id,
                            bf_dev_port_t dev_port,
                            uint32_t *tx_mtu,
                            uint32_t *rx_mtu);
bf_status_t bf_port_txff_truncation_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t port,
                                        uint32_t size,
                                        bool en);
bf_status_t bf_port_txff_mode_set(bf_dev_id_t dev_id,
                                  bf_dev_port_t port,
                                  bool crc_check_disable,
                                  bool crc_removal_disable,
                                  bool fcs_insert_disable,
                                  bool pad_disable);
bf_status_t bf_port_txff_preamble_ipg_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          uint8_t *preamble,
                                          uint32_t ipg);
bf_status_t bf_port_ifg_set(bf_dev_id_t dev_id,
                            bf_dev_port_t port,
                            uint32_t ifg,
                            bool ieee);
bf_status_t bf_port_preamble_length_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t port,
                                        uint32_t preamble_length);
bf_status_t bf_port_promiscuous_mode_set(bf_dev_id_t dev_id,
                                         bf_dev_port_t port,
                                         bool en);
bf_status_t bf_port_force_local_fault_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          bool force);
bf_status_t bf_port_force_remote_fault_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t port,
                                           bool force);
bf_status_t bf_port_force_idle_set(bf_dev_id_t dev_id,
                                   bf_dev_port_t port,
                                   bool force);
bf_status_t bf_port_tx_drain_set(bf_dev_id_t dev_id,
                                 bf_dev_port_t dev_port,
                                 bool en);
bf_status_t bf_port_is_rx_only(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_is_decoupled_mode(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port);
bf_status_t bf_port_mac_tx_enable_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t port,
                                      bool en);
bf_status_t bf_port_mac_rx_enable_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t port,
                                      bool en);
bf_status_t bf_port_mac_address_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t port,
                                    uint8_t *mac_addr);
bf_status_t bf_port_xoff_pause_time_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t port,
                                        uint32_t pause_time);
bf_status_t bf_port_xon_pause_time_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t port,
                                       uint32_t pause_time);
bf_status_t bf_port_loopback_mode_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t port,
                                      bf_loopback_mode_e mode);
bf_status_t bf_port_direction_mode_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t dev_port,
                                       bf_port_dir_e mode);
bf_status_t bf_port_dfe_type_set(bf_dev_id_t dev_id,
                                 bf_dev_port_t port,
                                 bf_dfe_type_e type);
bf_status_t bf_port_1588_timestamp_delta_tx_set(bf_dev_id_t dev_id,
                                                bf_dev_port_t port,
                                                uint16_t delta);
bf_status_t bf_port_1588_timestamp_delta_tx_get(bf_dev_id_t dev_id,
                                                bf_dev_port_t port,
                                                uint16_t *delta);
bf_status_t bf_port_1588_timestamp_delta_rx_set(bf_dev_id_t dev_id,
                                                bf_dev_port_t port,
                                                uint16_t delta);
bf_status_t bf_port_1588_timestamp_delta_rx_get(bf_dev_id_t dev_id,
                                                bf_dev_port_t port,
                                                uint16_t *delta);
bf_status_t bf_port_1588_timestamp_tx_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          uint64_t *ts,
                                          bool *ts_valid,
                                          int *ts_id);
bf_status_t bf_port_pcs_status_get(bf_dev_id_t dev_id,
                                   bf_dev_port_t port,
                                   bool *pcs_status,
                                   uint32_t *block_lock_per_pcs_lane,
                                   uint32_t *alignment_marker_lock_per_pcs_lane,
                                   bool *hi_ber,
                                   bool *block_lock_all,
                                   bool *alignment_marker_lock_all);
bf_status_t bf_port_pcs_status_get_v2(bf_dev_id_t dev_id,
                                      bf_dev_port_t port,
                                      bool *pcs_status,
                                      bool *hi_ber,
                                      bool *block_lock_all);
bf_status_t bf_port_pcs_counters_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t port,
                                     uint32_t *ber_cnt,
                                     uint32_t *errored_blk_cnt,
                                     uint32_t *sync_loss,
                                     uint32_t *block_lock_loss,
                                     uint32_t *hi_ber_cnt,
                                     uint32_t *valid_error_cnt,
                                     uint32_t *unknown_error_cnt,
                                     uint32_t *invalid_error_cnt,
                                     uint32_t *bip_errors_per_pcs_lane);
bf_status_t bf_port_pcs_cumulative_counters_get(bf_dev_id_t dev_id,
                                                bf_dev_port_t dev_port,
                                                uint32_t *ber_cnt,
                                                uint32_t *errored_blk_cnt);
bf_status_t bf_port_pcs_cumulative_counters_clr(bf_dev_id_t dev_id,
                                                bf_dev_port_t dev_port);
bf_status_t bf_port_rs_fec_control_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t port,
                                       bool byp_corr_ena,
                                       bool byp_ind_ena);
bf_status_t bf_port_rs_fec_status_and_counters_get(bf_dev_id_t dev_id,
                                                   bf_dev_port_t port,
                                                   bool *hi_ser,
                                                   bool *fec_align_status,
                                                   uint32_t *fec_corr_cnt,
                                                   uint32_t *fec_uncorr_cnt,
                                                   uint32_t *fec_ser_lane_0,
                                                   uint32_t *fec_ser_lane_1,
                                                   uint32_t *fec_ser_lane_2,
                                                   uint32_t *fec_ser_lane_3);
bf_status_t bf_port_fc_fec_control_set(bf_dev_id_t dev_id,
                                       bf_dev_port_t port,
                                       bool byp_corr_en,
                                       bool byp_ind_en);
bf_status_t bf_port_fc_fec_status_and_counters_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t port,
    uint32_t vl,
    bool *block_lock_status,
    uint32_t *fec_corr_blk_cnt,
    uint32_t *fec_uncorr_blk_cnt);
bf_status_t bf_port_mac_rd(bf_dev_id_t dev_id,
                           bf_dev_port_t port,
                           uint32_t reg,
                           uint32_t *val);
bf_status_t bf_port_mac_wr(bf_dev_id_t dev_id,
                           bf_dev_port_t port,
                           uint32_t reg,
                           uint32_t val);

bf_status_t bf_port_num_lanes_get(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  int *num_lanes);
void bf_an_base_page_log(bf_dev_id_t dev_id,
                         int ring,
                         int sd,
                         uint64_t base_pg);
bf_status_t bf_port_mac_ch_enable_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bool en);
bf_status_t bf_port_is_valid(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_is_enabled(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

bf_status_t bf_port_rs_fec_reset_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     bool assert_reset);

bf_status_t bf_port_speed_get(bf_dev_id_t dev_id,
                              bf_dev_port_t dev_port,
                              bf_port_speed_t *speed);
bf_status_t bf_port_fec_get(bf_dev_id_t dev_id,
                            bf_dev_port_t dev_port,
                            bf_fec_type_t *fec);

bf_status_t bf_port_is_loopback_enb(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    bool *enb);

bf_status_t bf_port_prbs_mode_set(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  bf_port_prbs_mode_t prbs_mode);
bf_status_t bf_port_prbs_mode_get(bf_dev_id_t dev_id,
                                  bf_dev_port_t dev_port,
                                  bf_port_prbs_mode_t *prbs_mode);

bf_status_t bf_port_prbs_init(bf_dev_id_t dev_id,
                              bf_dev_port_t dev_port,
                              bf_port_prbs_speed_t prbs_speed,
                              bf_port_prbs_mode_t prbs_mode);

bf_status_t bf_port_prbs_rx_eq_run(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

bf_status_t bf_port_prbs_cmp_mode_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port);

bf_status_t bf_port_prbs_cleanup(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

bf_status_t bf_port_eye_quality_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    int qualifying_eye_ht_1e06,
                                    int qualifying_eye_ht_1e10,
                                    int qualifying_eye_ht_1e12);
bf_status_t bf_port_eye_quality_reset(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port);
bf_status_t bf_port_fec_type_validate(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_port_speeds_t speed,
                                      bf_fec_type_t fec,
                                      bool *is_valid);

bf_status_t bf_port_prbs_debug_stats_get(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         bf_sds_debug_stats_t *stats);

bf_status_t bf_port_prbs_debug_stats_per_quad_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_sds_debug_stats_per_quad_t *q_stats);

bf_status_t bf_port_lf_rf_get(bf_dev_id_t dev_id,
                              bf_dev_port_t dev_port,
                              bool *latched_lf,
                              bool *latched_rf);
bf_status_t bf_port_local_fault_int_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bool en);
bf_status_t bf_port_remote_fault_int_set(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port,
                                         bool en);
bf_status_t bf_port_mac_int_set(bf_dev_id_t dev_id,
                                bf_dev_port_t dev_port,
                                bool en);
bf_status_t bf_port_interrupt_based_link_monitoring_enable(bf_dev_id_t dev_id);
bf_status_t bf_port_mac_int_all_disable(bf_dev_id_t dev_id);

bf_status_t bf_port_lt_disable_set(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   bool disable);
bf_status_t bf_port_lt_disable_get(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   bool *disable);
int bf_port_overhead_len_get(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_is_optical_xcvr_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bool *is_optical);
bf_status_t bf_port_is_optical_xcvr_set(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bool is_optical);
bf_status_t bf_port_optical_los_get(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    bool *los);
bf_status_t bf_port_optical_los_set(bf_dev_id_t dev_id,
                                    bf_dev_port_t dev_port,
                                    bool los);
bf_status_t bf_port_optical_xcvr_ready_get(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           bool *ready);
bf_status_t bf_port_optical_xcvr_ready_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port,
                                           bool ready);
bf_status_t bf_port_time_in_state_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      uint32_t *cycles);
bf_status_t bf_port_time_in_state_set(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      uint32_t cycles);
bf_status_t bf_port_rx_path_reset(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_rs_fec_scrambler_en_set(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            bool en);
bf_status_t bf_port_errored_block_thresh_set(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             uint32_t max_errors);
bf_status_t bf_port_errored_block_thresh_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             uint32_t *max_errors);
bf_status_t bf_port_mac_stats_hw_direct_get(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port,
                                            bf_rmon_counter_t *ctr_type_array,
                                            uint64_t *ctr_data,
                                            uint32_t num_of_ctr);
bf_status_t bf_port_no_auto_adaptive_tuning_set(bf_dev_id_t dev_id, bool val);
bf_status_t bf_port_no_auto_adaptive_tuning_get(bf_dev_id_t dev_id, bool *val);
bf_status_t bf_port_mac_set_tx_mode(bf_dev_id_t dev_id,
                                    bf_dev_port_t port,
                                    bool enable);
bf_status_t bf_port_force_sig_ok_low_set(bf_dev_id_t dev_id,
                                         bf_dev_port_t dev_port);
bf_status_t bf_port_un_force_sig_ok_low_set(bf_dev_id_t dev_id,
                                            bf_dev_port_t dev_port);
bf_status_t bf_port_force_sig_ok_high_set(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port);
bf_status_t bf_port_un_force_sig_ok_high_set(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port);
bf_status_t bf_port_lane_map_set(bf_dev_id_t dev_id,
                                 bf_mac_block_id_t mac_id,
                                 bf_mac_block_lane_map_t *lane_map);
bf_status_t bf_port_speed_to_str(bf_port_speed_t speed, char **speed_str);

bf_status_t bf_port_oper_state_get_skip_intr_check(bf_dev_id_t dev_id,
                                                   bf_dev_port_t dev_port,
                                                   int *state);
bf_status_t bf_port_oper_state_callbacks_issue_with_intr_check(
    bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_serdes_los_get(bf_dev_id_t dev_id,
                                   bf_dev_port_t dev_port,
                                   int *state);
bf_status_t bf_port_flow_control_frame_src_mac_address_set(
    bf_dev_id_t dev_id, bf_dev_port_t dev_port, uint8_t *mac_addr);
const char *get_loopback_mode_str(bf_loopback_mode_e mode);
bf_status_t bf_port_clkobs_set(bf_dev_id_t dev_id,
                               bf_dev_port_t dev_port,
                               bf_clkobs_pad_t pad,
                               bf_sds_clkobs_clksel_t clk_src,
                               int divider);
bf_status_t bf_serdes_encoding_mode_get(uint32_t speed,
                                        uint32_t n_lanes,
                                        bf_serdes_encoding_mode_t *enc_mode);
bf_status_t bf_serdes_an_lp_base_page_get(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port,
                                          uint64_t *base_page);
bf_status_t bf_port_encoding_mode_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      bf_serdes_encoding_mode_t *enc_mode);
bf_status_t bf_port_umac4_status_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     uint64_t *reg64,
                                     uint64_t *txclkpresentall,
                                     uint64_t *rxclkpresentall,
                                     uint64_t *rxsigokall,
                                     uint64_t *blocklockall,
                                     uint64_t *amlockall,
                                     uint64_t *aligned,
                                     uint64_t *nohiber,
                                     uint64_t *nolocalfault,
                                     uint64_t *noremotefault,
                                     uint64_t *linkup,
                                     uint64_t *hiser,
                                     uint64_t *fecdegser,
                                     uint64_t *rxamsf);
bf_status_t bf_port_umac4_interrupt_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        uint64_t *reg64);
bf_status_t bf_port_umac4_interrupt_dn_up_get(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port,
                                              uint64_t *dn,
                                              uint64_t *up);
bf_status_t bf_port_forced_sigok_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     uint32_t *force_hi_raw_val,
                                     uint32_t *force_lo_raw_val,
                                     uint32_t *force_hi,
                                     uint32_t *force_lo);
bf_status_t bf_port_fec_mode_set(bf_dev_id_t dev_id,
                                 bf_dev_port_t dev_port,
                                 bf_fec_type_t fec);
bf_status_t bf_port_loopback_mode_to_str(bf_loopback_mode_e lpbk_mode,
                                         char **str);
bf_status_t bf_port_prbs_mode_to_str(bf_port_prbs_mode_t prbs_mode, char **str);
bf_status_t bf_port_fw_get(bf_dev_id_t dev_id,
                           char **fw_grp_0_7,
                           char **fw_grp_8);
bf_status_t bf_port_tx_reset_set(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_rx_reset_set(bf_dev_id_t dev_id, bf_dev_port_t dev_port);
bf_status_t bf_port_mac_stats_historical_get(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             bf_rmon_counter_array_t *ctr_data);
bf_status_t bf_port_mac_stats_historical_set(bf_dev_id_t dev_id,
                                             bf_dev_port_t dev_port,
                                             bf_rmon_counter_array_t *ctr_data);
bf_status_t bf_port_mac_stats_historical_update_set(bf_dev_id_t dev_id,
                                                    bf_dev_port_t dev_port);
bf_status_t bf_port_link_fault_status_get(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_port_link_fault_st_t *link_fault_st);
bf_status_t bf_port_link_fault_status_set(
    bf_dev_id_t dev_id,
    bf_dev_port_t dev_port,
    bf_port_link_fault_st_t link_fault_st);
bf_status_t bf_port_oper_state_set_and_issue_callbacks(bf_dev_id_t dev_id,
                                                       bf_dev_port_t dev_port,
                                                       int st);
bf_status_t bf_port_oper_state_set_pending_callbacks(bf_dev_id_t dev_id,
                                                     bf_dev_port_t dev_port,
                                                     int st);
bf_status_t bf_port_tx_ignore_rx_set(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     bool en);
bf_status_t bf_port_bring_up_time_get(bf_dev_id_t dev_id,
                                      bf_dev_port_t dev_port,
                                      uint64_t *but_us);
bf_status_t bf_port_link_up_time_get(bf_dev_id_t dev_id,
                                     bf_dev_port_t dev_port,
                                     uint64_t *but_us);
bf_status_t bf_port_signal_detect_time_set(bf_dev_id_t dev_id,
                                           bf_dev_port_t dev_port);
bf_status_t bf_port_debounce_set(bf_dev_id_t dev_id,
                                 bf_dev_port_t dev_port,
                                 uint32_t value);
bf_status_t bf_port_tof3_pcs_status_get(bf_dev_id_t dev_id,
                                        bf_dev_port_t dev_port,
                                        bf_tof3_pcs_status_t *pcs);
#ifdef __cplusplus
}
#endif /* C++ */

#endif  // BF_PORT_IF_H_INCLUDED
