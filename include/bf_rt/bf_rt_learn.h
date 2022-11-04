/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_learn.h
 *
 *
 *  @brief Contains BF-RT Learn Object APIs
 */
#ifndef _BF_RT_LEARN_H
#define _BF_RT_LEARN_H

#include <bf_rt/bf_rt_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C Learn Callback Function. This gets called upon receiving
 * learn digests from the device.
 * Only one cb can be associated with a Learn object at a time.
 * If a session already has registered, then another session cannot overwrite.
 * Do not deallocate/free bf_rt_table_key_hdl *key, this is taken care
 * by bfrt internal_callback function.
 * @param[in] bf_rt_tgt Bf Rt target associated with the learn data
 * @param[in] session Session object
 * @param[in] learnData Array of learn data objects
 * @param[in] num Size of array of learn data objects
 * @param[in] learn_msg_hdl Handle for the msg which can be used to notify ack
 * @param[in] cookie Optional user Cookie registered
 */
typedef bf_status_t (*bf_rt_cb_function)(
    const bf_rt_target_t *bf_rt_tgt,
    const bf_rt_session_hdl *session,
    const bf_rt_learn_data_hdl **learnData,
    uint32_t num,
    bf_rt_learn_msg_hdl *const learn_msg_hdl,
    const void *cookie);

/**
 * @brief Register Callback function to be called on a Learn event
 *
 * @param[in] learn Learn object handle
 * @param[in] session Ptr to the session
 * @param[in] dev_tgt Device target
 * @param[in] callback_fn Callback function to be called upon incurring a
 * learn event
 * @param[in] cookie Optional cookie to be received with the callback
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_callback_register(const bf_rt_learn_hdl *learn,
                                          const bf_rt_session_hdl *session,
                                          const bf_rt_target_t *dev_tgt,
                                          const bf_rt_cb_function callback_fn,
                                          const void *cookie);

/**
 * @brief Deregister the callback from the device
 *
 * @param[in] learn Learn object handle
 * @param[in] session Ptr to the session
 * @param[in] dev_tgt Device target
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_callback_deregister(const bf_rt_learn_hdl *learn,
                                            const bf_rt_session_hdl *session,
                                            const bf_rt_target_t *dev_tgt);

/**
 * @brief Notify the device that the learn_msg was received and
 * processed. This is required to be done in order to free up digest
 * related resources in the device
 *
 * @param[in] learn Learn object handle
 * @param[in] session Shared_ptr to the session
 * @param[in] learn_msg_hdl Handle of the msg to be notified
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_notify_ack(const bf_rt_learn_hdl *learn,
                                   const bf_rt_session_hdl *session,
                                   bf_rt_learn_msg_hdl *const learn_msg_hdl);

/**
 * @brief Get ID of the learn Object
 *
 * @param[in] learn Learn object handle
 * @param [out] learn_id ID of the learn Object
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_id_get(const bf_rt_learn_hdl *learn,
                               bf_rt_id_t *learn_id);

/**
 * @brief Get Name of the learn Object
 *
 * @param[in] learn Learn object handle
 * @param [out] learn_name_ret Name of the learn Object. User doesn't need
 * to allocate space for the string
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_name_get(const bf_rt_learn_hdl *learn,
                                 const char **learn_name_ret);

/**
 * @brief Get the size of list of IDs of the Data Fields associated with
 *    the Learn obj
 *
 * @param[in] learn Learn object handle
 * @param[out] num Size of Array of IDs to contain the list of IDs.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_id_list_size_get(const bf_rt_learn_hdl *learn,
                                               uint32_t *num);

/**
 * @brief Get the list of IDs of the Data Fields associated with
 *    the Learn obj
 *
 * @param[in] learn Learn object handle
 * @param[out] field_id_list Array of IDs to contain the list of IDs. User
 * needs to allocate space for the array
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_id_list_get(const bf_rt_learn_hdl *learn,
                                          bf_rt_id_t *field_id_list);

/**
 * @brief Get the ID of the Learn data field
 *
 * @param[in] learn Learn object handle
 * @param[in] name Data field Name
 * @param[out] field_id Field ID of the Data field
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_id_get(const bf_rt_learn_hdl *learn,
                                     const char *name,
                                     bf_rt_id_t *field_id);

/**
 * @brief Get the size of the Learn Data Field
 *
 * @param[in] learn Learn object handle
 * @param[in] field_id Data field ID
 * @param[out] size Size of the Data field
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_size_get(const bf_rt_learn_hdl *learn,
                                       const bf_rt_id_t field_id,
                                       size_t *size);

/**
 * @brief Find out whether the Learn Data field is a pointer or not
 *
 * @param[in] learn Learn object handle
 * @param[in] field_id Data field ID
 * @param[out] is_ptr Is it a pointer?
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_is_ptr_get(const bf_rt_learn_hdl *learn,
                                         const bf_rt_id_t field_id,
                                         bool *is_ptr);

/**
 * @brief Get the Name of the Learn Data field
 *
 * @param[in] learn Learn object handle
 * @param[in] field_id Data Field ID
 * @param[out] field_name_ret Field name. Memory allocation by user is not
 *required
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_field_name_get(const bf_rt_learn_hdl *learn,
                                       const bf_rt_id_t field_id,
                                       const char **field_name_ret);

#ifdef __cplusplus
}
#endif

#endif  // _BF_RT_LEARN_H
