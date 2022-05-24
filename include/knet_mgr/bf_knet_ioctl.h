/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _BF_KNET_IOCTL_H_
#define _BF_KNET_IOCTL_H_

#include <linux/if.h>

#define SIOCBFKNETCMD SIOCDEVPRIVATE
#define BF_KNET_NETDEV_NAME_MAX IFNAMSIZ
#define BF_KNET_FILTER_BYTES_MAX 64
#define BF_KNET_DATA_BYTES_MAX 64

typedef uint64_t bf_knet_cpuif_t;
typedef uint64_t bf_knet_hostif_t;
typedef uint64_t bf_knet_filter_t;
typedef int bf_knet_msg_len_t;
typedef int bf_knet_priority_t;
typedef uint8_t bf_knet_filter_buffer_t;
typedef uint8_t bf_knet_filter_mask_t;
typedef uint8_t bf_knet_data_size_t;
typedef uint8_t bf_knet_data_buffer_t;
typedef uint16_t bf_knet_data_offset_t;
typedef uint16_t bf_knet_action_count_t;
typedef uint16_t bf_knet_count_t;

typedef enum bf_knet_msg_type_s {
  BF_KNET_M_CPUIF_NDEV_ADD = 1,
  BF_KNET_M_CPUIF_NDEV_DELETE = 2,
  BF_KNET_M_HOSTIF_KNDEV_ADD = 3,
  BF_KNET_M_HOSTIF_KNDEV_DELETE = 4,
  BF_KNET_M_RX_FILTER_ADD = 5,
  BF_KNET_M_RX_FILTER_DELETE = 6,
  BF_KNET_M_TX_ACTION_ADD = 7,
  BF_KNET_M_TX_ACTION_DELETE = 8,
  /* Retrieve #cpuif/hostif netdevs,rx filters per cpuif,
  mutations per rx filter/tx action */
  BF_KNET_M_OBJ_CNT_GET = 9,
  BF_KNET_M_CPUIF_LIST_GET = 10,
  BF_KNET_M_HOSTIF_LIST_GET = 11,
  BF_KNET_M_RX_FILTER_LIST_GET = 12,
  BF_KNET_M_RX_FILTER_GET = 13,
  BF_KNET_M_TX_ACTION_GET = 14,
  BF_KNET_M_IS_MOD_INIT = 15,
  // BF_KNET_M_KNET_INFO_GET = 16,
} bf_knet_msg_type_t;

typedef enum bf_knet_obj_type_s {
  BF_KNET_O_CPUIF = 0,
  BF_KNET_O_HOSTIF = 1,
  BF_KNET_O_RX_FILTER = 2,
  BF_KNET_O_RX_MUTATION = 3,
  BF_KNET_O_TX_MUTATION = 4,
} bf_knet_obj_type_t;

typedef enum bf_knet_filter_dest_s {

  /* Send packet to cpuif_knetdev */
  BF_KNET_FILTER_DESTINATION_CPUIF,

  /* Send packet to hostif_knetdev */
  BF_KNET_FILTER_DESTINATION_HOSTIF,

  /* Drop the packet */
  BF_KNET_FILTER_DESTINATION_NULL,
} bf_knet_filter_dest_t;

typedef enum bf_knet_status_s {
  /* No error/success */
  BF_KNET_E_NONE,

  /* To be removed */
  BF_KNET_E_NOT_FOUND,

  BF_KNET_E_CPUIF_NOT_FOUND,

  BF_KNET_E_HOSTIF_NOT_FOUND,

  BF_KNET_E_RX_FILTER_NOT_FOUND,

  BF_KNET_E_TX_ACTION_NOT_FOUND,

  /* Error in accessing user passed pointer*/
  BF_KNET_E_MEM_ACCESS,

  /* Netdev name invalid*/
  BF_KNET_E_NAME_INVALID,

  /* Module failed to allocate resource such as memory */
  BF_KNET_E_RESOURCE,

  /* One of the passed arguments has a value which in not valid */
  BF_KNET_E_PARAM,

  /* Invalid message type */
  BF_KNET_E_MSG_TYPE,

  /* Invalid object type */
  BF_KNET_E_OBJ_TYPE,
} bf_knet_status_t;

typedef struct bf_knet_msg_hdr_s {
  /* One of the message types BF_KNET_M_XXX */
  bf_knet_msg_type_t type;

  bf_knet_msg_len_t len;

  /* cpuif_netdev ID (uint64_t) */
  bf_knet_cpuif_t knet_cpuif_id;

  /* Indicates result of IOCTL command */
  bf_knet_status_t status;

  union {
    /* hostif_knetdev ID (unit64_t)*/
    bf_knet_hostif_t hostif_id;

    /* RX filter ID (uint64_t)*/
    bf_knet_filter_t filter_id;
  } id;
} bf_knet_msg_hdr_t;

typedef struct bf_knet_msg_is_mod_init_s {
  /* message type is BF_KNET_M_IS_MOD_INIT */
  bf_knet_msg_hdr_t hdr;

} bf_knet_msg_is_mod_init_t;

typedef struct bf_knet_msg_cpuif_ndev_add_s {
  /* message type is BF_KNET_M_CPUIF_NDEV_ADD */
  bf_knet_msg_hdr_t hdr;

  /* Name of cpuif_netdev to add to bf_knet */
  char cpuif_netdev_name[BF_KNET_NETDEV_NAME_MAX];

  /* cpuif_netdev ID allocated by bf_knet */
  bf_knet_cpuif_t knet_cpuif_id;

  /* cpuif_knetdev name assigned by bf_knet */
  char cpuif_knetdev_name[BF_KNET_NETDEV_NAME_MAX];
} bf_knet_msg_cpuif_ndev_add_t;

typedef struct bf_knet_msg_cpuif_ndev_delete_s {
  /* message type is BF_KNET_M_CPUIF_NDEV_DELETE */
  bf_knet_msg_hdr_t hdr;
} bf_knet_msg_cpuif_ndev_delete_t;

typedef struct bf_knet_hostif_knetdev_s {
  /* hostif_knetdev ID allocated by bf_knet*/
  bf_knet_hostif_t knet_hostif_id;

  char name[BF_KNET_NETDEV_NAME_MAX];
} bf_knet_hostif_knetdev_t;

typedef struct bf_knet_msg_hostif_knetdev_add_s {
  /* message type is BF_KNET_M_HOSTIF_KNDEV_ADD */
  bf_knet_msg_hdr_t hdr;

  /* Structure that defines hostif_knetdev attributes */
  bf_knet_hostif_knetdev_t hostif_knetdev;
} bf_knet_msg_hostif_kndev_add_t;

typedef struct bf_knet_msg_hostif_kndev_delete_s {
  /* message type is BF_KNET_M_HOSTIF_KNDEV_DELETE */
  bf_knet_msg_hdr_t hdr;
} bf_knet_msg_hostif_kndev_delete_t;

typedef struct bf_knet_rx_filter_spec_s {
  /* Filter ID (uint64_t) */
  bf_knet_filter_t filter_id;

  /* Filter priority (int). Lower value higher priority */
  bf_knet_priority_t priority;

  /* array of type uint8_t for filter */
  bf_knet_filter_buffer_t filter[BF_KNET_FILTER_BYTES_MAX];

  /* array of type uint8_t for mask */
  bf_knet_filter_mask_t mask[BF_KNET_FILTER_BYTES_MAX];

  /* Number of bytes (from start) to use for filtering */
  uint8_t filter_size;
} bf_knet_rx_filter_spec_t;

typedef enum bf_knet_mutation_type_s {
  /* TODO: Remove Rx from enum names */
  /* No packet modifications */
  BF_KNET_RX_MUT_NONE = 0,
  /* Insert data */
  BF_KNET_RX_MUT_INSERT = 1,
  /* Strip data */
  BF_KNET_RX_MUT_STRIP = 2,
} bf_knet_mutation_type_t;

typedef struct bf_knet_packet_mutation_s {
  /* Insert or strip action , one of the types
  BF_KNET_RX_MUT_XXX */
  bf_knet_mutation_type_t mutation_type;

  /* offset for stripping/inserting data (uint8_t) */
  bf_knet_data_offset_t offset;

  /* data strip/insert length (uint8_t) */
  bf_knet_data_size_t len;

  /* data to insert if action type is BF_KNET_RX_MUT_INSERT
  (array of uint8_t) */
  bf_knet_data_buffer_t data[BF_KNET_DATA_BYTES_MAX];
} bf_knet_packet_mutation_t;

typedef struct bf_knet_rx_filter_action_s {
  /* Destination for matching packets
  BF_FILTER_DESTINATION_XXX */
  bf_knet_filter_dest_t dest_type;

  /* Destination netdev when dest type is
  BF_FILTER_DESTINATION_HOSTIF */
  bf_knet_hostif_t knet_hostif_id;

  /* Overwrites skb->proto field if set to non zero */
  int dest_proto;

  /* Size of dynamically allocated packet mutations array*/
  bf_knet_action_count_t count;

  /* Pointer to an array of packet mutations*/
  bf_knet_packet_mutation_t *pkt_mutation;
} bf_knet_rx_filter_action_t;

typedef struct bf_knet_rx_filter_s {
  bf_knet_rx_filter_spec_t spec;
  bf_knet_rx_filter_action_t action;
} bf_knet_rx_filter_t;

typedef struct bf_knet_msg_rx_filter_add_s {
  /* message type is BF_KNET_M_RX_FILTER_ADD */
  bf_knet_msg_hdr_t hdr;

  /* Structure for RX Filter information */
  bf_knet_rx_filter_t rx_filter;
} bf_knet_msg_rx_filter_add_t;

typedef struct bf_knet_msg_rx_filter_delete_s {
  /* message type is BF_KNET_M_HOSTIF_KNDEV_DELETE */
  bf_knet_msg_hdr_t hdr;
} bf_knet_msg_rx_filter_delete_t;

typedef struct bf_knet_tx_action_s {
  bf_knet_action_count_t count;
  bf_knet_packet_mutation_t *pkt_mutation;
} bf_knet_tx_action_t;

typedef struct bf_knet_msg_tx_action_add_s {
  /* message type is BF_KNET_M_TX_ACTION_ADD */
  bf_knet_msg_hdr_t hdr;

  /*Structure for Tx Action Information */
  bf_knet_tx_action_t tx_action;
} bf_knet_msg_tx_action_add_t;

typedef struct bf_knet_msg_tx_action_delete_s {
  bf_knet_msg_hdr_t hdr;
} bf_knet_msg_tx_action_delete_t;

/* Get the #objects (rx_filters,cpuif/hostif netdevs,rx filter mutations,
tx action mutatino) */
typedef struct bf_knet_msg_obj_cnt_get_s {
  bf_knet_msg_hdr_t hdr;
  bf_knet_obj_type_t type;
  bf_knet_count_t obj_count;
} bf_knet_msg_obj_cnt_get_t;

typedef struct bf_knet_cpuif_list_s {
  bf_knet_cpuif_t id;
  char name[IFNAMSIZ];
} bf_knet_cpuif_list_t;

typedef struct bf_knet_msg_cpuif_list_get_s {
  bf_knet_msg_hdr_t hdr;
  /* #elements in the cpuif list */
  bf_knet_count_t size;
  bf_knet_cpuif_list_t *cpuif_list;
} bf_knet_msg_cpuif_list_get_t;

typedef struct bf_knet_hostif_list_s {
  bf_knet_hostif_t id;
  char name[IFNAMSIZ];
} bf_knet_hostif_list_t;

typedef struct bf_knet_msg_hostif_list_get_s {
  bf_knet_msg_hdr_t hdr;
  /* #elements in the cpuif list */
  bf_knet_count_t size;
  bf_knet_hostif_list_t *hostif_list;
} bf_knet_msg_hostif_list_get_t;

typedef struct bf_knet_msg_rx_filter_get_s {
  bf_knet_msg_hdr_t hdr;
  /* User must allocate enough memory to hold the array of packet mutations.
  If the user given array is not large enough BF_KNET_E_RESOURCE error is
  returned.
  The size of array needed to hold the action is returned to the user in tx
  action
  cound field. */

  /* Size of rx_filter action mutation array (memory allocated by user)
  expressed in terms of Count. Can be retireved via obj_cnt_get msg */
  bf_knet_count_t mutation_count;

  bf_knet_rx_filter_t rx_filter;
} bf_knet_msg_rx_filter_get_t;

typedef struct bf_knet_msg_rx_filter_list_get_s {
  bf_knet_msg_hdr_t hdr;
  /* Size of user allocated allocated array filter_list */
  bf_knet_count_t size;
  bf_knet_filter_t *filter_list;
} bf_knet_msg_rx_filter_list_get_t;

typedef struct bf_knet_msg_tx_action_get_s {
  bf_knet_msg_hdr_t hdr;
  /* Size of tx action mutation array (memory allocated by user)
  expressed in terms of Count. Can be retireved via obj_cnt_get msg */
  bf_knet_count_t mutation_count;
  bf_knet_tx_action_t tx_action;
} bf_knet_msg_tx_action_get_t;

typedef union bf_knet_msg_s {
  /* Common message header shared by all messages */
  bf_knet_msg_hdr_t hdr;

  /* Add cpuif_ndev to bf_knet */
  bf_knet_msg_cpuif_ndev_add_t cpuif_ndev_add;

  /* Delete cpuif_ndev from bf_knet */
  bf_knet_msg_cpuif_ndev_delete_t cpuif_ndev_delete;

  /* Add hostif_kndev for cpuif_ndev*/
  bf_knet_msg_hostif_kndev_add_t hostif_kndev_add;

  /* Delete hostif_kndev for cpuif_ndev */
  bf_knet_msg_hostif_kndev_delete_t hostif_kndev_delete;

  /* Add Rx Filter for cpuif_ndev */
  bf_knet_msg_rx_filter_add_t rx_filter_add;

  /* Delete Rx Filter for cpuif_ndev*/
  bf_knet_msg_rx_filter_delete_t rx_filter_delete;

  /* Add Tx action for hostif_kndev */
  bf_knet_msg_tx_action_add_t tx_action_add;

  /* Delete Tx action for hostif_kndev */
  bf_knet_msg_tx_action_delete_t tx_action_delete;

  /* Get #Mutations for a Rx Filter */
  bf_knet_msg_obj_cnt_get_t obj_cnt_get;

  bf_knet_msg_cpuif_list_get_t cpuif_list_get;

  bf_knet_msg_hostif_list_get_t hostif_list_get;

  /* Get Rx Filter Info for cpuif_ndev */
  bf_knet_msg_rx_filter_get_t rx_filter_get;

  /* Get a list of Rx filter handles */
  bf_knet_msg_rx_filter_list_get_t rx_filter_list_get;

  bf_knet_msg_tx_action_get_t tx_action_get;

  bf_knet_msg_is_mod_init_t is_mod_init;
#if 0
	/* Get KNET specific info such as max filter size*/
	bf_knet_msg_get_info_t knet_info;
#endif
} bf_knet_msg_t;

#endif /* _BF_KNET_IOCLT_H_ */
