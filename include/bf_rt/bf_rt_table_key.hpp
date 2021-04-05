/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_table_key.hpp
 *
 *  @brief Contains BF-RT Table Key APIs
 */
#ifndef _BF_RT_TABLE_KEY_HPP
#define _BF_RT_TABLE_KEY_HPP

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
 * @brief Key Field Type. A key can have multiple fields, each with a different
 * type
 */
enum class KeyFieldType {
  INVALID = 0,
  EXACT = 1,
  TERNARY = 2,
  RANGE = 3,
  LPM = 4,
};

/**
 * @brief Class to construct key for a table.<br>
 * <B>Creation: </B> Can only be created using \ref
 * bfrt::BfRtTable::keyAllocate()
 */
class BfRtTableKey {
 public:
  virtual ~BfRtTableKey() = default;

  /**
   * @name Set APIs
   * @{
   */
  /**
   * @brief Set value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
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
   * @brief Set value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
   *
   * @param[in] field_id Field ID
   * @param[in] value Start address of the input Byte-array. The input
   * needs to be in network order.
   * @param[in] size  Number of bytes of the byte-array. The input
   * number of bytes should ceil the size of the field which is in bits.
   * For example, if field size is 28 bits, then size should be 4.
   * if input = 0xdedbeef, then the input expected is 0x0dedbeef
   * and size=4 when using this API
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const uint8_t *value,
                               const size_t &size) = 0;

  /**
   * @brief Set value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
   *                   and DataType "String"
   * @param[in] field_id Field ID
   * @param[in] value    String
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValue(const bf_rt_id_t &field_id,
                               const std::string &value) = 0;

  /**
   * @brief Set value. Only valid on fields of \ref bfrt::KeyFieldType "TERNARY"
   *
   * @param[in] field_id Field ID
   * @param[in] value   Value. Cannot be greater in value than what the field
   * size allows. For example, if the field size is 3 bits, passing in a value
   * of 14 will throw an error even though uint64_t can easily hold 14.
   * @param[in] mask    Mask. Same bounds length conditions as value apply
   * here too
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueandMask(const bf_rt_id_t &field_id,
                                      const uint64_t &value,
                                      const uint64_t &mask) = 0;

  /**
   * @brief Set value. Only valid on fields of \ref bfrt::KeyFieldType "TERNARY"
   *
   * @param[in] field_id Field ID
   * @param[in] value Start address of the input Byte-array Value.
   * The input needs to be in network order.
   * @param[in] mask Start address of the input Byte-array mask.
   * The input needs to be in network order.
   * @param[in] size  Number of bytes of the byte-arrays. The size of the value
   * and mask array are always considered to be of equal length
   * The input number of bytes should ceil the size of the field which is in
   * bits. For example, if field size is 28 bits, then size should be 4.
   * if input = 0xdedbeef, then the input expected is 0x0dedbeef
   * and size=4 when using this API
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueandMask(const bf_rt_id_t &field_id,
                                      const uint8_t *value,
                                      const uint8_t *mask,
                                      const size_t &size) = 0;

  /**
   * @brief Set Value. Only valid on fields of \ref bfrt::KeyFieldType "RANGE"
   *
   * @param[in] field_id Field ID
   * @param[in] start Range start value
   * @param[in] end Range end value
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueRange(const bf_rt_id_t &field_id,
                                    const uint64_t &start,
                                    const uint64_t &end) = 0;

  /**
   * @brief Set Value. Only valid on fields of \ref bfrt::KeyFieldType "RANGE"
   *
   * @param[in] field_id Field ID
   * @param[in] start Byte-array for range start value in network order
   * @param[in] end Byte-array for range end value in network order
   * @param[in] size Number of bytes of the byte-arrays. The size of the start
   * and end array are always considered to be of equal length
   * The input number of bytes should ceil the size of the field which is in
   * bits. For example, if field size is 28 bits, then size should be 4.
   * if input = 0xdedbeef, then the input expected is 0x0dedbeef
   * and size=4 when using this API
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueRange(const bf_rt_id_t &field_id,
                                    const uint8_t *start,
                                    const uint8_t *end,
                                    const size_t &size) = 0;

  /**
   * @brief Set Value. Only valid on fields of \ref bfrt::KeyFieldType "LPM"
   *
   * @param[in] field_id Field ID
   * @param[in] value  Value
   * @param[in] p_length Prefix-length. The value of the prefix length
   * cannot be greater than the value of size of the field in bits.
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueLpm(const bf_rt_id_t &field_id,
                                  const uint64_t &value,
                                  const uint16_t &p_length) = 0;

  /**
   * @brief Set Value. Only valid on fields of \ref bfrt::KeyFieldType "LPM"
   *
   * @param[in] field_id Field ID
   * @param[in] value  Byte-array for value in network order.
   * @param[in] p_length Prefix-length. The value of the prefix length
   * cannot be greater than the value of size of the field in bits.
   * @param[in] size Number of bytes of the byte-array.
   * The input number of bytes should ceil the size of the field which is in
   * bits. For example, if field size is 28 bits, then size should be 4.
   * if input = 0xdedbeef, then the input expected is 0x0dedbeef,  size=4
   * and 0<=prefix_len<=28 when using this API.
   *
   * @return Status of the API call
   */
  virtual bf_status_t setValueLpm(const bf_rt_id_t &field_id,
                                  const uint8_t *value,
                                  const uint16_t &p_length,
                                  const size_t &size) = 0;

  /** @} */  // End of group Set APIs

  /**
   * @name Get APIs
   * @{
   */
  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
   *
   * @param[in] field_id Field ID
   * @param[out] value   Value.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               uint64_t *value) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
   *
   * @param[in] field_id  Field ID
   * @param[in] size      Size of value byte-array
   * @param[out] value    Value byte-array. The output is in network order with
   * byte padded. If a 28-bit field with value 0xdedbeef is being queried, then
   * input size needs to be 4 and the output array will be 0x0dedbeef
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               const size_t &size,
                               uint8_t *value) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "EXACT"
   *                   and DataType "String"
   *
   * @param[in] field_id  Field ID
   * @param[out] value    String.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValue(const bf_rt_id_t &field_id,
                               std::string *value) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "TERNARY"
   *
   * @param[in] field_id  Field ID
   * @param[out] value    Value.
   * @param[out] mask     Mask.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueandMask(const bf_rt_id_t &field_id,
                                      uint64_t *value,
                                      uint64_t *mask) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "TERNARY"
   *
   * @param[in] field_id  Field ID
   * @param[in] size      Size of the value and mask byte-arrays
   * @param[out] value    Value byte-array. The output is in network order with
   * byte padded. If a 28-bit field with value 0xdedbeef is being queried, then
   * input size needs to be 4 and the output array will be 0x0dedbeef
   * @param[out] mask     Mask byte-array. Padding done is similar to value.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueandMask(const bf_rt_id_t &field_id,
                                      const size_t &size,
                                      uint8_t *value,
                                      uint8_t *mask) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "RANGE"
   *
   * @param[in] field_id  Field ID
   * @param[out] start    Range start value
   * @param[out] end      Range end value
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueRange(const bf_rt_id_t &field_id,
                                    uint64_t *start,
                                    uint64_t *end) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "RANGE"
   *
   * @param[in] field_id  Field ID
   * @param[in] size      Size of the start and end byte-arrays
   * @param[out] start    Range start value byte-array. The output is in network
   * order with byte padding. If a 28-bit field with value 0xdedbeef is being
   * queried, then input size needs to be 4 and the output array will be
   * 0x0dedbeef
   * @param[out] end      Range end value byte-array. Padding done is similar to
   * value.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueRange(const bf_rt_id_t &field_id,
                                    const size_t &size,
                                    uint8_t *start,
                                    uint8_t *end) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "LPM"
   *
   * @param[in] field_id  Field ID
   * @param[out] value    Value.
   * @param[out] p_length Prefix-length.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueLpm(const bf_rt_id_t &field_id,
                                  uint64_t *value,
                                  uint16_t *p_length) const = 0;

  /**
   * @brief Get value. Only valid on fields of \ref bfrt::KeyFieldType "LPM"
   *
   * @param[in] field_id  Field ID
   * @param[in] size      Size of the value byte-array
   * @param[out] value    Value byte-array. The output is in network order with
   * byte padded. If a 28-bit field with value 0xdedbeef is being queried, then
   * input size needs to be 4 and the output array will be 0x0dedbeef
   * @param[out] p_length Prefix-length.
   *
   * @return Status of the API call
   */
  virtual bf_status_t getValueLpm(const bf_rt_id_t &field_id,
                                  const size_t &size,
                                  uint8_t *value,
                                  uint16_t *p_length) const = 0;

  /** @} */  // End of group Get APIs

  /**
   * @brief Get the BfRtTable Object associated with this Key Object
   *
   * @param[out] table BfRtTable Object
   *
   * @return Status of the API call
   */
  virtual bf_status_t tableGet(const BfRtTable **table) const = 0;
};

}  // bfrt

#endif  // _BF_RT_TABLE_KEY_HPP
