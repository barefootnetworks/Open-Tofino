/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_PDFIXED_PD_CONN_MGR_H
#define _TOFINO_PDFIXED_PD_CONN_MGR_H

/**
 * @file pd_conn_mgr.h
 *
 * @brief PD fixed client management APIs.
 *
 */

#include <tofino/pdfixed/pd_common.h>

/**
 * @brief Initialize the pipe manager service
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_init(void);

/**
 * @brief Cleanup the pipe manager service
 *
 * @return None
 */
void p4_pd_cleanup(void);

/**
 * @brief Create a client session
 *
 * @param[out] sess_hdl Pointer to return the session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_client_init(p4_pd_sess_hdl_t *sess_hdl);

/**
 * @brief Destroy the client session
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_client_cleanup(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Begin a transaction
 *
 * @param[in] sess_hdl Session handle
 * @param[in] isAtomic Flag to identify the transaction is atomic or not
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_begin_txn(p4_pd_sess_hdl_t shdl, bool isAtomic);

/**
 * @brief Verify the transaction
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_verify_txn(p4_pd_sess_hdl_t shdl);

/**
 * @brief Abort the transaction
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_abort_txn(p4_pd_sess_hdl_t shdl);

/**
 * @brief Commit the transaction
 *
 * @param[in] sess_hdl Session handle
 * @param[in] hwSynchronous Flag to be used to wait for
 * completion callbacks from HW
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_commit_txn(p4_pd_sess_hdl_t shdl, bool hwSynchronous);

/**
 * @brief Wait for completion callbacks from HW so that all DMA buffers
 * would be returned
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_complete_operations(p4_pd_sess_hdl_t shdl);

/**
 * @brief Start the batching for the session
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_begin_batch(p4_pd_sess_hdl_t shdl);

/**
 * @brief Push the queued DMA buffers to HW
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_flush_batch(p4_pd_sess_hdl_t shdl);

/**
 * @brief End the batching session
 *
 * @param[in] sess_hdl Session handle
 * @param[in] hwSynchronous Flag to be used to wait for
 * completion callbacks from HW
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_end_batch(p4_pd_sess_hdl_t shdl, bool hwSynchronous);

/**
 * @brief Log the device state
 *
 * @param[in] dev Device identifier
 * @param[in] filepath File to log the device state
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_log_state(bf_dev_id_t dev, const char *filepath);

/**
 * @brief Restore the device state
 *
 * @param[in] dev Device identifier
 * @param[in] filepath File used to restore the device state
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_restore_state(bf_dev_id_t dev, const char *filepath);

/* This is an internal API for testing purposes on model */
p4_pd_status_t p4_pd_advance_model_time(p4_pd_sess_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        uint64_t tick_time);

/**
 * @brief Enable recirculation for the given port
 *
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_recirculation_enable(p4_pd_sess_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          uint32_t port);

/**
 * @brief Disable recirculation for the given port
 *
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_recirculation_disable(p4_pd_sess_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           uint32_t port);

/**
 * @brief Enable packet generator for the given port
 *
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_enable(p4_pd_sess_hdl_t shdl,
                                   bf_dev_id_t dev,
                                   uint32_t port);

/**
 * @brief Disable packet generator for the given port
 *
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_disable(p4_pd_sess_hdl_t shdl,
                                    bf_dev_id_t dev,
                                    uint32_t port);

/**
 * @brief Get packet generator enable state for the given port
 *
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 * @param[out] enabled Pktgen enable state
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_enable_state_get(p4_pd_sess_hdl_t shdl,
                                             bf_dev_id_t dev,
                                             uint32_t port,
                                             bool *enabled);

/**
 * @brief Enable recirculation pattern matching for the given port
 *
 * @param[in] shdl Session handle
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_enable_recirc_pattern_matching(
    p4_pd_sess_hdl_t shdl, bf_dev_id_t dev, uint32_t port);

/**
 * @brief Disable recirculation pattern matching for the given port
 *
 * @param[in] shdl Session handle
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_disable_recirc_pattern_matching(
    p4_pd_sess_hdl_t shdl, bf_dev_id_t dev, uint32_t port);

/**
 * @brief Get recirculation pattern matching state for the given port
 *
 * @param[in] shdl Session handle
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 * @param[out] enabled Recirc pattern matching enable state
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_recirc_pattern_matching_state_get(
    p4_pd_sess_hdl_t shdl, bf_dev_id_t dev, uint32_t port, bool *enabled);

/**
 * @brief Clear packet generator port down for the given port
 *
 * @param[in] shdl Session handle
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_clear_port_down(p4_pd_sess_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            uint32_t port);

/**
 * @brief Get packet generator port down state for the given port
 *
 * @param[in] shdl Session handle
 * @param[in] dev Device identifier
 * @param[in] port Port identifier
 * @param[out] is_cleared Cleared state identifier
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_port_down_get(p4_pd_sess_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          uint32_t port,
                                          bool *is_cleared);

/**
 * @brief Configure packet gen for the given application id of Tofino
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] cfg Packet gen configuration parameters
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_cfg_app(p4_pd_sess_hdl_t shdl,
                                    p4_pd_dev_target_t dev_tgt,
                                    uint32_t app_id,
                                    p4_pd_pktgen_app_cfg cfg);

/**
 * @brief Configure packet gen for the given application id of Tofino2
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] cfg Packet gen configuration parameters
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_cfg_app_tof2(p4_pd_sess_hdl_t shdl,
                                         p4_pd_dev_target_t dev_tgt,
                                         uint32_t app_id,
                                         p4_pd_pktgen_app_cfg_tof2 cfg);

/**
 * @brief Get packet gen configuration for the given application id of Tofino
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[out] cfg Pointer to return the packet gen configuration parameters
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_cfg_app_get(p4_pd_sess_hdl_t shdl,
                                        p4_pd_dev_target_t dev_tgt,
                                        uint32_t app_id,
                                        p4_pd_pktgen_app_cfg *cfg);

/**
 * @brief Get packet gen configuration for the given application id of Tofino2
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[out] cfg Pointer to return the packet gen configuration parameters
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_cfg_app_tof2_get(p4_pd_sess_hdl_t shdl,
                                             p4_pd_dev_target_t dev_tgt,
                                             uint32_t app_id,
                                             p4_pd_pktgen_app_cfg_tof2 *cfg);

/**
 * @brief Configure packet gen port down mask for Tofino2
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] port_mask_sel Port mask select
 * @param[in] mask Port mask
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_port_down_msk_tof2(p4_pd_sess_hdl_t shdl,
                                               p4_pd_dev_target_t dev_tgt,
                                               uint32_t port_mask_sel,
                                               p4_pd_port_down_mask_tof2 mask);

/**
 * @brief Get packet gen port down mask for Tofino2
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] port_mask_sel Port mask select
 * @param[out] mask Port mask
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_port_down_msk_tof2_get(
    p4_pd_dev_target_t dev_tgt,
    uint32_t port_mask_sel,
    p4_pd_port_down_mask_tof2 *mask);

/**
 * @brief Configure packet gen port down replay mode
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] mode Packet gen port down replay mode
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_port_down_replay_mode_set(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_pktgen_port_down_mode_t mode);

/**
 * @brief Get packet gen port down replay mode
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[out] mode Pointer to return packet gen port down replay mode
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_port_down_replay_mode_get(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_pktgen_port_down_mode_t *mode);

/**
 * @brief Enable packet gen for the given application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_app_enable(p4_pd_sess_hdl_t shdl,
                                       p4_pd_dev_target_t dev_tgt,
                                       uint32_t app_id);

/**
 * @brief Disable packet gen for the given application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_app_disable(p4_pd_sess_hdl_t shdl,
                                        p4_pd_dev_target_t dev_tgt,
                                        uint32_t app_id);

/**
 * @brief Get packet gen app enable state for the given application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[out] enabled Packet gen application enable state
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_app_enable_state_get(p4_pd_sess_hdl_t shdl,
                                                 p4_pd_dev_target_t dev_tgt,
                                                 uint32_t app_id,
                                                 bool *enabled);

/**
 * @brief Write the given packet to the packet buffer of packet gen
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] offset Offset address for the packet
 * @param[in] size Size of the packet
 * @param[in] buf Pointer to the packet contents
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_write_pkt_buffer(p4_pd_sess_hdl_t shdl,
                                             p4_pd_dev_target_t dev_tgt,
                                             uint32_t offset,
                                             uint32_t size,
                                             uint8_t *buf);

/**
 * @brief Read the given packet to the packet buffer of packet gen
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] offset Offset address for the packet
 * @param[in] size Size of the packet
 * @param[in] buf Pointer to the packet contents
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_read_pkt_buffer(p4_pd_sess_hdl_t shdl,
                                            p4_pd_dev_target_t dev_tgt,
                                            uint32_t offset,
                                            uint32_t size,
                                            uint8_t *buf);

/**
 * @brief Get the batch counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[out] count Pointer to return the batch counter
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_get_batch_counter(p4_pd_sess_hdl_t shdl,
                                              p4_pd_dev_target_t dev_tgt,
                                              uint32_t app_id,
                                              uint64_t *count);

/**
 * @brief Get the packet counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[out] count Pointer to return the packet counter
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_get_pkt_counter(p4_pd_sess_hdl_t shdl,
                                            p4_pd_dev_target_t dev_tgt,
                                            uint32_t app_id,
                                            uint64_t *count);

/**
 * @brief Get the trigger counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[out] count Pointer to return the trigger counter
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_get_trigger_counter(p4_pd_sess_hdl_t shdl,
                                                p4_pd_dev_target_t dev_tgt,
                                                uint32_t app_id,
                                                uint64_t *count);

/**
 * @brief Set the batch counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[in] count Batch counter value
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_set_batch_counter(p4_pd_sess_hdl_t shdl,
                                              p4_pd_dev_target_t dev_tgt,
                                              uint32_t app_id,
                                              uint64_t count);

/**
 * @brief Set the packet counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[in] count Packet counter value
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_set_pkt_counter(p4_pd_sess_hdl_t shdl,
                                            p4_pd_dev_target_t dev_tgt,
                                            uint32_t app_id,
                                            uint64_t count);

/**
 * @brief Set the trigger counter for packet gen application id
 *
 * @param[in] shdl Session handle
 * @param[in] dev_tgt Device and pipe information
 * @param[in] app_id Packet gen application id
 * @param[in] count Trigger counter value
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pktgen_set_trigger_counter(p4_pd_sess_hdl_t shdl,
                                                p4_pd_dev_target_t dev_tgt,
                                                uint32_t app_id,
                                                uint64_t count);

/**
 * @brief Get the parser instance for the given port
 *
 * @param[in] device_id Device identifier
 * @param[in] port Port identifier
 * @param[out] parser_id Pointer to return the parser instance
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_parser_id_get(bf_dev_id_t device_id,
                                   int port,
                                   uint8_t *parser_id);

/**
 * @brief Get the pipe id for the given port
 *
 * @param[in] device_id Device identifier
 * @param[in] port Port identifier
 * @param[out] parser_id Pointer to return the pipe id
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_pipe_id_get(bf_dev_id_t device_id,
                                 int port,
                                 bf_dev_pipe_t *pipe_id);

/**
 * @brief Write the given value at the given register address
 *
 * @param[in] dev Device identifier
 * @param[in] addr Register address where to write the value
 * @param[in] data Value to be written at the register address
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_reg_wr(bf_dev_id_t dev, uint32_t addr, uint32_t data);

/**
 * @brief Read the value at the given register address
 *
 * @param[in] dev Device identifier
 * @param[in] addr Register address where to read the value
 *
 * @return Value at the given register address
 *
 */
uint32_t p4_pd_reg_rd(bf_dev_id_t dev, uint32_t addr);

/**
 * @brief Write the given value at the given wide register address
 *
 * @param[in] dev Device identifier
 * @param[in] addr Wide register address where to write the value
 * @param[in] data_hi Value (MSB) to be written at the register address
 * @param[in] data_lo Value (LSB) to be written at the register address
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ind_reg_wr(bf_dev_id_t dev,
                                uint64_t addr,
                                uint64_t data_hi,
                                uint64_t data_lo);

/**
 * @brief Write the given value at the given wide register address
 *
 * @param[in] dev Device identifier
 * @param[in] addr Wide register address where to write the value
 * @param[out] data_hi Pointer to return the value (MSB) at the register
 * address
 * @param[out] data_lo Pointer to return the value (LSB) at the register
 * address
 *
 * @return None
 *
 */
void p4_pd_ind_reg_rd(bf_dev_id_t dev,
                      uint64_t addr,
                      uint64_t *data_hi,
                      uint64_t *data_lo);

/**
 * @brief Set the timer value for the TCAM scrub timer
 *
 * @param[in] dev Device identifier
 * @param[in] msec_timer Timer value in ms
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_tcam_scrub_timer_set(bf_dev_id_t dev, uint32_t msec_timer);

/**
 * @brief Get the timer value for the TCAM scrub timer
 *
 * @param[in] dev Device identifier
 *
 * @return Timer value in ms
 *
 */
uint32_t p4_pd_tcam_scrub_timer_get(bf_dev_id_t dev);

/**
 * @brief Set the interrupt mode for learn digest DR
 *
 * @param[in] sess_hdl Session handle
 * @param[in] dev Device identifier
 * @param[in] en Enable/disable flag to set the interrupt mode
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_flow_lrn_set_intr_mode(p4_pd_sess_hdl_t sess_hdl,
                                            bf_dev_id_t dev,
                                            bool en);

/**
 * @brief Get the interrupt mode for learn digest DR
 *
 * @param[in] sess_hdl Session handle
 * @param[in] dev Device identifier
 * @param[out] en Pointer to return the interrupt mode
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_flow_lrn_get_intr_mode(p4_pd_sess_hdl_t sess_hdl,
                                            bf_dev_id_t dev,
                                            bool *en);

/*
 * Reprogram the GFM with the provided data patterns, the row parity will be
 * set to good (even) parity unless the row is marked for bad parity by the
 * caller.
 * @param  sess_hdl         Session handle
 * @param  dev_tgt          Target device and pipe(s)
 * @param  pipe_api_flags   Flags for the API, e.g. sychronous
 * @param  gress            Gress to be used when creating wide bubbles for
 *                          atomic GFM updates
 * @param  stage_id         Stage to update or 0xFF for all stages
 * @param  num_patterns     Number of entries in the row_patterns array
 * @param  row_patterns     Pointer to an array of data patterns to program.
 *                          The patterns will be applied to each row of the GFM,
 *                          the pattern used for any given row is given by
 *                          row_patterns[ row_num MOD num_patterns ]
 * @param  row_bad_parity   Pointer to an array of 16 u64s, any set bits
 *                          indicate the corresponding row should have bad
 *                          parity.
 * @return                  Status of the API call
 */
p4_pd_status_t p4_pd_gfm_test_pattern_set(p4_pd_sess_hdl_t sess_hdl,
                                          p4_pd_dev_target_t dev_tgt,
                                          uint32_t pipe_api_flags,
                                          bf_dev_direction_t gress,
                                          dev_stage_t stage_id,
                                          int num_patterns,
                                          uint64_t *row_patterns,
                                          uint64_t *row_bad_parity);

/*
 * Reprogram one GFM column (all 1024 rows) with the data provided.  This may
 * be used to atomically flip between good and bad parity for all 1024 rows of
 * GFM.
 * @param  shdl             Session handle
 * @param  dev_tgt          Target device and pipe(s)
 * @param  pipe_api_flags   Flags for the API, e.g. sychronous
 * @param  gress            Gress to be used when creating wide bubbles for
 *                          atomic GFM updates
 * @param  stage_id         Stage to update or 0xFF for all stages
 * @param  column           The column to reprogram, 0..51
 * @param  col_data         An array of 1024 bits given as 64 u16 values.  This
 *                          carries the data to program in the specified column
 *                          for all 1024 rows.
 * @return                  Status of the API call
 */
pipe_status_t p4_pd_gfm_test_col_set(p4_pd_sess_hdl_t shdl,
                                     p4_pd_dev_target_t dev_tgt,
                                     uint32_t pipe_api_flags,
                                     bf_dev_direction_t gress,
                                     dev_stage_t stage_id,
                                     int column,
                                     uint16_t col_data[64]);
#endif
