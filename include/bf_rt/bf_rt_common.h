/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_common.h
 *
 *  @brief Contains Common data types used in BF-RT
 */
#ifndef _BF_RT_COMMON_H
#define _BF_RT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pipe_mgr/pipe_mgr_intf.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <bf_types/bf_types.h>

/**
 * @brief 32-bit ID for all BF-RT entities
 */
typedef uint32_t bf_rt_id_t;

/**
 * @brief 32-bit ID for table entry handles
 */
typedef uint32_t bf_rt_handle_t;

/**
 * @brief For sake of strict typechecking, instead of typedef-ing
 * \n void to our handle, we make a small struct with one element.
 */
#define DECLARE_HANDLE(name) \
  struct name##__ {          \
    int unused;              \
  };                         \
  typedef struct name##__ name

/**
 * @brief Macro used to set flag for table API calls.
 */
#define BF_RT_FLAG_SET(vf, offset) vf |= 1ull << offset;
/**
 * @brief Macro used to init flags variable.
 */
#define BF_RT_FLAG_INIT(vf) vf = 0;
/**
 * @brief Macro used to clear a flag.
 */
#define BF_RT_FLAG_CLEAR(vf, offset) vf &= ~(1ull << offset);
/**
 * @brief Conditional macro to check if specific flag is set.
 */
#define BF_RT_FLAG_IS_SET(vf, offset) (vf & (1ull << offset))

/**
 * @brief Used to set/clear bits in flag vector. Flags that are
 *        not used/handled by a specific function will be ignored.
 */
typedef enum bf_rt_flag_offset_t {
  /** Flag to operate on HW instead of SW. Supported with various
   * tableEntryGet() calls. Set for HW, unset for SW.
   */
  BF_RT_FROM_HW = 0,
  /** Flag used to perform delete action in tableEntryModInc
   * function calls. Bit set means delete otherwise add.
   */
  BF_RT_INC_DEL = 1,
  /** Flag used to skip TTL reset when modifying entry. */
  BF_RT_SKIP_TTL_RESET = 2,
  /** Flag used to skip direct stats reset when modifying entry. */
  BF_RT_SKIP_STAT_RESET = 3,
} bf_rt_flag_offset_t;

// C frontend typedefs
DECLARE_HANDLE(bf_rt_info_hdl);
DECLARE_HANDLE(bf_rt_session_hdl);
DECLARE_HANDLE(bf_rt_table_hdl);
DECLARE_HANDLE(bf_rt_table_key_hdl);
DECLARE_HANDLE(bf_rt_table_data_hdl);
DECLARE_HANDLE(bf_rt_table_attributes_hdl);
DECLARE_HANDLE(bf_rt_entry_scope_arguments_hdl);
DECLARE_HANDLE(bf_rt_learn_hdl);
DECLARE_HANDLE(bf_rt_table_operations_hdl);

/**
 * @brief learn_data_hdl and table_data_hdl are the same,
 *  \n hence simple typedef like BfRtLearnData in
 *  \n include/bf_rt_learn.hpp
 */
typedef bf_rt_table_data_hdl bf_rt_learn_data_hdl;

/* C and C++ void typedefs */
DECLARE_HANDLE(bf_rt_learn_msg_hdl);

/**
 * @brief Structure definition for configuration target
 */
typedef struct bf_rt_target_ {
  /** Device ID */
  bf_dev_id_t dev_id;
  /** Pipe ID */
  bf_dev_pipe_t pipe_id;
  /** Gress direction In/Eg */
  bf_dev_direction_t direction;
  /** Parser ID */
  uint8_t prsr_id;
} bf_rt_target_t;

/**
 * @brief Get error details string from an error status
 *
 * @param[in] sts Status of type @c bf_status_t
 * @param[out] err_str Pointer to error string. Doesn't require user to allocate
 *space
 */
void bf_rt_err_str(bf_status_t sts, const char **err_str);

#ifdef __cplusplus
}
#endif

#endif
