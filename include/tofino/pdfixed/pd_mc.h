/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _TOFINO_PDFIXED_PD_MC_H
#define _TOFINO_PDFIXED_PD_MC_H

/**
 * @file pd_mc.h
 *
 * @brief PD fixed APIs for multicast manager.
 *
 */

#include <tofino/pdfixed/pd_common.h>

/**
 * @brief Initialize the driver. This should be called exactly once before any
 * other
 * APIs are called.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_init();

/* Multicast session APIs. */

/**
 * @brief Create a session within the driver.
 *
 * @param[out] shdl Pointer to the session handle which will be filled in.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_create_session(p4_pd_sess_hdl_t *sess_hdl);

/**
 * @brief Destroy a session within the driver.
 *
 * @param[in] shdl The session handle associated with the session to clean up.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_destroy_session(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Wait for completion callbacks from HW outstanding DMA buffers.
 *
 * @param[in] shdl The session handle associated with the session.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_complete_operations(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Begin a batch on a session.  All hardware programming will be held
 * until
 * the batch is ended and then pushed to hardware in as few DMA operations as
 * possible.
 *
 * @param[in] shdl The session handle associated with the session.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_begin_batch(p4_pd_sess_hdl_t shdl);

/**
 * @brief Flush a batch on a session pushing all pending updates to hardware.
 *
 * @param[in] shdl The session handle associated with the session.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_flush_batch(p4_pd_sess_hdl_t shdl);

/**
 * @brief End a batch on a session. All queued hardware updates will be pushed
 * to the hardward.
 *
 * @param[in] shdl The session handle associated with the session.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_end_batch(p4_pd_sess_hdl_t shdl, bool hwSynchronous);

/* Multicast group APIs. */
/**
 * @brief Allocate a multicast group id.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] grp The multicast group id to reserve.  Note that this is the
 * value
 * which must be programmed into any pipeline match table result for
 * packets to use this multicast group.
 * @param[out] hdl Pointer to the group handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_create(p4_pd_sess_hdl_t sess_hdl,
                                    bf_dev_id_t dev,
                                    uint16_t grp,
                                    p4_pd_entry_hdl_t *grp_hdl);

/**
 * @brief Get a multicast group's attributes.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl Handle of the group to query.
 * @param[out] grp Buffer to be filled with the multicast group.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_attr(p4_pd_sess_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      p4_pd_entry_hdl_t mgrp_hdl,
                                      uint16_t *grp);

/**
 * @brief Release a multicast group id.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl Handle of the group to release.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_destroy(p4_pd_sess_hdl_t sess_hdl,
                                     bf_dev_id_t dev,
                                     p4_pd_entry_hdl_t grp_hdl);

/**
 * @brief Get first multicast group id.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] mgrp_hdl Pointer to first multicast group id handle will be
 * returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_first(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       p4_pd_entry_hdl_t *mgrp_hdl);

/**
 * @brief Retreive quantity of multicast group ids.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] count Pointer to return the number of total multicast group ids.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_count(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       uint32_t *count);

/**
 * @brief Get next i multicast group ids. If there is at least 1 but not i ids,
 * the
 * remaining space in the array will be filled with -1.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl Pointer to multicast group from which search begins.
 * @param[in] i Number of multicast group ids to retreive.
 * @param[out] next_mgrp_hdls An array of at least i multicast group ids
 *handles.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_next_i(p4_pd_sess_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        p4_pd_entry_hdl_t mgrp_hdl,
                                        uint32_t i,
                                        p4_pd_entry_hdl_t *next_mgrp_hdls);

/**
 * @brief Get the first node member of the multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[out] node_hdl Pointer to a node handle which will be returned.
 * @param[out] node_l1_xid_valid Pointer to a bool to return the l1 xid valid
 * status.
 * @param[out] node_l1_xid Pointer to a l1 xid which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_first_node_mbr(p4_pd_sess_hdl_t shdl,
                                                bf_dev_id_t dev,
                                                p4_pd_entry_hdl_t mgrp_hdl,
                                                p4_pd_entry_hdl_t *node_hdl,
                                                bool *xid_valid,
                                                uint16_t *xid);

/**
 * @brief Get the count of node members in the specified multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[out] count Pointer to a uint32_t to return the quantity of node
 * members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_node_mbr_count(p4_pd_sess_hdl_t shdl,
                                                bf_dev_id_t dev,
                                                p4_pd_entry_hdl_t mgrp_hdl,
                                                uint32_t *count);

/**
 * @brief Get the next i node members in the multicast group after the
 * parameterized member. If there is at least 1 but not i members, then the
 * remaining space in the array will be filled with invlaid handles.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[in] node_hdl The node handle at which to start the query.
 * @param[in] i The quantity of node handles to return.
 * @param[out] next_node_hdls An array of node handles with enough space to fit
 * at least i node handles.
 * @param[out] next_node_l1_xids_valid An array of bool with enough space to fit
 * at least i nodes' l1 xid valid status.
 * @param[out] next_node_l1_xids An array of l1 xids with enough space to fit
 * at least i nodes' l1 xid values.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_next_i_node_mbr(
    p4_pd_sess_hdl_t shdl,
    bf_dev_id_t dev,
    p4_pd_entry_hdl_t mgrp_hdl,
    p4_pd_entry_hdl_t node_hdl,
    uint32_t i,
    p4_pd_entry_hdl_t *next_node_hdls,
    bool *next_node_xids_valid,
    uint16_t *next_node_xids);

/**
 * @brief Get the first ECMP member of the multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[out] ecmp_hdl Pointer to a ECMP handle which will be returned.
 * @param[out] xid_valid Pointer to a bool to return the l1 xid valid status.
 * @param[out] ecmp_xid Pointer to a l1 xid which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_first_ecmp_mbr(p4_pd_sess_hdl_t shdl,
                                                bf_dev_id_t dev,
                                                p4_pd_entry_hdl_t mgrp_hdl,
                                                p4_pd_entry_hdl_t *ecmp_hdl,
                                                bool *xid_valid,
                                                uint16_t *ecmp_xid);

/**
 * @brief Get the count of ECMP members in the specified multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[out] count Pointer to a uint32_t to return the quantity of ECMP
 * members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_ecmp_mbr_count(p4_pd_sess_hdl_t shdl,
                                                bf_dev_id_t dev,
                                                p4_pd_entry_hdl_t mgrp_hdl,
                                                uint32_t *count);

/**
 * @brief Get the next i ECMP members in the multicast group after the
 * parameterized member. If there is at least 1 but not i members, then the
 * remaining space in the array will be filled with invalid handles.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The multicast group handle.
 * @param[in] ecmp_hdl The ECMP handle at which to start the query.
 * @param[in] i The quantity of ECMP handles to return.
 * @param[out] next_ecmp_hdls An array of ECMP handles with enough space to fit
 * at least i ECMP handles.
 * @param[out] next_ecmp_xids_valid An array of bool with enough space to fit
 * at least i ECMPs' l1 xid valid status.
 * @param[out] next_ecmp_xids An array of l1 xids with enough space to fit
 * at least i ECMPs' l1 xid values.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_mgrp_get_next_i_ecmp_mbr(
    p4_pd_sess_hdl_t shdl,
    bf_dev_id_t dev,
    p4_pd_entry_hdl_t mgrp_hdl,
    p4_pd_entry_hdl_t ecmp_hdl,
    uint32_t i,
    p4_pd_entry_hdl_t *next_ecmp_hdls,
    bool *next_ecmp_xids_valid,
    uint16_t *next_ecmp_xids);

/**
 * @brief Get the member of the group associated with the ECMP handle based on
 * the input hash value.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[in] level1_mcast_hash The hash value of the packet.
 * @param[in] pkd_xid The L1 exclusion ID of the packet.
 * @param[out] node_hdl Pointer to a node handle which will be
 * returned. Returns 0 if invalid (e.g. empty ecmp group).
 * @param[out] is_pruned Is the chosen node pruned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_mbr_from_hash(p4_pd_sess_hdl_t shdl,
                                               bf_dev_id_t dev,
                                               p4_pd_entry_hdl_t mgrp_hdl,
                                               p4_pd_entry_hdl_t ecmp_hdl,
                                               uint16_t level1_mcast_hash,
                                               uint16_t pkt_xid,
                                               p4_pd_entry_hdl_t *node_hdl,
                                               bool *is_pruned);

/* Multicast ECMP APIs. */
/**
 * @brief Create an ECMP group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] ecmp_hdl Pointer to the group handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_create(p4_pd_sess_hdl_t sess_hdl,
                                    bf_dev_id_t dev,
                                    p4_pd_entry_hdl_t *ecmp_hdl);

/**
 * @brief Release an ECMP group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl Handle of the group to release.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_destroy(p4_pd_sess_hdl_t sess_hdl,
                                     bf_dev_id_t dev,
                                     p4_pd_entry_hdl_t ecmp_hdl);

/**
 * @brief Get the first ECMP handle.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] ecmp_hdl Pointer to a ECMP handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_first(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       p4_pd_entry_hdl_t *ecmp_hdl);

/**
 * @brief Get the current quantity of ECMP groups.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] count Pointer to return the number of ECMP groups.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_count(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       uint32_t *count);

/**
 * @brief Get the next i ECMP handles after the parameterized handle. If there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle at which to start the query.
 * @param[in] i The quantity of ECMP handles to return.
 * @param[out] next_ecmp_hdls An array of ECMP handles with enough space to fit
 * at least i ECMP handles.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_next_i(p4_pd_sess_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        p4_pd_entry_hdl_t ecmp_hdl,
                                        uint32_t i,
                                        p4_pd_entry_hdl_t *next_ecmp_hdls);

/**
 * @brief Add a member (a node) to an ECMP group.  The node cannot be in any
 * other
 * ECMP groups or be associated with a multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl Handle of the ecmp group.
 * @param[in] node_hdl Handle of the node.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_mbr_add(p4_pd_sess_hdl_t sess_hdl,
                                     bf_dev_id_t dev,
                                     p4_pd_entry_hdl_t ecmp_hdl,
                                     p4_pd_entry_hdl_t l1_hdl);

/**
 * @brief Remove a member (a node) from an ECMP group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl Handle of the ecmp group.
 * @param[in] node_hdl Handle of the node.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_mbr_rem(p4_pd_sess_hdl_t sess_hdl,
                                     bf_dev_id_t dev,
                                     p4_pd_entry_hdl_t ecmp_hdl,
                                     p4_pd_entry_hdl_t l1_hdl);

/**
 * @brief Get the first member of the group associated with the ECMP handle.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[out] node_hdl Pointer to a node handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_first_mbr(p4_pd_sess_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           p4_pd_entry_hdl_t ecmp_hdl,
                                           p4_pd_entry_hdl_t *node_hdl);

/**
 * @brief Get the current quantity members in the specified ECMP group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[out] count Pointer to a uint32_t representing the quantity of members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_mbr_count(p4_pd_sess_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           p4_pd_entry_hdl_t ecmp_hdl,
                                           uint32_t *count);

/**
 * @brief Get the next i ECMP group members after the parameterized member. If
 * there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[in] node_hdl The node handle at which to start the query.
 * @param[in] i The quantity of node handles to return.
 * @param[out] next_ecmp_hdls An array of node handles with enough space to fit
 * at least i node handles.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_next_i_mbr(p4_pd_sess_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            p4_pd_entry_hdl_t ecmp_hdl,
                                            p4_pd_entry_hdl_t node_hdl,
                                            uint32_t i,
                                            p4_pd_entry_hdl_t *next_node_hdls);

/**
 * @brief Get the first association of the parameterized ECMP group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[out] mgrp_hdl Pointer to a node handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_first_assoc(p4_pd_sess_hdl_t shdl,
                                             bf_dev_id_t dev,
                                             p4_pd_entry_hdl_t ecmp_hdl,
                                             p4_pd_entry_hdl_t *mgrp_hdl);

/**
 * @brief Get the current quantity of associations with the specified ECMP
 * group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[out] count Pointer to a uint32_t representing the quantity of members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_assoc_count(p4_pd_sess_hdl_t shdl,
                                             bf_dev_id_t dev,
                                             p4_pd_entry_hdl_t ecmp_hdl,
                                             uint32_t *count);

/**
 * @brief Get the next i ECMP associations after the parameterized association.
 * If there is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] ecmp_hdl The ECMP handle in question.
 * @param[in] mgrp_hdl The node handle at which to start the query.
 * @param[in] i The quantity of node handles to return.
 * @param[out] next_ecmp_hdls An array of node handles with enough space to fit
 * at least i node handles.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_next_i_assoc(
    p4_pd_sess_hdl_t shdl,
    bf_dev_id_t dev,
    p4_pd_entry_hdl_t ecmp_hdl,
    p4_pd_entry_hdl_t mgrp_hdl,
    uint32_t i,
    p4_pd_entry_hdl_t *next_mgrp_hdls);

/**
 * @brief Associate an ECMP group with a multicast group.  Unlike individual
 * nodes
 * ECMP groups may be associated with many multicast groups.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The handle of the multicast group.
 * @param[in] ecmp_hdl The handle of the ECMP group.
 * @param[in] xid L1 exclusion id assigned to the ECMP group in the
 * specified multicast group.
 * @param[in] xid_valid A bool indicating if the exclusion id is
 * needed.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_associate_ecmp(p4_pd_sess_hdl_t sess_hdl,
                                       bf_dev_id_t dev,
                                       p4_pd_entry_hdl_t grp_hdl,
                                       p4_pd_entry_hdl_t ecmp_hdl,
                                       uint16_t xid,
                                       bool xid_valid);

/**
 * @brief Get attributes of an ecmp - multicast group association.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The handle of the multicast group.
 * @param[in] ecmp_hdl The handle of the ECMP group.
 * @param[out] xid_valid Pointer to bool indicating if the exclusion
 * id is needed.
 * @param[out] xid Pointer to L1 exclusion id assigned to the ECMP
 * group in the specified multicast group.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_ecmp_get_assoc_attr(p4_pd_sess_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            p4_pd_entry_hdl_t mgrp_hdl,
                                            p4_pd_entry_hdl_t ecmp_hdl,
                                            bool *xid_valid,
                                            uint16_t *xid);

/**
 * @brief Remove an ECMP group from a multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The handle of the multicast group.
 * @param[in] ecmp_hdl The handle of the ECMP group.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_dissociate_ecmp(p4_pd_sess_hdl_t sess_hdl,
                                        bf_dev_id_t dev,
                                        p4_pd_entry_hdl_t grp_hdl,
                                        p4_pd_entry_hdl_t ecmp_hdl);

/* Multicast L1-Node APIs. */
/**
 * @brief Create a node.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] rid The RID to assign to the node
 * @param[in] port_map Member ports belonging to the node.
 * @param[in] lag_map Member LAGs belonging to the node.
 * @param[out] node_hdl Pointer to a node handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_create(p4_pd_sess_hdl_t sess_hdl,
                                    bf_dev_id_t dev,
                                    uint16_t rid,
                                    uint8_t *port_map,
                                    uint8_t *lag_map,
                                    p4_pd_entry_hdl_t *node_hdl);

/**
 * @brief Get a node's attributes.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl Node for which to query attributes
 * @param[out] rid Pointer to buffer to be filled with rid.
 * @param[out] port_map Pointer to buffer to be filled with the port_map.
 * @param[out] lag_map Pointer to buffer to be filled with the lag_map.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_get_attr(p4_pd_sess_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      p4_pd_entry_hdl_t node_hdl,
                                      uint16_t *rid,
                                      uint8_t *port_map,
                                      uint8_t *lag_map);

/**
 * @brief Update the membership of a node.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl Handle of the node.
 * @param[in] port_map The new set of ports belonging to the node.  Note that
 * this set replaces any existing members.
 * @param[in] lag_map The new set of LAGs belonging to the node.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_update(p4_pd_sess_hdl_t sess_hdl,
                                    bf_dev_id_t dev,
                                    p4_pd_entry_hdl_t node_hdl,
                                    uint8_t *port_map,
                                    uint8_t *lag_map);

/**
 * @brief Destroy a node.  The node must first be removed from any ECMP groups
 * or multicast groups before it can be deleted.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl The handle of the node to delete.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_destroy(p4_pd_sess_hdl_t sess_hdl,
                                     bf_dev_id_t dev,
                                     p4_pd_entry_hdl_t node_hdl);

/**
 * @brief Get the first multicast node.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] node_hdl Pointer to a node handle which will be returned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_get_first(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       p4_pd_entry_hdl_t *node_hdl);

/**
 * @brief Get the quantity of multicast nodes.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[out] count Pointer to return the number of multicast nodes.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_get_count(p4_pd_sess_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       uint32_t *count);

/**
 * @brief Get the next i multicast nodes after the parameterized node hdl. If
 * there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl The node handle where the query starts.
 * @param[in] i The number of node handles to retreive.
 * @param[out] next_node_hdls An array with space for at least i node handles.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_get_next_i(p4_pd_sess_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        p4_pd_entry_hdl_t node_hdl,
                                        uint32_t i,
                                        p4_pd_entry_hdl_t *next_node_hdls);

/**
 * @brief Returns whether the parameterized node is associated, and information
 * about
 * the association if it is. If a pointer param is null, the information won't
 * be returned.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl The node handle in question.
 * @param[out] is_associated Pointer to a bool representing association status.
 * @param[out] mgrp_hdl Pointer to multicast group which is filled with
 * associated
 * group.
 * @param[out] Pointer to buffer to be filled with xid validity.
 * @param[out] Pointer to buffer to be filled with the xid.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_get_association(p4_pd_sess_hdl_t shdl,
                                             bf_dev_id_t dev,
                                             p4_pd_entry_hdl_t node_hdl,
                                             bool *is_associated,
                                             p4_pd_entry_hdl_t *mgrp_hdl,
                                             bool *xid_valid,
                                             uint16_t *xid);

/**
 * @brief Returns whether the parameterized node is a member of an ECMP group.
 * If a pointer param is null, the information won't be returned.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl The node handle in question.
 * @param[out] is_ecmp_mbr Pointer to a bool representing member status.
 * @param[out] ecmp_hdl Pointer to an ECMP group which is filled with the
 * ECMP group (if is_ecmp_mbr is true).
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_node_is_mbr(p4_pd_sess_hdl_t shdl,
                                    bf_dev_id_t dev,
                                    p4_pd_entry_hdl_t node_hdl,
                                    bool *is_ecmp_mbr,
                                    p4_pd_entry_hdl_t *ecmp_hdl);

/**
 * @brief Associate a node with a multicast group.  The node cannot be
 * associated with
 * any other multicast groups or ECMP groups.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The handle of the multicast group.
 * @param[in] node_hdl The handle of the node.
 * @param[in] xid L1 exclusion id assigned to the node.
 * @param[in] xid_valid A bool indicating if the exclusion id is
 * needed.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_associate_node(p4_pd_sess_hdl_t sess_hdl,
                                       bf_dev_id_t dev,
                                       p4_pd_entry_hdl_t grp_hdl,
                                       p4_pd_entry_hdl_t l1_hdl,
                                       uint16_t xid,
                                       bool xid_valid);

/**
 * @brief Remove a node from a multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] mgrp_hdl The handle of the multicast group.
 * @param[in] node_hdl The handle of the node.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_dissociate_node(p4_pd_sess_hdl_t sess_hdl,
                                        bf_dev_id_t dev,
                                        p4_pd_entry_hdl_t grp_hdl,
                                        p4_pd_entry_hdl_t node_hdl);

/* Multicast misc APIs. */
/**
 * @brief Update the LAG membership table.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] lag_id The LAG id.
 * @param[in] port_map The new set of ports belonging to the LAG.  Note that
 * this set replaces any existing members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_lag_membership(p4_pd_sess_hdl_t sess_hdl,
                                           bf_dev_id_t dev,
                                           uint8_t lag,
                                           uint8_t *port_map);

/**
 * @brief Get entry from the LAG membership table.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] lag_id The LAG id.
 * @param[out] port_map Buffer for the set of ports belonging to the LAG.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_get_lag_membership(p4_pd_sess_hdl_t sess_hdl,
                                           bf_dev_id_t dev,
                                           uint8_t lag,
                                           uint8_t *port_map);

/**
 * @brief Get port from the LAG and the level 2 hash.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_hdl The handle of the node that owns this LAG.
 * @param[in] lag The LAG id.
 * @param[in] level2_mcast_hash The hash to determine the LAG port.
 * @param[in] pkt_xid The xid of the ig pkt for pruning.
 * @param[in] pkt_yid The yid of the ig pkt for pruning.
 * @param[in] pkt_rid The rid of the ig pkt for pruning.
 * @param[out] port Pointer to the value of the chosen port.
 * Returns 0xffff if invalid (e.g. empty lag group, remote index, etc).
 * @param[out] is_pruned Is the chosen port pruned.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_get_lag_member_from_hash(p4_pd_sess_hdl_t sess_hdl,
                                                 bf_dev_id_t dev,
                                                 p4_pd_entry_hdl_t node_hdl,
                                                 uint8_t lag,
                                                 uint16_t level2_mcast_hash,
                                                 uint16_t pkt_xid,
                                                 uint16_t pkt_yid,
                                                 uint16_t pkt_rid,
                                                 int *port,
                                                 bool *is_pruned);

/**
 * @brief Update one ASIC with the LAG membership counts of other ASICs.  When a
 * LAG
 * member is selected, the index of the lag member is taken as x mod y where x
 * is the hash value and y is the number of LAG members.  For example, a LAG of
 * two members and a hash value of 10 will select member 10 % 2, which is zero.
 * The left and right counts set by this API will increase the member count of
 * the LAG when selecting a member.  However, if the index is equal to or less
 * than right count or greater than right count plus the number of LAG members
 * the ASIC will not select a LAG member.  This can be used to allow a LAG
 * spanning multiple ASICs to have the same member selected on each ASIC and
 * only the ASIC owning that member will send a packet.  For example consider
 * 4 ASICs (A,B,C and D) with 2, 3, 6 and 8 members respectivly.  ASIC C (local)
 * would have a left(msb) count  of 2+3=5 and a right(lsb) count of 8.
 * When selecting a member, it would use index = hash % (5 + 6 + 8).
 * If the index is 0-7, it is assumed ASIC D (right/lsb) will make a copy so
 * ASIC C will not.
 * If the index is 8-13 ASIC C will select local LAG member 0-5.
 * If the index is more than 13 it is assumed either ASIC A or B (left/msb)
 * will make a copy so ASIC C will not.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] lag_id The LAG id.
 * @param[in] left_count The number of LAG members on all ASICs to the "left" of
 * this ASIC.
 * @param[in] right_count The number of LAG members on all ASICs to the "right"
 * of
 * this ASIC.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_remote_lag_member_count(p4_pd_sess_hdl_t sess_hdl,
                                                    bf_dev_id_t dev,
                                                    uint8_t lag,
                                                    int left,
                                                    int right);

/**
 * @brief Program the L2 exclusion id pruning table.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] l2_exclusion_id The table index to program.
 * @param[in] pruned_ports The new set of ports to prune.  Note that this set
 * replaces replaces any existing members.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_update_port_prune_table(p4_pd_sess_hdl_t sess_hdl,
                                                bf_dev_id_t dev,
                                                uint16_t yid,
                                                uint8_t *port_map);

/**
 * @brief Read the L2 exclusion id pruning table.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] yid The table index to read.
 * @param[out] port_map The data will be read into this pointer.
 * @param[in] from_hw Read from the software shadow or hardware.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_get_port_prune_table(p4_pd_sess_hdl_t sess_hdl,
                                             bf_dev_id_t dev,
                                             uint16_t yid,
                                             uint8_t *port_map,
                                             bool from_hw);

/**
 * @brief Program the global RID.  Packets with ingress RIDs matching this value
 * will
 * be subject to L2 pruning using the values configured with
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] rid The RID value to program.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_global_rid(p4_pd_sess_hdl_t sess_hdl,
                                       bf_dev_id_t dev,
                                       uint16_t rid);

/**
 * @brief Update the multicast forwarding state of a port.  If set to "inactive"
 * the
 * multicast engine will treat the port as if it is down when the multicast
 * engine is generating copies. By default, all ports are in the forwarding
 * state.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] port_id The port to set.
 * @param[in] is_active @c false mark the port as inactive, @c true mark the
 *port
 * as active.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_port_mc_fwd_state(p4_pd_sess_hdl_t sess_hdl,
                                              bf_dev_id_t dev,
                                              uint16_t port,
                                              bool is_active);

/**
 * @brief Enable HW fast failover on port down events.  If the hardware detects
 * a port
 * has gone down traffic will automatically re-hash to another LAG member or,
 * if enabled, traffic will be sent over the backup port.
 * By default this is disabled.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_enable_port_ff(p4_pd_sess_hdl_t sess_hdl,
                                       bf_dev_id_t dev);

/**
 * @brief Disable HW fast failover on port down events.  By default this is
 * disabled.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_disable_port_ff(p4_pd_sess_hdl_t sess_hdl,
                                        bf_dev_id_t dev);

/**
 * @brief Clear the HW fast failover state on a port.  This must be called to
 * bring
 * the port back to a forwarding state once it has come up after going down.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] port_id The port to set.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_clr_port_ff_state(p4_pd_sess_hdl_t sess_hdl,
                                          bf_dev_id_t dev,
                                          int port);

/**
 * @brief Enable backup ports.  This allows a port to be configured to "backup"
 * another port.  If the protected port goes down, multicast traffic will be
 * sent on the backup port instead.  By default this is disabled.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_enable_port_protection(p4_pd_sess_hdl_t sess_hdl,
                                               bf_dev_id_t dev);

/**
 * @brief Disable backup ports.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_disable_port_protection(p4_pd_sess_hdl_t sess_hdl,
                                                bf_dev_id_t dev);

/**
 * @brief Program the backup port table.  If the protected port goes down
 * multicast
 * traffic will automatically be sent over the backup port.  Note that a port
 * can only be backed up by a single port; multiple ports cannot backup a
 * single protected port.  For a given protected port, if a previous call to
 * this API set a backup port, it will be replaced with the new backup port.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] protected_port The port which is being protected.
 * @param[in] backup_port The port which will protect @c protected_port.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_port_protection(p4_pd_sess_hdl_t sess_hdl,
                                            bf_dev_id_t dev,
                                            uint16_t pport,
                                            uint16_t bport);

/**
 * @brief Clear the backup port for a given port.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] port The port for which protection is being removed.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_clear_port_protection(p4_pd_sess_hdl_t sess_hdl,
                                              bf_dev_id_t dev,
                                              uint16_t pport);

/**
 * @brief Configure the maximum number of nodes which will be processed for a
 * packet
 * before the packet is returned to the input FIFO and a new packet is started.
 * The default value is 16.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] count The number of nodes a packet can traverse before it is fed
 * back to the end of its FIFO.  Valid range is 1 to 1M.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_max_nodes_before_yield(p4_pd_sess_hdl_t sess_hdl,
                                                   bf_dev_id_t dev,
                                                   int count);

/**
 * @brief Configure the maximum size of a multicast tree.  This caps the number
 * of
 * nodes a packet can visit before the hardware discards the packet.  The
 * default value is 4k nodes (@c node_count) and 128 ports/lags (@c
 * node_port_lag_count).
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] node_count The number of nodes a packet can traverse before it is
 * discarded. Valid range is 1 to 1M.
 * @param[in] port_lag_count The maximum number of LAGs and ports on a given
 * node that can be processed before the packet is discarded.
 * Valid range is 1 to 1M.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_set_max_node_threshold(p4_pd_sess_hdl_t sess_hdl,
                                               bf_dev_id_t dev,
                                               int node_count,
                                               int port_lag_count);

/**
 * @brief Query the pipe vector used by a multicast group.
 *
 * @param[in] shdl Session handle.
 * @param[in] dev The ASIC id.
 * @param[in] grp The multicast group id.
 * @param[out] logical_pipe_vector Pointer to return the pipe vector.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_mc_get_pipe_vector(p4_pd_sess_hdl_t sess_hdl,
                                        bf_dev_id_t dev,
                                        uint16_t grp,
                                        int *logical_pipe_vector);
#endif
