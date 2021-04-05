/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_learn.hpp
 *
 *  @brief Contains BF-RT Learn Object APIs
 */
#ifndef _BF_RT_LEARN_HPP
#define _BF_RT_LEARN_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_data.hpp>
#include <bf_rt/bf_rt_session.hpp>

namespace bfrt {

/**
 * @brief Only the below functions in comments are supported for BfRtLearnData.
 * \n Use of anything else will return a BF_NOT_SUPPORTED error
 * \n BfRtTableData::getValue(const bf_rt_id_t &field_id, uint64_t *val) const;
 * \n BfRtTableData::getValue(const bf_rt_id_t &field_id, const size_t &size,
 *                     uint8_t *val) const;
 */
using BfRtLearnData = BfRtTableData;

/**
 * @brief Learn Callback Function. This gets called upon receiving learn digests
 * from the device. Onlye one cb can be associated with a Learn object at a
 *time.
 * If a session already has registered, then another session cannot overwrite
 *
 * @param[in] bf_rt_tgt Bf Rt target associated with the learn data
 * @param[in] session @c std::shared_ptr to the session
 * @param[in] learnData Vector of learn data objs
 * @param[in] learn_msg_hdl Handle for the msg which can be used to notify ack
 * @param[in] cookie Cookie registered
 */
typedef std::function<bf_status_t(
    const bf_rt_target_t &bf_rt_tgt,
    const std::shared_ptr<BfRtSession> session,
    std::vector<std::unique_ptr<BfRtLearnData>> learnDataVec,
    bf_rt_learn_msg_hdl *const learn_msg_hdl,
    const void *cookie)> bfRtCbFunction;

/**
 * @brief Class to contain metadata of Learn Obj and perform functions
 *  like register and deregister Learn Callback <br>
 * <B>Creation: </B> Cannot be created. can only be queried from \ref
 * bfrt::BfRtInfo
 */
class BfRtLearn {
 public:
  virtual ~BfRtLearn() = default;

  /**
   * @brief Register Callback function to be called on a Learn event
   *
   * @param[in] session @c std::shared_ptr to the session. shared_ptr is used
   * in all of learn APIs because the callbacks return back a valid session
   * object as well. So it is required for us to make sure that the session
   * exists when the callback is being called. Hence the necessity to create
   * a shared_ptr to share ownership
   * @param[in] dev_tgt Device target
   * @param[in] callback_fn Callback function to be called upon incurring a
   *learn event
   * @param[in] cookie Optional cookie to be received with the callback
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfRtLearnCallbackRegister(
      const std::shared_ptr<BfRtSession> session,
      const bf_rt_target_t &dev_tgt,
      const bfRtCbFunction &callback_fn,
      const void *cookie) const = 0;

  /**
   * @brief Deregister the callback from the device
   *
   * @param[in] session @c std::shared_ptr to the session
   * @param[in] dev_tgt Device target
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfRtLearnCallbackDeregister(
      const std::shared_ptr<BfRtSession> session,
      const bf_rt_target_t &dev_tgt) const = 0;

  /**
   * @brief Notify the device that the learn_msg was received and
   * processed. This is required to be done in order to free up digest
   * related resources in the device
   *
   * @param[in] session Shared_ptr to the session
   * @param[in] learn_msg_hdl Handle of the msg to be notified
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfRtLearnNotifyAck(
      const std::shared_ptr<BfRtSession> session,
      bf_rt_learn_msg_hdl *const learn_msg_hdl) const = 0;

  /**
   * @brief Get ID of the learn Object
   *
   * @param [out] id ID of the learn Object
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnIdGet(bf_rt_id_t *id) const = 0;

  /**
   * @brief Get Name of the learn Object
   *
   * @param [out] name Name of the learn Object
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnNameGet(std::string *name) const = 0;

  /**
   * @brief Get the list of IDs of the Data Fields associated with
   *    the Learn obj
   *
   * @param[out] id_vec Vector of IDs to contain the list of IDs
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnFieldIdListGet(
      std::vector<bf_rt_id_t> *id_vec) const = 0;

  /**
   * @brief Get the ID of the Learn data field
   *
   * @param[in] name Data field Name
   * @param[out] field_id Field ID of the Data field
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnFieldIdGet(const std::string &name,
                                      bf_rt_id_t *field_id) const = 0;

  /**
   * @brief Get the size of the Learn Data Field
   *
   * @param[in] field_id Data field ID
   * @param[out] size Size of the Data field
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnFieldSizeGet(const bf_rt_id_t &field_id,
                                        size_t *size) const = 0;

  /**
   * @brief Find out whether the Learn Data field is a pointer or not
   *
   * @param[in] field_id Data field ID
   * @param[out] is_ptr Is it a pointer?
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnFieldIsPtrGet(const bf_rt_id_t &field_id,
                                         bool *is_ptr) const = 0;

  /**
   * @brief Get the Name of the Learn Data field
   *
   * @param[in] field_id Data Field ID
   * @param[out] name Data Field Name
   *
   * @return Status of the API call
   */
  virtual bf_status_t learnFieldNameGet(const bf_rt_id_t &field_id,
                                        std::string *name) const = 0;
};

}  // namespace bfrt

#endif
