/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_info.hpp
 *
 *  @brief Contains BF-RT Info APIs. Mostly to get Table and Learn Object
 *metadata
 */
#ifndef _BF_RT_INFO_HPP
#define _BF_RT_INFO_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

#include <bf_rt/bf_rt_common.h>

/**
 * @brief Namespace for BFRT
 */
namespace bfrt {

// Forward declarations
class BfRtLearn;
class BfRtTable;

/**
 * @brief A vector of pairs of (P4Pipeline names and pipe_array)
 */
using PipelineProfInfoVec = std::vector<
    std::pair<std::reference_wrapper<const std::string>,
              std::reference_wrapper<const std::vector<bf_dev_pipe_t>>>>;

/**
 * @brief Class to maintain metadata of all tables and learn objects. Note that
 *    all the objects returned are representations of the actual HW tables.\n So
 *    BfRtInfo doesn't provide ownership of any of its internal structures.
 *    Furthermore, all the metadata is read-only hence only const pointers are
 *    returned.<br>
 * <B>Creation: </B> Cannot be created. Can only be retrived using \ref
 * bfrt::BfRtDevMgr::bfRtInfoGet()
 */
class BfRtInfo {
 public:
  /**
   * @brief Destructor destroys all metadata contained in this object.
   */
  virtual ~BfRtInfo() = default;

  /**
   * @brief Get all the BfRtTable objs.
   *
   * @param[out] table_vec_ret Vector of BfRtTable obj pointers
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtInfoGetTables(
      std::vector<const BfRtTable *> *table_vec_ret) const = 0;
  /**
   * @brief Get a BfRtTable obj from its fully qualified name
   *
   * @param[in] name Fully qualified P4 table name
   * @param[out] table_ret BfRtTable obj pointer
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtTableFromNameGet(
      const std::string &name, const BfRtTable **table_ret) const = 0;
  /**
   * @brief Get a BfRtTable obj from its ID
   *
   * @param[in] id ID of the BfRtTable obj
   * @param[out] table_ret BfRtTable obj pointer
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtTableFromIdGet(bf_rt_id_t id,
                                         const BfRtTable **table_ret) const = 0;

  /**
   * @brief Get all the BfRtLearn Objs
   *
   * @param[out] learn_vec_ret Vector of BfRtLearn Obj pointers
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtInfoGetLearns(
      std::vector<const BfRtLearn *> *learn_vec_ret) const = 0;
  /**
   * @brief Get a BfRtLearn Object from its fully qualified name
   *
   * @param[in] name Fully qualified P4 Learn Obj name
   * @param[out] learn_ret BfRtLearn Obj Pointer
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtLearnFromNameGet(
      std::string name, const BfRtLearn **learn_ret) const = 0;
  /**
   * @brief Get a BfRtLearn Obj from its ID
   *
   * @param[in] id ID of the BfRtLearn Obj
   * @param[out] learn_ret BfRtLearn Obj pointer
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtLearnFromIdGet(bf_rt_id_t id,
                                         const BfRtLearn **learn_ret) const = 0;

  /**
   * @brief Get a list of tables that are dependent on a given table. When
   * 	we say that table2 is dependent on table1, we imply that a entry
   * 	needs to be added in table1 before we can add a corresponding entry
   * 	in table2
   *
   * @param[in] tbl_id ID of the BfRtTable obj
   * @param[out] table_vec_ret List of tables that depend on the given table
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtInfoTablesDependentOnThisTableGet(
      const bf_rt_id_t &tbl_id,
      std::vector<bf_rt_id_t> *table_vec_ret) const = 0;

  /**
   * @brief Get a list of tables that the given table is dependent on. When
   *    we say that table1 is dependent on table2, we imply that a entry
   *    cannot be added in table1 unless a corresponding entry is added
   *	to table2
   *
   * @param[in] tbl_id ID of the BfRtTable obj
   * @param[out] table_vec_ret List of tables that the given table depends on
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfrtInfoTablesThisTableDependsOnGet(
      const bf_rt_id_t &tbl_id,
      std::vector<bf_rt_id_t> *table_vec_ret) const = 0;

  /**
   * @brief Get pipeline info of a Program.
   *
   * @param[out] pipeline_map Get a vector of a pair of <ref to Pipeline Profile
   * name, ref to vector of pipe IDs>
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfRtInfoPipelineInfoGet(
      PipelineProfInfoVec *pipe_info) const = 0;

  /**
   * @brief Get the path of the BfRtInfo json file
   *
   * @param[out] file_name_vec A vector of string references to absolute paths.
   * This returns the paths of all the bf-rt.json files including the fixed json
   * files. If the program name is $SHARED, then it returns only the fixed
   * files,else it returns all the file names with the p4 bf-rt.json file
   * at the end of the vector
   *
   * @return Status of the API call
   */
  virtual bf_status_t bfRtInfoFilePathGet(
      std::vector<std::reference_wrapper<const std::string>> *file_name_vec)
      const = 0;

  /**
   * @brief Get the file paths of the context.jsons associated with
   * this program
   *
   * @param[out] file_name_vec A vector of string references to paths
   *
   * @return Status of the API call
   */
  virtual bf_status_t contextFilePathGet(
      std::vector<std::reference_wrapper<const std::string>> *file_name_vec)
      const = 0;

  /**
   * @brief Get the file paths of the binaries associated with
   * this program
   *
   * @param[out] file_name_vec A vector of string references to paths
   *
   * @return Status of the API call
   */
  virtual bf_status_t binaryFilePathGet(
      std::vector<std::reference_wrapper<const std::string>> *file_name_vec)
      const = 0;
};

}  // namespace bfrt

#endif
