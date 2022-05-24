/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_session.hpp
 *
 *  @brief Contains BF-RT Session APIs
 */
#ifndef _BF_RT_SESSION_HPP
#define _BF_RT_SESSION_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <bf_rt/bf_rt_common.h>

namespace bfrt {

/**
 * @brief Class to create session objects. <br>
 * <B>Creation: </B> Can only be created using \ref
 * bfrt::BfRtSession::sessionCreate()
 */
class BfRtSession {
 public:
  /**
   * @brief Destructor which destroys the connection if it is established
   */
  virtual ~BfRtSession() = default;

  /**
   * @name Session APIs
   * @{
   */
  /**
   * @brief Factory function to create a session object
   *
   * @returns @c std::shared_ptr<BfRtSession> to session object. The
   * session object is a shared_ptr so that along with the user, the
   * APIs can take over ownership of the session at times too if and
   * when required
   */
  static std::shared_ptr<BfRtSession> sessionCreate();

  /**
   * @brief Destroy session
   *
   * @return Status of the API call
   *
   */
  virtual bf_status_t sessionDestroy() = 0;

  /**
   * @brief Wait for all operations to complete under this session
   *
   * @return Status of the API call
   */
  virtual bf_status_t sessionCompleteOperations() const = 0;

  /**
   * @brief Get the Session Handle
   *
   * @returns Session Handle of an active session
   */
  virtual const bf_rt_id_t &sessHandleGet() const = 0;

  /**
   * @brief Get the PRE Session Handle
   *
   * @returns PRE Session Handle of an active session
   */
  virtual const bf_rt_id_t &preSessHandleGet() const = 0;

  /**
   * @brief Check whether this Session Object's handle is still valid or not
   *
   * @retval true if session exists
   * @retval false if session has been destroyed
   */
  virtual const bool &isValid() const = 0;
  /** @} */  // End of group Session

  // Batching functions
  /**
   * @name Batching Batching APIs
   * @{
   */
  /**
   * @brief Begin a batch on a session. Only one batch can be in progress
   * on any given session.  Updates to the hardware will be batch together
   * and delayed until the batch is ended.
   *
   * @return Status of the API call
   */
  virtual bf_status_t beginBatch() const = 0;

  /**
   * @brief Flush a batch on a session pushing all pending updates to hardware.
   *
   * @return Status of the API call
   */
  virtual bf_status_t flushBatch() const = 0;

  /**
   * @brief End a batch on a session and push all batched updated to hardware.
   *
   * @param[in] hwSynchronous If @c true, block till all operations are
   *complete,
   *  in this transaction.
   *
   * @return Status of the API call
   */
  virtual bf_status_t endBatch(bool hwSynchronous) const = 0;
  /** @} */  // End of group Batching

  /**
   * @name Transaction
   * Transaction APIs
   * @{
   */
  // Transaction functions
  /**
   * @brief Begin a transaction on a session. Only one transaction can be in
   *progress
   *      \n on any given session
   *
   * @param[in] isAtomic If @c true, upon committing the transaction, all
   *changes
   *        \n will be applied atomically such that a packet being processed
   *will
   *        \n see either all of the changes or none of the changes.
   * @return Status of the API call
   */
  virtual bf_status_t beginTransaction(bool isAtomic) const = 0;

  /**
   * @brief Verify if all the API requests against the transaction in progress
   *have
   *        \n resources to be committed. This also ends the transaction
   *implicitly
   *
   * @return Status of the API call
   */
  virtual bf_status_t verifyTransaction() const = 0;

  /**
   * @brief Commit all changes in a transaction
   *
   * @param[in] hwSynchronous If @c true, block till all operations are
   *complete,
   *  in this transaction.
   *
   * @return Status of the API call
   */
  virtual bf_status_t commitTransaction(bool hwSynchronous) const = 0;

  /**
   * @brief Abort and rollback all API requests against the transaction in
   *progress.
   * \n This also ends the transaction implicitly
   *
   * @return Status of the API call
   */
  virtual bf_status_t abortTransaction() const = 0;
  /** @} */  // End of group Transaction
};

}  // namespace bfrt

#endif
