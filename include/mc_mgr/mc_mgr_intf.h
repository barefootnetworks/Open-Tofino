/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _MC_MGR_INTF_H
#define _MC_MGR_INTF_H

#include <mc_mgr/mc_mgr_types.h>

/**
 * @file mc_mgr_intf.h
 * \brief Details multicast specific APIs.
 *
 */

/**
 * Initialize the driver.  This should be called exactly once before any other
 * APIs are called.
 */
bf_status_t bf_mc_init();

/**
 * Create a session within the driver.
 * @param shdl Pointer to the session handle which will be filled in.
 * @return Status of the API call.
 */
bf_status_t bf_mc_create_session(bf_mc_session_hdl_t *shdl);
/**
 * Destroy a session within the driver.
 * @param shdl The session handle associated with the session to clean up.
 * @return Status of the API call.
 */
bf_status_t bf_mc_destroy_session(bf_mc_session_hdl_t hdl);

/**
 * Begin a batch on a session.  All hardware programming will be held until
 * the batch is ended and then pushed to hardware in as few DMA operations as
 * possible.
 * @param shdl The session handle associated with the session to clean up.
 * @return Status of the API call.
 */
bf_status_t bf_mc_begin_batch(bf_mc_session_hdl_t shdl);

/**
 * Flush a batch on a session pushing all pending updates to hardware.
 * @param shdl The session handle associated with the session to clean up.
 * @return Status of the API call.
 */
bf_status_t bf_mc_flush_batch(bf_mc_session_hdl_t shdl);

/**
 * End a batch on a session.  All queued hardware updates will be pushed to
 * the hardward.
 * @param shdl The session handle associated with the session to clean up.
 * @return Status of the API call.
 */
bf_status_t bf_mc_end_batch(bf_mc_session_hdl_t shdl, bool hwSynchronous);

/**
 * Wait for outstanding DMA buffers to come back.
 */
bf_status_t bf_mc_complete_operations(bf_mc_session_hdl_t hdl);

/**
 * @addtogroup mc_mgr-mgmt
 * @{
 * This is a description of the multicast driver APIs.
 */

/**
 * Allocate a multicast group id.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param grp The multicast group id to reserve.  Note that this is the value
 *            which must be programmed into any pipeline match table result for
 *            packets to use this multicast group.
 * @param hdl Pointer to the group handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_create(bf_mc_session_hdl_t shdl,
                              bf_dev_id_t dev,
                              bf_mc_grp_id_t grp,
                              bf_mc_mgrp_hdl_t *hdl);

/**
 * Get a multicast group's attributes.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl Handle of the group to query.
 * @param grp buffer to be filled with the multicast group.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_attr(bf_mc_session_hdl_t shdl,
                                bf_dev_id_t dev,
                                bf_mc_mgrp_hdl_t mgrp_hdl,
                                bf_mc_grp_id_t *grp);
/**
 * Release a multicast group id.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl Handle of the group to release.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_destroy(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_mgrp_hdl_t mgrp_hdl);

/**
 * Get first multicast group id.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl Pointer to first multicast group id handle will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_first(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 bf_mc_mgrp_hdl_t *mgrp_hdl);

/**
 * Retreive quantity of multicast group ids.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count Number of total multicast group ids.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_count(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 uint32_t *count);

/**
 * Retreive the size of LAG table available to the user. In Tofino, one entry
 * at idx 255 is reserved, so the size is smaller for that chip
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count Size of LAG table.
 * @return Status of the API call.
 */
bf_status_t bf_mc_lag_get_size(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               uint32_t *count);

/**
 * Get next i multicast group ids. If there is at least 1 but not i ids, the
 * remaining space in the array will be filled with -1.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl Pointer to multicast group from which search begins.
 * @param i Number of multicast group ids to retreive.
 * @param next_mgrp_hdls An array of at least i multicast group ids handles.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_next_i(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_mgrp_hdl_t mgrp_hdl,
                                  uint32_t i,
                                  bf_mc_mgrp_hdl_t *next_mgrp_hdls);

/**
 * Get the first node member of the multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param node_hdl Pointer to a node handle which will be returned.
 * @param node_l1_xid_valid Pointer to a bool to return the l1 xid valid status.
 * @param node_l1_xid Pointer to a l1 xid which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_first_node_mbr(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          bf_mc_mgrp_hdl_t mgrp_hdl,
                                          bf_mc_node_hdl_t *node_hdl,
                                          bool *node_l1_xid_valid,
                                          bf_mc_l1_xid_t *node_l1_xid);

/**
 * Get the count of node members in the specified multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param count Pointer to a uint32_t to return the quantity of node members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_node_mbr_count(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          bf_mc_mgrp_hdl_t mgrp_hdl,
                                          uint32_t *count);

/**
 * Get the next i node members in the multicast group after the
 * parameterized member. If there is at least 1 but not i members, then the
 * remaining space in the array will be filled with invlaid handles.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param node_hdl The node handle at which to start the query.
 * @param i The quantity of node handles to return.
 * @param next_node_hdls An array of node handles with enough space to fit
 * at least i node handles.
 * @param next_node_l1_xids_valid An array of bool with enough space to fit
 * at least i nodes' l1 xid valid status.
 * @param next_node_l1_xids An array of l1 xids with enough space to fit
 * at least i nodes' l1 xid values.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_next_i_node_mbr(bf_mc_session_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           bf_mc_mgrp_hdl_t mgrp_hdl,
                                           bf_mc_node_hdl_t node_hdl,
                                           uint32_t i,
                                           bf_mc_node_hdl_t *next_node_hdls,
                                           bool *next_node_l1_xids_valid,
                                           bf_mc_l1_xid_t *next_node_l1_xids);

/**
 * Get the first ECMP member of the multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param ecmp_hdl Pointer to a ECMP handle which will be returned.
 * @param ecmp_l1_xid_valid Pointer to a bool to return the l1 xid valid status.
 * @param ecmp_l1_xid Pointer to a l1 xid which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_first_ecmp_mbr(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          bf_mc_mgrp_hdl_t mgrp_hdl,
                                          bf_mc_ecmp_hdl_t *ecmp_hdl,
                                          bool *ecmp_l1_xid_valid,
                                          bf_mc_l1_xid_t *ecmp_l1_xid);

/**
 * Get the count of ECMP members in the specified multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param count Pointer to a uint32_t to return the quantity of ECMP members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_ecmp_mbr_count(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          bf_mc_mgrp_hdl_t mgrp_hdl,
                                          uint32_t *count);

/**
 * Get the next i ECMP members in the multicast group after the
 * parameterized member. If there is at least 1 but not i members, then the
 * remaining space in the array will be filled with invalid handles.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The multicast group handle.
 * @param ecmp_hdl The ECMP handle at which to start the query.
 * @param i The quantity of ECMP handles to return.
 * @param next_ecmp_hdls An array of ECMP handles with enough space to fit
 * at least i ECMP handles.
 * @param next_ecmp_l1_xids_valid An array of bool with enough space to fit
 * at least i ECMPs' l1 xid valid status.
 * @param next_ecmp_l1_xids An array of l1 xids with enough space to fit
 * at least i ECMPs' l1 xid values.
 * @return Status of the API call.
 */
bf_status_t bf_mc_mgrp_get_next_i_ecmp_mbr(bf_mc_session_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           bf_mc_mgrp_hdl_t mgrp_hdl,
                                           bf_mc_ecmp_hdl_t ecmp_hdl,
                                           uint32_t i,
                                           bf_mc_ecmp_hdl_t *next_ecmp_hdls,
                                           bool *next_ecmp_l1_xids_valid,
                                           bf_mc_l1_xid_t *next_ecmp_l1_xids);

/**
 * Create a node.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param rid The RID to assign to the node
 * @param port_map Member ports belonging to the node.
 * @param lag_map Member LAGs belonging to the node.
 * @param node_hdl Pointer to a node handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_create(bf_mc_session_hdl_t shdl,
                              bf_dev_id_t dev,
                              bf_mc_rid_t rid,
                              bf_mc_port_map_t port_map,
                              bf_mc_lag_map_t lag_map,
                              bf_mc_node_hdl_t *node_hdl);

/**
 * Get a node's attributes.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl node for which to query attributes
 * @param pointer to buffer to be filled with rid.
 * @param pointer to buffer to be filled with the port_map.
 * @param pointer to buffer to be filled with the lag_map.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_get_attr(bf_mc_session_hdl_t shdl,
                                bf_dev_id_t dev,
                                bf_mc_node_hdl_t node_hdl,
                                bf_mc_rid_t *rid,
                                bf_mc_port_map_t port_map,
                                bf_mc_lag_map_t lag_map);

/**
 * Destroy a node.  The node must first be removed from any ECMP groups or
 * multicast groups before it can be deleted.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl The handle of the node to delete.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_destroy(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_node_hdl_t node_hdl);

/**
 * Get the first multicast node.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl Pointer to a node handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_get_first(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 bf_mc_node_hdl_t *node_hdl);

/**
 * Get the quantity of multicast nodes.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count Pointer to a uint32_t representing the multicast node quantity.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_get_count(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 uint32_t *count);

/**
 * Get the next i multicast nodes after the parameterized node hdl. If there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl The node handle where the query starts.
 * @param i The number of node handles to retreive.
 * @param next_node_hdls An array with space for at least i node handles.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_get_next_i(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_node_hdl_t node_hdl,
                                  uint32_t i,
                                  bf_mc_node_hdl_t *next_node_hdls);

/**
 * Returns whether the parameterized node is associated, and information about
 * the association if it is. If a pointer param is null, the information won't
 * be returned.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl The node handle in question.
 * @param is_associated Pointer to a bool representing association status.
 * @param mgrp_hdl Pointer to multicast group which is filled with associated
 * group.
 * @param pointer to buffer to be filled with xid validity.
 * @param pointer to buffer to be filled with the xid.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_get_association(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_node_hdl_t node_hdl,
                                       bool *is_associated,
                                       bf_mc_mgrp_hdl_t *mgrp_hdl,
                                       bool *level1_exclusion_id_valid,
                                       bf_mc_l1_xid_t *level1_exclusion_id);

/**
 * Returns whether the parameterized node is a member of an ECMP group.
 * If a pointer param is null, the information won't be returned.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl The node handle in question.
 * @param is_ecmp_mbr Pointer to a bool representing member status.
 * @param ecmp_hdl Pointer to an ECMP group which is filled with the
 * ECMP group (if is_ecmp_mbr is true).
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_is_mbr(bf_mc_session_hdl_t shdl,
                              bf_dev_id_t dev,
                              bf_mc_node_hdl_t node_hdl,
                              bool *is_ecmp_mbr,
                              bf_mc_ecmp_hdl_t *ecmp_hdl);

/**
 * Update the membership of a node.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl Handle of the node.
 * @param port_map The new set of ports belonging to the node.  Note that
 *                 this set replaces any existing members.
 * @param lag_map The new set of LAGs belonging to the node.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_update(bf_mc_session_hdl_t shdl,
                              bf_dev_id_t dev,
                              bf_mc_node_hdl_t node_hdl,
                              bf_mc_port_map_t port_map,
                              bf_mc_lag_map_t lag_map);

/**
 * Create an ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Pointer to the group handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_create(bf_mc_session_hdl_t shld,
                              bf_dev_id_t dev,
                              bf_mc_ecmp_hdl_t *ecmp_hdl);
/**
 * Release an ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Handle of the group to release.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_destroy(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_ecmp_hdl_t ecmp_hdl);

/**
 * Release an ECMP group when no mgid is associated with it.
 * Otherwise return BF_IN_USE error
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Handle of the group to release.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_destroy_checked(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_ecmp_hdl_t ehdl);

/**
 * Get the first ECMP handle.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Pointer to a ECMP handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_first(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 bf_mc_ecmp_hdl_t *ecmp_hdl);

/**
 * Get the current quantity of ECMP groups.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count Pointer to a uint32_t representing the ECMP group quantity.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_count(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 uint32_t *count);

/**
 * Get the next i ECMP handles after the parameterized handle. If there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle at which to start the query.
 * @param i The quantity of ECMP handles to return.
 * @param next_ecmp_hdls An array of ECMP handles with enough space to fit
 * at least i ECMP handles.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_next_i(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_ecmp_hdl_t ecmp_hdl,
                                  uint32_t i,
                                  bf_mc_ecmp_hdl_t *next_ecmp_hdls);

/**
 * Add a member (a node) to an ECMP group.  The node cannot be in any other
 * ECMP groups or be associated with a multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Handle of the ecmp group.
 * @param node_hdl Handle of the node.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_mbr_add(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_ecmp_hdl_t ecmp_hdl,
                               bf_mc_node_hdl_t node_hdl);
/**
 * Remove a member (a node) from an ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Handle of the ecmp group.
 * @param node_hdl Handle of the node.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_mbr_rem(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_ecmp_hdl_t ecmp_hdl,
                               bf_mc_node_hdl_t node_hdl);

/**
 * replace all members (nodes) in an ECMP group with given nodes
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl Handle of the ecmp group.
 * @param node_hdls Handle of the list of nodes.
 * @param size the array size of the list of nodes.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_mbr_mod(bf_mc_session_hdl_t shdl,
                               bf_dev_id_t dev,
                               bf_mc_ecmp_hdl_t ecmp_hdl,
                               bf_mc_node_hdl_t *node_hdls,
                               uint32_t size);

/**
 * Get the first member of the group associated with the ECMP handle.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param node_hdl Pointer to a node handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_first_mbr(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_mc_ecmp_hdl_t ecmp_hdl,
                                     bf_mc_node_hdl_t *node_hdl);

/**
 * Get the current quantity members in the specified ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param count Pointer to a uint32_t representing the quantity of members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_mbr_count(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_mc_ecmp_hdl_t ecmp_hdl,
                                     uint32_t *count);

/**
 * Get the next i ECMP group members after the parameterized member. If there
 * is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param node_hdl The node handle at which to start the query.
 * @param i The quantity of node handles to return.
 * @param next_ecmp_hdls An array of node handles with enough space to fit
 * at least i node handles.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_next_i_mbr(bf_mc_session_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      bf_mc_ecmp_hdl_t ecmp_hdl,
                                      bf_mc_node_hdl_t node_hdl,
                                      uint32_t i,
                                      bf_mc_node_hdl_t *next_node_hdls);

/**
 * Get the member of the group associated with the ECMP handle based on
 * the input hash value.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param level1_mcast_hash The hash value of the packet.
 * @param node_hdl Pointer to a node handle which will be
 * returned. Returns 0 if invalid (e.g. empty ecmp group).
 * @param is_pruned Is the chosen node pruned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_mbr_from_hash(bf_mc_session_hdl_t shdl,
                                         bf_dev_id_t dev,
                                         bf_mc_mgrp_hdl_t mgrp_hdl,
                                         bf_mc_ecmp_hdl_t ecmp_hdl,
                                         uint16_t level1_mcast_hash,
                                         bf_mc_l1_xid_t level1_exclusion_id,
                                         bf_mc_node_hdl_t *node_hdl,
                                         bool *is_pruned);

/**
 * Get the first association of the parameterized ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param mgrp_hdl Pointer to a node handle which will be returned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_first_assoc(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_ecmp_hdl_t ecmp_hdl,
                                       bf_mc_mgrp_hdl_t *mgrp_hdl);

/**
 * Get the current quantity of associations with the specified ECMP group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param count Pointer to a uint32_t representing the quantity of members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_assoc_count(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_ecmp_hdl_t ecmp_hdl,
                                       uint32_t *count);

/**
 * Get the next i ECMP associations after the parameterized association. If
 * there is at least 1 but not i ids, the remaining space in the array will
 * be filled with -1.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ecmp_hdl The ECMP handle in question.
 * @param mgrp_hdl The node handle at which to start the query.
 * @param i The quantity of node handles to return.
 * @param next_ecmp_hdls An array of node handles with enough space to fit
 * at least i node handles.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_next_i_assoc(bf_mc_session_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        bf_mc_ecmp_hdl_t ecmp_hdl,
                                        bf_mc_mgrp_hdl_t mgrp_hdl,
                                        uint32_t i,
                                        bf_mc_mgrp_hdl_t *next_mgrp_hdls);

/**
 * Associate a node with a multicast group.  The node cannot be associated with
 * any other multicast groups or ECMP groups.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The handle of the multicast group.
 * @param node_hdl The handle of the node.
 * @param level1_exclusion_id_valid A bool indicating if the exclusion id is
 *        needed.
 * @param level1_exclusion_id L1 exclusion id assigned to the node.
 * @return Status of the API call.
 */
bf_status_t bf_mc_associate_node(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 bf_mc_mgrp_hdl_t mgrp_hdl,
                                 bf_mc_node_hdl_t node_hdl,
                                 bool level1_exclusion_id_valid,
                                 bf_mc_l1_xid_t level1_exclusion_id);
/**
 * Remove a node from a multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The handle of the multicast group.
 * @param node_hdl The handle of the node.
 * @return Status of the API call.
 */
bf_status_t bf_mc_dissociate_node(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_mgrp_hdl_t mgrp_hdl,
                                  bf_mc_node_hdl_t node_hdl);

/**
 * Associate an ECMP group with a multicast group.  Unlike individual nodes
 * ECMP groups may be associated with many multicast groups.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The handle of the multicast group.
 * @param ecmp_hdl The handle of the ECMP group.
 * @param level1_exclusion_id_valid A bool indicating if the exclusion id is
 *        needed.
 * @param level1_exclusion_id L1 exclusion id assigned to the ECMP group in the
 *        specified multicast group.
 * @return Status of the API call.
 */
bf_status_t bf_mc_associate_ecmp(bf_mc_session_hdl_t shdl,
                                 bf_dev_id_t dev,
                                 bf_mc_mgrp_hdl_t mgrp_hdl,
                                 bf_mc_ecmp_hdl_t ecmp_hdl,
                                 bool level1_exclusion_id_valid,
                                 bf_mc_l1_xid_t level1_exclusion_id);

/**
 * Get attributes of an ecmp - multicast group association.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The handle of the multicast group.
 * @param ecmp_hdl The handle of the ECMP group.
 * @param level1_exclusion_id_valid buffer to bool indicating if the exclusion
 *        id is needed.
 * @param level1_exclusion_id buffer to L1 exclusion id assigned to the ECMP
 *        group in the specified multicast group.
 * @return Status of the API call.
 */
bf_status_t bf_mc_ecmp_get_assoc_attr(bf_mc_session_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      bf_mc_mgrp_hdl_t mgrp_hdl,
                                      bf_mc_ecmp_hdl_t ecmp_hdl,
                                      bool *level1_exclusion_id_valid,
                                      bf_mc_l1_xid_t *level1_exclusion_id);

/**
 * Remove an ECMP group from a multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgrp_hdl The handle of the multicast group.
 * @param ecmp_hdl The handle of the ECMP group.
 * @return Status of the API call.
 */
bf_status_t bf_mc_dissociate_ecmp(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_mgrp_hdl_t mgrp_hdl,
                                  bf_mc_ecmp_hdl_t ecmp_hdl);

/**
 * Program the L2 exclusion id pruning table.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param l2_exclusion_id The table index to program.
 * @param pruned_ports The new set of ports to prune.  Note that this set
 *                     replaces replaces any existing members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_port_prune_table(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_l2_xid_t l2_exclusion_id,
                                       bf_mc_port_map_t pruned_ports);

/**
 * Read the L2 exclusion id pruning table.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param l2_exclusion_id The table index to read.
 * @param pruned_ports The data will be read into this pointer.
 * @param from_hw Read from the software shadow or hardware.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_port_prune_table(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_mc_l2_xid_t l2_exclusion_id,
                                       bf_mc_port_map_t *pruned_ports,
                                       bool from_hw);

/**
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count Table size
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_port_prune_table_size(bf_mc_session_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            uint32_t *count);

/**
 * Update the LAG membership table.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param lag_id The LAG id.
 * @param port_map The new set of ports belonging to the LAG.  Note that
 *                 this set replaces any existing members.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_lag_membership(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_mc_lag_id_t lag_id,
                                     bf_mc_port_map_t port_map);

/**
 * Get entry from the LAG membership table.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param lag_id The LAG id.
 * @param port_map Buffer for the set of ports belonging to the LAG.
 * @param from_hw Read from the software shadow or hardware.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_lag_membership(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_mc_lag_id_t lag_id,
                                     bf_mc_port_map_t port_map,
                                     bool from_hw);

/**
 * Get port from the LAG and the level 2 hash.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param node_hdl The handle of the node that owns this LAG.
 * @param lag_id The LAG id.
 * @param level2_mcast_hash The hash to determine the LAG port.
 * @param pkt_xid The xid of the ig pkt for pruning.
 * @param pkt_yid The yid of the ig pkt for pruning.
 * @param pkt_rid The rid of the ig pkt for pruning.
 * @param port Pointer to the value of the chosen port.
 * Returns 0xffff if invalid (e.g. empty lag group, remote index, etc).
 * @param is_pruned Is the chosen port pruned.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_lag_member_from_hash(bf_mc_session_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           bf_mc_node_hdl_t node_hdl,
                                           bf_mc_lag_id_t lag_id,
                                           uint16_t level2_mcast_hash,
                                           bf_mc_l1_xid_t l1_exclusion_id,
                                           bf_mc_l2_xid_t l2_exclusion_id,
                                           bf_mc_rid_t rid,
                                           bf_dev_port_t *port,
                                           bool *is_pruned);

/**
 * Update one ASIC with the LAG membership counts of other ASICs.  When a LAG
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
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param lag_id The LAG id.
 * @param left_count The number of LAG members on all ASICs to the "left" of
 *                   this ASIC.
 * @param right_count The number of LAG members on all ASICs to the "right" of
 *                    this ASIC.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_remote_lag_member_count(bf_mc_session_hdl_t shdl,
                                              bf_dev_id_t dev,
                                              bf_mc_lag_id_t lag_id,
                                              int left_count,
                                              int right_count);

/**
 * Get info on one ASIC with the LAG membership counts of other ASICs.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param lag_id The LAG id.
 * @param *left_count The number of LAG members on all ASICs to the "left" of
 *                   this ASIC.
 * @param *right_count The number of LAG members on all ASICs to the "right" of
 *                    this ASIC.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_remote_lag_member_count(bf_mc_session_hdl_t shdl,
                                              bf_dev_id_t dev,
                                              bf_mc_lag_id_t lag_id,
                                              int *left_count,
                                              int *right_count);
/**
 * Update the multicast forwarding state of a port.  If set to "inactive" the
 * multicast engine will treat the port as if it is down when the multicast
 * engine is generating copies. By default, all ports are in the forwarding
 * state.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param port_id The port to set.
 * @param is_active @c false mark the port as inactive, @c true mark the port
 *                  as active.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_port_mc_fwd_state(bf_mc_session_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        bf_dev_port_t port_id,
                                        bool is_active);

/**
 * Get the multicast forwarding state of a port.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param port_id The port to get.
 * @param *is_active @c false means the port as inactive, @c true means the port
 *                  as active.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_port_mc_fwd_state(bf_mc_session_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        bf_dev_port_t port_id,
                                        bool *is_active);
/**
 * Enable HW fast failover on port down events.  If the hardware detects a port
 * has gone down traffic will automatically re-hash to another LAG member or,
 * if enabled, traffic will be sent over the backup port (configured with
 * @ref bf_mc_enable_port_protection and @ref bf_mc_set_port_protection ).
 * By default this is disabled.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @return Status of the API call.
 */
bf_status_t bf_mc_enable_port_fast_failover(bf_mc_session_hdl_t shdl,
                                            bf_dev_id_t dev);
/**
 * Disable HW fast failover on port down events.  By default this is disabled.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @return Status of the API call.
 */
bf_status_t bf_mc_disable_port_fast_failover(bf_mc_session_hdl_t shdl,
                                             bf_dev_id_t dev);
/**
 * Clear the HW fast failover state on a port.  This must be called to bring
 * the port back to a forwarding state once it has come up after going down.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param port_id The port to set.
 * @return Status of the API call.
 */
bf_status_t bf_mc_clear_fast_failover_state(bf_mc_session_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            bf_dev_port_t port_id);

/**
 * Get the fast failover state on a port.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param port_id The port to get.
 * @param is_active the boolean state of fast failover for the port
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_fast_failover_state(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev,
                                          bf_dev_port_t port_id,
                                          bool *is_active);
/**
 * Enable backup ports.  This allows a port to be configured to "backup"
 * another port.  If the protected port goes down, multicast traffic will be
 * sent on the backup port instead.  By default this is disabled.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @return Status of the API call.
 */
bf_status_t bf_mc_enable_port_protection(bf_mc_session_hdl_t shdl,
                                         bf_dev_id_t dev);
/**
 * Disable backup ports.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @return Status of the API call.
 */
bf_status_t bf_mc_disable_port_protection(bf_mc_session_hdl_t shdl,
                                          bf_dev_id_t dev);

/**
 * Program the backup port table.  If the protected port goes down multicast
 * traffic will automatically be sent over the backup port.  Note that a port
 * can only be backed up by a single port; multiple ports cannot backup a
 * single protected port.  For a given protected port, if a previous call to
 * this API set a backup port, it will be replaced with the new backup port.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param protected_port The port which is being protected.
 * @param backup_port The port which will protect @c protected_port.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_port_protection(bf_mc_session_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      bf_dev_port_t protected_port,
                                      bf_dev_port_t backup_port);
/**
 * Get the backup port table for give protected_port
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param protected_port The port which is being protected.
 * @param *backup_port The port which will protect @c protected_port.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_port_protection(bf_mc_session_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      bf_dev_port_t protected_port,
                                      bf_dev_port_t *backup_port);

/**
 * Clear the backup port for a given port.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param port The port for which protection is being removed.
 * @return Status of the API call.
 */
bf_status_t bf_mc_clear_port_protection(bf_mc_session_hdl_t shdl,
                                        bf_dev_id_t dev,
                                        bf_dev_port_t port);

/**
 * Program the global RID.  Packets with ingress RIDs matching this value will
 * be subject to L2 pruning using the values configured with
 * @ref bf_mc_set_port_prune_table.  The default value is zero.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param rid The RID value to program.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_global_exclusion_rid(bf_mc_session_hdl_t shdl,
                                           bf_dev_id_t dev,
                                           bf_mc_rid_t rid);

/**
 * Configure the maximum number of nodes which will be processed for a packet
 * before the packet is returned to the input FIFO and a new packet is started.
 * The default value is 16.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param count The number of nodes a packet can traverse before it is fed
 *              back to the end of its FIFO.  Valid range is 1 to 1M.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_max_nodes_before_yield(bf_mc_session_hdl_t shdl,
                                             bf_dev_id_t dev,
                                             int count);

/**
 * Configure the maximum size of a multicast tree.  This caps the number of
 * nodes a packet can visit before the hardware discards the packet.  The
 * default value is 4k nodes (@c node_count) and 128 ports/lags (@c
 * node_port_lag_count).
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param nodes_count The number of nodes a packet can traverse before it is
 *              discarded.  Valid range is 1 to 1M.
 * @param node_port_lag_count The maximum number of LAGs and ports on a given
 *              node that can be processed before the packet is discarded.
 *              Valid range is 1 to 1M.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_max_node_threshold(bf_mc_session_hdl_t shdl,
                                         bf_dev_id_t dev,
                                         int node_count,
                                         int node_port_lag_count);

/**
 * Query the pipe vector used by a multicast group.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param grp The multicast group id.
 * @param logical_pipe_vector The pipe vector is returned here.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_pipe_vector(bf_mc_session_hdl_t shdl,
                                  bf_dev_id_t dev,
                                  bf_mc_grp_id_t grp,
                                  int *logical_pipe_vector);
/* @} */

/**
 * @addtogroup mc_mgr-dbug
 * @{
 */

/**
 * Read the state recorded when the node limit set by
 * @ref bf_mc_set_max_node_threshold is exceeded.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgid Pointer to where the multicast group id of the packet will be
 *             stored.
 * @param addr Pointer to where the RDM address the packet was reading will be
 *             stored.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_max_node_threshold_exceeded_data(bf_mc_session_hdl_t shdl,
                                                       bf_dev_id_t dev,
                                                       bf_mc_grp_id_t *grp,
                                                       uint32_t *addr);

/**
 * Read the state recorded when the port/LAG limit set by
 * @ref bf_mc_set_max_node_threshold is exceeded.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param mgid Pointer to where the multicast group id of the packet will be
 *             stored.
 * @param addr Pointer to where the RDM address the packet was reading will be
 *             stored.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_max_node_threshold_exdeeded_data_port_lag(
    bf_mc_session_hdl_t shdl,
    bf_dev_id_t dev,
    bf_mc_grp_id_t *grp,
    uint32_t *addr);

/**
 * Configure the debug filter.  Five key/mask pairs which are compared against
 * a packet.  The comparison is a bitwise ternary match.  Each 1-bit in the
 * mask requires the corresponding bit in the key to match the packet.  Each
 * 0-bit in the mask wildcards the corresponding bit in the key.
 * This filter affects the @ref bf_mc_get_debug_counters and
 * @ref bf_mc_get_copy_dest_vector APIs.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param ingress_port The ingress port to match.
 * @param mgid The multicast group id to match.
 * @param ingress_rid The ingress rid to match.
 * @param l1_xid The level1 exclusion id to match.
 * @param l2_xid The level2 excludion id to match.
 * @param ingress_port_m The ingress port mask.
 * @param mgid_m The multicast group id mask.
 * @param ingress_rid_m The ingress rid mask.
 * @param l1_xid_m The level1 exclusion id mask.
 * @param l2_xid_m The level2 excludion id mask.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_debug_filter(bf_mc_session_hdl_t shdl,
                                   bf_dev_id_t dev,
                                   uint16_t ingress_port,
                                   uint16_t mgid,
                                   uint16_t ingress_rid,
                                   uint16_t l1_xid,
                                   uint16_t l2_xid,
                                   uint16_t ingress_port_m,
                                   uint16_t mgid_m,
                                   uint16_t ingress_rid_m,
                                   uint16_t l1_xid_m,
                                   uint16_t l2_xid_m);

/**
 * Configure the node memory capture address.  If a packet reads the specified
 * hardware memory address its state will be logged and can be read with
 * @ref bf_mc_get_node_watch_point_data
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @param enable If true, enable the watch point otherwise disable it.
 * @param address The RDM address which will trigger a log.
 * @return Status of the API call.
 */
bf_status_t bf_mc_set_node_watch_point(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_dev_pipe_t pipe,
                                       bool enable,
                                       uint32_t address);

/**
 * Read and clear the captured node watch point data.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @param hit Pointer to the value which will be set with the hit state.
 * @param state Pointer to a struct which will be set with the logged packet's
 * state.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_node_watch_point_data(bf_mc_session_hdl_t shdl,
                                            bf_dev_id_t dev,
                                            bf_dev_pipe_t pipe,
                                            bool *hit,
                                            bf_mc_debug_packet_state_t *state);

/**
 * Read and clear the PRE debug counters.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @param cntrs Pointer to a struct where the counters will be stored.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_debug_counters(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_dev_pipe_t pipe,
                                     bf_mc_debug_counters_t *cntrs);

/**
 * Read, and clear, the port vector.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @param ports_hi Pointer to the value which will be set with the mask of
 *                 ports 64-73 local to the pipe.
 * @param ports_lo Pointer to the value which will be set with the mask of
 *                 ports 0-63 local to the pipe.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_copy_dest_vector(bf_mc_session_hdl_t shdl,
                                       bf_dev_id_t dev,
                                       bf_dev_pipe_t pipe,
                                       uint64_t *ports_hi,
                                       uint64_t *ports_lo);

/**
 * Read the current amount of credit various internal FIFOs have.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @param credit Pointer to a struct where the information will be stored.
 * @return Status of the API call.
 */
bf_status_t bf_mc_get_int_fifo_credit(bf_mc_session_hdl_t shdl,
                                      bf_dev_id_t dev,
                                      bf_dev_pipe_t pipe,
                                      bf_mc_debug_int_fifo_credit_t *credit);
/* @} */

/**
 * @addtogroup mc_mgr-schd
 * @{
 */

/**
 * Configure how hardware memory for multicast nodes will be garbage collected;
 * either interrupt based or on demand.  If the mode is set to on demand then
 * the @ref bf_mc_do_node_garbage_collection function should be called
 * periodically.
 * The default is to use interrupts.
 * @param shdl Session handle.
 * @param interrupt_or_periodic If true use interrupts, if false interrupts will
 *        not be used.
 * @return Status of the API call.
 */
bf_status_t bf_mc_node_garbage_collection_scheduler_mode(
    bf_mc_session_hdl_t shdl, bool interrupt_or_periodic);
/**
 * Perform garbage collection of deleted nodes to free hardware node memory.
 * This function should be called periodically if
 * @ref bf_mc_node_garbage_collection_scheduler_mode is periodic.
 * @param shdl Session handle.
 * @return Status of the API call.
 */
bf_status_t bf_mc_do_node_garbage_collection(bf_mc_session_hdl_t shdl);

/* @} */

/* Used by TM API to set copy-to-cpu port */
bf_status_t bf_mc_set_copy_to_cpu(bf_dev_id_t dev,
                                  bool enable,
                                  bf_dev_port_t port);

/* Used by BFRT API to get copy-to-cpu port */
bf_status_t bf_mc_get_copy_to_cpu(bf_dev_id_t dev,
                                  bf_dev_port_t *port,
                                  bool *enable);

/* Needed for UT only. */
void mc_mgr_tree_size(
    bf_dev_id_t dev, bf_dev_pipe_t pipe, int mgid, int *l1_cnt, int *l2_cnt);
bf_mc_node_hdl_t mc_mgr_tree_first_handle(bf_dev_id_t dev,
                                          bf_dev_pipe_t pipe,
                                          int mgid);
int mc_mgr_rdm_alloc_cnt(bf_dev_id_t dev, int size);
uint32_t mc_mgr_l1_hdl_to_rdm_addr(bf_dev_id_t dev,
                                   bf_dev_pipe_t pipe,
                                   bf_mc_node_hdl_t node_hdl);
bool mc_mgr_get_port72_last(bf_dev_id_t dev, uint32_t addr);
int mc_mgr_get_port72_pipe(bf_dev_id_t dev, uint32_t addr);
int mc_mgr_test_misc(bf_dev_id_t dev);

/**
 * A callback function to handle the "RDM Change Done" interrupt.
 * @param shdl Session handle.
 * @param dev The ASIC id.
 * @param pipe The pipeline id.
 * @return Status of the API call.
 */
bf_status_t bf_mc_rdm_change_intr_cb(bf_mc_session_hdl_t shdl,
                                     bf_dev_id_t dev,
                                     bf_dev_pipe_t pipe);

bf_status_t bf_dma_service_write_list_completion(bf_dev_id_t dev);
bf_status_t bf_dma_service_write_list1_completion(bf_dev_id_t dev);
bf_status_t bf_dma_service_read_block0_completion(bf_dev_id_t dev);
bf_status_t bf_dma_service_read_block1_completion(bf_dev_id_t dev);
#endif
