/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */



/*!
 *  @file bf_ts_if.h
 *  @date
 *
 */

#ifndef __BF_TS_IF_H_
#define __BF_TS_IF_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup lld-ts-api
 * @{
 * This is a description of Timestamp APIs.
 */

/**
 * @brief bf_ts_global_ts_state_set
 *  Enable global timestamp function
 *
 * @param dev_id: int
 *  chip id
 *
 * @param enable:  bool
 *   Enable or disable timestamp function
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_state_set(bf_dev_id_t dev_id, bool enable);

/**
 * @brief bf_ts_global_ts_state_get
 *  Get the global timestamp enable state
 *
 * @param dev_id: int
 *  chip id
 *
 * @param enable: bool
 *   enable state of global timestamp function
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_state_get(bf_dev_id_t dev_id, bool *enable);

/**
 * @brief bf_ts_global_ts_value_set
 *  Configure the global timestamp counter
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_ns:  int64
 *   Global timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_value_set(bf_dev_id_t dev_id,
                                      uint64_t global_ts_ns);

/**
 * @brief bf_ts_global_ts_value_get
 *  Get the global timestamp counter
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_ns:  int64
 *   Global timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_value_get(bf_dev_id_t dev_id,
                                      uint64_t *global_ts_ns);

/**
 * @brief bf_ts_global_ts_inc_value_set
 *  Configure the global timestamp counter inc value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_ns:  int64
 *   Global timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_inc_value_set(bf_dev_id_t dev_id,
                                          uint32_t global_inc_ns);

/**
 * @brief bf_ts_global_ts_inc_value_get
 *  get the global timestamp counter inc value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_ns:  int64
 *   Global timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_inc_value_get(bf_dev_id_t dev_id,
                                          uint32_t *global_inc_ns);

/**
 * @brief bf_ts_global_ts_increment_one_time_set
 *  Increment Global timestamp register one-time
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_inc_time_ns:  int64
 *   Value to increment Global timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_increment_one_time_set(
    bf_dev_id_t dev_id, uint64_t global_ts_inc_time_ns);

/**
 * @brief bf_ts_global_ts_offset_set
 *  Set global timestamp offset value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_offset_ns:  int64
 *   Value to set for Global timestamp offset value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_offset_set(bf_dev_id_t dev_id,
                                       uint64_t global_ts_offset_ns);

/**
 * @brief bf_ts_global_ts_offset_get
 *  Get global timestamp offset value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_offset_ns:  int64
 *   Value to set for Global timestamp offset value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_ts_offset_get(bf_dev_id_t dev_id,
                                       uint64_t *global_ts_offset_ns);

/**
 * @brief bf_ts_global_baresync_ts_get
 *  Trigger and retrieve both global timestamp value and baresync timestamp
 *value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param global_ts_ns:  int64 ptr
 *   Pointer to buffer to store global timestamp value in ns
 *
 * @param baresync_ts_ns:  int64 ptr
 *   Pointer to buffer to store baresync timestamp value in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_global_baresync_ts_get(bf_dev_id_t dev_id,
                                         uint64_t *global_ts_ns,
                                         uint64_t *baresync_ts_ns);
// bf_status_t bf_ts_global_ts_periodic_distribution_timer_set(bf_dev_id_t
// dev_id,
//                                                            uint32_t
//                                                            timer_ns);

/**
 * @brief bf_ts_baresync_state_set
 *  Enable baresync function
 *
 * @param dev_id: int
 *  chip id
 *
 * @param enable:  bool
 *   Enable or disable baresync function
 *
 * @param reset_count_threshold: uint32_t
 *   reset count threshold value
 *
 * @param debounce_count: uint32_t
 *   debounce count value
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_state_set(bf_dev_id_t dev_id,
                                     uint32_t reset_count_threshold,
                                     uint32_t debounce_count,
                                     bool enable);

/**
 * @brief bf_ts_baresync_state_get
 *  get the configuration of baresync function
 *
 * @param dev_id: int
 *  chip id
 *
 * @param enable:  bool
 *   Enable state of baresync function
 *
 * @param reset_count_threshold: uint32_t
 *   reset count threshold value
 *
 * @param debounce_count: uint32_t
 *   debounce count value
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_state_get(bf_dev_id_t dev_id,
                                     uint32_t *reset_count_threshold,
                                     uint32_t *debounce_count,
                                     bool *enable);

/**
 * @brief bf_ts_baresync_reset_value_set
 *  Set the value to be written into baresync timestamp register on reset
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_time_ns:  int64
 *   Value to be written into baresync register on reset in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_reset_value_set(bf_dev_id_t dev_id,
                                           uint64_t baresync_time_ns);

/**
 * @brief bf_ts_baresync_reset_value_get
 *  Get the value of baresync timestamp register on reset
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_time_ns:  int64
 *   Value of the baresync register on reset in ns
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_reset_value_get(bf_dev_id_t dev_id,
                                           uint64_t *baresync_time_ns);

/**
 * @brief bf_ts_baresync_increment_set
 *  Set the baresync increment value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_inc_time_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 20-bit ns)
 *
 * @param baresync_inc_time_fract_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_increment_set(bf_dev_id_t dev_id,
                                         uint32_t baresync_inc_time_ns,
                                         uint32_t baresync_inc_time_fract_ns);

/**
 * @brief bf_ts_baresync_increment_get
 *  Get the baresync increment value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_inc_time_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 20-bit ns)
 *
 * @param baresync_inc_time_fract_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_ts_baresync_increment_get(bf_dev_id_t dev_id,
                                         uint32_t *baresync_inc_time_ns,
                                         uint32_t *baresync_inc_time_fract_ns);

/**
 * @brief bf_tof2_ts_baresync_increment_set
 *  Set the baresync increment value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_inc_time_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 20-bit ns)
 *
 * @param baresync_inc_time_fract_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 * @param baresync_inc_time_fract_den:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_tof2_ts_baresync_increment_set(
    bf_dev_id_t dev_id,
    uint32_t baresync_inc_time_ns,
    uint32_t baresync_inc_time_fract_ns,
    uint32_t baresync_inc_time_fract_den);

/**
 * @brief bf_tof2_ts_baresync_increment_get
 *  Get the baresync increment value
 *
 * @param dev_id: int
 *  chip id
 *
 * @param baresync_inc_time_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 20-bit ns)
 *
 * @param baresync_inc_time_fract_ns:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 * @param baresync_inc_time_fract_den:  int32
 *   baresync increment value (baresync_ts_inc_value: 28-bit fractional ns in
 *units of 2^-28 ns)
 *
 * @return status
 *   BF_SUCCESS on success
 *   BF ERROR code on failure
 *
 */

bf_status_t bf_tof2_ts_baresync_increment_get(
    bf_dev_id_t dev_id,
    uint32_t *baresync_inc_time_ns,
    uint32_t *baresync_inc_time_fract_ns,
    uint32_t *baresync_inc_time_fract_den);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* C++ */

#endif
