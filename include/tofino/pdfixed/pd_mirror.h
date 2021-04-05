/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _TOFINO_PDFIXED_PD_MIRROR_H
#define _TOFINO_PDFIXED_PD_MIRROR_H

/**
 * @file pd_mirror.h
 *
 * @brief PD fixed APIs for mirror.
 *
 */

#include <tofino/pdfixed/pd_common.h>
#include <tofino/pdfixed/pd_tm.h>

typedef struct p4_pd_mirror_session_info_s {
  p4_pd_mirror_type_e type; /** Mirror type */
  p4_pd_direction_t dir;    /** Mirror direction */
  p4_pd_mirror_id_t id;     /** Mirror session id */
  uint16_t egr_port;        /** Mirror session destination port */
  bool egr_port_v;          /** Flag to denote egress port is valid or not */
  p4_pd_tm_queue_t egr_port_queue; /** Queue for mirrored packet */
  p4_pd_color_t packet_color;      /** Packet color for mirrored packet */
  uint16_t mcast_grp_a;            /** Multicast group id - outer */
  bool mcast_grp_a_v;              /** Multicast group id - outer valid */
  uint16_t mcast_grp_b;            /** Multicast group id - inner */
  bool mcast_grp_b_v;              /** Multicast group id - inner vlaid */
  uint16_t max_pkt_len;            /** Maximum packet len of a mirrored copy */
  uint32_t level1_mcast_hash;      /** Outer hash value - to select path */
  uint32_t level2_mcast_hash;      /** Inner hash value - to select path */
  uint16_t mcast_l1_xid;           /** Multicast level 1 exclusion id */
  uint16_t mcast_l2_xid;           /** Multicast level 2 exclusion id */
  uint16_t mcast_rid;              /** Multicast ingress RID */
  uint8_t cos;                     /** CoS assigned to mirrored copy */
  bool c2c;                        /** Copy mirrored packet to cpu */
  uint16_t extract_len;            /** Number of bytes extracted from
                                    * each packet sample */
  uint32_t timeout_usec;           /** N*BaseTime, after which coalesced packet
                                    * is schedule for transmittion */
  uint32_t *int_hdr;               /** Internal Header upto 16Bytes, added to
                                    * coalesced packet.*/
  uint8_t int_hdr_len; /** Length of the internal header, in terms of word */
} p4_pd_mirror_session_info_t;

/**
 * @brief Create a mirror session
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe where this session is being created
 * @param[in] mirror_sess_info Mirror session configuration parameters
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_create(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_mirror_session_info_t *mirr_sess_info);

/**
 * @brief Update a mirror session
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe where this session is being updated
 * @param[in] mirror_sess_info Mirror session configuration parameters
 * @param[in] enable Flag to enable or disable the mirror session
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_update(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_mirror_session_info_t *mirr_sess_info,
    bool enable);

/**
 * @brief Delete a mirror session
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be deleted
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_delete(p4_pd_sess_hdl_t shdl,
                                           p4_pd_dev_target_t dev_tgt,
                                           p4_pd_mirror_id_t id);

/**
 * @brief Disable a mirror session created earlier
 *
 * @param[in] shdl API session handle
 * @param[in] dir Direction of the mirror session
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be disabled
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_disable(p4_pd_sess_hdl_t shdl,
                                            p4_pd_direction_t dir,
                                            p4_pd_dev_target_t dev_tgt,
                                            p4_pd_mirror_id_t id);

/**
 * @brief Enable a mirror session created earlier
 *
 * @param[in] shdl API session handle
 * @param[in] dir Direction of the mirror session
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be enabled
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_enable(p4_pd_sess_hdl_t shdl,
                                           p4_pd_direction_t dir,
                                           p4_pd_dev_target_t dev_tgt,
                                           p4_pd_mirror_id_t id);

/**
 * @brief Set the multicast pipe vector for a mirror session.  This is a bitmap
 * of
 * pipes for ports in the multicast groups used by the session.  For example, if
 * the session used multicast groups 1 and 2 and group 1 replicated to port 5
 * and group 2 replicated to ports 12 and 130 then the total set of ports would
 * be (5, 12, 130) and by taking the pipe each of those ports is in would give a
 * set of (0, 1) since 5 and 12 are in pipe 0 and 130 is in pipe 1.  This leads
 * to a pipe vector with bits zero and one set, 0x3
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt Device and pipe of the mirror session
 * @param[in] id Mirror session id to program
 * @param[in] logical_pipe_vector The pipe vector to program
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_pipe_vector_set(p4_pd_sess_hdl_t shdl,
                                                    p4_pd_dev_target_t dev_tgt,
                                                    p4_pd_mirror_id_t id,
                                                    int logical_pipe_vector);

/**
 * @brief Get the multicast pipe vector for a mirror session.
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt Device and pipe of the mirror session
 * @param[in] id Mirror session id to query
 * @param[out] logical_pipe_vector  The pipe vector is returned here
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_pipe_vector_get(p4_pd_sess_hdl_t shdl,
                                                    p4_pd_dev_target_t dev_tgt,
                                                    p4_pd_mirror_id_t id,
                                                    int *logical_pipe_vector);

/**
 * @brief Update flags in session meta entry. Only for Tofino2.
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be updated
 * @param[in] flag Flag that needs to be updated
 * @param[in] value Bool value to update
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_meta_flag_update(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_mirror_id_t id,
    p4_pd_mirror_meta_flag_e flag,
    bool value);

/**
 * @brief Update priority, default low priority. Only for Tofino2.
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be updated
 * @param[in] value Priority value, 1:low, 0:high
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_priority_update(p4_pd_sess_hdl_t shdl,
                                                    p4_pd_dev_target_t dev_tgt,
                                                    p4_pd_mirror_id_t id,
                                                    bool value);

/**
 * @brief Update coalescing mode. Only for Tofino2.
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] id Mirror session id to be updated
 * @param[in] value Mode value, 1:same with tofino, 0: tofino2 mode
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_coal_mode_update(p4_pd_sess_hdl_t shdl,
                                                     p4_pd_dev_target_t dev_tgt,
                                                     p4_pd_mirror_id_t id,
                                                     bool value);

/**
 * @brief Get the first existing mirror session information
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[out] first_mirr_sess_info Mirror session parameters
 * @param[out] first_mirr_sess_pipe_id Pipe id the session was configured
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_get_first(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_mirror_session_info_t *first_mirr_sess_info,
    bf_dev_pipe_t *first_mirr_sess_pipe_id);

/**
 * @brief Get the next existing mirror session info
 *
 * @param[in] shdl API session handle
 * @param[in] dev_tgt  Device and pipe of the mirror session
 * @param[in] current_mirr_id Mirror session id value after which search will
 * begin
 * @param[in] current_mirr_sess_pipe_id Current mirror session id's pipe id
 * @param[out]  next_mirr_sess_info Next mirror session parameters
 * @param[out]  next_mirr_sess_pipe_id Pipe id of next mirror sesssion
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_get_next(
    p4_pd_sess_hdl_t shdl,
    p4_pd_dev_target_t dev_tgt,
    p4_pd_mirror_id_t current_mirr_id,
    bf_dev_pipe_t current_mirr_sess_pipe_id,
    p4_pd_mirror_session_info_t *next_mirr_sess_info,
    bf_dev_pipe_t *next_mirr_sess_pipe_id);

/**
 * @brief Get max mirror sessions
 *
 * @param[in] shdl API session handle
 * @param[in] device_id Device identifier
 * @param[in] mir_type Type of mirror session
 * @param[out] id Pointer to return the max mirror session id
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_max_session_id_get(
    p4_pd_sess_hdl_t shdl,
    bf_dev_id_t device_id,
    p4_pd_mirror_type_e mir_type,
    p4_pd_mirror_id_t *id);

/**
 * @brief Get base session id
 *
 * @param[in] shdl API session handle
 * @param[in] device_id Device identifier
 * @param[in] mir_type Type of mirror session
 * @param[out] id Pointer to return the base mirror session id
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mirror_session_base_session_id_get(
    p4_pd_sess_hdl_t shdl,
    bf_dev_id_t device_id,
    p4_pd_mirror_type_e mir_type,
    p4_pd_mirror_id_t *id);
#endif
