/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_table_key.hpp
 *
 *  @brief Contains BF-RT Table Key APIs
 */
#ifndef _BF_RT_TABLE_KEY_H
#define _BF_RT_TABLE_KEY_H

#include <bf_rt/bf_rt_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Key Field Type. A key can have multiple fields, each with a different
 * type
 */
typedef enum bf_rt_key_field_type_ {
  INVALID = 0,
  EXACT = 1,
  TERNARY = 2,
  RANGE = 3,
  LPM = 4,
} bf_rt_key_field_type_t;

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *
 * @param[in] key_hdl           Key object handle
 * @param[in] field_id          Field ID
 * @param[in] value             Value to set. Cannot be greater in value than
 *                              what the field size allows. For example, if
 *                              the field size is 3 bits, passing in a value
 *                              of 14 will throw an error even though uint64_t
 *                              can easily hold 14.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value(bf_rt_table_key_hdl *key_hdl,
                                      const bf_rt_id_t field_id,
                                      uint64_t value);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *
 * @param[in] key_hdl           Key object handle
 * @param[in] field_id          Field ID
 * @param[in] value             Start address of the input Byte-array. The input
 *                              needs to be in network order.
 * @param[in] size              Number of bytes of the byte-array. The input
 *                              value should ceil the size of the field which
 *                              is in bits. For example, if field size isi
 *                              28 bits, then size should be 4.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_ptr(bf_rt_table_key_hdl *key_hdl,
                                          const bf_rt_id_t field_id,
                                          const uint8_t *value,
                                          const size_t size);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *                   and DataType "String"
 *
 * @param[in] key_hdl           Key object handle
 * @param[in] field_id          Field ID
 * @param[in] value             String, must be null terminated
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_string(bf_rt_table_key_hdl *key_hdl,
                                             const bf_rt_id_t field_id,
                                             const char *value);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t
 * "TERNARY"
 *
 * @param[in] key_hdl    Key object handle
 * @param[in] field_id   Field ID
 * @param[in] value      Cannot be greater in value than what the field size
 *                       allows. For example, if the field size is 3 bits,
 *                       passing in a value of 14 will throw an error even
 *                       though uint64_t can easily hold 14.
 * @param[in] mask       Same bounds length conditions as value apply.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_and_mask(bf_rt_table_key_hdl *key_hdl,
                                               const bf_rt_id_t field_id,
                                               uint64_t value,
                                               uint64_t mask);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t
 * "TERNARY"
 *
 * @param[in] key_hdl    Key object handle
 * @param[in] field_id   Field ID
 * @param[in] value      Start address of the input Byte-array value.
 *                       The input needs to be in network order.
 * @param[in] mask       Start address of the input Byte-array mask.
 *                       The input needs to be in network order.
 * @param[in] size       Number of bytes of the byte-arrays. The size of
 *                       the value and mask array are always considered to bei
 *                       of equal length. The input number of bytes should ceil
 *                       the size of the field which is in bits. For example,
 *                       if field size is 28 bits, then size should be 4.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_and_mask_ptr(bf_rt_table_key_hdl *key_hdl,
                                                   const bf_rt_id_t field_id,
                                                   const uint8_t *value1,
                                                   const uint8_t *mask,
                                                   const size_t size);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "RANGE"
 *
 * @param[in] key_hdl           Key object handle
 * @param[in] field_id          Field ID
 * @param[in] start             Range start value
 * @param[in] end               Range end value
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_range(bf_rt_table_key_hdl *key_hdl,
                                            const bf_rt_id_t field_id,
                                            const uint64_t start,
                                            const uint64_t end);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "RANGE"
 *
 * @param[in] key_hdl    Key object handle
 * @param[in] field_id   Field ID
 * @param[in] start      Byte-array for range start value in network order
 * @param[in] end        Byte-array for range end value in network order
 * @param[in] size       Number of bytes of the byte-arrays. The size of the
 *                       start and end array are always considered to be of
 *                       equal length. The input number of bytes should ceil
 *                       the size of the field which is in bits. For example,
 *                       if field size is 28 bits, then size should be 4.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_range_ptr(bf_rt_table_key_hdl *key_hdl,
                                                const bf_rt_id_t field_id,
                                                const uint8_t *start,
                                                const uint8_t *end,
                                                const size_t size);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "LPM"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] value1      Value
 * @param[in] p_length    The value of the prefix length cannot be greater than
 *                        the value of size of the field in bits.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_lpm(bf_rt_table_key_hdl *key_hdl,
                                          const bf_rt_id_t field_id,
                                          const uint64_t value1,
                                          const uint16_t p_length);

/**
 * @brief Set value. Only valid on fields of \ref bf_rt_key_field_type_t "LPM"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] value1      Byte-array for value in network order.
 * @param[in] p_length    Prefix-length. The value of the prefix length cannot
 *                        be greater than the value of size of the field in
 *                        bits.
 * @param[in] size        Number of bytes of the byte-array. The input number
 *                        of bytes should ceil the size of the field which is in
 *                        bits. For example, if field size is 28 bits, then size
 *                        should be 4.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_set_value_lpm_ptr(bf_rt_table_key_hdl *key_hdl,
                                              const bf_rt_id_t field_id,
                                              const uint8_t *value1,
                                              const uint16_t p_length,
                                              const size_t size);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] value      Value
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value(const bf_rt_table_key_hdl *key_hdl,
                                      const bf_rt_id_t field_id,
                                      uint64_t *value);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] size        Size of value byte-array
 * @param[out] value      Value byte-array. The output is in network order with
 *                        byte padded. If a 28-bit field with value 0xdedbeef is
 *                        being queried, then input size needs to be 4 and the
 *                        output array will be set to 0x0dedbeef.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_ptr(const bf_rt_table_key_hdl *key_hdl,
                                          const bf_rt_id_t field_id,
                                          const size_t size,
                                          uint8_t *value);

/**
 * @brief Get string key value size.
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] str_size   Minimum size of buffer required to get value of
 *                        specified field.
 *
 * @return Status of the API call
 */

bf_status_t bf_rt_key_field_get_value_string_size(
    const bf_rt_table_key_hdl *key_hdl,
    const bf_rt_id_t field_id,
    uint32_t *str_size);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "EXACT"
 *                   and DataType "String"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] value      String.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_string(const bf_rt_table_key_hdl *key_hdl,
                                             const bf_rt_id_t field_id,
                                             char *value);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t
 * "TERNARY"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] value1     Value.
 * @param[out] value2     Mask.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_and_mask(
    const bf_rt_table_key_hdl *key_hdl,
    const bf_rt_id_t field_id,
    uint64_t *value1,
    uint64_t *value2);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t
 * "TERNARY"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] size        Size of the value and mask byte-arrays
 * @param[out] value1     Value byte-array. The output is in network order with
 *                        byte padded. If a 28-bit field with value 0xdedbeef is
 *                        being queried, then input size needs to be 4 and the
 *                        output array will be set to 0x0dedbeef
 * @param[out] value2     Mask byte-array. Padding done is similar to value.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_and_mask_ptr(
    const bf_rt_table_key_hdl *key_hdl,
    const bf_rt_id_t field_id,
    const size_t size,
    uint8_t *value1,
    uint8_t *value2);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "RANGE"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] start      Range start value
 * @param[out] end        Range end value
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_range(const bf_rt_table_key_hdl *key_hdl,
                                            const bf_rt_id_t field_id,
                                            uint64_t *start,
                                            uint64_t *end);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "RANGE"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] size        Size of the start and end byte-arrays
 * @param[out] start      Range start value byte-array. The output is in network
 *                        order with byte padding. If a 28-bit field with value
 *                        0xdedbeef is being queried, then input size needs to
 *                        be 4 and the output array will be set to 0x0dedbeef
 * @param[out] end        Range end value byte-array. Padding done is similar to
 *                        start param.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_range_ptr(
    const bf_rt_table_key_hdl *key_hdl,
    const bf_rt_id_t field_id,
    const size_t size,
    uint8_t *start,
    uint8_t *end);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "LPM"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[out] start      Value.
 * @param[out] p_length   Prefix-length.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_lpm(const bf_rt_table_key_hdl *key_hdl,
                                          const bf_rt_id_t field_id,
                                          uint64_t *start,
                                          uint16_t *p_length);

/**
 * @brief Get value. Only valid on fields of \ref bf_rt_key_field_type_t "LPM"
 *
 * @param[in] key_hdl     Key object handle
 * @param[in] field_id    Field ID
 * @param[in] size        Size of the value byte-array
 * @param[out] value1     Value byte-array. The output is in network order with
 *                        byte padded. If a 28-bit field with value 0xdedbeef is
 *                        being queried, then input size needs to be 4 and the
 *                        output array will be set to 0x0dedbeef.
 * @param[out] p_length   Prefix-length.
 *
 * @return Status of the API call
 */
bf_status_t bf_rt_key_field_get_value_lpm_ptr(
    const bf_rt_table_key_hdl *key_hdl,
    const bf_rt_id_t field_id,
    const size_t size,
    uint8_t *value1,
    uint16_t *p_length);

#ifdef __cplusplus
}
#endif

#endif  // _BF_RT_TABLE_KEY_H
