/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_PDFIXED_PD_DEVPORT_MGR_H
#define _TOFINO_PDFIXED_PD_DEVPORT_MGR_H

/**
 * @file pd_devport_mgr.h
 *
 * @brief PD fixed device management APIs.
 *
 */

#include <tofino/pdfixed/pd_common.h>

typedef enum p4_devport_mgr_ha_corrective_action {
  HA_CA_PORT_NONE = 0,
  HA_CA_PORT_ADD,
  HA_CA_PORT_ENABLE,
  HA_CA_PORT_ADD_THEN_ENABLE,
  HA_CA_PORT_FLAP,
  HA_CA_PORT_DISABLE,
  HA_CA_PORT_DELETE_THEN_ADD,
  HA_CA_PORT_DELETE_THEN_ADD_THEN_ENABLE,
  HA_CA_PORT_DELETE,
  HA_CA_PORT_MAX
} p4_devport_mgr_ha_corrective_action_e;

/**
 * @brief Adds a port to a device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] port_id The ASIC port id.
 * @param[in] port_speeds The supported speed of the port as defined in
 * bf_port_speed_t.
 * @param[in] port_fec_types Port FEC type as defined in bf_fec_type_t.
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_add_port(uint32_t dev_id,
                                       uint32_t port_id,
                                       uint32_t port_speeds,
                                       uint32_t port_fec_types);

/**
 * @brief Adds a port to a device. Specify lane numbers.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] port_id The ASIC port id.
 * @param[in] port_speeds The supported speed of the port as defined in
 * bf_port_speed_t.
 * @param[in] port_lanes The lane number the port is using
 * @param[in] port_fec_types Port FEC type as defined in bf_fec_type_t.
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_add_port_with_lanes(uint32_t dev_id,
                                                  uint32_t port_id,
                                                  uint32_t port_speeds,
                                                  uint32_t port_lanes,
                                                  uint32_t port_fec_types);

/**
 * @brief Remove a port from a device.
 *
 * @param[in] dev_id The ASIC id.
 * @param[in] port_id The ASIC port id.
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_remove_port(uint32_t dev_id, uint32_t port_id);

/** This is an internal function used for testing only */
int p4_devport_mgr_port_ca_get(const int32_t dev_id, const int32_t port_id);

/** This is an internal function used for testing only */
int p4_devport_mgr_serdes_ca_get(const int32_t dev_id, const int32_t port_id);

typedef enum p4_devport_mgr_warm_init_mode {
  DEV_INIT_COLD,
  /** Device incurs complete reset */
  DEV_WARM_INIT_FAST_RECFG,
  /** Device incurs a fast-reconfig reset with minimal traffic disruption */
  DEV_WARM_INIT_HITLESS,
  /** Device incurs a hitless warm init */
} p4_devport_mgr_warm_init_mode_e;

typedef enum p4_devport_mgr_serdes_upgrade_mode {
  DEV_SERDES_UPD_NONE,
  /** No serdes update needed */
  DEV_SERDES_UPD_FORCED_PORT_RECFG,
  /** Update serdes by flapping ports immediately */
  DEV_SERDES_UPD_DEFERRED_PORT_RECFG
  /** Defer the serdes update until a port flap occurs */
} p4_devport_mgr_serdes_upgrade_mode_e;

/**
 * @brief Initiate a warm init process for a device
 *
 * @param[in] dev_id The device id
 * @param[in] warm_init_mode The warm init mode to use for this device
 * @param[in] serdes_upgrade_mode The mode to use for updating SerDes
 * @param[in] upgrade_agents Flag to denote whether to upgrade the agents
 * or not
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_warm_init_begin(
    uint32_t dev_id,
    p4_devport_mgr_warm_init_mode_e warm_init_mode,
    p4_devport_mgr_serdes_upgrade_mode_e serdes_upgrade_mode,
    bool upgrade_agents);

/**
 * @brief End the warm init sequence for the device and resume normal operation
 *
 * @param[in] dev_id The device id
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_warm_init_end(uint32_t dev_id);

typedef int (*warm_init_begin_fn)(
    bf_dev_id_t asic,
    p4_devport_mgr_warm_init_mode_e warm_init_mode,
    p4_devport_mgr_serdes_upgrade_mode_e serdes_upgrade_mode,
    bool upgrade_agents);
typedef int (*warm_init_end_fn)(bf_dev_id_t asic);

/** This is an internal function used for testing only */
p4_pd_status_t p4_devport_mgr_register_test_handlers(warm_init_begin_fn f1,
                                                     warm_init_end_fn f2);

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
 *
 */
p4_pd_status_t p4_devport_mgr_set_copy_to_cpu(bf_dev_id_t dev,
                                              bool enable,
                                              uint16_t port);

/**
 * @brief Get Pcie CPU port number on the device. Based on skew and asci-type
 * the API will return appropriate port number to be used as CPU pcie port.
 *
 * @param[in] dev The ASIC id.
 *
 * @return Valid pcie port. In case of error returns -1.
 *
 */
int p4_devport_mgr_pcie_cpu_port_get(bf_dev_id_t dev);

/**
 * @brief Get ethernet CPU port number on the device. Based on skew and
 *asci-type
 * the API will return appropriate port number to be used as CPU ethernet port.
 *
 * @param[in] dev The ASIC id.
 *
 * @return Valid eth port. In case of error returns -1.
 *
 */
int p4_devport_mgr_eth_cpu_port_get(bf_dev_id_t dev);

/**
 * @brief Set device mode to be a virtual device slave
 *
 * @param[in] dev_id The ASIC id.
 *
 * @return Status of the API call.
 *
 */
p4_pd_status_t p4_devport_mgr_set_virtual_dev_slave_mode(uint32_t dev_id);

/**
 * @brief Get the clock speed of the given device
 *
 * @param[in] dev_id The device id
 * @param[out] bps_clock_speed Pointer to return the bps clock speed
 * @param[out] pps_clock_speed Pointer to return the pps clock speed
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_devport_mgr_get_clock_speed(uint32_t dev_id,
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
 *
 */
p4_pd_status_t p4_devport_mgr_lrt_dr_timeout_set(uint32_t dev_id,
                                                 uint32_t timeout_ms);

/*
 * @brief This allows you to get the timeout (ms) for the LRT descriptor ring
 *timer
 *
 * This is amount of time hardware waits for before shipping the buffer to sw
 * @param[in] dev_id The device id
 * @param[out] timeout_ms Pointer to return the timeout in milliseconds
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_devport_mgr_lrt_dr_timeout_get(uint32_t dev_id,
                                                 uint32_t *timeout_ms);
#endif
