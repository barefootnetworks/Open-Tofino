/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#ifndef _TOFINO_PDFIXED_PD_COMMON_H
#define _TOFINO_PDFIXED_PD_COMMON_H

/**
 * @file pd_common.h
 *
 * @brief PD fixed common APIs.
 *
 */

#ifndef __KERNEL__
#include <stdint.h>
#include <stdbool.h>
#endif
#include <pipe_mgr/pipe_mgr_intf.h>

typedef bf_status_t p4_pd_status_t;

typedef uint32_t p4_pd_sess_hdl_t;

typedef uint32_t p4_pd_entry_hdl_t;
typedef uint32_t p4_pd_mbr_hdl_t;
typedef uint32_t p4_pd_grp_hdl_t;
typedef uint32_t p4_pd_tbl_hdl_t;
typedef uint32_t p4_pd_idx_t;
typedef uint32_t p4_pd_pvs_hdl_t;

#define PD_DEV_PIPE_ALL 0xffff
typedef struct p4_pd_dev_target {
  bf_dev_id_t device_id; /*!< Device Identifier the API request is for */
  bf_dev_pipe_t
      dev_pipe_id; /*!< If specified localizes target to the resources
                     * only accessible to the port. DEV_PIPE_ALL serves
                     * as a wild-card value.
                     * If User defined scope has been set, this should be the
                     * lowest pipe in that scope
                     */
} p4_pd_dev_target_t;

typedef struct p4_pd_counter_value {
  uint64_t packets;
  uint64_t bytes;
} p4_pd_counter_value_t;

typedef uint64_t p4_pd_register_value_t;

typedef p4_pd_status_t (*p4_pd_sel_tbl_update_cb)(p4_pd_sess_hdl_t sess_hdl,
                                                  p4_pd_dev_target_t dev_tgt,
                                                  void *cookie,
                                                  p4_pd_grp_hdl_t sel_grp_hdl,
                                                  p4_pd_mbr_hdl_t adt_ent_hdl,
                                                  int logical_table_index,
                                                  bool is_add);
/** POLL_MODE or NOTIFY_MODE */
typedef enum p4_pd_idle_time_mode {
  PD_POLL_MODE,
  PD_NOTIFY_MODE,
  PD_INVALID_MODE
} p4_pd_idle_time_mode_e;

typedef enum p4_pd_idle_time_hit_state {
  PD_ENTRY_IDLE,
  PD_ENTRY_ACTIVE
} p4_pd_idle_time_hit_state_e;

typedef void (*p4_pd_idle_tmo_expiry_cb)(bf_dev_id_t dev_id,
                                         p4_pd_entry_hdl_t entry_hdl,
                                         void *cookie);

typedef void (*p4_pd_idle_time_update_complete_cb)(bf_dev_id_t dev_id,
                                                   void *cookie);

typedef struct p4_pd_idle_time_params {
  p4_pd_idle_time_mode_e mode;

  union {
    struct {
      uint32_t ttl_query_interval;
      uint32_t max_ttl;
      uint32_t min_ttl;
      p4_pd_idle_tmo_expiry_cb callback_fn;
      void *cookie;
      bool default_callback_choice;
    } notify;
  } params;

} p4_pd_idle_time_params_t;

typedef void (*p4_pd_stat_ent_sync_cb)(bf_dev_id_t device_id, void *cookie);

typedef void (*p4_pd_stat_sync_cb)(bf_dev_id_t device_id, void *cookie);

typedef void (*p4_pd_register_ent_sync_cb)(bf_dev_id_t device_id, void *cookie);

typedef void (*p4_pd_register_sync_cb)(bf_dev_id_t device_id, void *cookie);

#define COUNTER_READ_HW_SYNC (1 << 0)

#define REGISTER_READ_HW_SYNC (1 << 0)

typedef enum {
  PD_METER_TYPE_COLOR_AWARE,   /** Color aware meter */
  PD_METER_TYPE_COLOR_UNAWARE, /** Color unaware meter */
} p4_pd_meter_type_t;

typedef struct p4_pd_packets_meter_spec {
  uint32_t cir_pps;
  uint32_t cburst_pkts;
  uint32_t pir_pps;
  uint32_t pburst_pkts;
  p4_pd_meter_type_t meter_type;
} p4_pd_packets_meter_spec_t;

typedef struct p4_pd_bytes_meter_spec {
  uint32_t cir_kbps;
  uint32_t cburst_kbits;
  uint32_t pir_kbps;
  uint32_t pburst_kbits;
  p4_pd_meter_type_t meter_type;
} p4_pd_bytes_meter_spec_t;

typedef enum {
  PD_LPF_TYPE_RATE,
  PD_LPF_TYPE_SAMPLE,
} p4_pd_lpf_type_t;

typedef struct p4_pd_lpf_spec {
  /** A flag indicating if a separate rise/fall time constant is desired */
  bool gain_decay_separate_time_constant;
  /** Rise time constant, in nanoseconds, valid only if the above flag
   * is set */
  float gain_time_constant;
  /** Fall time constant, in nanoseconds valid only if the above flag is set */
  float decay_time_constant;
  /** A common time constant, in nanoseconds valid only if the flag
   * is not set */
  float time_constant;
  /** An integer indicating the scale down factor, right-shifted by
     these many bits. Values range from 0 to 31 */
  uint32_t output_scale_down_factor;
  p4_pd_lpf_type_t lpf_type;
} p4_pd_lpf_spec_t;

typedef struct p4_pd_red_spec {
  /** Time constant in nanoseconds */
  float time_constant;
  /** Queue threshold above which the probabilistic dropping starts, in
   * units of packet buffer cells.
   */
  uint32_t red_min_threshold;
  /** Queue threshold above which all packets are dropped in cell units */
  uint32_t red_max_threshold;
  /** Maximum probability desired for marking the packet, with range from 0.0
   * to 1.0 */
  float max_probability;
} p4_pd_wred_spec_t;

/* Definitions for mirroring primitives/apis */
typedef enum {
  PD_MIRROR_TYPE_NORM = 0,
  PD_MIRROR_TYPE_COAL,
  PD_MIRROR_TYPE_MAX
} p4_pd_mirror_type_e;

typedef enum {
  PD_DIR_NONE = 0,
  PD_DIR_INGRESS,
  PD_DIR_EGRESS,
  PD_DIR_BOTH
} p4_pd_direction_t;

typedef enum {
  PD_HASH_CFG = 0, /**< Hash cfg from session table(0,default) or MAU(1) */
  PD_HASH_CFG_P,   /**< Valid only Hash cfg from session table(PD_HASH_CFG=1)
                    *   replace either hash1(1) or hash2(0) */
  PD_ICOS_CFG,     /**< ICOS cfg from session table(0,default) or MAU(1) */
  PD_DOD_CFG,  /**< Deflect_on_drop cfg from session table(0,default) or MAU(1)
                  */
  PD_C2C_CFG,  /**< Copy to cpu cfg from session table(0,default) or MAU(1) */
  PD_MC_CFG,   /**< Multicast cfg from session table(0,default) or MAU(1) */
  PD_EPIPE_CFG /**< Epipe cfg from session table(0,default) or MAU(1) */
} p4_pd_mirror_meta_flag_e;

typedef uint16_t p4_pd_mirror_id_t;

typedef enum pd_p4_pktgen_trigger_type {
  PD_PKTGEN_TRIGGER_TIMER_ONE_SHOT = 0,
  PD_PKTGEN_TRIGGER_TIMER_PERIODIC = 1,
  PD_PKTGEN_TRIGGER_PORT_DOWN = 2,
  PD_PKTGEN_TRIGGER_RECIRC_PATTERN = 3,
  PD_PKTGEN_TRIGGER_DPRSR = 4,
  PD_PKTGEN_TRIGGER_PFC = 5
} p4_pd_pktgen_trigger_type_e;
typedef struct p4_pd_pktgen_app_cfg {
  p4_pd_pktgen_trigger_type_e trigger_type;
  uint16_t batch_count;
  uint16_t packets_per_batch;
  uint32_t pattern_value;
  uint32_t pattern_mask;
  uint32_t timer_nanosec;
  uint32_t ibg;
  uint32_t ibg_jitter;
  uint32_t ipg;
  uint32_t ipg_jitter;
  uint16_t source_port;
  bool increment_source_port;
  uint16_t pkt_buffer_offset;
  uint16_t length;
} p4_pd_pktgen_app_cfg;

typedef struct p4_pd_pktgen_app_cfg_tof2 {
  p4_pd_pktgen_trigger_type_e trigger_type;
  uint16_t batch_count;
  uint16_t packets_per_batch;
  uint32_t timer_nanosec;
  uint32_t ibg;
  uint32_t ibg_jitter;
  uint32_t ipg;
  uint32_t ipg_jitter;
  uint16_t source_port;
  bool increment_source_port;
  uint16_t pkt_buffer_offset;
  uint16_t length;
  /* Only for Tofino2 */
  uint8_t pattern_value_long[16];
  uint8_t pattern_mask_long[16];
  uint8_t pfc_hdr[16];
  bool pfc_timer_en;
  uint16_t pfc_timer;
  uint16_t pfc_max_msgs;
  uint8_t port_mask_sel;
  uint8_t source_port_wrap_max;
  uint8_t assigned_chnl_id;
  bool offset_len_from_recir_pkt;
} p4_pd_pktgen_app_cfg_tof2;
struct p4_pd_port_down_mask_tof2 {
  uint8_t port_mask[9];
};

typedef enum p4_pd_pktgen_port_down_mode_t {
  PD_PKTGEN_PORT_DOWN_REPLAY_NONE = 0,
  PD_PKTGEN_PORT_DOWN_REPLAY_ALL = 1,
  PD_PKTGEN_PORT_DOWN_REPLAY_MISSED = 2
} p4_pd_pktgen_port_down_mode_t;

typedef enum p4_pd_tbl_prop_type_ {
  PD_TABLE_PROP_NONE = 0,
  PD_TABLE_ENTRY_SCOPE,
  PD_TERN_TABLE_ENTRY_PLACEMENT,
  PD_DUPLICATE_ENTRY_CHECK,
  PD_IDLETIME_REPEATED_NOTIFICATION,
  PD_MAX_PROPERTY
} p4_pd_tbl_prop_type_t;

typedef enum p4_pd_tbl_prop_scope_value {
  PD_ENTRY_SCOPE_ALL_PIPELINES = 0,
  PD_ENTRY_SCOPE_SINGLE_PIPELINE = 1,
  PD_ENTRY_SCOPE_USER_DEFINED = 2,
} p4_pd_tbl_prop_scope_value_t;

typedef enum p4_pd_tbl_prop_tern_placement_value {
  PD_TERN_ENTRY_PLACEMENT_DRV_MANAGED = 0,
  PD_TERN_ENTRY_PLACEMENT_APP_MANAGED = 1,
} p4_pd_tbl_prop_tern_placement_value_t;

typedef enum p4_pd_tbl_prop_duplicate_entry_check_value {
  PD_DUPLICATE_ENTRY_CHECK_DISABLE = 0,
  PD_DUPLICATE_ENTRY_CHECK_ENABLE = 1,
} p4_pd_tbl_prop_duplicate_entry_check_value_t;

typedef enum p4_pd_tbl_prop_idletime_repeated_notification_enable_value {
  PD_IDLETIME_REPEATED_NOTIFICATION_DISABLE = 0,
  PD_IDLETIME_REPEATED_NOTIFICATION_ENABLE = 1,
} p4_pd_tbl_prop_idletime_repeated_notification_enable_value_t;

typedef union p4_pd_tbl_prop_value {
  uint32_t value;
  p4_pd_tbl_prop_scope_value_t scope;
  p4_pd_tbl_prop_tern_placement_value_t tern_placement;
  p4_pd_tbl_prop_duplicate_entry_check_value_t duplicate_check;
  p4_pd_tbl_prop_idletime_repeated_notification_enable_value_t repeated_notify;
} p4_pd_tbl_prop_value_t;

#define PD_MAX_USER_DEFINED_SCOPES 4
typedef union p4_pd_tbl_prop_args {
  uint32_t value;
  uint8_t user_defined_entry_scope[PD_MAX_USER_DEFINED_SCOPES];
} p4_pd_tbl_prop_args_t;

enum p4_pd_grp_mbr_state_e {
  P4_PD_GRP_MBR_STATE_ACTIVE = 0,
  P4_PD_GRP_MBR_STATE_INACTIVE = 1
};

/* -------- Tbl Debug counters start -------- */

typedef enum p4_pd_tbl_dbg_counter_type_e {
  PD_TBL_DBG_CNTR_DISABLED = BF_TBL_DBG_CNTR_DISABLED,
  PD_TBL_DBG_CNTR_LOG_TBL_MISS = BF_TBL_DBG_CNTR_LOG_TBL_MISS,
  PD_TBL_DBG_CNTR_LOG_TBL_HIT = BF_TBL_DBG_CNTR_LOG_TBL_HIT,
  PD_TBL_DBG_CNTR_GW_TBL_MISS = BF_TBL_DBG_CNTR_GW_TBL_MISS,
  PD_TBL_DBG_CNTR_GW_TBL_HIT = BF_TBL_DBG_CNTR_GW_TBL_HIT,
  PD_TBL_DBG_CNTR_GW_TBL_INHIBIT = BF_TBL_DBG_CNTR_GW_TBL_INHIBIT,
  PD_TBL_DBG_CNTR_MAX = BF_TBL_DBG_CNTR_MAX
} p4_pd_tbl_dbg_counter_type_t;

/* Debug counter per logical table */
typedef struct p4_pd_stage_tbl_dbg_cntr {
  char tbl_name[BF_TBL_NAME_LEN];
  p4_pd_tbl_dbg_counter_type_t cntr_type;
  uint32_t value;
} p4_pd_stage_tbl_dbg_cntr_t;

/* Tbl Dbg Counters for an entire stage */
typedef struct p4_pd_stage_tbl_dbg_counters {
  p4_pd_stage_tbl_dbg_cntr_t counter[BF_MAX_LOG_TBLS];
  int num_counters;
} p4_pd_stage_tbl_dbg_counters_t;

/**
 * @brief Table debug counter type set
 *
 * @param[in] dev_tgt Device information.
 * @param[in] tbl_name Table name.
 * @param[in] type Counter type.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_type_set(
    p4_pd_dev_target_t dev_tgt,
    char *tbl_name,
    p4_pd_tbl_dbg_counter_type_t type);

/**
 * @brief Table debug counter type get
 *
 * @param[in] dev_tgt Device information.
 * @param[in] tbl_name Table name.
 * @param[in] type Counter type.
 * @param[out] counter_value Pointer to return the counter value.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_get(p4_pd_dev_target_t dev_tgt,
                                         char *tbl_name,
                                         p4_pd_tbl_dbg_counter_type_t *type,
                                         uint32_t *counter_value);

/**
 * @brief Clear the table debug counter
 *
 * @param[in] dev_tgt Device information.
 * @param[in] tbl_name Table name.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_clear(p4_pd_dev_target_t dev_tgt,
                                           char *tbl_name);

/**
 * @brief Tbl debug Counter type set for all tables in stage
 *
 * @param[in] dev_tgt Device information.
 * @param[in] stage_id Stage.
 * @param[in] type Counter type.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_type_stage_set(
    p4_pd_dev_target_t dev_tgt,
    uint8_t stage_id,
    p4_pd_tbl_dbg_counter_type_t type);

/**
 * @brief Tbl debug Counter get for all tables in stage
 *
 * @param[in] dev_tgt Device information.
 * @param[in] stage_id Stage.
 * @param[out] stage_cntrs Pointer to return the counter values.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_stage_get(
    p4_pd_dev_target_t dev_tgt,
    uint8_t stage_id,
    p4_pd_stage_tbl_dbg_counters_t *stage_cntrs);

/**
 * @brief Tbl debug Counter clear for all tables in stage
 *
 * @param[in] dev_tgt Device information.
 * @param[in] stage_id Stage.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_tbl_dbg_counter_stage_clear(p4_pd_dev_target_t dev_tgt,
                                                 uint8_t stage_id);

/* -------- Tbl Debug counters end -------- */

/* -------- Snapshot start -------- */

/**
 * @brief Enable Snapshot timer
 *
 * @param[in] hdl Snapshot handle
 * @param[in] disable disable timer if true.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_timer_enable(pipe_snapshot_hdl_t hdl,
                                           bool disable);

/**
 * @brief Set Snapshot state
 *
 * @param[in] hdl Snapshot handle
 * @param[in] state Snapshot state.
 * @param[in] timer_usec Timer value in usec.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_state_set(pipe_snapshot_hdl_t hdl,
                                        bf_snapshot_state_t state,
                                        uint32_t timer_usec);

/**
 * @brief Get Snapshot state
 *
 * @param[in] hdl Snapshot handle
 * @param[in] dev_pipe_id Pipe.
 * @param[out] state Pointer to return the snapshot state.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_state_get(pipe_snapshot_hdl_t hdl,
                                        bf_dev_pipe_t dev_pipe_id,
                                        bf_snapshot_state_t *state);

/**
 * @brief Clear snapshot trigger fields
 *
 * @param[in] hdl Snapshot handle
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_capture_trigger_fields_clr(
    pipe_snapshot_hdl_t hdl);

/**
 * @brief Check if snapshot capture field name is in scope in the stage
 *
 * @param[in] dev_tgt Device information.
 * @param[in] stage_id Stage.
 * @param[in] dir DIrection.
 * @param[in] field_name Field name.
 * @param[out] field_exists Flag to return whether field exists or not.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_field_in_scope(p4_pd_dev_target_t dev_tgt,
                                             uint8_t stage,
                                             bf_snapshot_dir_t dir,
                                             char *field_name,
                                             bool *field_exists);
/**
 * @brief Check if snapshot trigger field name is in scope in the stage
 *
 * @param[in] dev_tgt Device information.
 * @param[in] stage_id Stage.
 * @param[in] dir DIrection.
 * @param[in] field_name Field name.
 * @param[out] field_exists Flag to return whether field exists or not.
 *
 * @return Status of the API call.
 */
p4_pd_status_t p4_pd_snapshot_trigger_field_in_scope(p4_pd_dev_target_t dev_tgt,
                                                     uint8_t stage,
                                                     bf_snapshot_dir_t dir,
                                                     char *field_name,
                                                     bool *field_exists);
/* -------- Snapshot end -------- */

/* For PI support */
typedef struct {
  int end_of_extras;
  const char *key;
  const char *v;
} pd_assign_extra_t;

/* One of the following modes can be set on a PVS construct.
 * Different PVS construct can have different modes in a P4; however
 * once mode is set on a PVS, it can only be changed if PVS is empty
 * PVS can be emptied by using pipe_mgr_pvs_parser_tcam_delete() API
 * over all in use elememts of the set. A Parser Value Set is empty
 * when none of the set elements are in use at runtime.
 *
 *
 * PD_PVS_SCOPE_ALL_PARSERS_IN_PIPE :   All parser engines in a pipe.
 *                          In this mode, when a PVS element is updated
 *                          the update is applied on all parser engines
 *                          in one or more pipes. Pipes are identified by
 *                          by either BF_DEV_PIPE_ALL or singel pipeid in
 *                          the run time API.
 *
 * PD_PVS_SCOPE_SINGLE_PARSER :  Single parser mode.
 *                          In this mode, when a PVS element is updated
 *                          the update is applied to only one parser engine
 *                          in a pipe. Parser engine id is specified by
 *                          parser_id. Parser id for a device port can be
 *                          obtained by using p4_pd_pvs_get_parser_id() API.
 *                          However its possible to update same parser engine
 *                          instance in all pipes or one pipe by specifyng
 *                          pipe_id as BF_DEV_PIPE_ALL or single pipeid vlaue
 *                          in the run time API.
 */
typedef enum {
  PD_PVS_SCOPE_ALL_PARSERS_IN_PIPE = 0,
  PD_PVS_SCOPE_SINGLE_PARSER = 1
} p4_pd_pvs_parser_scope_en;

typedef enum {
  PD_PVS_SCOPE_ALL_GRESS = 0,
  PD_PVS_SCOPE_SINGLE_GRESS = 1
} p4_pd_pvs_gress_scope_en;

typedef enum {
  PD_PVS_SCOPE_ALL_PIPELINES = 0,
  PD_PVS_SCOPE_SINGLE_PIPELINE = 1,
  PD_PVS_SCOPE_USER_DEFINED = 2
} p4_pd_pvs_pipe_scope_en;

typedef enum {
  PD_PVS_GRESS_INGRESS = 0,
  PD_PVS_GRESS_EGRESS = 1,
  PD_PVS_GRESS_ALL = 0xff
} p4_pd_pvs_gress_en;

typedef enum p4_pd_pvs_prop_type_ {
  PD_PVS_PROP_NONE = 0,
  PD_PVS_GRESS_SCOPE,
  PD_PVS_PIPE_SCOPE,
  PD_PVS_PARSER_SCOPE
} p4_pd_pvs_prop_type_t;

typedef union p4_pd_pvs_prop_value {
  uint32_t value;
  p4_pd_pvs_gress_scope_en gress_scope;
  p4_pd_pvs_pipe_scope_en pipe_scope;
  p4_pd_pvs_parser_scope_en parser_scope;
} p4_pd_pvs_prop_value_t;

typedef union p4_pd_pvs_prop_args {
  uint32_t value;
  p4_pd_pvs_gress_en gress;
  uint8_t user_defined_entry_scope[PD_MAX_USER_DEFINED_SCOPES];
} p4_pd_pvs_prop_args_t;

/* parser target type used by PVS APIs to add/modify/delete PVS set elements */
#define PD_DEV_PIPE_PARSER_ALL 0xff
typedef struct p4_pd_dev_parser_target {
  bf_dev_id_t device_id;       /* Device Identifier the API request is for */
  p4_pd_pvs_gress_en gress_id; /* If specified localizes target to the
                                * resources only accessible within a
                                * gress. PD_PVS_GRESS_ALL (0xff) serves
                                * as a wild-card value */
  bf_dev_pipe_t dev_pipe_id;   /* If specified localizes target to the resources
                           * only accessible to the pipeline. BF_DEV_PIPE_ALL
                           * serves as a wild-card value
                           */
  uint8_t parser_id;           /* If specified localizes target to the
                                * resources only accessible to the parser.
                                * PD_DEV_PIPE_PARSER_ALL serves as a wild-card
                                * value.
                                */
} p4_pd_dev_parser_target_t;

/* Indirect resource INDEX value to indicate that the resource index is invalid
 */
#define PD_INVALID_INDIRECT_RESOURCE_IDX 0xffffffff

#endif
