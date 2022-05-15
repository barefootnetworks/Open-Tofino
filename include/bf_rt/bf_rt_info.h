/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_info.h
 *
 *  @brief Contains BF-RT Info APIs. Mostly to get Table and Learn Object
 *  metadata
 */
#ifndef _BF_RT_INFO_H
#define _BF_RT_INFO_H

#include <bf_rt/bf_rt_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get size of the list of BfRtTable Objs
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] num_tables Size of array of BfRtLearn Obj pointers
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_num_tables_get(const bf_rt_info_hdl *bf_rt, int *num_tables);

/**
 * @brief Get Array of BfRtTable Objs from bf_rt_info
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] bf_rt_table_hdl_ret Array of BfRtTable Obj pointers. Memory needs
 * to be allocated by user
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_tables_get(const bf_rt_info_hdl *bf_rt,
                             const bf_rt_table_hdl **bf_rt_table_hdl_ret);

/**
 * @brief Get a BfRtTable Object from its fully qualified name
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] table_name Fully qualified P4 Table Obj name
 * @param[out] bf_rt_table_hdl_ret BfRtTable Obj Pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_table_from_name_get(
    const bf_rt_info_hdl *bf_rt,
    const char *table_name,
    const bf_rt_table_hdl **bf_rt_table_hdl_ret);

/**
 * @brief Get a BfRtTable Obj from its ID
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] id ID of the BfRtTable Obj
 * @param[out] bf_rt_table_hdl_ret BfRtTable Obj pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_table_from_id_get(
    const bf_rt_info_hdl *bf_rt,
    bf_rt_id_t id,
    const bf_rt_table_hdl **bf_rt_table_hdl_ret);

/**
 * @brief Convert Table obj name to ID
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] table_name Fully qualified name of the BfRtTable Obj
 * @param[out] id ID of the BfRtTable Obj
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_table_name_to_id(const bf_rt_info_hdl *bf_rt,
                                   const char *table_name,
                                   bf_rt_id_t *id_ret);

/**
 * @brief Get size of the list of BfRtLearn Objs
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] num_learns Size of array of BfRtLearn Obj pointers
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_num_learns_get(const bf_rt_info_hdl *bf_rt_info,
                                 int *num_learns);

/**
 * @brief Get Array of BfRtLearn Objs from bf_rt_info
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] bf_rt_learn_hdl_ret Array of BfRtLearn Obj pointers. Memory needs
 * to be allocated by user
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learns_get(const bf_rt_info_hdl *bf_rt_info,
                             const bf_rt_learn_hdl **bf_rt_learn_hdl_ret);

/**
 * @brief Get a BfRtLearn Object from its fully qualified name
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] learn_name Fully qualified P4 Learn Obj name
 * @param[out] bf_rt_learn_hdl_ret BfRtLearn Obj Pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_from_name_get(
    const bf_rt_info_hdl *bf_rt_info,
    const char *learn_name,
    const bf_rt_learn_hdl **bf_rt_learn_hdl_ret);

/**
 * @brief Get a BfRtLearn Obj from its ID
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] id ID of the BfRtLearn Obj
 * @param[out] bf_rt_learn_hdl_ret BfRtLearn Obj pointer
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_from_id_get(
    const bf_rt_info_hdl *bf_rt_info,
    bf_rt_id_t id,
    const bf_rt_learn_hdl **bf_rt_learn_hdl_ret);

/**
 * @brief Convert Learn obj name to ID
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] learn_name Fully qualified name of the BfRtLearn Obj
 * @param[out] id ID of the BfRtLearn Obj
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_learn_name_to_id(const bf_rt_info_hdl *bf_rt_info,
                                   const char *learn_name,
                                   bf_rt_id_t *id);

/**
 * @brief Get size of list of tables that are dependent on a given table
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] tbl_id ID of the BfRtTable obj
 * @param[out] num_tables Size of list of tables that depend on the given table
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_num_tables_dependent_on_this_table_get(
    const bf_rt_info_hdl *bf_rt_info, const bf_rt_id_t tbl_id, int *num_tables);

/**
 * @brief Get a list of tables that are dependent on a given table. When
 *  we say that table2 is dependent on table1, we imply that a entry
 *  needs to be added in table1 before we can add a corresponding entry
 *  in table2
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] tbl_id ID of the BfRtTable obj
 * @param[out] table_list Array of tables that depend on the given table. API
 * assumes
 * that the correct memory has been allocated by user
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_tables_dependent_on_this_table_get(
    const bf_rt_info_hdl *bf_rt_info,
    const bf_rt_id_t tbl_id,
    bf_rt_id_t *table_list);

/**
 * @brief Get size of list of tables that the given table is dependent on
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] tbl_id ID of the BfRtTable obj
 * @param[out] num_tables Size of list of tables that this table depends upon
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_num_tables_this_table_depends_on_get(
    const bf_rt_info_hdl *bf_rt_info, const bf_rt_id_t tbl_id, int *num_tables);

/**
 * @brief Get a list of tables that the given table is dependent on. When
 *    we say that table1 is dependent on table2, we imply that a entry
 *    cannot be added in table1 unless a corresponding entry is added
 *    to table2
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[in] tbl_id ID of the BfRtTable obj
 * @param[out] table_list Array of tables that depend on the given table. API
 * assumes
 * that the correct memory has been allocated by user
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_tables_this_table_depends_on_get(
    const bf_rt_info_hdl *bf_rt_info,
    const bf_rt_id_t tbl_id,
    bf_rt_id_t *table_list);

/**
 * @brief Get pipeline info of a Program.
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] num Size of array of profile names and pipes
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_info_num_pipeline_info_get(const bf_rt_info_hdl *bf_rt_info,
                                             int *num);

/**
 * @brief Get pipeline info of a Program.
 *
 * @param[in] bf_rt_info Handle of BfRtInfo object. Retrieved using
 * bf_rt_info_get()
 * @param[out] prof_names Array of char ptrs of pipeline profile names. API
 * assumes
 * that the correct memory has been allocated by user
 * @param[out] pipes Array of bf_dev_pipe_t ptrs of pipe IDs. API assumes
 * that the correct memory has been allocated by user for the ptr array. The
 * pipe
 * array would already be allocated by the API
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_info_pipeline_info_get(const bf_rt_info_hdl *bf_rt_info,
                                         const char **prof_names,
                                         const bf_dev_pipe_t **pipes);

#ifdef __cplusplus
}
#endif

#endif  // _BF_RT_INFO_H
