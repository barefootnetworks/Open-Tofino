/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_table_data.hpp
 *
 *  @brief Contains BF-RT Table Data APIs
 */

#ifndef _BF_RT_TABLE_DATA_HPP
#define _BF_RT_TABLE_DATA_HPP

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <memory>
#include <set>

#include <bf_rt/bf_rt_common.h>

namespace bfrt {

// Forward declarations
class BfRtLearn;
class BfRtTable;

/**
 * @brief Enum of Data Types each Data field can take. This is NOT
 *    Data field Type
 */
enum class DataType {
  INT_ARR = 0,
  BOOL_ARR = 1,
  UINT64 = 2,
  BYTE_STREAM = 3,
  FLOAT = 4,
  CONTAINER = 5,
  STRING = 6,
  BOOL = 7,
  STRING_ARR = 8,
};

/**
 * @brief Class to construct data for a table.<br>
 * <B>Creation: </B> Can only be created using one of \ref
 * bfrt::BfRtTable::dataAllocate() (as applicable) <br>
 */

class BfRtTableData {
 public:
  virtual ~BfRtTableData() = default;

  /**
   * @name Set APIs
   * @{
   */
  /**
   * @brief Set value. Only valid on fields of size <= 64 bits
   *
   * @param[in] field_id Field ID
   * @param[in] value   Value. Cannot be greater in value than what the field
   * size allows. For example, if the field size is 3 bits, passing in a value
   * of 14 will throw an error even though uint64_t can easily hold 14.
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const uint64_t &value) = 0;

  /**
   * @brief Set value. Valid on fields of all sizes
   *
   * @param[in] field_id Field ID
   * @param[in] value Start address of the input Byte-array. The input
   * needs to be in network order with byte padding at the MSBs.
   * @param[in] size  Number of bytes of the byte-array. The input
   * number of bytes should ceil the size of the field which is in bits.
   * For example, if field size is 28 bits, then size should be 4.
   * if input = 0xdedbef0, then the input expected is 0x0dedbeef
   * and size=4 when using this API
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const uint8_t *value,
                               const size_t &size) = 0;

  /**
   * @brief Set value. Valid only on fields with integer array type
   *
   * @param[in] field_id Field ID
   * @param[in] arr A vector representing the integer array
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const std::vector<bf_rt_id_t> &arr) = 0;

  /**
   * @brief Set value. Valid only on fields with bool array type
   *
   * @param[in] field_id Field ID
   * @param[in] arr A vector representing the bool array
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const std::vector<bool> &arr) = 0;

  /**
   * @brief Set value. Valid only on fields with string array type
   *
   * @param[in] field_id Field ID
   * @param[in] arr A vector representing the string array
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const std::vector<std::string> &arr) = 0;

  /**
   * @brief Set value. Valid only on fields with float type
   *
   * @param[in] field_id Field ID
   * @param[in] value Float value
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const float &value) = 0;

  /**
   * @brief Set value. Valid only on fields with bool type
   *
   * @param[in] field_id Field ID
   * @param[in] value Bool value
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const bool &value) = 0;

  /**
   * @brief Set value. Valid only on fields with string type
   *
   * @param[in] field_id Field ID
   * @param[in] value String value
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const std::string &str) = 0;

  /** @} */  // End of group Set APIs

  /**
   * @name Get APIs
   * @{
   */
  /**
   * @brief Get value. Only valid on fields of size <= 64 bits
   *
   * @param[in] field_id Field ID
   * @param[out] value Pointer to the value to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               uint64_t *value) const = 0;

  /**
   * @brief Get value. Valid on fields of all sizes
   *
   * @param[in] field_id Field ID
   * @param[in] size Size of the byte array passed in. The size has to be equal
   *to the byte-padded<br>
   *                 size of the field. For eg: field size of 12 bits needs the
   *size to be passed in as 2
   * @param[out] value Pointer to the value to be filled in. The array is
   *assumed to be able to hold "size" number of bytes
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               const size_t &size,
                               uint8_t *value) const = 0;

  /**
   * @brief Get value. Valid on fields of integer array type
   *
   * @param[in] field_id Field ID
   * @param[out] arr Pointer to the vector to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::vector<bf_rt_id_t> *arr) const = 0;

  /**
   * @brief Get value. Valid on fields of bool array type
   *
   * @param[in] field_id Field ID
   * @param[out] arr Pointer to the vector to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::vector<bool> *arr) const = 0;

  /**
   * @brief Get value. Valid on fields of string array type
   *
   * @param[in] field_id Field ID
   * @param[out] arr Pointer to the vector to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::vector<std::string> *arr) const = 0;

  /**
   * @brief Get value. Valid on fields of float type
   *
   * @param[in] field_id Field ID
   * @param[out] value Pointer to the float value to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               float *value) const = 0;

  /**
   * @brief Get value. Valid on fields of bool type
   *
   * @param[in] field_id Field ID
   * @param[out] value Pointer to the bool value to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               bool *value) const = 0;

  /**
   * @brief Get value. Valid on fields of uint64_t array type
   *
   * @param[in] field_id Field ID
   * @param[out] arr Pointer to the vector value to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::vector<uint64_t> *arr) const = 0;
  /**
   * @brief Get value. Valid on fields of BfRtTableData type
   *
   * @param[in] field_id Field ID
   * @param[out] Pointer To the list of BfRtTableData values to be filled in.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::vector<BfRtTableData *> *ret_vec) const = 0;

  /**
   * @brief Get value. Valid on fields of string type
   *
   * @param[in] field_id Field ID
   * @param[out] String Value
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::string *str) const = 0;

  /** @} */  // End of group Get APIs
             /**
              * @brief Get actionId.
              *
              * @param[out] act_id Pointer to the action id to be filled in. Applicable for
              *data<br>
              *             objects associated with tables for which action id is
              *applicable
              *
              * @return Status of the API call
              */
  virtual bf_status_t actionIdGet(bf_rt_id_t *act_id) const = 0;

  /**
   * @brief Get parent Table object.
   *
   * @param[out] table Pointer to the pointer to BfRtTable to be filled in. <br>
   *             A table data object can be associated with a parent table
   *object. This API <br>
   *             returns the parent object if it exists.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getParent(const BfRtTable **table) const = 0;
  /**
   * @brief Get parent Learn object.
   *
   * @param[out] learn Pointer to the pointer to BfRtLearn to be filled in. <br>
   *             A table data object can be associated with a parent Learn
   *object. This API <br>
   *             returns the parent learn object if it exists.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getParent(const BfRtLearn **learn) const = 0;
  /**
   * @brief Get whether the field is active or not. If the data object was
   * allocated with a certain set of fields, then only those fields are
   * considered active.
   * For oneof fields
   * 1. If allocation was done for all fields, then by default all oneof fields
   * are active in the beginning. The fields set with setValue will remain on
   * and other oneofs in their oneof-group will be turned off during setValue()
   * 2. If allocation was done for specific fields, then
   *  a. If all oneofs in a group were set, then behavior is just like above
   *  b. If only one of the oneofs were set, then only that field is expected
   *  to be set during setValue().
   *
   * @param[in] field_id Field ID
   * @param[out] is_active Bool value indicating if a field is active
   *
   * @return Status of the API call
   */
  virtual bf_status_t isActive(const bf_rt_id_t &field_id,
                               bool *is_active) const = 0;
};

}  // bfrt

#endif  // _BF_RT_TABLE_DATA_HPP
