/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _TOFINO_PDFIXED_PD_MS_H
#define _TOFINO_PDFIXED_PD_MS_H

/**
 * @file pd_ms.h
 *
 * @brief PD Fixed APIs for Action Profile and Action Selector management.
 *
 */

#include <tofino/pdfixed/pd_common.h>
#include <bfutils/cJSON.h>

typedef uint32_t p4_pd_act_hdl_t;
typedef struct p4_pd_ms_table_state_s p4_pd_ms_table_state_t;

typedef struct pd_res_spec_t {
  uint32_t tbl_hdl;
  uint32_t tbl_idx;
} pd_res_spec_t;

typedef p4_pd_status_t (*PDMSGrpFn)(bf_dev_id_t dev_id,
                                    p4_pd_mbr_hdl_t mbr_hdl,
                                    p4_pd_grp_hdl_t grp_hdl,
                                    void *aux);

/**
 * @brief Initialize Action Profile and Action Selector management layer
 *
 * @return None
 *
 */
void p4_pd_ms_init(void);

/**
 * @brief Cleanup Action Profile and Action Selector management layer
 *
 * @return None
 *
 */
void p4_pd_ms_cleanup(void);

/**
 * @brief Initialize the state for Action Profile and Action Selector management
 *
 * @return Pointer to the state of Action Profile and
 * Action Selector management
 *
 */
p4_pd_ms_table_state_t *p4_pd_ms_init_state(void);

/**
 * @brief Destroy the state for Action Profile and Action Selector management
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 *
 * @return None
 *
 */
void p4_pd_ms_destroy_state(p4_pd_ms_table_state_t *state);

/**
 * @brief Destroy the state of given device for Action Profile and
 * Action Selector management
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 *
 * @return None
 *
 */
void p4_pd_ms_destroy_state_for_dev(p4_pd_ms_table_state_t *state,
                                    uint32_t dev_id);

/**
 * @brief Destroy the transaction state of given device for Action Profile and
 * Action Selector management
 *
 * @param[in] dev_id Device identifier
 *
 * @return None
 *
 */
void p4_pd_ms_destroy_txn_state_for_dev(uint32_t dev_id);

/**
 * @brief Add a new selector member
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_new_mbr(p4_pd_sess_hdl_t sess_hdl,
                                p4_pd_ms_table_state_t *state,
                                bf_dev_id_t dev_id,
                                p4_pd_mbr_hdl_t mbr_hdl);

/**
 * @brief Delete the given selector member
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_del_mbr(p4_pd_sess_hdl_t sess_hdl,
                                p4_pd_ms_table_state_t *state,
                                bf_dev_id_t dev_id,
                                p4_pd_mbr_hdl_t mbr_hdl);

/**
 * @brief Add a member to the given selector group
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 * @param[in] grp_hdl Handle to the selector group
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_add_mbr_to_grp(p4_pd_sess_hdl_t sess_hdl,
                                       p4_pd_ms_table_state_t *state,
                                       bf_dev_id_t dev_id,
                                       p4_pd_mbr_hdl_t mbr_hdl,
                                       p4_pd_grp_hdl_t grp_hdl);

/**
 * @brief Delete a member from the given selector group
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 * @param[in] grp_hdl Handle to the selector group
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_del_mbr_from_grp(p4_pd_sess_hdl_t sess_hdl,
                                         p4_pd_ms_table_state_t *state,
                                         bf_dev_id_t dev_id,
                                         p4_pd_mbr_hdl_t mbr_hdl,
                                         p4_pd_grp_hdl_t grp_hdl);

/**
 * @brief Apply a member to the selector groups
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 * @param[in] grp_fn Function to be executed
 * @param[in] aux Pointer to the user data
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_mbr_apply_to_grps(p4_pd_ms_table_state_t *state,
                                          bf_dev_id_t dev_id,
                                          p4_pd_mbr_hdl_t mbr_hdl,
                                          PDMSGrpFn grp_fn,
                                          void *aux);

/**
 * @brief Get the action handle for the given selector member
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 * @param[out] act_hdl Pointer to return the action handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_get_mbr_act(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_mbr_hdl_t mbr_hdl,
                                    p4_pd_act_hdl_t *act_hdl);

/**
 * @brief Set the action handle for the given selector member
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector member
 * @param[in] act_hdl Action handle to be set
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_set_mbr_act(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_mbr_hdl_t mbr_hdl,
                                    p4_pd_act_hdl_t act_hdl);

/**
 * @brief Get the action handle for the given selector group
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mdr_hdl Handle to the selector group
 * @param[out] act_hdl Pointer to return the action handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_get_grp_act(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_grp_hdl_t grp_hdl,
                                    p4_pd_act_hdl_t *act_hdl);

/**
 * @brief Set the action handle for the given selector group
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] grp_hdl Handle to the selector group
 * @param[in] act_hdl Action handle to be set
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_set_grp_act(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_grp_hdl_t grp_hdl,
                                    p4_pd_act_hdl_t act_hdl);

/**
 * @brief Delete a selector group
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] grp_hdl Handle to the selector group
 * @param[in] num_mbrs Number of members in the selector group
 * @param[in] mbr_hdls Array of member handles of size num_mbrs
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_del_grp(p4_pd_sess_hdl_t sess_hdl,
                                p4_pd_ms_table_state_t *state,
                                bf_dev_id_t dev_id,
                                p4_pd_grp_hdl_t grp_hdl,
                                uint32_t num_mbrs,
                                p4_pd_mbr_hdl_t *mbr_hdls);

/**
 * @brief Create a new selector group
 *
 * @param[in] sess_hdl Session handle
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] grp_hdl Handle to the selector group
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_new_grp(p4_pd_sess_hdl_t sess_hdl,
                                p4_pd_ms_table_state_t *state,
                                bf_dev_id_t dev_id,
                                p4_pd_grp_hdl_t grp_hdl);

/**
 * @brief Get the members of the given selector group
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] grp_hdl Handle to the selector group
 * @param[out] mbr_hdls Pointer to return  the member handles of size num_mbrs
 * @param[out] num_mbrs Number of members in the selector group
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_grp_get_mbrs(p4_pd_ms_table_state_t *state,
                                     bf_dev_id_t dev_id,
                                     p4_pd_grp_hdl_t grp_hdl,
                                     p4_pd_mbr_hdl_t *mbr_hdls,
                                     int *num_mbrs);

/**
 * @brief Add a resource to the selector member
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mbr_hdl Handle to the selector member
 * @param[in] res_spec Pointer to the resource spec
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_mbr_add_res(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_mbr_hdl_t mbr_hdl,
                                    pd_res_spec_t *res_spec);

/**
 * @brief Get the resource details of the given selector member
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mbr_hdl Handle to the selector member
 * @param[out] count Pointer to return the number of resources
 * @param[out] res_spec Pointer to return the resource specs
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_mbr_get_res(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_mbr_hdl_t mbr_hdl,
                                    int *count,
                                    pd_res_spec_t **res_specs);

/**
 * @brief Update the resource details of the given selector group
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mbr_hdl Handle to the selector member
 * @param[in] grp_hdl Handle to the selector group
 * @param[in] aux Pointer to the resource specs
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_grp_update_res(bf_dev_id_t dev_id,
                                       p4_pd_mbr_hdl_t mbr_hdl,
                                       p4_pd_grp_hdl_t grp_hdl,
                                       void *aux);

/**
 * @brief Get the resource details of the given selector group
 *
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[in] dev_id Device identifier
 * @param[in] mbr_hdl Handle to the selector member
 * @param[in] grp_hdl Handle to the selector group
 * @param[out] count Pointer to return the number of resources
 * @param[out] res_spec Pointer to return the resource specs
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_grp_get_res(p4_pd_ms_table_state_t *state,
                                    bf_dev_id_t dev_id,
                                    p4_pd_grp_hdl_t grp_hdl,
                                    int *count,
                                    pd_res_spec_t **res_specs);

/**
 * @brief Begin the transaction for the given session
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_begin_txn(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Commit the transaction for the given session
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_commit_txn(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Abort the transaction for the given session
 *
 * @param[in] sess_hdl Session handle
 *
 * @return Status of the API call
 *
 */
p4_pd_status_t p4_pd_ms_abort_txn(p4_pd_sess_hdl_t sess_hdl);

/**
 * @brief Log the state of Action Profile and
 * Action Selector management
 *
 * @param[in] dev_id Device identifier
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[out] prof Pointer to cJSON to log the state
 *
 * @return Status of the API call
 *
 */
void p4_pd_ms_log_state(bf_dev_id_t dev_id,
                        p4_pd_ms_table_state_t *state,
                        cJSON *prof);

/**
 * @brief Restore the state of Action Profile and
 * Action Selector management
 *
 * @param[in] dev_id Device identifier
 * @param[in] Pointer to the state of Action Profile and
 * Action Selector management
 * @param[out] prof Pointer to cJSON to restore the state
 *
 * @return Status of the API call
 *
 */
void p4_pd_ms_restore_state(bf_dev_id_t dev_id,
                            p4_pd_ms_table_state_t *state,
                            cJSON *prof);

#endif
