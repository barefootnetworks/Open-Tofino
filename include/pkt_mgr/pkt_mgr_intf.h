/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _PKT_MGR_INTF_H
#define _PKT_MGR_INTF_H

/**
 * @file pkt_mgr_intf.h
 *
 * @brief Packet manager interface APIs
 *
 */

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

#include "target-sys/bf_sal/bf_sys_dma.h"
#include "dvm/bf_dma_types.h"
#include "dvm/bf_drv_intf.h"
#include <pkt_mgr/bf_pkt.h>

#define PKT_MGR_NUM_DEVICES BF_MAX_DEV_COUNT
#define PKT_MGR_NUM_SUBDEVICES BF_MAX_SUBDEV_COUNT

/**
 * @addtogroup pkt_mgr-txrx
 * @{
 */

/**
 * Enum to define PKT TX ring
 */
typedef enum bf_pkt_tx_ring_e {
  BF_PKT_TX_RING_0,
  BF_PKT_TX_RING_1,
  BF_PKT_TX_RING_2,
  BF_PKT_TX_RING_3,
  BF_PKT_TX_RING_MAX
} bf_pkt_tx_ring_t;

/**
 * Enum to define PKT RX ring
 */
typedef enum bf_pkt_rx_ring_e {
  BF_PKT_RX_RING_0,
  BF_PKT_RX_RING_1,
  BF_PKT_RX_RING_2,
  BF_PKT_RX_RING_3,
  BF_PKT_RX_RING_4,
  BF_PKT_RX_RING_5,
  BF_PKT_RX_RING_6,
  BF_PKT_RX_RING_7,
  BF_PKT_RX_RING_MAX
} bf_pkt_rx_ring_t;

/**
 * Packet Transmit Done notification callback
 */
typedef bf_status_t (*bf_pkt_tx_done_notif_cb)(bf_dev_id_t dev_id,
                                               bf_pkt_tx_ring_t tx_ring,
                                               uint64_t tx_cookie,
                                               uint32_t status);

/**
 * Packet rx callback for processing
 */
typedef bf_status_t (*bf_pkt_rx_callback)(bf_dev_id_t dev_id,
                                          bf_pkt *pkt,
                                          void *cookie,
                                          bf_pkt_rx_ring_t rx_ring);

/**
 * @brief Check if the packet manager is initialized or not
 *
 * @param[in] dev_id Device identifier
 * @param[in] port Port identifier
 *
 * @return true: if packet manager is initialized, false: otherwise
 *
 */
bool bf_pkt_is_inited(bf_dev_id_t dev_id);

/**
 * @brief Register callback for RX packet processing
 *
 * @param[in] dev_id Device identifier
 * @param[in] cb Callback handler
 * @param[in] rx_ring RX DR ring index
 * @param[in] rx_cookie Cookie to pass back to callback function
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_rx_register(bf_dev_id_t dev_id,
                               bf_pkt_rx_callback cb,
                               bf_pkt_rx_ring_t rx_ring,
                               void *rx_cookie);

/**
 * @brief Deregister callback from RX packet processing
 *
 * @param[in] dev_id Device identifier
 * @param[in] rx_ring RX DR ring index
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_rx_deregister(bf_dev_id_t dev_id, bf_pkt_rx_ring_t rx_ring);

/**
 * @brief Transmit a packet
 *
 * @param[in] dev_id Device identifier
 * @param[in] pkt Packet to be transmitted
 * @param[in] tx_ring TX DR ring index
 * @param[in] tx_cookie Cookie to pass back to TX done notification
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_tx(bf_dev_id_t dev_id,
                      bf_pkt *pkt,
                      bf_pkt_tx_ring_t tx_ring,
                      void *tx_cookie);

/**
 * @brief Register callback for TX done notification
 *
 * @param[in] dev_id Device identifier
 * @param[in] cb Callback handler
 * @param[in] tx_ring TX DR ring index
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_tx_done_notif_register(bf_dev_id_t dev_id,
                                          bf_pkt_tx_done_notif_cb cb,
                                          bf_pkt_tx_ring_t tx_ring);

/**
 * @brief Deregister callback from TX done notification
 *
 * @param[in] dev_id Device identifier
 * @param[in] tx_ring TX DR ring index
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_tx_done_notif_deregister(bf_dev_id_t dev_id,
                                            bf_pkt_tx_ring_t tx_ring);

/**
 * @brief Packet manager lock device message handler
 *
 * @param[in] dev_id Device identifier
 *
 * @return Status of the API call
 *
 */
bf_status_t pkt_mgr_lock_device(bf_dev_id_t dev_id);

/**
 * @brief Packet manager unlock device message handler
 *
 * @param[in] dev_id Device identifier
 *
 * @return Status of the API call
 *
 */
bf_status_t pkt_mgr_unlock_device(bf_dev_id_t dev_id);

/**
 * @brief Check if the device is locked or not in packet manager
 *
 * @param[in] dev_id Device identifier
 *
 * @param[in] subdev_id subdevice id within the chip id
 *
 * @return true: if the device is locked, false: otherwise
 *
 */
bool bf_pkt_is_locked(bf_dev_id_t dev_id, bf_subdev_id_t subdev_id);

/* @} */

/**
 * @addtogroup pkt_mgr-pkt_mgr
 * @{
 */

/**
 * Structure for the TBUS configuration
 */
typedef struct bf_tbus_cfg_s {
  uint8_t pfc_fm_thr;        /** Num of free mem messages to turn on PFC */
  uint8_t pfc_rx_thr;        /** Mum of rx messages to turn on PFC */
  uint8_t ecc_dis;           /** Disable ECC decoder/checker */
  uint8_t crcrmv_dis;        /** Control bit to keep/remove crc32 */
  uint8_t port_en;           /** Enable MAC port for PCIE */
  uint8_t port_alive;        /** Port aliveness */
  uint16_t ts_offset;        /** Timestamp offset */
  uint8_t rx_channel_offset; /** RX channel byte offset, 4 bits */
  uint8_t crcerr_keep;       /** Enable keeping the CRC of input error pkt */
} bf_tbus_cfg_t;

/**
 * configure tbus control register
 *
 * @param chip
 *  chip id
 * @param subdev_id
 *  subdevice id within the chip id
 * @param tsu_cfg
 *  configuration param structure
 * @return
 *  0 on Success, -1 on error
 */
int bf_tbus_config(uint8_t chip,
                   bf_subdev_id_t subdev_id,
                   bf_tbus_cfg_t *tbus_cfg);

/**
 * @brief Flush TBUS DMA buffers
 *
 * @param[in] dev_id Device identifier
 *
 * @param[in] subdev_id subdevice id within the chip id
 *
 * @return 0 on success, -1 on error
 *
 */
int bf_tbus_flush_dma(uint8_t chip, bf_subdev_id_t subdev_id);

/**
 * @brief Set TBUS timestamp offset
 *
 * @param[in] chip Device identifier
 *
 * @param[in] subdev_id subdevice id within the chip id
 *
 * @param[in] offset Offset to be set
 *
 * @return 0 on success, -1 on error
 *
 */
int bf_tbus_set_ts_offset(uint8_t chip,
                          bf_subdev_id_t subdev_id,
                          uint16_t offset);

/**
 * @brief Set TBUS interrupt enable register
 *
 * @param[in] chip Device identifier
 * @param[in] subdev_id subdevice id within the chip id
 * @param[in] int_id Interrupt register enumeration ID
 * @param[in] high_prio Interrupt priority register to access,
 * 1 for high priority, 0 for low priority
 * @param[inout] Pointer to value to be read or written
 * @param[in] Flag to get or set, 1 for get, 0 for set
 *
 * @return 0 on success, -1 on error
 *
 */
int bf_tbus_int(uint8_t chip,
                bf_subdev_id_t subdev_id,
                int int_id,
                int high_prio,
                uint32_t *val,
                int get);

/**
 * @brief Service TX completions and packet receive DRs
 *
 * @param[in] chip Device identifier
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_dma_service_pkt(bf_dev_id_t chip);

/**
 * @brief Enable/disable packet tx and rx DR interrupts at leaf level
 *
 * @param[in] chip Device identifier
 * @param[in] en Flag to enable/disable
 *
 * @return Status of the API call
 *
 */
bf_status_t pkt_mgr_dr_int_en(bf_dev_id_t chip, bool en);

/**
 * @brief Initialize the driver. This should be called exactly once
 * before any other APIs are called.
 *
 * @return Status of the API call
 *
 */
bf_status_t bf_pkt_init();

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif
