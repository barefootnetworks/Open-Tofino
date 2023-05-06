/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef port_mgr_intf_included
#define port_mgr_intf_included

typedef enum {
  PORT_MGR_OK = 0,
  PORT_MGR_ERR_BAD_PARM,
  PORT_MGR_ERR_NOT_READY,
  PORT_MGR_ERR_LOCK_FAILED,
  PORT_MGR_ERR_INVALID_CFG,
  PORT_MGR_ERR,
} port_mgr_err_t;

/*
 * User-defined function via bf_bind_mac_stats_cb to be called
 * whenever the a MAC stats DMA operation completes. The msg_id
 * field will match that given in the call to bf_port_mac_stats_get
 * that initiated the DMA.
 */
typedef void (*bf_mac_stats_callback_t)(bf_dev_id_t dev_id,
                                        int status,
                                        uint64_t msg_id,
                                        uint64_t dma_timestamp_nsec);

typedef void (*bf_port_intbh_wakeup_callback_t)(bf_dev_id_t dev_id);

/**
 * @brief Register a user defined callback to be issued upon completion
 *        of MAC stats DMA operations for a given dev_id.
 *
 * @param[in] dev_id Device id system-assigned identifier
 *(0..BF_MAX_DEV_COUNT-1)
 * @param[in] fn  User defined callback function - bf_mac_stats_callback_t
 *
 * @return BF_SUCCESS    : callback registered successfully
 * @return BF_INVALID_ARG : dev_id never added or dev_id > BF_MAX_DEV_COUNT-1
 */
bf_status_t bf_bind_mac_stats_cb(bf_dev_id_t dev_id,
                                 bf_mac_stats_callback_t fn);

void port_mgr_init(void);
bf_status_t port_mgr_stats_persistent_get(bf_dev_id_t dev_id, bool *enable);
bf_status_t port_mgr_stats_persistent_set(bf_dev_id_t dev_id, bool enable);
bf_status_t port_mgr_intbh_init(void);
bf_status_t port_mgr_wakeup_intbh(void);
bf_status_t port_mgr_check_port_int(void);
bf_status_t port_mgr_port_bind_int_bh_wakeup_callback(
    bf_dev_id_t dev_id, bf_port_intbh_wakeup_callback_t fn);
#endif  // port_mgr_intf_included
