/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _MC_MGR_TYPES_H
#define _MC_MGR_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#include <bf_types/bf_types.h>

/**
 * @file mc_mgr_types.h
 * \brief Details multicast specific types.
 */
/**
 * @addtogroup mc_mgr-mc_mgr
 * @{
 */

/** Corresponds to mcast_grp_a/mcast_grp_b metadata. */
typedef uint16_t bf_mc_grp_id_t;
/** Corresponds to an RID metadata field. */
typedef uint16_t bf_mc_rid_t;
/** Corresponds to level1_exclusion_id metadata. */
typedef uint16_t bf_mc_l1_xid_t;
/** Corresponds to level2_exclusion_id metadata. */
typedef uint16_t bf_mc_l2_xid_t;

/** Handle for a driver session. */
typedef uint32_t bf_mc_session_hdl_t;
/** Handle for a multicast group. */
typedef uint32_t bf_mc_mgrp_hdl_t;
/** Handle for a ECMP group. */
typedef uint32_t bf_mc_ecmp_hdl_t;
/** Handle for a L1 node. */
typedef uint32_t bf_mc_node_hdl_t;
/** The number of bytes required to store a bit map of all port ids. */
#define BF_MC_PORT_ARRAY_SIZE ((BF_PORT_COUNT + 7) / 8)
/** Bitmap representing the ports on an ASIC.  Initialize with
 *  @ref BF_MC_PORT_MAP_INIT and add ports with @ref BF_MC_PORT_MAP_SET. */
typedef uint8_t bf_mc_port_map_t[BF_MC_PORT_ARRAY_SIZE];
/** The number of bytes required to store a bit map of all LAG ids. */
#define BF_MC_LAG_ARRAY_SIZE ((BF_LAG_COUNT + 7) / 8)
/** Bitmap representing the LAGs on an ASIC.  Initialize with
 *  @ref BF_MC_LAG_MAP_INIT and add LAGs with @ref BF_MC_LAG_MAP_SET. */
typedef uint8_t bf_mc_lag_map_t[BF_MC_LAG_ARRAY_SIZE];
/** Corresponds to a LAG group. */
typedef uint8_t bf_mc_lag_id_t;

/** Utility to initialize a @ref bf_mc_port_map_t. A @ref mc_port_map_t must be
 * passed
 *  as @a pm */
#define BF_MC_PORT_MAP_INIT(pm)                                  \
  do {                                                           \
    size_t _MC_i;                                                \
    for (_MC_i = 0; _MC_i < sizeof(bf_mc_port_map_t); ++_MC_i) { \
      (pm)[_MC_i] = 0;                                           \
    }                                                            \
  } while (0);
/** Utility to add a port to a @ref bf_mc_port_map_t.  A @ref mc_port_map_t must
 *  be passed as @a pm and a port number must be passed as @a port_id. Note the
 *  port specified should be of type @ref bf_dev_port_t */
#define BF_MC_PORT_MAP_SET(pm, port_id)      \
  do {                                       \
    bf_dev_port_t _MC_p = (port_id);         \
    bf_mc_port_map_t *_MC_pm = &(pm);        \
    int _MC_bi = DEV_PORT_TO_BIT_IDX(_MC_p); \
    if (_MC_bi >= BF_PORT_COUNT) break;      \
    size_t _MC_i = (_MC_bi) / 8;             \
    int _MC_j = (_MC_bi) % 8;                \
    (*_MC_pm)[_MC_i] |= 1 << _MC_j;          \
  } while (0);
/** Utility to remove a port from a @ref bf_mc_port_map_t.  A @ref mc_port_map_t
 *  must be passed as @a pm and a port number must be passed as @a port_id. Note
 *  the port specified should be of type @ref bf_dev_port_t */
#define BF_MC_PORT_MAP_CLR(pm, port_id)      \
  do {                                       \
    bf_dev_port_t _MC_p = (port_id);         \
    bf_mc_port_map_t *_MC_pm = &(pm);        \
    int _MC_bi = DEV_PORT_TO_BIT_IDX(_MC_p); \
    if (_MC_bi >= BF_PORT_COUNT) break;      \
    size_t _MC_i = (_MC_bi) / 8;             \
    int _MC_j = (_MC_bi) % 8;                \
    (*_MC_pm)[_MC_i] &= ~(1 << _MC_j);       \
  } while (0);
/** Utility to get a port from a @ref bf_mc_port_map_t.  A @ref mc_port_map_t
 * must be passed as @a pm and a port number must be passed as @a port_id. If
 * the port has been set @a val will be set to one.  If the port is not set in
 * the map then @a will be set to zero.  Note the port specified should be of
 * type @ref bf_dev_port_t */
#define BF_MC_PORT_MAP_GET(pm, port_id, val) \
  do {                                       \
    bf_dev_port_t _MC_p = (port_id);         \
    bf_mc_port_map_t *_MC_pm = &(pm);        \
    int _MC_bi = DEV_PORT_TO_BIT_IDX(_MC_p); \
    if (_MC_bi >= BF_PORT_COUNT) break;      \
    size_t _MC_i = (_MC_bi) / 8;             \
    int _MC_j = (_MC_bi) % 8;                \
    val = ((*_MC_pm)[_MC_i] >> _MC_j) & 1;   \
  } while (0);
/** Utility to initialize a bf_mc_lag_map_t. A @c mc_lag_map_t must be passed
 *  as @a lm */
#define BF_MC_LAG_MAP_INIT(lm)                                  \
  do {                                                          \
    size_t _MC_i;                                               \
    for (_MC_i = 0; _MC_i < sizeof(bf_mc_lag_map_t); ++_MC_i) { \
      (lm)[_MC_i] = 0;                                          \
    }                                                           \
  } while (0);
/** Utility to add a lag to a @c bf_mc_lag_map_t.  A @c mc_lag_map_t must be
 *  passed as @a lm and a @c bf_mc_lag_id_t must be passed as @a lag_id. */
#define BF_MC_LAG_MAP_SET(lm, lag_id) \
  do {                                \
    int _MC_p = (lag_id);             \
    bf_mc_lag_map_t *_MC_pm = &(lm);  \
    if (_MC_p >= BF_LAG_COUNT) break; \
    size_t _MC_i = (_MC_p) / 8;       \
    int _MC_j = (_MC_p) % 8;          \
    (*_MC_pm)[_MC_i] |= 1 << _MC_j;   \
  } while (0);
/** Utility to remove a lag to a @c bf_mc_lag_map_t.  A @c mc_lag_map_t must be
 *  passed as @a lm and a @c bf_mc_lag_id_t must be passed as @a lag_id. */
#define BF_MC_LAG_MAP_CLR(lm, lag_id)  \
  do {                                 \
    int _MC_p = (lag_id);              \
    bf_mc_lag_map_t *_MC_pm = &(lm);   \
    if (_MC_p >= BF_LAG_COUNT) break;  \
    size_t _MC_i = (_MC_p) / 8;        \
    int _MC_j = (_MC_p) % 8;           \
    (*_MC_pm)[_MC_i] &= ~(1 << _MC_j); \
  } while (0);
/** Utility to check if a LAG is is set or clear in a @c bf_mc_lag_map_t.  A
 *  @c mc_lag_map_t must be passed as @a lm, a @c bf_mc_lag_id_t must be passed
 *  as @a lag_id.  The argument @a v will be set to one if the LAG is set in
 *  the map and set to zero otherwise. */
#define BF_MC_LAG_MAP_GET(lm, lag_id, v) \
  do {                                   \
    int _MC_p = (lag_id);                \
    bf_mc_lag_map_t *_MC_pm = &(lm);     \
    if (_MC_p >= BF_LAG_COUNT) break;    \
    size_t _MC_i = (_MC_p) / 8;          \
    int _MC_j = (_MC_p) % 8;             \
    v = ((*_MC_pm)[_MC_i] >> _MC_j) & 1; \
  } while (0);

/* @} */

/**
 * @addtogroup mc_mgr-dbug
 * @{
 */

/** Represents the various debug counters in the hardware. */
typedef struct bf_mc_debug_counters_t {
  uint16_t fifo_ph_count[4];  /**< Count of packets currently in each FIFO. */
  uint32_t table_ph_count[2]; /**< Count of packets per version in the PRE. */
  uint64_t cpu_copies;        /**< Total Copy-to-CPU copies generated. */
  uint64_t ph_processed;      /**< Total packets processed. */
  uint64_t total_copies;      /**< Total copies made. */
  uint64_t l1_exclusion_id_prunes; /**< Total L1 prunes. */
  uint64_t l2_exclusion_id_prunes; /**< Total L2 prunes. */
  uint64_t filtered_ph_processed;  /**< Total packets processed which match the
                                      filter. */
  uint64_t filtered_total_copies;  /**< Total copies made from packest matching
                                      the filter. */
  uint64_t filtered_l1_xid_prunes; /**< Total L1 prunes from packets matching
                                      the filter. */
  uint64_t filtered_l2_xid_prunes; /**< Total L2 prunes from packets matching
                                      the filter. */
  uint64_t ph_lost;                /**< Packets lost due to FIFO overflow. */
  uint64_t ph_misc;                /**< Misc counter within the PRE. */
} bf_mc_debug_counters_t;

/** Represents the state of a packet captured from the RDM trigger. */
typedef struct bf_mc_debug_packet_state_t {
  uint16_t
      pkt_len; /**< Packet length, valid only when cut-through is enabled. */
  uint8_t pkt_ver;       /**< Pipeline packet version. */
  uint16_t inport;       /**< Ingress port. */
  uint8_t eport_qid;     /**< qid metadata. */
  uint8_t color;         /**< packet_color metadata. */
  uint8_t icos;          /**< ingress_cos metadata. */
  bool ppg_en;           /**< */
  bool pause_en;         /**< */
  uint8_t ppg;           /**< */
  uint8_t ing_pool_id;   /**< */
  bool cut_thru;         /**< enable_mcast_cutthru metadata. */
  bool def_on_drop;      /**< deflect_on_drop metadata. */
  uint8_t pkt_cells_cnt; /**< */
  bool accepted_uc_copy; /**< */
  bool egress_bypass;    /**< bypass_egress metadata. */
  uint8_t pre_fifo_id;   /**< Which FIFO the packet came from. */
  bool pre_fifo_shared;  /**< */
  uint16_t hash1;        /**< level1_mcast_hash metadata. */
  uint16_t hash2;        /**< level2_mcast_hash metadata. */
  uint16_t xid_l1;       /**< level1_exclusion_id metadata. */
  uint16_t xid_l2;       /**< level2_exclusion_id metadata. */
  uint16_t rid;          /**< ingress rid metadata. */
  bool mcid1_valid;      /**< */
  uint16_t mcid1;        /**< mcast_grp_a metadata. */
  bool mcid2_valid;      /**<  */
  uint16_t mcid2;        /**< mcast_grp_b metadata. */
  bool c2c_valid;        /**< copy_to_cpu metadata. */
  uint8_t c2c_cos;       /**< icos_for_copy_to_cpu metadata. */
  bool tableid;          /**< Multicast packet version. */
} bf_mc_debug_packet_state_t;

/** Records the credit available to various internal FIFOs. */
typedef struct bf_mc_debug_int_fifo_credit_t {
  uint8_t l0_fifo_credit;
  uint8_t l1_fifo_credit;
  uint8_t l1l2_fifo_credit;
  uint8_t l2_fifo_credit;
  uint8_t l2_vec_fifo_credit;
  uint8_t l2_lag_fifo_credit;
  uint8_t l2_vec_port_fifo_credit;
  uint8_t l2_lag_port_fifo_credit;
} bf_mc_debug_int_fifo_credit_t;

/* @} */

typedef bf_mc_ecmp_hdl_t mc_l1_ecmp_hdl_t;
typedef bf_mc_node_hdl_t mc_l1_node_hdl_t;
typedef bf_mc_node_hdl_t mc_l2_node_hdl_t;
#endif
