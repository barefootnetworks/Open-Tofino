/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_table_attributes.hpp
 *
 *  @brief Contains BF-RT Table Attribute APIs
 */

#ifndef _BF_RT_TABLE_ATTRIBUTES_HPP
#define _BF_RT_TABLE_ATTRIBUTES_HPP

#include <bitset>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_attributes.h>

#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_session.hpp>

namespace bfrt {

/**
 * @brief Attribute types. Table attributes properties on an entire
 * table. Main difference between Attributes and Operations is that
 * Attributes deal with some state but operations don't.
 */
enum class TableAttributesType {
  /** Pipe scope of all entries. Applicable to all Match Action
       Tables(MAT) */
  ENTRY_SCOPE = 0,
  /** Dynamic key mask on MATs if applicable*/
  DYNAMIC_KEY_MASK = 1,
  /** Idle table on MATs if applicable*/
  IDLE_TABLE_RUNTIME = 2,
  /** Dynamic hash table attribute to set seed. To be deprecated soon */
  DYNAMIC_HASH_ALG_SEED = 3,
  /** Meter byte count asjust. Applicable to meter
                               tables and MATs if they have direct meters*/
  METER_BYTE_COUNT_ADJ = 4,
  /** Port status change cb set attribute. Applicable to Port table */
  PORT_STATUS_NOTIF = 5,
  /** Port stat poll interval set. Applicable to port stats table*/
  PORT_STAT_POLL_INTVL_MS = 6,
  /** PRE device config. Applicable to PRE tables*/
  PRE_DEVICE_CONFIG = 7,
  /** Selector update CB*/
  SELECTOR_UPDATE_CALLBACK = 8
};

/**
 * @brief IdleTable Mode
 */
enum class TableAttributesIdleTableMode {
  /** Idle poll mode. When set, entry_hit_state on MAT entries can be
     queried to check idle time */
  POLL_MODE = 0,
  /** Idle notify mode. Can be used to set CB for idletimeout on a MAT */
  NOTIFY_MODE = 1,
  INVALID_MODE = 2
};

/**
 * @brief Pipe Entry scope
 */
enum class TableEntryScope {
  /** Set scope to all pipelines of current profile for this table. Turns
     table to symmetric. Default mode of tables */
  ENTRY_SCOPE_ALL_PIPELINES = 0,
  /** Set scope to a single logical pipe in this profile for this table.
      Turns table to assymmetric */
  ENTRY_SCOPE_SINGLE_PIPELINE = 1,
  /** Set scope to user defined scope in this profile for this table.
      Turns table to assymmetric but can be used to group some pipes
      together and hence can be used differently from single scope */
  ENTRY_SCOPE_USER_DEFINED = 2
};

/**
 * @brief Gress Scope. Similar to pipe scope but for gress
 */
enum class TableGressScope {
  /** Both ingress and egress in scope */
  GRESS_SCOPE_ALL_GRESS,
  /** Either Ingress or Egress in scope */
  GRESS_SCOPE_SINGLE_GRESS
};

/**
 * @brief Parser Scope. Similar to pipe_scope bit for parser
 */
enum class TablePrsrScope {
  /** All parsers in scope*/
  PRSR_SCOPE_ALL_PRSRS_IN_PIPE,
  /** Single parser scope*/
  PRSR_SCOPE_SINGLE_PRSR
};

/**
 * @brief Gress Target. Similar to Pipe ID but for gress
 */
enum class GressTarget {
  /** Ingress */
  GRESS_TARGET_INGRESS,
  /** Egress */
  GRESS_TARGET_EGRESS,
  /** All gress */
  GRESS_TARGET_ALL = 0xFF
};

/**
 * @brief IdleTimeout Callback
 * @param[in] dev_tgt Device target
 * @param[in] key Table Key
 * @param[in] cookie User provided cookie during cb registration
 */
typedef std::function<void(
    const bf_rt_target_t &dev_tgt, const BfRtTableKey *key, void *cookie)>
    BfRtIdleTmoExpiryCb;
typedef BfRtIdleTmoExpiryCb BfRtIdleTmoActiveCb;

/**
 * @brief PortStatusChange Callback
 * @param[in] dev_id Device ID
 * @param[in] key Port Table Key
 * @param[in] port_up If port is up
 * @param[in] cookie User provided cookie during cb registration
 */
typedef std::function<void(const bf_dev_id_t &dev_id,
                           const BfRtTableKey *key,
                           const bool &port_up,
                           void *cookie)>
    BfRtPortStatusNotifCb;

/**
 * @brief Selector Table Update Callback. This can be used to get notification
 * of data plane triggered Sel table update
 *
 * @param[in] session shared_ptr to session
 * @param[in] dev_tgt Device target
 * @param[in] cookie User provided cookie during cb registration
 * @param[in] sel_grp_id Selector-grp ID which was updated
 * @param[in] act_mbr_id action-mbr ID which was updated
 * @param[in] logical_entry_index Table logical entry index
 * @param[in] is_add If the operation was add or del
 */
typedef std::function<void(const std::shared_ptr<BfRtSession> session,
                           const bf_rt_target_t &dev_tgt,
                           const void *cookie,
                           const bf_rt_id_t &sel_grp_id,
                           const bf_rt_id_t &act_mbr_id,
                           const int &logical_entry_index,
                           const bool &is_add)>
    selUpdateCb;
/**
 * @brief Class to expose APIs to set/get the entry scope arguments with
 * std::bitset.The absolute scope val is a 64 bit unsigned int which can be set
 * using a 64-bit std::bitset object or can be set on a per byte basis using an
 * array of size 8 of 8-bit std::bitset objects. The 64-bit bitset looks like
 * 0x00 0x00 0x00 0x00 0x00 0x00 0x0c 0x03 if we want pipes 2,3 to be in
 * scope 1 and pipes 0,1 to be in scope 0. Each "byte" of the bitset is a scope.
 * There can be a max of 8 scopes, but number of supported scopes depends on
 * the Tofino version. No pipe can belong to more than 1 scope at once.
 * The bf_rt_dev_target should contain the lowest pipe in the scope for
 * the entry programming. So in the above example, if one wants add an entry to
 * scope 1, then pipe_id in the target should be equal to 2, and then both pipes
 * will be programmed accordingly.
 * <B>Creation: </B> Can only be created using \ref
 * bfrt::BfRtTableAttributes::entryScopeArgumentsAllocate()
 */
class BfRtTableEntryScopeArguments {
 public:
  virtual ~BfRtTableEntryScopeArguments() = default;
  /**
   * @brief Set entry scope as a 64-bit bitset
   *
   * @param[in] val 32-bit bitset
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const std::bitset<64> &val) = 0;
  /**
   * @brief Set entry scope as a 4-length array of 8-bit bitsets
   * <B>Note: </B>The least significant byte of the
   * scope, i.e. byte 0, corresponds to std::bitset array[0], byte 1 = array[1],
   * byte 2 = array[2], byte 3 = array[3] etc.<br>
   *
   * @param[in] val_arr 4-length array of 8-bit bitset
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(
      const std::array<std::bitset<8>, 8> &val_arr) = 0;
  /**
   * @brief Get entry scope as a 64-bit bitset
   *
   * @param[in] val 64-bit bitset
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(std::bitset<64> *val) const = 0;
  /**
   * @brief Get entry scope as a 8-length array of 8-bit bitsets
   * <B>Note: </B>The least significant byte of the
   * scope val (byte 0) corresponds to std::bitset array[0], byte 1 = array[1],
   * byte 2 = array[2], byte 3 = array[3] etc.<br>
   *
   * @param[in] val_arr 4-length array of 8-bit bitset
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(
      std::array<std::bitset<8>, 8> *val_arr) const = 0;
};

/**
 * @brief Class to Set and Get Attributes<br>
 * <B>Creation: </B> Can only be created using \ref
 * bfrt::BfRtTable::attributeAllocate()
 */
class BfRtTableAttributes {
 public:
  virtual ~BfRtTableAttributes() = default;
  /**
   * @brief Set IdleTable Poll Mode options. This is only valid if the
   *Attributes Object
   * was allocated using the correct
   * \ref bfrt::BfRtTable::attributeAllocate(const TableAttributesType &, const
   *TableAttributesIdleTableMode &,std::unique_ptr<BfRtTableAttributes>
   **attr)const "attributeAllocate()"
   *
   * @param[in] enable Flag to enable IdleTable
   *
   * @return Status of the API call
   */
  virtual bf_status_t idleTablePollModeSet(const bool &enable) = 0;

  /**
   * @brief Set IdleTable Notify Mode options. This is only valid if the
   * Attributes Object was allocated using the correct
   * \ref bfrt::BfRtTable::attributeAllocate(const TableAttributesType &, const
   *TableAttributesIdleTableMode &,std::unique_ptr<BfRtTableAttributes>
   **attr)const "attributeAllocate()"
   *
   * @param[in] enable Flag to enable IdleTable
   * @param[in] idle Callback on IdleTime Timeout
   * @param[in] ttl_query_interval Ttl query interval
   * @param[in] max_ttl Max ttl value that an entry can have
   * @param[in] min_ttl Min ttl value that an entry can have
   * @param[in] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t idleTableNotifyModeSet(const bool &enable,
                                             const BfRtIdleTmoExpiryCb &idle_cb,
                                             const uint32_t &ttl_query_interval,
                                             const uint32_t &max_ttl,
                                             const uint32_t &min_ttl,
                                             const void *cookie) = 0;

  /**
   * @brief Set IdleTable Notify 2 Way Mode options. This is only valid if the
   * Attributes Object was allocated using the correct
   * \ref bfrt::BfRtTable::attributeAllocate(const TableAttributesType &, const
   *TableAttributesIdleTableMode &,std::unique_ptr<BfRtTableAttributes>
   **attr)const "attributeAllocate()"
   *
   * @param[in] enable Flag to enable IdleTable
   * @param[in] idle Callback on IdleTime Timeout
   * @param[in] active_cb Callback on IdleTime Activation
   * @param[in] ttl_query_interval Ttl query interval
   * @param[in] max_ttl Max ttl value that an entry can have
   * @param[in] min_ttl Min ttl value that an entry can have
   * @param[in] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t idleTableNotifyModeSet(
      const bool &enable,
      const BfRtIdleTmoExpiryCb &idle_cb,
      const BfRtIdleTmoActiveCb &active_cb,
      const uint32_t &ttl_query_interval,
      const uint32_t &max_ttl,
      const uint32_t &min_ttl,
      const void *cookie) = 0;

  /**
   * @brief Get Idle Table params
   *
   * @param[out] mode Mode of IdleTable (POLL/NOTIFY)
   * @param[out] enable Enable flag
   * @param[out] idle_cb Calbback on Idle Timeout
   * @param[out] ttl_query_interval Ttl query interval
   * @param[out] max_ttl Max ttl value that an entry can have
   * @param[out] min_ttl Min ttl value that an entry can have
   * @param[out] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t idleTableGet(TableAttributesIdleTableMode *mode,
                                   bool *enable,
                                   BfRtIdleTmoExpiryCb *idle_cb,
                                   uint32_t *ttl_query_interval,
                                   uint32_t *max_ttl,
                                   uint32_t *min_ttl,
                                   void **cookie) const = 0;

  /**
   * @brief Get Idle Table params for 2 way notifications
   *
   * @param[out] mode Mode of IdleTable (POLL/NOTIFY)
   * @param[out] enable Enable flag
   * @param[out] idle_cb Calbback on Idle Timeout
   * @param[out] active_cb Calbback on Idle Activation
   * @param[out] ttl_query_interval Ttl query interval
   * @param[out] max_ttl Max ttl value that an entry can have
   * @param[out] min_ttl Min ttl value that an entry can have
   * @param[out] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t idleTableGet(TableAttributesIdleTableMode *mode,
                                   bool *enable,
                                   BfRtIdleTmoExpiryCb *idle_cb,
                                   BfRtIdleTmoActiveCb *active_cb,
                                   uint32_t *ttl_query_interval,
                                   uint32_t *max_ttl,
                                   uint32_t *min_ttl,
                                   void **cookie) const = 0;

  /**
   * @brief Allocate entryScopeArguments Object
   *
   * @param[out] scop_args_ret EntryScopeArguments Object
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeArgumentsAllocate(
      std::unique_ptr<BfRtTableEntryScopeArguments> *scop_args_ret) const = 0;

  /**
   * @brief Set entry scope params in the Attributes Object
   *
   * @param[in] entry_scope Pipe entry scope (ALL/SINGLE/USER)
   * @param[in] scope_args EntryScopeArguments Object. Contains data for
   * user-defined pipe entry scope if applicable
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeParamsSet(
      const TableEntryScope &entry_scope,
      const BfRtTableEntryScopeArguments &scope_args) = 0;

  /**
   * @brief Set entry scope params in the Attributes Object
   *
   * @param[in] entry_scope Pipe entry scope (All/Single/User)
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeParamsSet(
      const TableEntryScope &entry_scope) = 0;

  /**
   * @brief Set entry scope params in the Attributes Object
   *
   * @param[in] gress_scope   Gress (All/Single)
   * @param[in] pipe    Pipe entry scope (All/Single/User)
   * @param[in] pipe_args EntryScopeArguments Object. Contains data for
   * user-defined pipe entry scope if applicable
   * @param[in] prsr    Parser scope (All/Single)
   * @param[in] gress_target   Gress Target (In/Eg/All)
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeParamsSet(
      const TableGressScope &gress_scope,
      const TableEntryScope &pipe,
      const BfRtTableEntryScopeArguments &pipe_args,
      const TablePrsrScope &prsr,
      const GressTarget &gress_target) = 0;
  /**
   * @brief Get entry scope params from the Attributes Object
   *
   * @param[out] entry_scope Entry scope
   * @param[out] scope_args EntryScopeArguments Object
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeParamsGet(
      TableEntryScope *entry_scope,
      BfRtTableEntryScopeArguments *scope_args) const = 0;

  /**
   * @brief Get entry scope params from the Attributes Object
   *
   * @param[out] gress_scope   Gress (All/Single)
   * @param[out] pipe    Pipe entry scope (All/Single/User)
   * @param[out] pipe_args EntryScopeArguments Object. Contains data for
   * user-defined pipe entry scope if applicable
   * @param[out] prsr    Parser scope (All/Single)
   * @param[out] gress_target   Gress Target (In/Eg/All)
   *
   * @return Status of the API call
   */
  virtual bf_status_t entryScopeParamsGet(
      TableGressScope *gress_scope,
      TableEntryScope *pipe,
      BfRtTableEntryScopeArguments *pipe_args,
      TablePrsrScope *prsr,
      GressTarget *gress_target) const = 0;

  /**
   * @brief Set Dynamic Key Mask params in the Attributes Object
   *
   * @param[in] field_mask Map from key field id to key field mask
   *
   * @return Status of the API call
   */
  virtual bf_status_t dynKeyMaskSet(
      const std::unordered_map<bf_rt_id_t, std::vector<uint8_t>>
          &field_mask) = 0;

  /**
   * @brief Get Dynamic Key Mask params in the Attributes Object
   *
   * @param[out] field_mask Map from key field id to key field mask
   *
   * @return Status of the API call
   */
  virtual bf_status_t dynKeyMaskGet(
      std::unordered_map<bf_rt_id_t, std::vector<uint8_t>> *field_mask)
      const = 0;

  /**
   * @brief Set Dynamic Hashing Algorithm and Seed in the Attributes Object
   * @param[in] alg_hdl Algorithm handler for hashing
   * @param[in] seed Seed for hashing
   *
   * @return Status of the API call
   */
  virtual bf_status_t dynHashingSet(const uint32_t &alg_hdl,
                                    const uint64_t &seed) = 0;

  /**
   * @brief Get Dynamic Hashing Algorithm and Seed in the Attributes Object
   * @param[out] alg_hdl Algorithm handler for hashing
   * @param[out] seed Seed for hashing
   *
   * @return Status of the API call
   */
  virtual bf_status_t dynHashingGet(uint32_t *alg_hdl,
                                    uint64_t *seed) const = 0;

  /**
   * @brief Set Meter Byte Count Adjust in the Attributes Object
   * @param[in] byte_count Number of adjust bytes for meter tables
   *
   * @return Status of the API call
   */
  virtual bf_status_t meterByteCountAdjSet(const int &byte_count) = 0;

  /**
   * @brief Get Meter Byte Count Adjust in the Attributes Object
   * @param[out] byte_count Number of adjust bytes for meter tables
   *
   * @return Status of the API call
   */
  virtual bf_status_t meterByteCountAdjGet(int *byte_count) const = 0;

  /**
   * @brief Set Port Status Change Notification Callback Function in the
   *Attributes Object
   * @param[in] enable Port status change notification enable
   * @param[in] callback Callback function
   * @param[in] cookie Pointer client data
   *
   * @return Status of the API call
   */
  virtual bf_status_t portStatusChangeNotifSet(
      const bool &enable,
      const BfRtPortStatusNotifCb &callback,
      const bf_rt_port_status_chg_cb &callback_c,
      const void *cookie) = 0;

  /**
   * @brief Get Port Status Change Notification Callback Function in the
   *Attributes Object
   * @param[out] enable Port status change notification enable
   * @param[out] callback Callback function
   * @param[out] cookie Pointer client data
   *
   * @return Status of the API call
   */
  virtual bf_status_t portStatusChangeNotifGet(
      bool *enable,
      BfRtPortStatusNotifCb *callback,
      bf_rt_port_status_chg_cb *callback_c,
      void **cookie) const = 0;

  /**
   * @brief Set Port Stat Poll Interval in Millisecond in the Attributes Object
   * @param[in] poll_intvl_ms Poll interval
   *
   * @return Status of the API call
   */
  virtual bf_status_t portStatPollIntvlMsSet(const uint32_t &poll_intvl_ms) = 0;

  /**
   * @brief Get Port Stat Poll Interval in Millisecond in the Attributes Object
   * @param[out] poll_intvl_ms Poll interval
   *
   * @return Status of the API call
   */
  virtual bf_status_t portStatPollIntvlMsGet(uint32_t *poll_intvl_ms) const = 0;

  /**
   * @brief Set PRE Global RID in the PRE Attributes Object
   * @param[in] global_rid Global rid value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preGlobalRidSet(const uint32_t &global_rid) = 0;

  /**
   * @brief Get PRE Global RID in the PRE Attributes Object
   * @param[out] global_rid Global rid value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preGlobalRidGet(uint32_t *global_rid) const = 0;

  /**
   * @brief Set PRE Port Protection Status in the PRE Attributes Object
   * @param[in] enable Enable/Disable status value
   *
   * @return Status of the API call
   */
  virtual bf_status_t prePortProtectionSet(const bool &enable) = 0;

  /**
   * @brief Get PRE Port Protection Status in the PRE Attributes Object
   * @param[out] enable Enable/Disable status value
   *
   * @return Status of the API call
   */
  virtual bf_status_t prePortProtectionGet(bool *enable) const = 0;

  /**
   * @brief Set PRE Fast Failover Status in the PRE Attributes Object
   * @param[in] enable Enable/Disable status value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preFastFailoverSet(const bool &enable) = 0;

  /**
   * @brief Get PRE Fast Failover Status in the PRE Attributes Object
   * @param[out] enable Enable/Disable status value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preFastFailoverGet(bool *enable) const = 0;

  /**
   * @brief Set PRE Max Nodes Before Yield in the PRE Attributes Object
   * @param[in] count  Max node before yield count value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preMaxNodesBeforeYieldSet(const uint32_t &count) = 0;

  /**
   * @brief Get PRE Max Nodes Before Yield in the PRE Attributes Object
   * @param[out] count  Max node before yield count value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preMaxNodesBeforeYieldGet(uint32_t *count) const = 0;

  /**
   * @brief Set PRE Max Node Threshold in the PRE Attributes Object
   * @param[in] node_count  Node count value
   * @param[in] node_port_lag_count  Node port lag count value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preMaxNodeThresholdSet(
      const uint32_t &node_count, const uint32_t &node_port_lag_count) = 0;

  /**
   * @brief Get PRE Max Node Threshold in the PRE Attributes Object
   * @param[out] node_count  Node count value
   * @param[out] node_port_lag_count  Node port lag count value
   *
   * @return Status of the API call
   */
  virtual bf_status_t preMaxNodeThresholdGet(
      uint32_t *node_count, uint32_t *node_port_lag_count) const = 0;

  /**
   * @brief Set Selector Update Notification Callback
   * @param[in] enable Flag to enable selector update notifications
   * @param[in] session Session
   * @param[in] callback_fn Callback on Selector table update
   * @param[in] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t selectorUpdateCbSet(
      const bool &enable,
      const std::shared_ptr<BfRtSession> session,
      const selUpdateCb &callback_fn,
      const void *cookie) = 0;

  /**
   * @brief Get Selector Update Notification Callback
   * @param[out] enable Enable Flag
   * @param[out] session Session
   * @param[out] callback_fn Callback fn set for Selector table update
   * @param[out] cookie User cookie
   *
   * @return Status of the API call
   */
  virtual bf_status_t selectorUpdateCbGet(bool *enable,
                                          BfRtSession **session,
                                          selUpdateCb *callback_fn,
                                          void **cookie) const = 0;
};
}  // namespace bfrt

#endif  // _BF_RT_TABLE_ATTRIBUTES_HPP
