/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_session.h
 *
 *  @brief Contains BF-RT Session APIs
 */
#ifndef _BF_RT_SESSION_H
#define _BF_RT_SESSION_H

#include <bf_rt/bf_rt_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API to create session
 *
 * @param[out] session Ptr to allocated session object
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_session_create(bf_rt_session_hdl **session);

/**
 * @brief Destroy a session
 *
 * @param[in] session Ptr to session object
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_session_destroy(bf_rt_session_hdl *const session);

/**
 * @brief Get the internal session ID.
 *
 * @param[in] session Ptr to session object
 *
 * @return Internal session ID
 */
bf_rt_id_t bf_rt_sess_handle_get(const bf_rt_session_hdl *const session);

/**
 * @brief Get the internal PRE session ID
 *
 * @param[in] session Ptr to session object
 *
 * @return Internal PRE session ID
 */
bf_rt_id_t bf_rt_pre_sess_handle_get(const bf_rt_session_hdl *const session);

/**
 * @brief Check whether this Session Object  is still valid or not
 *
 * @param[in] session Ptr to session object
 *
 * @retval true if session exists
 * @retval false if session has been destroyed
 */
bool bf_rt_session_is_valid(const bf_rt_session_hdl *const session);

/**
 * @brief Wait for all operations to complete under this session
 *
 * @param[in] session Ptr to session object
 * @return Status of the API call
 */
bf_status_t bf_rt_session_complete_operations(
    const bf_rt_session_hdl *const session);

// Batching functions
/**
 * @brief Begin a batch on a session. Only one batch can be in progress
 * on any given session.  Updates to the hardware will be batch together
 * and delayed until the batch is ended.
 *
 * @param[in] session Ptr to session object
 * @return Status of the API call
 */
bf_status_t bf_rt_begin_batch(bf_rt_session_hdl *const session);

/**
 * @brief Flush a batch on a session pushing all pending updates to hardware.
 *
 * @param[in] session Ptr to session object
 * @return Status of the API call
 */
bf_status_t bf_rt_flush_batch(bf_rt_session_hdl *const session);

/**
 * @brief End a batch on a session and push all batched updated to hardware.
 *
 * @param[in] session Ptr to session object
 * @param[in] hwSynchronous If @c true, block till all operations are
 * complete,
 *  in this transaction.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_end_batch(bf_rt_session_hdl *const session,
                            bool hwSynchronous);

// Transaction functions
/**
 * @brief Begin a transaction on a session. Only one transaction can be in
 *progress
 *      \n on any given session
 *
 * @param[in] session Ptr to session object
 * @param[in] isAtomic If @c true, upon committing the transaction, all
 *changes
 *        \n will be applied atomically such that a packet being processed
 *will
 *        \n see either all of the changes or none of the changes.
 * @return Status of the API call
 */
bf_status_t bf_rt_begin_transaction(bf_rt_session_hdl *const session,
                                    bool isAtomic);

/**
 * @brief Verify if all the API requests against the transaction in progress
 *have
 *        \n resources to be committed. This also ends the transaction
 *implicitly
 *
 * @param[in] session Ptr to session object
 * @return Status of the API call
 */
bf_status_t bf_rt_verify_transaction(bf_rt_session_hdl *const session);

/**
 * @brief Commit all changes in a transaction
 *
 * @param[in] session Ptr to session object
 * @param[in] hwSynchronous If @c true, block till all operations are
 *complete,
 *  in this transaction.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_commit_transaction(bf_rt_session_hdl *const session,
                                     bool hwSynchronous);

/**
 * @brief Abort and rollback all API requests against the transaction in
 *progress.
 * \n This also ends the transaction implicitly
 *
 * @param[in] session Ptr to session object
 * @return Status of the API call
 */
bf_status_t bf_rt_abort_transaction(bf_rt_session_hdl *const session);

#ifdef __cplusplus
}
#endif

#endif  // _BF_RT_SESSION_H
