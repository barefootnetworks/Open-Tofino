/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef BF_DRV_INTF_H_INCLUDED
#define BF_DRV_INTF_H_INCLUDED

/**
 * @file bf_drv_intf.h
 *
 * @brief BF drivers interface APIs
 *
 */

#ifndef PORT_MGR_HA_UNIT_TESTING
#define PORT_MGR_HA_UNIT_TESTING
#endif

#include <bf_types/bf_types.h>
#include <dvm/bf_drv_profile.h>
#include <dvm/bf_dma_types.h>
#include <port_mgr/port_mgr_port_evt.h>
#include <bfsys/bf_sal/bf_sys_intf.h>
#ifndef __KERNEL__
#include <tofino/pdfixed/pd_devport_mgr.h>
#include <bfutils/uCli/ucli.h>
#include <bfutils/cJSON.h>
#endif

/**
 * @addtogroup dvm-device-mgmt
 * @{
 */

typedef enum bf_client_prio_e {
  BF_CLIENT_PRIO_0 = 0, /* Lowest priority */
  BF_CLIENT_PRIO_1,
  BF_CLIENT_PRIO_2,
  BF_CLIENT_PRIO_3,
  BF_CLIENT_PRIO_4,
  BF_CLIENT_PRIO_5, /* Highest priority */
} bf_drv_client_prio_t;

typedef struct bf_dma_dr_info_s {
  unsigned int dma_dr_entry_count[BF_DMA_DR_DIRS];
} bf_dma_dr_info_t;

typedef struct bf_dma_bufpool_info_s {
  bf_sys_dma_pool_handle_t dma_buf_pool_handle;
  unsigned int dma_buf_size;
  unsigned int dma_buf_cnt;
} bf_dma_buf_info_t;

typedef struct bf_dma_info_s {
#ifndef __KERNEL__
  bf_sys_dma_pool_handle_t dma_dr_pool_handle;
#else
  void *dr_mem_vaddr;
  dma_addr_t dr_mem_dma_addr;
#endif
  unsigned int dma_dr_buf_size;
  bf_dma_dr_info_t dma_dr_info[BF_DMA_TYPE_MAX];
  bf_dma_buf_info_t dma_buff_info[BF_DMA_TYPE_MAX];
} bf_dma_info_t;

typedef enum bf_dev_init_mode_s {
#ifndef __KERNEL__
  /** Device incurs complete reset. */
  BF_DEV_INIT_COLD = DEV_INIT_COLD,

  /** Device incurs a fast-reconfig reset with minimal traffic disruption. */
  BF_DEV_WARM_INIT_FAST_RECFG = DEV_WARM_INIT_FAST_RECFG,

  /** Device incurs a hitless warm init. */
  BF_DEV_WARM_INIT_HITLESS = DEV_WARM_INIT_HITLESS,

  /** Device incurs a fast-reconfig reset with minimal traffic disruption.
   * bf-drivers pushes all the cached config back to the asic.
   * There is no config replay.
   */
  BF_DEV_WARM_INIT_FAST_RECFG_QUICK,
#else
  BF_DEV_INIT_COLD,
  BF_DEV_WARM_INIT_FAST_RECFG,
  BF_DEV_WARM_INIT_HITLESS,
  BF_DEV_WARM_INIT_FAST_RECFG_QUICK
#endif
} bf_dev_init_mode_t;

typedef enum bf_dev_serdes_upgrade_mode_s {
  /** No serdes update needed. */
  BF_DEV_SERDES_UPD_NONE,

  /** Update serdes by flapping ports immediately. */
  BF_DEV_SERDES_UPD_FORCED_PORT_RECFG,

  /** Defer the serdes update until a port flap occurs. */
  BF_DEV_SERDES_UPD_DEFERRED_PORT_RECFG
} bf_dev_serdes_upgrade_mode_t;

typedef struct bf_port_attributes_ {
  bool autoneg_enable;
  bf_port_speeds_t port_speeds;
  bf_fec_types_t port_fec_types;
  bf_pause_cfg_t tx_pause;
  bf_pause_cfg_t rx_pause;
  uint32_t n_lanes;
} bf_port_attributes_t;

/*
 * HA corrective action type so that the port delta can be successfully applied
 * during the delta push phase
 */
typedef enum bf_ha_corrective_action {
  BF_HA_CA_PORT_NONE = 0,
  BF_HA_CA_PORT_ADD,
  BF_HA_CA_PORT_ENABLE,
  BF_HA_CA_PORT_ADD_THEN_ENABLE,
  BF_HA_CA_PORT_FLAP,
  BF_HA_CA_PORT_DISABLE,
  BF_HA_CA_PORT_DELETE_THEN_ADD,
  BF_HA_CA_PORT_DELETE_THEN_ADD_THEN_ENABLE,
  BF_HA_CA_PORT_DELETE,
  BF_HA_CA_PORT_FSM_LINK_MONITORING,
  BF_HA_CA_PORT_MAX
} bf_ha_corrective_action_t;

/*
 * This is per port structure which holds the port number and the corrective
 * action
 * required for the delta to be applied during delta push phase of HA
 */
typedef struct bf_ha_port_reconcile_info {
  bf_dev_id_t dev_id;
  bf_dev_port_t dev_port;
  bf_ha_corrective_action_t ca;
  bf_port_attributes_t port_attrib;
} bf_ha_port_reconcile_info_t;

typedef struct bf_ha_port_reconcile_info_per_device {
  bf_ha_port_reconcile_info_t recon_info_per_port[BF_PORT_COUNT];
} bf_ha_port_reconcile_info_per_device_t;

/**
 * @brief Get the dma buff pool handle associated with a DR
 *
 * @param[in] dma_info Pointer to DMA info
 * @param[in] dma_type Type of DMA
 *
 * @return DMA buffer pool handle
 */
static inline void *bf_dma_get_buf_pool_hndl(bf_dma_info_t *dma_info,
                                             bf_dma_type_t dma_type) {
  if (!dma_info || dma_type >= BF_DMA_TYPE_MAX) {
    return NULL;
  }
  return (dma_info->dma_buff_info[dma_type].dma_buf_pool_handle);
}

/**
 * @brief Get the dma buffer size associated with a DR
 *
 * @param[in] dma_info Pointer to DMA info
 * @param[in] dma_type Type of DMA
 *
 * @return DMA buffer size
 */
static inline unsigned int bf_dma_get_buf_size(bf_dma_info_t *dma_info,
                                               bf_dma_type_t dma_type) {
  if (!dma_info || dma_type >= BF_DMA_TYPE_MAX) {
    return 0;
  }
  return (dma_info->dma_buff_info[dma_type].dma_buf_size);
}

/**
 * @brief Get the dma buffer count associated with a DR
 *
 * @param[in] dma_info Pointer to DMA info
 * @param[in] dma_type Type of DMA
 *
 * @return DMA buffer count
 */
static inline unsigned int bf_dma_get_buf_cnt(bf_dma_info_t *dma_info,
                                              bf_dma_type_t dma_type) {
  if (!dma_info || dma_type >= BF_DMA_TYPE_MAX) {
    return 0;
  }
  return (dma_info->dma_buff_info[dma_type].dma_buf_cnt);
}

/**
 * @brief Initialize device mgr
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_init(void);

/**
 * @brief Add a new device to the pktmgr on the device. After this call, the
 * device is ready for packet rx/tx api processing.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dma_info Information regarding DMA DRs and associated buffer pools
 * for the CPU Pkt receive/transmit DRs
 * @param[in] flags Device related flags passed by application to drivers
 *
 * @return Status of the API call.
 */
bf_status_t bf_pktmgr_device_add(bf_dev_family_t dev_family,
                                 bf_dev_id_t dev_id,
                                 bf_dma_info_t *dma_info,
                                 bf_dev_flags_t flags);

/**
 * @brief Add a new device.
 *
 * @param[in] dev_family The type of device, e.g. BF_DEV_FAMILY_TOFINO
 * @param[in] dev_id The ASIC id.
 * @param[in] profile Profile-details, program name, cfg file path.
 * @param[in] dma_info Information regarding DMA DRs and associated buffer pools
 * @param[in] flags Device related flags passed by application to drivers
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_add(bf_dev_family_t dev_family,
                          bf_dev_id_t dev_id,
                          bf_device_profile_t *profile,
                          bf_dma_info_t *dma_info,
                          bf_dev_flags_t flags);

/**
 * @brief Add a new virtual device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] profile Profile-details, program name, cfg file path.
 * @param[in] dev_type Type of device.
 *
 * @return Status of the API call.
 */
bf_status_t bf_virtual_device_add(bf_dev_id_t dev_id,
                                  bf_device_profile_t *profile,
                                  bf_dev_type_t dev_type);

/**
 * @brief Deletes an existing device.
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_remove(bf_dev_id_t dev_id);

/**
 * @brief Logs an existing device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] filepath The logfile path/name.
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_log(bf_dev_id_t dev_id, const char *filepath);

/**
 * @brief Restores an existing device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] filepath The logfile path/name.
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_restore(bf_dev_id_t dev_id, const char *filepath);

/**
 * @brief Enables/Disables Error interrupt handling on asic
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] enable Enable/Disable
 *
 * @return Status of the API call.
 */
bf_status_t bf_err_interrupt_handling_mode_set(bf_dev_id_t dev_id, bool enable);

/* @} */

/**
 * @addtogroup dvm-port-mgmt
 * @{
 */

typedef bf_status_t (*bf_drv_port_status_cb)(bf_dev_id_t dev_id,
                                             bf_dev_port_t port,
                                             bool up,
                                             void *userdata);
typedef bf_status_t (*bf_drv_port_speed_cb)(bf_dev_id_t dev_id,
                                            bf_dev_port_t port,
                                            bf_port_speed_t speed,
                                            void *userdata);

typedef bf_status_t (*bf_drv_port_mode_change_cb)(bf_dev_id_t dev_id,
                                                  bf_dev_port_t port,
                                                  uint32_t num_packet,
                                                  uint32_t pkt_size);

/**
 * @brief Adds a port to a device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dev_port The ASIC port id.
 * @param[in] port_speed The supported speed of the port as defined in
 * bf_port_speed_t.
 * @param[in] port_fec_type Port FEC type as defined in bf_fec_type_t.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_add(bf_dev_id_t dev_id,
                        bf_dev_port_t port,
                        bf_port_speeds_t port_speed,
                        bf_fec_types_t port_fec_type);

/**
 * @brief Adds a port to a device. Specify lane numbers.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dev_port The ASIC port id.
 * @param[in] port_speed The supported speed of the port as defined in
 * bf_port_speed_t.
 * @param[in] lane_numb The lane number the port is using
 * @param[in] port_fec_type Port FEC type as defined in bf_fec_type_t.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_add_with_lane_numb(bf_dev_id_t dev_id,
                                       bf_dev_port_t port,
                                       bf_port_speeds_t port_speed,
                                       uint32_t lane_numb,
                                       bf_fec_types_t port_fec_type);

/**
 * @brief Get the number of lanes is going to use for a port_speed by default.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] port_speed The supported speed of the port as defined in
 * bf_port_speed_t.
 * @param[out] lane_numb Pointer to return the lane number the port
 * is using by default.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_get_default_lane_numb(bf_dev_id_t dev_id,
                                          bf_port_speeds_t port_speed,
                                          uint32_t *lane_numb);

/**
 * @brief Remove a port from a device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dev_port The ASIC port id.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_remove(bf_dev_id_t dev_id, bf_dev_port_t dev_port);

/**
 * @brief Enable/Disable a port on a device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dev_port The ASIC port id.
 * @param[in] enable Enable/disable flag.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_enable(bf_dev_id_t dev_id, bf_dev_port_t port, bool enable);

/**
 * @brief Mark a port for serdes upgrade on a device
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] dev_port The ASIC port id
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_serdes_upgrade_notify(bf_dev_id_t dev_id,
                                          bf_dev_port_t dev_port);

/**
 * @brief Get Pcie CPU port number on the device. Based on skew and asci-type
 * the API will return appropriate port number to be used as CPU pcie port.
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Valid pcie port. In case of error returns -1.
 */
int bf_pcie_cpu_port_get(bf_dev_id_t dev_id);

/**
 * @brief Get ethernet CPU port number on the device. Based on skew and
 * asci-type
 * the API will return appropriate port number to be used as CPU ethernet port.
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Valid eth port. In case of error returns -1.
 */
int bf_eth_cpu_port_get(bf_dev_id_t dev_id);

/**
 * @brief Register port status (link up/down) callback.
 *
 * @param[in] port_status Port status callback.
 * @param[in] cookie Data to be given back in callback fn.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_client_register_status_notif(
    bf_drv_port_status_cb port_status, void *cookie);

/**
 * @brief Register port speed callback.
 *
 * @param[in] port_speed Port speed callback.
 * @param[in] cookie Data to be given back in callback fn.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_client_register_speed_notif(bf_drv_port_speed_cb port_speed,
                                                void *cookie);

/**
 * @brief Register port mode change callback.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] port_mode_change Port mode change callback.
 * @param[in] cookie Data to be given back in callback fn.
 *
 * @return Status of the API call.
 */
bf_status_t bf_port_client_register_mode_change_notif(
    bf_dev_id_t dev_id,
    bf_drv_port_mode_change_cb port_mode_change,
    void *cookie);

/**
 * @brief Configure the C2C (Copy-to-CPU) control.
 * The default state is disabled.
 *
 * @param[in] dev The ASIC id.
 * @param[in] enable If @c true C2C is enabled, if @c false C2C is disabled and
 * the @a port is ignored.
 * @param[in] port The port representing the CPU port.
 *
 * @return Status of the API call.
 */
bf_status_t bf_set_copy_to_cpu(bf_dev_id_t dev,
                               bool enable,
                               bf_dev_port_t port);

/**
 * @brief Lock device
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_reconfig_lock_device(bf_dev_id_t dev_id);

/**
 * @brief Set device mode to be a virtual device slave
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_set_virtual_dev_slave_mode(bf_dev_id_t dev_id);

typedef enum bf_error_sev_level_e {
  BF_ERR_SEV_CORRECTABLE,
  BF_ERR_SEV_NON_CORRECTABLE,
  BF_ERR_SEV_FATAL,
} bf_error_sev_level_t;

typedef enum bf_error_type_e {
  BF_ERR_TYPE_GENERIC = 0,
  BF_ERR_TYPE_SINGLE_BIT_ECC,
  BF_ERR_TYPE_MULTI_BIT_ECC,
  BF_ERR_TYPE_PARITY,
  BF_ERR_TYPE_OVERFLOW,
  BF_ERR_TYPE_UNDERFLOW,
} bf_error_type_t;

typedef enum bf_error_block_e {
  BF_ERR_BLK_NONE = 0,
  BF_ERR_BLK_MAU,
  BF_ERR_BLK_TCAM,
  BF_ERR_BLK_SRAM,
  BF_ERR_BLK_MAP_RAM,
  BF_ERR_BLK_STATS,
  BF_ERR_BLK_METERS,
  BF_ERR_BLK_SYNTH2PORT,
  BF_ERR_BLK_SELECTOR_ALU,
  BF_ERR_BLK_IMEM,
  BF_ERR_BLK_MIRROR,
  BF_ERR_BLK_TM_PRE,
  BF_ERR_BLK_TM_WAC,
  BF_ERR_BLK_TM_QAC,
  BF_ERR_BLK_TM_CLC,
  BF_ERR_BLK_TM_PEX,
  BF_ERR_BLK_TM_QLC,
  BF_ERR_BLK_TM_PRC,
  BF_ERR_BLK_TM_PSC,
  BF_ERR_BLK_TM_CAA,
  BF_ERR_BLK_TM_SCH,
  BF_ERR_BLK_PRSR,
  BF_ERR_BLK_DEPRSR,
  BF_ERR_BLK_PKTGEN,
  BF_ERR_BLK_GFM,
  BF_ERR_BLK_DMA,
  BF_ERR_BLK_LFLTR
} bf_error_block_t;

typedef enum bf_error_block_location_e {
  BF_ERR_LOC_NONE = 0,
  BF_ERR_LOC_MAU_IDLETIME,
  BF_ERR_LOC_MAU_STATEFUL_LOG,
  BF_ERR_LOC_SELECTOR_ALU_ST_MINMAX,
  BF_ERR_LOC_SELECTOR_ALU_DIV_BY0,
  /* For backward compatibility with original misspelling */
  BF_ERR_LOC_SELECTOR_ALU_DEV_BY0 = BF_ERR_LOC_SELECTOR_ALU_DIV_BY0,
  BF_ERR_LOC_SELECTOR_ALU_SALU_PRED,
  BF_ERR_LOC_MIRR_POINTER_FIFO,
  BF_ERR_LOC_MIRR_IG,
  BF_ERR_LOC_MIRR_EG,
  BF_ERR_LOC_MIRR_OUT_DESC,
  BF_ERR_LOC_MIRR_DATA_BUFFER,
  BF_ERR_LOC_MIRR_DROP_NEG,
  BF_ERR_LOC_MIRR_DROP_COAL,
  BF_ERR_LOC_MIRR_IG_DIS_SESS,
  BF_ERR_LOC_MIRR_EG_DIS_SESS,
  BF_ERR_LOC_MIRR_OUT,
  BF_ERR_LOC_MIRR_CRC12,
  BF_ERR_LOC_MIRR_SESSION,
  BF_ERR_LOC_MIRR_S2P_CREDIT,
  BF_ERR_LOC_MIRR_IDPRSR_SOPEOP_MISMATCH,
  BF_ERR_LOC_MIRR_EDPRSR_SOPEOP_MISMATCH,
  BF_ERR_LOC_MIRR_DATA_MEM,
  BF_ERR_LOC_TM_PRE_FIFO,
  BF_ERR_LOC_TM_PRE_MIT,
  BF_ERR_LOC_TM_PRE_LIT0_BM,
  BF_ERR_LOC_TM_PRE_LIT1_BM,
  BF_ERR_LOC_TM_PRE_LIT0_NP,
  BF_ERR_LOC_TM_PRE_LIT1_NP,
  BF_ERR_LOC_TM_PRE_PMT0,
  BF_ERR_LOC_TM_PRE_PMT1,
  BF_ERR_LOC_TM_PRE_RDM,
  BF_ERR_LOC_TM_PRE_BANKID_MEM,
  BF_ERR_LOC_TM_WAC_PPG_MAP,
  BF_ERR_LOC_TM_WAC_DROP_CNT,
  BF_ERR_LOC_TM_WAC_PFC_VIS,
  BF_ERR_LOC_TM_WAC_SCH_FCR,
  BF_ERR_LOC_TM_WAC_QID_MAP,
  BF_ERR_LOC_TM_WAC_WAC2QAC,
  BF_ERR_LOC_TM_QAC_QUE_DROP,
  BF_ERR_LOC_TM_QAC_PORT_DROP,
  BF_ERR_LOC_TM_QAC_QID_MAP,
  BF_ERR_LOC_TM_QAC_QAC2PRC,
  BF_ERR_LOC_TM_QAC_PRC2PSC,
  BF_ERR_LOC_TM_CLC_ENQ_FIFO,
  BF_ERR_LOC_TM_CLC_QAC_FIFO,
  BF_ERR_LOC_TM_CLC_PH_FIFO,
  BF_ERR_LOC_TM_CLC_QAC_PH_FIFO,
  BF_ERR_LOC_TM_PEX_CLM,
  BF_ERR_LOC_TM_PEX_PH_FIFO,
  BF_ERR_LOC_TM_PEX_META_FIFO,
  BF_ERR_LOC_TM_PEX_PH_AFIFO,
  BF_ERR_LOC_TM_PEX_DISCARD_FIFO,
  BF_ERR_LOC_TM_QLC_QLM,
  BF_ERR_LOC_TM_QLC_SCHDEQ,
  BF_ERR_LOC_TM_QLC_PH_FIFO,
  BF_ERR_LOC_TM_PRC_T3,
  BF_ERR_LOC_TM_PSC_PSM,
  BF_ERR_LOC_TM_PSC_COMM,
  BF_ERR_LOC_TM_CAA,
  BF_ERR_LOC_TM_SCH_TDM,
  BF_ERR_LOC_TM_SCH_UPD_WAC,
  BF_ERR_LOC_TM_SCH_UPD_EDPRSR_ADVFC,
  BF_ERR_LOC_TM_SCH_Q_MINRATE,
  BF_ERR_LOC_TM_SCH_Q_EXCRATE,
  BF_ERR_LOC_TM_SCH_Q_MAXRATE,
  BF_ERR_LOC_TM_SCH_L1_MINRATE,
  BF_ERR_LOC_TM_SCH_L1_EXCRATE,
  BF_ERR_LOC_TM_SCH_L1_MAXRATE,
  BF_ERR_LOC_TM_SCH_P_MAXRATE,
  BF_ERR_LOC_TM_SCH_UPD_PEX,
  BF_ERR_LOC_TM_SCH_UPD_EDPRSR,
  BF_ERR_LOC_TM_SCH_PEX_CREDIT,
  BF_ERR_LOC_TM_SCH_PEX_MAC_CREDIT,
  BF_ERR_LOC_TM_SCH_Q_WATCHDOG,
  BF_ERR_LOC_PRSR_ACT_RAM,
  BF_ERR_LOC_PRSR_INP_BUFF,
  BF_ERR_LOC_PRSR_OUT_FIFO,
  BF_ERR_LOC_PRSR_TCAM_PARITY,
  BF_ERR_LOC_PRSR_CSUM,
  BF_ERR_LOC_DEPRSR_PIPE_VEC_TBL0,
  BF_ERR_LOC_DEPRSR_PIPE_VEC_TBL1,
  BF_ERR_LOC_DEPRSR_MIRRTBL,
  BF_ERR_LOC_DEPRSR_IPKT_MAC,
  BF_ERR_LOC_DEPRSR_CMD_FIFO,
  BF_ERR_LOC_DEPRSR_CRED_ERR,
  BF_ERR_LOC_DEPRSR_PKTST,
  BF_ERR_LOC_DEPRSR_META_FIFO,
  BF_ERR_LOC_DEPRSR_PKTHDR,
  BF_ERR_LOC_DEPRSR_MIRRHDR,
  BF_ERR_LOC_DEPRSR_DATAST,
  BF_ERR_LOC_DEPRSR_PKTDATA,
  BF_ERR_LOC_DEPRSR_TMSCH,
  BF_ERR_LOC_DEPRSR_ARB_FIFO,
  BF_ERR_LOC_DEPRSR_CTL_CHAN,
  BF_ERR_LOC_PKTGEN_BUFFER,
  BF_ERR_LOC_PKTGEN_PFC,
  BF_ERR_LOC_PKTGEN_TBC_FIFO,
  BF_ERR_LOC_PKTGEN_ETH_CPU_FIFO,
  BF_ERR_LOC_PKTGEN_EBUF_P0_FIFO,
  BF_ERR_LOC_PKTGEN_EBUF_P1_FIFO,
  BF_ERR_LOC_PKTGEN_EBUF_P2_FIFO,
  BF_ERR_LOC_PKTGEN_EBUF_P3_FIFO,
  BF_ERR_LOC_PKTGEN_APP_EVT,
  BF_ERR_LOC_PKTGEN_IPB_CHNL_SEQ,
  BF_ERR_LOC_PKTGEN_ETH_CPU_TBC_SAMECHNL,
  BF_ERR_LOC_PKTGEN_ETH_PORT_FIFO,
  BF_ERR_LOC_PKTGEN_PHASE0,
  BF_ERR_LOC_GFM_INGRESS,
  BF_ERR_LOC_GFM_EGRESS,
  BF_ERR_LOC_DMA_PBC,
  BF_ERR_LOC_DMA_CBC,
  BF_ERR_LOC_DMA_MBC,
  BF_ERR_LOC_LFLTR_BFT_CLR,
  BF_ERR_LOC_LFLTR_BFT0,
  BF_ERR_LOC_LFLTR_BFT1,
  BF_ERR_LOC_LFLTR_LQT0,
  BF_ERR_LOC_LFLTR_LQT1,
  BF_ERR_LOC_LFLTR_LBUF,
} bf_error_block_location_t;

typedef bf_status_t (*bf_error_event_cb)(bf_error_sev_level_t severity,
                                         bf_dev_id_t dev_id,
                                         bf_dev_pipe_t pipe,
                                         uint8_t stage,
                                         uint64_t address,
                                         bf_error_type_t type,
                                         bf_error_block_t blk,
                                         bf_error_block_location_t loc,
                                         bf_dev_port_t *port_list,
                                         int num_ports,
                                         char *string,
                                         void *cookie);

/**
 * @brief The function is used to register for error notifications
 *
 * @param[in] dev ASIC device identifier
 * @param[in] event_cb Event callback API
 * @param[in] cookie User data passed into the callback
 *
 * @return Status of the API call
 */
bf_status_t bf_register_error_events(bf_dev_id_t dev,
                                     bf_error_event_cb event_cb,
                                     void *cookie);

typedef bf_status_t (*bf_port_stuck_cb)(bf_dev_id_t dev_id,
                                        bf_dev_port_t port_id,
                                        void *cookie);

/**
 * @brief The function is used to register for port stuck notifications
 *
 * @param[in] dev ASIC device identifier
 * @param[in] port_stuck_cb Port stuck callback API
 * @param[in] cookie User data passed into the callback
 *
 * @return Status of the API call
 */
bf_status_t bf_register_port_stuck_events(bf_dev_id_t dev,
                                          bf_port_stuck_cb port_stuck_cb,
                                          void *cookie);

/* @} */

/* Non-doxy */
typedef enum bf_port_cb_direction_ {
  BF_PORT_CB_DIRECTION_INGRESS = 0,
  BF_PORT_CB_DIRECTION_EGRESS
} bf_port_cb_direction_t;

typedef bf_status_t (*bf_drv_pkt_mgr_device_add_cb)(
    bf_dev_id_t dev_id,
    bf_dev_family_t dev_family,
    bf_dma_info_t *dma_info,
    bf_dev_init_mode_t warm_init_mode);

typedef bf_status_t (*bf_drv_device_add_cb)(bf_dev_id_t dev_id,
                                            bf_dev_family_t dev_family,
                                            bf_device_profile_t *profile,
                                            bf_dma_info_t *dma_info,
                                            bf_dev_init_mode_t warm_init_mode);

typedef bf_status_t (*bf_drv_virtual_device_add_cb)(
    bf_dev_id_t dev_id,
    bf_dev_type_t dev_type,
    bf_device_profile_t *profile,
    bf_dev_init_mode_t warm_init_mode);

/**
 * @brief The function is used to notify memory error events
 *
 * @param[in] sev Severity of event
 * @param[in] dev ASIC device identifier
 * @param[in] pipe Pipeline identifier
 * @param[in] stage Stage number in the pipe
 * @param[in] address Address of the memory for the error event
 * @param[in] type Type of the memory error
 * @param[in] blk Memory error block
 * @param[in] loc Memory error block location
 * @param[in] all_ports_in_pipe Flag to identify all ports in pipe or not
 * @param[in] port_list Port list
 * @param[in] num_ports Number of ports
 * @param[in] format Memory event string
 *
 * @return Status of the API call
 */
bf_status_t bf_notify_error_events(bf_error_sev_level_t sev,
                                   bf_dev_id_t dev,
                                   bf_dev_pipe_t pipe,
                                   uint8_t stage,
                                   uint64_t address,
                                   bf_error_type_t type,
                                   bf_error_block_t blk,
                                   bf_error_block_location_t loc,
                                   bool all_ports_in_pipe,
                                   bf_dev_port_t *port_list,
                                   int num_ports,
                                   const char *format,
                                   ...);
/**
 * @brief The function is used to notify port stuck events
 *
 * @param[in] dev ASIC device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 */
bf_status_t bf_notify_port_stuck_events(bf_dev_id_t dev,
                                        bf_dev_port_t port,
                                        ...);

/**
 * @brief Get all added ports in a pipe
 *
 * @param[in] dev Device identifier
 * @param[in] pipe Pipe number
 * @param[out] port_list Pointer to return the added ports list
 * @param[out] num_ports Pointer to return the number of ports added in the pipe
 *
 * @return Status of the API call
 */
bf_status_t bf_fill_pipe_ports(bf_dev_id_t dev,
                               bf_dev_pipe_t pipe,
                               bf_dev_port_t *port_list,
                               int *num_ports);
typedef bf_status_t (*bf_drv_device_del_cb)(bf_dev_id_t dev_id);
#ifndef __KERNEL__
typedef bf_status_t (*bf_drv_device_log_cb)(bf_dev_id_t dev_id, cJSON *node);
typedef bf_status_t (*bf_drv_device_restore_cb)(bf_dev_id_t dev_id,
                                                cJSON *node);
#else
typedef bf_status_t (*bf_drv_device_log_cb)(bf_dev_id_t dev_id, void *node);
typedef bf_status_t (*bf_drv_device_restore_cb)(bf_dev_id_t dev_id, void *node);
#endif
typedef bf_status_t (*bf_drv_port_add_cb)(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          bf_port_attributes_t *port_attrib,
                                          bf_port_cb_direction_t direction);
typedef bf_status_t (*bf_drv_port_del_cb)(bf_dev_id_t dev_id,
                                          bf_dev_port_t port,
                                          bf_port_cb_direction_t direction);
typedef bf_status_t (*bf_drv_port_status_int_cb)(bf_dev_id_t dev_id,
                                                 bf_dev_port_t port,
                                                 bool up);
typedef bf_status_t (*bf_drv_port_speed_int_cb)(bf_dev_id_t dev_id,
                                                bf_dev_port_t port);
typedef bf_status_t (*bf_drv_port_admin_state_cb)(bf_dev_id_t dev_id,
                                                  bf_dev_port_t port,
                                                  bool enable);
typedef bf_status_t (*bf_drv_err_intr_hdl_mode_cb)(bf_dev_id_t dev_id,
                                                   bool enable);

typedef bf_status_t (*bf_drv_rcfg_step_cb)(bf_dev_id_t dev_id);

typedef bf_status_t (*bf_drv_ha_complete_hitless_hw_read_cb)(
    bf_dev_id_t dev_id);
typedef bf_status_t (*bf_drv_ha_compute_delta_changes_cb)(bf_dev_id_t dev_id);
typedef bf_status_t (*bf_drv_ha_push_delta_changes_cb)(bf_dev_id_t dev_id);
typedef bf_status_t (*bf_drv_ha_set_dev_type_virtual_dev_slave_cb)(
    bf_dev_id_t dev_id);
typedef bf_status_t (*bf_drv_ha_register_port_corr_action_cb)(
    bf_dev_id_t dev_id, bf_ha_port_reconcile_info_per_device_t *recon_info);
typedef bf_status_t (*bf_drv_ha_port_delta_push_done_cb)(bf_dev_id_t dev_id);
typedef bf_status_t (*bf_drv_ha_port_serdes_upgrade_cb)(bf_dev_id_t dev_id,
                                                        bf_dev_port_t port,
                                                        uint32_t serdes_fw_ver,
                                                        char *serdes_fw_path);

typedef struct bf_drv_client_callbacks_s {
  bf_drv_pkt_mgr_device_add_cb pkt_mgr_dev_add;
  bf_drv_device_add_cb device_add;
  bf_drv_virtual_device_add_cb virtual_device_add;
  bf_drv_device_del_cb device_del;
  bf_drv_device_log_cb device_log;
  bf_drv_device_restore_cb device_restore;
  bf_drv_port_add_cb port_add;
  bf_drv_port_del_cb port_del;
  bf_drv_port_status_int_cb port_status;
  bf_drv_port_speed_int_cb port_speed;
  bf_drv_port_admin_state_cb port_admin_state;
  bf_drv_err_intr_hdl_mode_cb err_intr_mode;

  /* Fast reconfig callbacks */
  bf_drv_rcfg_step_cb lock;
  bf_drv_rcfg_step_cb create_dma;
  bf_drv_rcfg_step_cb disable_input_pkts;
  bf_drv_rcfg_step_cb wait_for_flush;
  bf_drv_rcfg_step_cb core_reset;
  bf_drv_rcfg_step_cb unlock_reprogram_core;
  bf_drv_rcfg_step_cb config_complete;
  bf_drv_rcfg_step_cb enable_input_pkts;
  bf_drv_rcfg_step_cb error_cleanup;
  bf_drv_rcfg_step_cb warm_init_quick;

  /* Hitless Restart */
  bf_drv_ha_complete_hitless_hw_read_cb complete_hitless_hw_read;
  bf_drv_ha_compute_delta_changes_cb compute_delta_changes;
  bf_drv_ha_push_delta_changes_cb push_delta_changes;
  bf_drv_ha_set_dev_type_virtual_dev_slave_cb device_mode_virtual_dev_slave;

  /* Warm Init Port callbacks */
  bf_drv_ha_register_port_corr_action_cb register_port_corr_action;
  bf_drv_ha_port_delta_push_done_cb port_delta_push_done;
  bf_drv_ha_port_serdes_upgrade_cb port_serdes_upgrade;
} bf_drv_client_callbacks_t;

typedef int bf_drv_client_handle_t;

/**
 * @brief Get sizing requirements for DMA descriptor rings (DRs)
 *
 * @param[in] dma_dr_entry_count DMA DR entry counts desired per DMA function
 * @param[out] total_bytes Pointer to return the total contiguous bytes
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_get_dma_dr_size_in_bytes(
    int dma_dr_entry_count[BF_DMA_TYPE_MAX][BF_DMA_DR_DIRS],
    unsigned int *total_bytes);

/**
 * @brief Register as device-manager-client.
 *
 * @param[in] client_name Client Name.
 * @param[out] client_handle Pointer to return the allocated client handle.
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_register(const char *client_name,
                            bf_drv_client_handle_t *client_handle);

/**
 * @brief De-Register as device-manager-client.
 *
 * @param[in] client_handle The client handle allocated.
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_deregister(bf_drv_client_handle_t client_handle);

/**
 * @brief Register all callbacks.
 *
 * @param[in] client_handle The client handle allocated.
 * @param[in] callbacks Pointer to the callback.
 * @param[in] add_priority Priority of the client for add callback
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_client_register_callbacks(
    bf_drv_client_handle_t client_handle,
    bf_drv_client_callbacks_t *callbacks,
    bf_drv_client_prio_t add_priority);

/**
 * @brief Register warm init flags
 *
 * @param[in] client_handle The client handle allocated.
 * @param[in] override_fast_recfg Flag to indicate if the fast recfg sequence
 * is to be overriden by the hitless ha sequence
 * @param[in] issue_fast_recfg_port_cb Flag to indicate if the client supports
 * port add/del/enb/dis callbacks during fast reconfig
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_client_register_warm_init_flags(
    bf_drv_client_handle_t client_handle,
    bool override_fast_recfg,
    bool issue_fast_recfg_port_cb);

/**
 * @brief Get the drivers version
 *
 * @return Pointer to return the version string
 */
const char *bf_drv_get_version(void);

/**
 * @brief Get the drivers internal version
 *
 * @return Pointer to return the version string
 */
const char *bf_drv_get_internal_version(void);

/* Driver shell related API */
/**
 * @brief Initialize the debug shell
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_shell_init(void);

/**
 * @brief Start the debug shell
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_shell_start(void);

/**
 * @brief Stop the debug shell
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_shell_stop(void);

#ifndef __KERNEL__
/**
 * @brief Register the ucli node with bf-sde shell
 *
 * @param[in] ucli_node UCLI node pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_shell_register_ucli(ucli_node_t *ucli_node);

/**
 * @brief Unregister uthe cli node with bf-sde shell
 *
 * @param[in] ucli_node UCLI node pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_shell_unregister_ucli(ucli_node_t *ucli_node);

/**
 * @brief Start the UCLI
 *
 * @param[in] fin Input stream file pointer
 * @param[in] fout Output stream file pointer
 *
 * @return None
 */
void bf_drv_ucli_run(FILE *fin, FILE *fout);
#endif

/**
 * @brief Perform a compatibility check prior to planned warm-restarts
 *
 * @param[in] dev_id The device id
 * @param[in] new_profile The new profile to perform compatibility check against
 * @param[in] new_drv_path The path of the new bf-drivers
 * @param[out] warm_init_mode_p Pointer to return the best warm initialization
 * mode
 * @param[out] serdes_upgrade_p Pointer to return whether SerDes upgrade is
 * needed or not
 *
 * @return Status of the API call
 */

bf_status_t bf_device_compat_check(bf_dev_id_t dev_id,
                                   bf_device_profile_t *new_profile,
                                   char *new_drv_path,
                                   bf_dev_init_mode_t *warm_init_mode_p,
                                   bool *serdes_upgrade_p);

/**
 * @brief Initiate a warm init process for a device
 *
 * @param[in] dev_id The device id
 * @param[in] warm_init_mode The warm init mode to use for this device
 * @param[in] serdes_upgrade_mode The mode to use for updating SerDes
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_warm_init_begin(
    bf_dev_id_t dev_id,
    bf_dev_init_mode_t warm_init_mode,
    bf_dev_serdes_upgrade_mode_t serdes_upgrade_mode);

/**
 * @brief End the warm init sequence for the device and resume normal operation
 *
 * @param[in] dev_id The device id
 *
 * @return Status of the API call.
 */
bf_status_t bf_device_warm_init_end(bf_dev_id_t dev_id);

/**
 * @brief Return if a port needs to be brought up using the FSM at the end
 * of warm init
 *
 * @param[in] dev_id The device id
 * @param[in] dev_port Device port number
 * @param[out] bring_up Flag to indicate if this port needs to be brought up
 *
 * @return Status of the API call
 */
bf_status_t bf_device_warm_init_port_bring_up(bf_dev_id_t dev_id,
                                              bf_dev_port_t dev_port,
                                              bool *bring_up);

/**
 * @brief Get the init mode of a device
 *
 * @param[in] dev_id The device id
 * @param[out] warm_init_mode The warm init mode to use for this device
 *
 * @return Status of the API call
 */
bf_status_t bf_device_init_mode_get(bf_dev_id_t dev_id,
                                    bf_dev_init_mode_t *warm_init_mode);

/**
 * @brief Get the family of a device
 *
 * @param[in] dev_id The device id
 * @param[out] dev_family The family for this device
 *
 * @return Status of the API call
 */
bf_status_t bf_device_family_get(bf_dev_id_t dev_id,
                                 bf_dev_family_t *dev_family);

#ifdef PORT_MGR_HA_UNIT_TESTING
/**
 * @brief Get the mac/port corrective action for a port as determined by the
 * port mgr
 *
 * @param[in] dev_id The device id
 * @param[in] dev_port The port identifier in a device
 * @param[out] ca Pointer to return mac/port corrective action req for the port
 *
 * @return Status of the API call.
 */
bf_status_t bf_ha_port_mgr_port_corrective_action_get(
    bf_dev_id_t dev_id, bf_dev_port_t dev_port, bf_ha_corrective_action_t *ca);

/**
 * @brief Get the serdes corrective action for a port as determined by the port
 * mgr
 *
 * @param[in] dev_id The device id
 * @param[in] dev_port The port identifier in a device
 * @param[out] ca Pointer to return serdes corrective action req for that port
 *
 * @return Status of the API call.
 */
bf_status_t bf_ha_port_mgr_serdes_corrective_action_get(
    bf_dev_id_t dev_id, bf_dev_port_t dev_port, bf_ha_corrective_action_t *ca);
#endif
/**
 * @brief Get the device type (model or asic)
 *
 * @param[in] dev_id The device id
 * @param[out] is_sw_model Pointer to bool flag to return true for model and
 * false for asic devices
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_device_type_get(bf_dev_id_t dev_id, bool *is_sw_model);

/**
 * @brief Check if the device type is a virtual device slave
 *
 * @param[in] dev_id The device id
 * @param[out] is_virtual_dev_slave Pointer to bool flag to return true for
 * virtual device slave and false otherwise
 *
 * @return Status of the API call.
 */
bf_status_t bf_drv_device_type_virtual_dev_slave(bf_dev_id_t dev_id,
                                                 bool *is_virtual_dev_slave);

/**
 * @brief Get the clock speed of the given device
 *
 * @param[in] dev_id The device id
 * @param[out] bps_clock_speed Pointer to return the bps clock speed
 * @param[out] pps_clock_speed Pointer to return the pps clock speed
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_get_clock_speed(bf_dev_id_t dev_id,
                                   uint64_t *bps_clock_speed,
                                   uint64_t *pps_clock_speed);

/*
 * @brief Set the timeout (ms) for the LRT descriptor ring timer
 * This is amount of time hardware waits for before shipping the buffer to sw,
 * hence this increases the utilization of these buffers by allowing hardware
 * pack in more messages per buffer
 *
 * @param[in] dev_id The device id
 * @param[in] timeout_ms The timeout in milliseconds for the timer
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_lrt_dr_timeout_set(bf_dev_id_t dev_id, uint32_t timeout_ms);

/*
 * @brief Get the timeout (ms) for the LRT descriptor ring
 * timer
 *
 * This is amount of time hardware waits for before shipping the buffer to sw
 * @param[in] dev_id The device id
 * @param[out] timeout_ms Pointer to return the timeout in milliseconds
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_lrt_dr_timeout_get(bf_dev_id_t dev_id, uint32_t *timeout_ms);

/*
 * @brief This function is used to complete the port mode transition workaround
 *
 * @param[in] dev_id The device id
 * @param[in] port_id The port identifier
 * @param[in] port_speed The supported speed of the port as defined in
 * bf_port_speed_t.
 *
 * @return Status of the API call
 */
bf_status_t bf_drv_complete_port_mode_transition_wa(
    bf_dev_id_t dev_id, bf_dev_port_t port_id, bf_port_speeds_t port_speed);

#endif  // BF_DRV_INTF_H_INCLUDED
