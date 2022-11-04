/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO_PDFIXED_PD_TM_H
#define _TOFINO_PDFIXED_PD_TM_H

/**
 * @file pd_tm.h
 * @brief This file contains all the PD APIs for Traffic Manager application to
 *        set/read/get configuration from hardware.
 */

#include <tofino/pdfixed/pd_common.h>

/**
 * @addtogroup pd-tm-api
 * @{
 */

typedef int32_t p4_pd_tm_dev_t;
typedef uint32_t p4_pd_tm_port_t;
typedef uint32_t p4_pd_tm_ppg_t;
typedef uint16_t p4_pd_tm_icos_map_t;
typedef uint32_t p4_pd_tm_queue_t;
typedef uint32_t p4_pd_tm_l1_node_t;
typedef uint16_t p4_pd_tm_icos_t;
typedef uint32_t p4_pd_tm_pipe_t;
typedef uint32_t p4_pd_tm_thres_t;

typedef enum {
  PD_COLOR_GREEN = 0,
  PD_COLOR_YELLOW = 1,
  PD_COLOR_RED = 2
} p4_pd_color_t;

typedef enum {
  PD_TM_COLOR_12_POINT_5_PERCENT,
  PD_TM_COLOR_25_PERCENT,
  PD_TM_COLOR_37_POINT_5_PERCENT,
  PD_TM_COLOR_50_PERCENT,
  PD_TM_COLOR_62_POINT_5_PERCENT,
  PD_TM_COLOR_75_PERCENT,
  PD_TM_COLOR_87_POINT_5_PERCENT,
  PD_TM_COLOR_100_PERCENT,
} p4_pd_color_limit_t;

typedef enum {
  PD_INGRESS_POOL_0,
  PD_INGRESS_POOL_1,
  PD_INGRESS_POOL_2,
  PD_INGRESS_POOL_3,
  PD_EGRESS_POOL_0,
  PD_EGRESS_POOL_1,
  PD_EGRESS_POOL_2,
  PD_EGRESS_POOL_3,
} p4_pd_pool_id_t;

typedef enum {
  PD_PPG_BAF_1_POINT_5_PERCENT,
  PD_PPG_BAF_3_PERCENT,
  PD_PPG_BAF_6_PERCENT,
  PD_PPG_BAF_11_PERCENT,
  PD_PPG_BAF_20_PERCENT,
  PD_PPG_BAF_33_PERCENT,
  PD_PPG_BAF_50_PERCENT,
  PD_PPG_BAF_66_PERCENT,
  PD_PPG_BAF_80_PERCENT,
  PD_PPG_BAF_DISABLE,
} p4_pd_tm_ppg_baf_t;

typedef enum {
  PD_Q_BAF_1_POINT_5_PERCENT,
  PD_Q_BAF_3_PERCENT,
  PD_Q_BAF_6_PERCENT,
  PD_Q_BAF_11_PERCENT,
  PD_Q_BAF_20_PERCENT,
  PD_Q_BAF_33_PERCENT,
  PD_Q_BAF_50_PERCENT,
  PD_Q_BAF_66_PERCENT,
  PD_Q_BAF_80_PERCENT,
  PD_Q_BAF_DISABLE,
} p4_pd_tm_queue_baf_t;

typedef enum {
  PD_TM_SCH_PRIO_LOW,
  PD_TM_SCH_PRIO_0 = PD_TM_SCH_PRIO_LOW,
  PD_TM_SCH_PRIO_1,
  PD_TM_SCH_PRIO_2,
  PD_TM_SCH_PRIO_3,
  PD_TM_SCH_PRIO_4,
  PD_TM_SCH_PRIO_5,
  PD_TM_SCH_PRIO_6,
  PD_TM_SCH_PRIO_7,
  PD_TM_SCH_PRIO_HIGH = PD_TM_SCH_PRIO_7
} p4_pd_tm_sched_prio_t;

typedef enum {
  PD_TM_SCH_RATE_UPPER,     /** Upper limit for the rate, Over-provisioning*/
  PD_TM_SCH_RATE_LOWER,     /** Lower limit for the rate, Under-provisioning*/
  PD_TM_SCH_RATE_MIN_ERROR, /** Min Error betweeen the Upper/Lower rate */
} p4_pd_tm_sched_shaper_prov_type_t;

typedef enum {
  PD_TM_PAUSE_NONE,
  PD_TM_PAUSE_PFC,
  PD_TM_PAUSE_PORT
} p4_pd_tm_flow_ctrl_type_t;

typedef enum {
  PD_TM_SCH_ADV_FC_MODE_CRE = 0,
  PD_TM_SCH_ADV_FC_MODE_XOFF = 1,
} p4_pd_tm_sched_adv_fc_mode_t;

typedef struct _p4_pd_tm_blklvl_cntrs {
  uint64_t wac_no_dest_drop;   // wac_reg.ctr_drop_no_dst
  uint64_t qac_no_dest_drop;   // qac_reg.qac_ctr32_drop_no_dst
  uint64_t wac_buf_full_drop;  // wac_reg.wac_drop_buf_full.wac_drop_buf_full
  uint64_t psc_pkt_drop;       // psc pkt_dropcnt.pkt_dropcnt
  uint64_t egress_pipe_total_drop;  // pkt_dropcnt.pkt_dropcnt
  uint64_t pex_total_disc;          // pex_dis_cnt.pex_dis_cnt
  uint64_t qac_total_disc;          // qac_dis_cnt.qac_dis_cnt
  uint64_t total_disc_dq;           // tot_dis_dq_cnt.tot_dis_dq_cnt
  uint64_t pre_total_drop;          // PRE packet_drop.packet_drop
  uint64_t qac_pre_mc_drop;         // qac_reg.qac_ctr32_pre_mc_drop

} p4_pd_tm_blklvl_cntrs_t;

typedef struct _p4_pd_tm_pre_fifo_cntrs {
  uint64_t wac_drop_cnt_pre0_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre1_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre2_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre3_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre4_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre5_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre6_fifo[BF_PRE_FIFO_COUNT];
  uint64_t wac_drop_cnt_pre7_fifo[BF_PRE_FIFO_COUNT];
} p4_pd_tm_pre_fifo_cntrs_t;

typedef struct _p4_pd_tm_dev_cfg_t {
  uint8_t pipe_cnt;
  uint8_t pg_per_pipe;
  uint8_t q_per_pg;
  uint8_t ports_per_pg;
  uint16_t pfc_ppg_per_pipe;
  uint16_t total_ppg_per_pipe;
  uint8_t pre_fifo_per_pipe;
  uint8_t l1_per_pg;
  uint16_t l1_per_pipe;

} p4_pd_tm_dev_cfg_t;

/**
 * @brief Allocate an unused PPG. The new PPG can be used to implement
 * part of QoS behaviour at ingress TM. If PPGs are exhausted, the API fails.
 *
 * Related APIs: p4_pd_tm_free_ppg(), p4_pd_tm_get_default_ppg()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[out] ppg       Ppg handle is allocated if available.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_allocate_ppg(p4_pd_tm_dev_t dev,
                                     p4_pd_tm_port_t port,
                                     p4_pd_tm_ppg_t *ppg);

/**
 * @brief Free PPG back to free pool.
 *
 * Related APIs: p4_pd_tm_allocate_ppg()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg handle to free. PPG handle specifies
 *                       pipe to which PPG belongs to. PPG resource
 *                       will be returned to that pipe.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_free_ppg(p4_pd_tm_dev_t dev, p4_pd_tm_ppg_t ppg);

/**
 * @brief Get default PPG associated with port.
 * The default PPG handle can be used to
 * configure PPG limit. One or many iCoS (iCoS = ig_intr_md.ingress_cos)
 * traffic is mapped to default PPG. At the start all iCoS traffic on a port
 * is mapped to default PPG. By using API p4_pd_tm_set_ppg_icos_mapping(), all
 *or
 * subset of iCoS traffic can be moved to a PPG of their own.
 *
 * Related APIs: p4_pd_tm_allocate_ppg()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[out] ppg       Ppg handle associated with port
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_default_ppg(p4_pd_tm_dev_t dev,
                                        p4_pd_tm_port_t port,
                                        p4_pd_tm_ppg_t *ppg);

/**
 * @brief Set PPG Ingress cos to cos mapping
 * This API will assign iCoS (iCoS = ig_intr_md.ingress_cos) traffic on port
 * that PPG is attached to. PPG handle can be default ppg, non-default ppg.
 * If its default ppg, then all the icos mapped traffic is treated lossy. If
 * not, then depending on whether lossless treatment is attached to ppg using
 * p4_pd_tm_enable_lossless_treatment() or not, all the icos traffic
 * specified using icos_bmap will be treated accordingly.
 *
 * Default : When PPG is allocated, no icos is mapped. User has to explicit
 *           assign one or more iCoS to the PPG.
 *
 * Related APIs: p4_pd_tm_ppg_icos_mapping_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg handle
 * @param[in] icos_map   Bit map of iCoS (iCoS = ig_intr_md.ingress_cos).
 *                       Bit 7 is interpreted as iCoS 7.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_set_ppg_icos_mapping(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_ppg_t ppg,
                                             p4_pd_tm_icos_map_t icos_map);

/**
 * @brief Get all iCoS (iCoS = ig_intr_md.ingress_cos) traffic is attached
 * to PPG
 *
 *
 * Related APIs: p4_pd_tm_set_ppg_icos_mapping()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg handle
 * @param[out] icos_map  Bit map of iCoS (iCoS = ig_intr_md.ingress_cos).
 *                       Bit 7 is interpreted as iCoS 7 that is attached.
 *                       ppg handle.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_ppg_icos_mapping_get(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_ppg_t ppg,
                                             uint8_t *icos_map);

/**
 * @brief Enable lossless treatment for PPG.
 * The PPG handle to use is obtained using p4_pd_tm_allocate_ppg().
 * All traffic mapped to the PPG is considered as lossless traffic.
 *
 * Default : Default property of PPG is to treat traffic as lossy.
 *
 * Related APIs: p4_pd_tm_disable_lossless_treatment()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg handle
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_lossless_treatment(p4_pd_tm_dev_t dev,
                                                  p4_pd_tm_ppg_t ppg);
/**
 * @brief Disable lossless treatment for PPG.
 * The PPG handle that is obtained using p4_pd_tm_allocate_ppg() can be
 * used for lossy traffic.
 *
 * Related APIs: p4_pd_tm_enable_lossless_treatment()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg handle
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_lossless_treatment(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_ppg_t ppg);
/**
 * @brief Set PPG Pool and limit properties
 * A non deafult PPG can be optionally assigned to any application pool.
 * When assigned to application pool, static or dynamic shared limit
 * can be set. This API aids to achieve that. If it is desired to not
 * assign PPG to any pool, then this API need not be invoked.
 *
 * Default: PPG is not assigned to any application pool.
 *
 * Related APIs: p4_pd_tm_disable_ppg_app_pool_usage(),
 *               p4_pd_tm_get_ppg_app_pool_usage()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] ppg             Ppg handle.
 * @param[in] pool_id         Application pool to which PPG is assigned to.
 * @param[in] base_use_limit  Limit to which PPG can grow inside application
 *                            pool. Limit is specified in terms of cells.
 *                            Once this limit is crossed, if PPG burst
 *                            absroption factor (BAF) is non zero, depending
 *                            availability of buffer, PPG is allowed to
 *                            use buffer upto BAF limit. If BAF limit is zero,
 *                            PPG is treated as static and no dynamic
 *                            thresholding.
 * @param[in] dynamic_baf     One of the values listed in p4_pd_tm_ppg_baf_t.
 *                            When BF_TM_PPG_BAF_DISABLE is used, PPG uses
 *                            static limit.
 * @param[in] hysteresis      Hysteresis value in cells. Hardware supports
 *                            limited number of different hysteresis values
 *                            that can be set on PPGs/Pools. When this limit
 *                            is exceeded API returns error.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ppg_app_pool_usage(p4_pd_tm_dev_t dev,
                                               p4_pd_tm_ppg_t ppg,
                                               p4_pd_pool_id_t pool_id,
                                               uint32_t base_use_limit,
                                               p4_pd_tm_ppg_baf_t dynamic_baf,
                                               uint32_t hysteresis);

/**
 * @brief This API can be used to move a PPG that is assigned to application
 * pool to default.
 *
 * Related APIs: p4_pd_tm_set_ppg_app_pool_usage(),
 *               p4_pd_tm_get_ppg_app_pool_usage()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pool_id    Pool handle. Valid pools are BF_TM_IG_POOL_0..3
 *                       and BF_TM_SKID_POOL.
 * @param[in] ppg        Ppg handle
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_disable_ppg_app_pool_usage(p4_pd_tm_dev_t dev,
                                                   p4_pd_pool_id_t pool_id,
                                                   p4_pd_tm_ppg_t ppg);
/**
 * @brief Set miminum limits of ppg.
 * Inorder to increase min limits, that many free cell should be available.
 *
 * Default : TM buffer is equally assigned to all PPGs.
 *
 * Related APIs: p4_pd_tm_get_ppg_guaranteed_min_limit(),
 *               p4_pd_tm_set_guaranteed_min_skid_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose limits has to be adjusted.
 * @param[in] cells      Number of cells by which minimum ppg limit
 *                       has be increased.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ppg_guaranteed_min_limit(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_ppg_t ppg,
                                                     uint32_t cells);
/**
 * @brief Set ppg skid limits.
 * Cannot be set more than the  size of Skid pool size. Once skit limit is
 * reached, even lossless traffic will be dropped. Before consuming skid or
 * head room buffer, PFC would be asserted for lossless flows.
 *
 * Default : Skid limits are set to zero.
 *
 * Related APIs: p4_pd_tm_get_ppg_skid_limit(),
 *               p4_pd_tm_set_guaranteed_min_skid_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose skid limits has to be set.
 * @param[in] cells      Limits in terms of number of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ppg_skid_limit(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_ppg_t ppg,
                                           uint32_t cells);

/**
 * @brief Set ppg hysteresis limits.
 * Hysterisis limits are numbers of cells the ppg usage should fall by
 * from its limit value. Once usage limits are below hysteresis, appropriate
 * condition is cleared. Example when non PFC enabled PPG's usage limit falls
 * below its allowed limit by hysteresis value, drop condition is
 * cleared. PPG's buffer usage limit depends whether it is allowed to use
 * shared pool buffers. If allowed to use shared pool buffers, then hysteresis
 * is applied on total buffer limit which is guaranteed min limit and shared
 * limit,  else hysteresis limit is applied on only guaranteed limit.
 *
 * Default: Skid hysteresis limit set to zero. No hysteresis.
 *
 * Related APIs: p4_pd_tm_set_ppg_skid_limit(),
 *               p4_pd_tm_get_ppg_guaranteed_min_skid_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose resume offset limits has to be increased.
 * @param[in] cells      Limits in terms of number of cells
 *                       Hardware supports limited
 *                       number of different hysteresis values that can be
 *                       set on PPGs/Pools. When this limit is exceeded
 *                       API returns error.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_guaranteed_min_skid_hysteresis(p4_pd_tm_dev_t dev,
                                                           p4_pd_tm_ppg_t ppg,
                                                           uint32_t cells);

/**
 * @brief Allocates Queues and Sets Queue Mapping per Port
 * This API can be used to allocate flexible number of queues behind a port.
 * By default straight mapping is established. If flexible queue mapping is
 * desired, this API can be used.
 * Default mapping or when q_mapping is NULL, queue mapping is according to
 * following calculation.
 *    queue# = Ig_intr_md.qid % q_count.
 *    Example Queue mapping in default mode:
 *                   Ig_intr_md.qid 0 --> port qid 0
 *                   Ig_intr_md.qid 1 --> port qid 1
 *                         :
 *                   Ig_intr_md.qid 7 --> port qid 7
 *                         :
 *
 * NOTE: If this API gets called for changing the queue carving (queue count),
 * then application should
 *   - always call queue mapping APIs strictly in increasing order of ports
 *      within a port group
 *   - if queue count gets changed for a port/channel in a port group,
 *      then application should call the queue mapping APIs
 *      for rest of the ports after it (if present) in that port group in
 *      increasing order
 *   - if queue count gets changed, then calling this API while traffic
 *      running on any port within the port group is not recommended as
 *      traffic would be disrupted for a short period of time and buffer
 *      accounting would be inconsistent
 * The above restrictions are not applicable if just queue mapping gets changed
 *
 *
 * Related APIs: p4_pd_tm_get_port_q_mapping()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] q_count    Number of queues being mapped.
 * @param[in] q_map      Array of integer values specifying queue mapping
 *                       Mapping is indexed by ig_intr_md.qid.
 *                       Value q_mapping[ig_intr_md.qid] is port's QID
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_port_q_mapping(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           int16_t q_count,
                                           uint8_t *q_map);
/**
 * @brief Configues queue with pool and limit properties
 * A queue can be optionally assigned to any application pool.
 * When assigned to application pool, static or dynamic shared limit
 * can be set. This API aids to achieve that. If it is desired to not
 * assign queue to any pool, then this API need not be invoked.
 * Advantage of belonging to a pool of queues is that queue can share
 * resources. Hence can grow to dynamic limit depending on burst absorption
 * factor and pool usage number. Dynamic limit will be higher than its own
 * queue limit. If static limit is used, then  queue can usage upto its
 * own limit before tail drop condition occurs.
 *
 * Default: Queues are not assigned to any application pool.
 *
 * Related APIs: p4_pd_tm_get_q_app_pool_usage()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port handle.
 * @param[in] queue           Queue identifier. Valid range [ 0..31 ]
 * @param[in] pool            Application pool to which queue is assigned to.
 *                            Valid values are BF_TM_EG_POOL0..3.
 * @param[in] base_use_limit  Limit to which PPG can grow inside application
 *                            pool. Limit is specified in terms of cells.
 *                            Once this limit is crossed, if queue burst
 *                            absroption factor (BAF) is non zero, depending
 *                            availability of buffer, queue is allowed to
 *                            use buffer upto BAF limit. If BAF limit is zero,
 *                            queue is treated as static and no dynamic
 *                            buffering is possible.
 * @param[in] dynamic_baf     One of the values listed in p4_pd_tm_queue_baf_t.
 *                            When BF_TM_QUEUE_BAF_DISABLE is used, queue uses
 *                            static limit.
 * @param[in] hysteresis      Hysteresis value of queue in cells .
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_app_pool_usage(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port,
                                             p4_pd_tm_queue_t queue,
                                             p4_pd_pool_id_t pool,
                                             uint32_t base_use_limit,
                                             p4_pd_tm_queue_baf_t dynamic_baf,
                                             uint32_t hysteresis);

/**
 * @brief Disable Queue Pool Usage
 * This API can be used to disable queue participation in application
 * pool. In such case, once q's gmin limits are used, queue tail drop
 * can occur.
 *
 * Related APIs: p4_pd_tm_set_q_app_pool_usage()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue identifier
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_disable_q_app_pool_usage(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_queue_t queue);

/**
 * @brief Set queue min limits.
 * Inorder to increase min limits, MIN pool should have that many free cells
 * unused. If not available, limits cannot be increased and API fails.
 * When queue limits are decreased, in the absence of shared/application pools,
 * decreased number of cells will be unused and earmarked for future use.
 * These unused cells can be used in future to increase queue limits.
 * If application pools are carved out, then the decreased number of cells are
 * equally distributed to all application pools by raising their pool limits.
 *
 * Default: TM buffer is equally distribted to all queues assuming all queues
 *          are active.
 *
 * Related APIs: p4_pd_tm_get_q_guaranteed_min_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue identifier. Valid range [ 0..31 ]
 * @param[in] cells      Queue limits specified in cell count
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_guaranteed_min_limit(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   p4_pd_tm_queue_t queue,
                                                   uint32_t cells);

/**
 * @brief  Set color drop limits for queue.
 * Color drop limits for red should be less than color drop limits of yellow,
 * which inturn is less than color drop limits of green.
 *
 * Default: Color drop limits for yellow and red are set to 75% of gmin
 *          size of queue.
 *
 * Related APIs: p4_pd_tm_enable_q_color_drop(), p4_pd_tm_get_q_color_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop limit to be set.
 * @param[in] color      Color (RED, YELLOW)
 * @param[in] limit      Color Limit is specified in percentage of
 *                       guaranteed queue limit.
 *                       Green Color limit is equal to queue limit.
 *                       For yellow, red, limit can be set as percentage of
 *                       overall queue limit. Once queue usage reaches the
 *                       limit, appropriate colored packets are dropped.
 *                       To set GREEN Color limit use
 *                       p4_pd_tm_set_q_guaranteed_min_limit()
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_color_limit(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          p4_pd_tm_queue_t queue,
                                          p4_pd_color_t color,
                                          p4_pd_color_limit_t limit);

/**
 * @brief Set queue's color drop hysteresis. When queue's usage for a color
 * falls below by hysteresis value, tail drop condition is cleared.
 *
 * Default: Color hysteresis limits are set to  zero.
 *
 * Related APIs: p4_pd_tm_enable_q_color_drop(), p4_pd_tm_get_q_color_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop limit to be set.
 * @param[in] color      Color (RED, YELLOW)
 * @param[in] cells      Number of cells queue usage should reduce before
 *                       drop condition for appropriate colored packets is
 *                       cleared. The value will be set to HW rounded down
 *                       to 8 cell units.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_color_hysteresis(p4_pd_tm_dev_t dev,
                                               p4_pd_tm_port_t port,
                                               p4_pd_tm_queue_t queue,
                                               p4_pd_color_t color,
                                               p4_pd_tm_thres_t cells);

/**
 * @brief Set queue hysteresis value.
 *
 * Related APIs: p4_pd_tm_set_q_app_pool_usage()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose hysteresis is to set.
 * @param[in] cells      Hysteresis value of queue in cells .
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_hysteresis(p4_pd_tm_dev_t dev,
                                         p4_pd_tm_port_t port,
                                         p4_pd_tm_queue_t queue,
                                         uint32_t cells);

/**
 * @brief Enable queue tail drop condition. When queue
 * threshold limits (guranteed min limit + shared limit)
 * are reached, packets are dropped.
 *
 * Default : Trigger drops when queue threshold limits are reached.
 *
 * Related APIs: p4_pd_tm_disable_q_tail_drop()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for which tail drop has to be enabled.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_q_tail_drop(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           p4_pd_tm_queue_t queue);

/**
 * @brief Disable queue tail drop condition.
 * When queue threshold limits (guranteed min limit + shared limit)
 * are reached, packets are not dropped in Egress. This
 * will lead to Ingress drops eventually.
 *
 * Default : Trigger drops when queue threshold limits are reached.
 *
 * Related APIs: p4_pd_tm_enable_q_tail_drop()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for which tail drop has to be disabled.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_q_tail_drop(p4_pd_tm_dev_t dev,
                                            p4_pd_tm_port_t port,
                                            p4_pd_tm_queue_t queue);

/**
 * @brief Enable queue color drop condition.
 * Based on packet color, when queue color threshold limit are reached,
 * packets are dropped. When color drop is not enabled, packets do not
 * get any treatment based on their color.
 *
 * Default : Trigger drops based on color.
 *
 * Related APIs: p4_pd_tm_disable_q_color_drop()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop is to set.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_q_color_drop(p4_pd_tm_dev_t dev,
                                            p4_pd_tm_port_t port,
                                            p4_pd_tm_queue_t queue);

/**
 * @brief Disable queue color drop condition.
 * Based on packet color, when queue color threshold limit are reached,
 * packets are dropped. When color drop is not enabled, packets do not
 * get any treatment based on their color.
 *
 * Related APIs: p4_pd_tm_enable_q_color_drop()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose color drop is to set.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_q_color_drop(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port,
                                             p4_pd_tm_queue_t queue);

/**
 * @brief Set negative mirror traffic destination port and its queue.
 * Use this API to set (port, queue) used for egressing out
 * negative mirror traffic on the pipe. It's possible to set one such
 * (port,queue) value for each pipe.
 * The device port id must be on the pipe given.
 * Port queue number must be chosen according to the current port queues
 * mapping. Make sure to update the destination port and its queue number
 * if that port, or any other port of its port group, has queue mapping
 * changed.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[in] port       Negative mirror traffic destination device port id.
 * @param[in] queue      Negative mirror traffic destination port queue number.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_negative_mirror_dest(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_pipe_t pipe,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_queue_t queue);

/**
 * @brief Get negative mirror destination in a pipe.
 * Use this API to get (port, queue) currently used for egressing out
 * negative mirror traffic on the pipe.
 * Port queue is get according to the port's current queue map.
 *
 * Related APIs: p4_pd_tm_set_negative_mirror_dest()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pipe        Device pipe.
 * @param[out] port       Negative mirror traffic destination device port id.
 * @param[out] queue      Negative mirror traffic destination port queue number.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_negative_mirror_dest(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_pipe_t pipe,
                                                 p4_pd_tm_port_t *port,
                                                 p4_pd_tm_queue_t *queue);

/**
 * @brief Set Queue PFC Cos Mapping
 * When egress queues need to honour received PFC from downstream,
 * by mapping cos to queue using the API below, queues
 * will not participate in scheduling until PFC gets cleared.
 *
 * Default: All queues are mapping CoS zero.
 *
 * Related APIs: p4_pd_tm_set_ppg_icos_mapping()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @param[in] queue       Queue whose color drop is to set.
 * @param[in] cos         CoS associated with the queue.
 * @return            Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_pfc_cos_mapping(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              p4_pd_tm_queue_t queue,
                                              p4_pd_tm_icos_t cos);

/**
 * @brief Set application pool size.
 * When application pool size is set, the required number of cells are
 * obtained from unassigned portion of TM buffer cells.
 * Unassigned cells = (Total buffer cells - gmin size of every PPG or
 * Queue - sizes of all applicaiton pools).
 * When required number of cells are not available, the API fails.
 * PPGs or Queues can be mapped to this new application pool.
 *
 * Default:   By default application pool size is set to zero.
 *
 * Related APIs: p4_pd_tm_set_skid_pool_size(), p4_pd_tm_get_app_pool_size()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pool        Pool identifier. Any of BF_TM_IG_APP_POOL_0..3
 *                        or BF_TM_EG_APP_POOL_0..3 is valid application pool.
 * @param[in] cells       Size of pool in terms of cells.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_app_pool_size(p4_pd_tm_dev_t dev,
                                          p4_pd_pool_id_t pool,
                                          uint32_t cells);

/**
 * @brief Enable Application pool's color drop condition.
 * If color drop condition is enabled, when pool color
 * threshold limit are reached packets are dropped.
 * When color drop is not enabled, packets do not get any
 * treatment based on their color at pool level.
 *
 * Default : Default policy is to trigger drops based on color.
 *
 * Related APIs: p4_pd_tm_disable_app_pool_color_drop()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_app_pool_color_drop(p4_pd_tm_dev_t dev,
                                                   p4_pd_pool_id_t pool);

/**
 * @brief Disable Application pool's color drop condition.
 * By disabling color drop condition, packets do not get
 * any treatment based on their color at pool level.
 *
 * Default : Default policy is to trigger drops based on color.
 *
 * Related APIs:  p4_pd_tm_enable_app_pool_color_drop()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_app_pool_color_drop(p4_pd_tm_dev_t dev,
                                                    p4_pd_pool_id_t pool);

/**
 * @brief Set Application pool color drop limit.
 *
 * Default : By default, color drop limits are set to 100% of pool size
 *           for all colors.
 *
 * Related APIs: p4_pd_tm_enable_app_pool_color_drop(),
 *               p4_pd_tm_get_app_pool_color_drop_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool handle.
 * @param[in] color           Color (Green, Yellow, Red)
 * @param[in] limit           Limits in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_app_pool_color_drop_limit(p4_pd_tm_dev_t dev,
                                                      p4_pd_pool_id_t pool,
                                                      p4_pd_color_t color,
                                                      uint32_t limit);

/**
 * @brief Set color drop hysteresis limits.
 * The same hysteresis value is applied on all application pools.
 * Resume condition is triggered when pool usage drops by hysteresis
 * value from the limit value when color drop condition was set.
 *
 * Default : By deafult hysteresis value is set to zero; or no hysterisis
 *
 * Related APIs:  p4_pd_tm_set_app_pool_color_drop_limit()
 *                p4_pd_tm_get_app_pool_color_drop_hysteresis()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] color           Color (Green, Yellow, Red)
 * @param[in] limit           Limit in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_app_pool_color_drop_hysteresis(p4_pd_tm_dev_t dev,
                                                           p4_pd_color_t color,
                                                           uint32_t limit);

/**
 * @brief Set per PFC level limit values.
 * PFC level limits are configurable on per applicaiton pool basis.
 * When PPG usage numbers hit pfc limits, PFC is triggered
 * for lossless traffic.
 *
 * Related APIs: p4_pd_tm_get_app_pool_pfc_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Application pool handle for which limits are
 *                            configured.
 * @param[in] icos            Internal CoS (iCoS = ig_intr_md.ingress_cos) level
 *                            on which limits are applied.
 * @param[in] limit           Limit value in terms of cell count to increase.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_app_pool_pfc_limit(p4_pd_tm_dev_t dev,
                                               p4_pd_pool_id_t pool,
                                               p4_pd_tm_icos_t icos,
                                               uint32_t limit);

/**
 * @brief Set skid pool size.
 * Skid pool size can be oversubscribed. It need not be sum of per
 * ppg skid limits that are mapped to skid pool.
 * The reason being it is not practical scenario. Bursty behaviour could
 * be one or few PPGs at a time.
 *
 * Default:  At the start, default skid pool size is set zero.
 *
 * Related APIs: p4_pd_tm_set_app_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_skid_pool_size(p4_pd_tm_dev_t dev, uint32_t cells);

/**
 * @brief Set global skid pool hysteresis.
 *
 * Default : By deafult hysteresis value is set to zero; or no hysterisis
 *
 * Related APIs: p4_pd_tm_set_skid_pool_size(),
 *p4_pd_tm_get_skid_pool_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      Number of cells set as skid pool hysteresis.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_skid_pool_hysteresis(p4_pd_tm_dev_t dev,
                                                 uint32_t cells);

/**
 * @brief Set negative mirror pool limit in units of cells.
 *
 * Default : Negative mirror pool size is zero.
 *
 * Related APIs: p4_pd_tm_get_negative_mirror_pool_size()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] cells      New size.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_negative_mirror_pool_size(p4_pd_tm_dev_t dev,
                                                      uint32_t cells);

/**
 * @brief Set cut through size for unicast traffic.
 * This size determines total buffer set aside for cut through traffic.
 *
 * Default: All ports are in unicast cut through mode. The size is set
 *          accomodate worst case TM latency to transfer packet from ingress
 *          TM to egress TM.
 *
 * Related APIs:  p4_pd_tm_set_port_uc_cut_through_limit(),
 *                p4_pd_tm_get_uc_cut_through_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_uc_cut_through_pool_size(p4_pd_tm_dev_t dev,
                                                     uint32_t cells);

/**
 * @brief Set cut through size for Multicast traffic.
 * This size determines total buffer set aside for cut through traffic.
 *
 * Default: Using TM latency as metric and average replications
 *          default value will be set such that multicast traffic
 *          leverages cut through performance.
 *
 * Related APIs:  p4_pd_tm_get_mc_cut_through_pool_size(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_mc_cut_through_pool_size(p4_pd_tm_dev_t dev,
                                                     uint32_t cells);

/**
 * @brief Set ingress global cell limit threshold
 * This size determines total cells usage in ingress.
 *
 * Default: The default value of 256000 would be set for
 *          global cell limit for Tofino 2.
 *          This API is not applicable for Tofino 1.
 *
 * Related APIs:  p4_pd_tm_get_ingress_buffer_limit(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ingress_buffer_limit(p4_pd_tm_dev_t dev,
                                                 uint32_t cells);

/**
 * @brief Enables the global buffer limit threshold
 * This state determines the usage of ingress buffer limit
 *
 * Default: Ingress global limit threshold is enabled by default for
 *          Tofino 2 . This API is not applicable for Tofino 1.
 *
 * Related APIs:  p4_pd_tm_get_ingress_buffer_limit_state(),
 *                p4_pd_tm_disable_ingress_buffer_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_ingress_buffer_limit(p4_pd_tm_dev_t dev);

/**
 * @brief Disables the global buffer limit threshold
 * This state determines the usage of ingress buffer limit
 *
 * Default: Ingress global limit threshold is enabled by default for
 *          Tofino 2 . This API is not applicable for Tofino 1.
 *
 * Related APIs:  p4_pd_tm_get_ingress_buffer_limit_state(),
 *                p4_pd_tm_enable_ingress_buffer_limit().
 *
 * @param[in] dev             ASIC device identifier.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_ingress_buffer_limit(p4_pd_tm_dev_t dev);

/**
 * @brief Set egress pipe limit.
 * Default value of the pipe limit is set to maximum buffering capability
 * of the traffic manager. When admitting packet into Traffic manager,
 * apart from other checks, the packet has to also pass usage check on
 * per egress pipe usage limit. A packet destined to egress pipe whose
 * usage limit has crossed, will not be admitted.
 *
 * Default: Set to maximum TM buffer.
 *
 * Related APIs: p4_pd_tm_set_egress_pipe_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[in] cells      Limit in terms of number of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_egress_pipe_limit(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_pipe_t pipe,
                                              uint32_t cells);

/**
 * @brief Set pipe hysteresis limit.
 * When usage of cells goes below the hysteresis limit, pipe level
 * drop condition will be cleared.
 *
 * Default : Hysteresis is set to zero or no hysteresis.
 *
 * Related APIs: p4_pd_tm_set_egress_pipe_limit(),
 *               p4_pd_tm_get_egress_pipe_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[in] cells      New threshold limit
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_egress_pipe_hysteresis(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_pipe_t pipe,
                                                   uint32_t cells);

/**
 * @brief Set ingress port limit.
 * When buffer usage accounted on port basis crosses the
 * limit, traffic is not admitted into traffic manager.
 *
 * Default: Set to 100% buffer usage. (286,720 cells for Tofino, 393,216 cells
 * for Tofino2).
 *
 * Related APIs: p4_pd_tm_get_ingress_port_drop_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Limits in terms of cells. The lowest 3 bits will be
 *                       lost, so the limit should be the module of 8 for the
 *                       correctness
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ingress_port_drop_limit(p4_pd_tm_dev_t dev,
                                                    p4_pd_tm_port_t port,
                                                    uint32_t cells);

/**
 * @brief Set egress port limit.
 * When buffer usage accounted on port basis crosses the
 * limit, traffic will be droppped on QAC stage.
 *
 * Default: Set to 100% buffer usage (286,720 cells for Tofino, 393,216 cells
 * for Tofino2).
 *
 * Related APIs: p4_pd_tm_get_egress_port_drop_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Limits in terms of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_egress_port_drop_limit(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   uint32_t cells);

/**
 * @brief Set port hysteresis limits.
 * When usage of cells goes below hysteresis value port pause or
 * drop condition will be cleared.
 *
 * Default : 32 cells.
 *
 * Related APIs: p4_pd_tm_set_ingress_port_drop_limit(),
 *               p4_pd_tm_get_ingress_port_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Offset Limit. The lowest 3 bits will be
 *                       lost, so the limit should be the module of 8 for the
 *                       correctness
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_ingress_port_hysteresis(p4_pd_tm_dev_t dev,
                                                    p4_pd_tm_port_t port,
                                                    uint32_t cells);

/**
 * @brief Set port egress hysteresis limits.
 * When usage of cells goes below hysteresis value port drop condition will be
 * cleared.
 *
 * Default : 32 cells.
 *
 * Related APIs: p4_pd_tm_set_egress_port_drop_limit(),
 *               p4_pd_tm_get_egress_port_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port Identifier
 * @param[in] cells      Offset Limit. The lowest 3 bits will be
 *                       lost, so the limit should be the module of 8 for the
 *                       correctness
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_egress_port_hysteresis(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   uint32_t cells);

/**
 * @brief Set Port Unicast Cut Through Limit
 * This API can be used to set cut through buffer size on per port basis.
 * The specified size is set aside for unicast traffic in cut through mode.
 *
 * Default : Set according to absorb TM processing cycle time.
 *
 * Related APIs: p4_pd_tm_get_port_uc_cut_through_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] cells      Size in terms of cells (upto 16). Valid value [1..15]
 *                       If size is set to zero, then cut through is disabled.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_port_uc_cut_through_limit(p4_pd_tm_dev_t dev,
                                                      p4_pd_tm_port_t port,
                                                      uint32_t cells);

/**
 * @brief Set the Flow Control Mode (Port Pause | PFC Pause)
 * This API can be used to set type of Pause desired. When pause type is
 * BF_TM_PAUSE_PFC, then pfc is asserted. If type set to BF_TM_PAUSE_PORT
 * port pause is asserted. When pause is set to BF_TM_PAUSE_NONE, then
 * no pause is asserted.
 *
 * Related APIs: bf_tm_port_pfc_cos_mapping_set()
 *               bf_tm_port_flowcontrol_rx_set()
 *               p4_pd_tm_get_port_flowcontrol_mode()
 *
 * Default : No Pause or flow control.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] type       Pause type.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_set_port_flowcontrol_mode(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_flow_ctrl_type_t fctype);

/**
 * @brief Set packet CoS to internal CoS
 * When PFC level pause is desired, it is required to map internal
 * iCoS(iCoS = ig_intr_md.ingress_cos) to packet CoS. This API can
 * be used to set up the CoS mapping.
 *
 * Default: No PFC
 *
 * Related APIs: bf_tm_q_pfc_cos_mapping_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @param[in] cos_map     Array of 8 uint8_t values.
 *                        Array index is CoS and array value is iCoS.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_set_port_pfc_cos_mapping(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 uint8_t *cos_map);

/**
 * @brief Set port skid limits.
 * Cannot be increased beyond the size of skid pool. If set to zero,
 * in transit  traffic from all lossless PPGs of the port will be dropped
 * once pause/pfc is asserted.
 *
 * Default : Skid limits are set to zero.
 *
 * Related APIs: bf_tm_ppg_skid_limit_get()
 *
 * @param[in] dev               ASIC device identifier.
 * @param[in] port              Port Identifier
 * @param[in] cells             Limits in terms of number of cells.
 * @return                      Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_port_skid_limit(p4_pd_tm_dev_t dev,
                                            p4_pd_tm_port_t port,
                                            uint32_t cells);

/**
 * @brief Sets CPU port.
 * This API can be used to specify which one of the ports on a pipe is connected
 * to CPU. Copy to CPU packet fucntionality uses the specified port.
 * This function overrides if a cpu port was already set.
 *
 * Default: No CPU port
 *
 * Related APIs: bf_tm_port_cpuport_reset()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_set_cpuport(p4_pd_tm_dev_t dev, p4_pd_tm_port_t port);

/**
 * @brief Clear CPU port binding
 * This API can be used to specify no CPU port. This API can be used
 * to indicate no cpu port or clear any previously set cpu port.
 *
 * Default: No CPU port
 *
 * Related APIs: bf_tm_port_cpuport_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_reset_cpuport(p4_pd_tm_dev_t dev);

/**
 * @brief Set queue scheduling priority.
 * Scheduling priority level used when serving guaranteed bandwidth.
 * Higher the number, higher the  priority to select the queue for
 * scheduling.
 *
 * Default: Queue scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_set ()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[in] priority              Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_sched_priority(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port,
                                             p4_pd_tm_queue_t queue,
                                             uint16_t priority);

/**
 * @brief Set queue DWRR weights.
 * These weights are used by queues at same priority level.
 * Across prioirty these weights serve as ratio to
 * share excess or remaining bandwidth.
 *
 * Default: Queue scheduling weights set to 1023
 *
 * Related APIs: bf_tm_sched_q_priority_set(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] weight          Weight value. Supported range [ 0.. 1023 ]
 *                            Weight 0  is used to disable the DWRR especially
 *                            when Max Rate Leakybucket is used.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_dwrr_weight(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          p4_pd_tm_queue_t queue,
                                          uint16_t weight);

/**
 * @brief Set queue shaping rate in units of kbps or pps.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burstsize       Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_shaping_rate(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           p4_pd_tm_queue_t queue,
                                           bool pps,
                                           uint32_t burstsize,
                                           uint32_t rate);

/**
 * @brief Set queue shaping rate in units of kbps or pps using provisioning
 *type.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burstsize       Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @param[in] pd_prov_type    Shaper provisioning type {UPPER, LOWER, MIN_ERR}
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_shaping_rate_provisioning(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    bool pps,
    uint32_t burstsize,
    uint32_t rate,
    p4_pd_tm_sched_shaper_prov_type_t pd_prov_type);

/**
 * @brief Set queue guaranteed rate in terms of pps or kbps.
 *
 * Default: Queue shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burstsize       Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_guaranteed_rate(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              p4_pd_tm_queue_t queue,
                                              bool pps,
                                              uint32_t burstsize,
                                              uint32_t rate);

/**
 * @brief Set scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the queue for
 * scheduling.
 *
 * Default: Queue scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_get ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[in] priority        Scheduling priority of queue.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_q_remaining_bw_sched_priority(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    uint16_t priority);

/**
 * @brief Associate queue with l1 node for further scheduling.
 *
 * Default: By deafult, queue is set to schedule with the default l1 node
 * for a port.
 *
 * Related APIs: bf_tm_sched_q_l1_reset
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_q_l1_set(p4_pd_tm_dev_t dev,
                                       p4_pd_tm_port_t port,
                                       p4_pd_tm_l1_node_t l1_node,
                                       p4_pd_tm_queue_t queue);

/**
 * @brief Set queue to default l1 node for its port.
 * Note that the port must have at least 1 l1 node assigned to it.
 *
 * Related APIs: bf_tm_sched_q_l1_set
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_q_l1_reset(p4_pd_tm_dev_t dev,
                                         p4_pd_tm_port_t port,
                                         p4_pd_tm_queue_t queue);

/**
 * @brief Set l1 node scheduling priority.
 * Scheduling priority level used when serving guaranteed bandwidth.
 * Higher the number, higher the  priority to select the l1 node for
 * scheduling.
 *
 * Default: l1 node scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_l1_remaining_bw_priority_set ()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] l1_node               L1 node
 * @param[in] priority              Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_priority_set(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              p4_pd_tm_l1_node_t l1_node,
                                              uint16_t priority);

/**
 * @brief Set l1 node DWRR weights.
 * These weights are used by l1 nodes at same priority level.
 * Across priority these weights serve as ratio to
 * share excess or remaining bandwidth.
 *
 * Default: l1 node scheduling weights set to 1023
 *
 * Related APIs: bf_tm_sched_l1_priority_set(),
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1 node         L1 node
 * @param[in] weight          Weight value. Supported range [ 0.. 1023 ]
 *                            Weight 0  is used to disable the DWRR especially
 *                            when Max Rate Leakybucket is used.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_dwrr_weight_set(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_l1_node_t l1_node,
                                                 uint16_t weight);

/**
 * @brief Get l1 node DWRR weight.
 * These weights are used by l1 nodes at same priority level.
 * Across priority these weights serve as ratio to
 * share excess or remaining bandwidth.
 *
 * Related APIs: p4_pd_tm_sched_l1_dwrr_weight_set()
 *
 * @param[in] dev             ASIC device identifier
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[out] weight         Weight value
 * @return                    Status of API call
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_dwrr_weight_get(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_l1_node_t l1_node,
                                                 uint16_t *weight);

/**
 * @brief Set l1 node shaping rate in units of kbps or pps.
 *
 * Default: l1 node shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_shaping_rate_set(p4_pd_tm_dev_t dev,
                                                  p4_pd_tm_port_t port,
                                                  p4_pd_tm_l1_node_t l1_node,
                                                  bool pps,
                                                  uint32_t burst_size,
                                                  uint32_t rate);

/**
 * @brief Enable token bucket that assures l1 node shaping rate (pps or bps)
 *
 * Default: l1 node shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_l1_max_shaping_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_max_shaping_rate_enable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Disable token bucket that assures l1 node shaping rate (pps or bps)
 *
 * Default: l1 node shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_l1_max_shaping_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_max_shaping_rate_disable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Enable priority propagation from child queues
 *
 * Default: priority propagation is disabled
 *
 * Related APIs: bf_tm_sched_l1_priority_prop_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_priority_prop_enable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Disable priority propagation from child queues
 *
 * Default: priority propagation is disabled
 *
 * Related APIs: bf_tm_sched_l1_priority_prop_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_priority_prop_disable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Set l1 node guaranteed rate in terms of pps or kbps.
 *
 * Default: l1 node shaping rate set to match port bandwidth.
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burst_size      Burst size in packets or bytes.
 * @param[in] rate            Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_guaranteed_rate_set(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_port_t port,
                                                     p4_pd_tm_l1_node_t l1_node,
                                                     bool pps,
                                                     uint32_t burst_size,
                                                     uint32_t rate);

/**
 * @brief Set scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the l1 node for
 * scheduling.
 *
 * Default: l1 node scheduling priority set to BF_TM_SCH_PRIO_7
 *
 * Related APIs: bf_tm_sched_l1_remaining_bw_priority_get ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @param[in] priority        Scheduling priority of l1 node.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_remaining_bw_priority_set(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_l1_node_t l1_node,
    uint16_t priority);

/**
 * @brief Enable token bucket that assures l1 node guaranteed rate (pps or bps)
 *
 * Default: l1 node guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_guaranteed_rate_enable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Disable token bucket that assures l1 node guaranteed rate
 * (pps or bps)
 *
 * Default: l1 node guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_l1_guaranteed_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_guaranteed_rate_disable(
    p4_pd_tm_dev_t dev, p4_pd_tm_port_t port, p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Enable l1 node scheduling.
 * If l1 is not associated with a port, the l1 node will be allocated
 * to the port. l1 node cannot already be associated with
 * another port.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_disable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_enable(p4_pd_tm_dev_t dev,
                                        p4_pd_tm_port_t port,
                                        p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Disable l1 node scheduling.
 * If disabled, l1 node will not participate in scheduling.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_enable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_disable(p4_pd_tm_dev_t dev,
                                         p4_pd_tm_port_t port,
                                         p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Disable l1 node and deallocate node from port.
 * All queues must have been dissocated from this l1 node before
 * calling this function.
 *
 * Default: By deafult, each port receives an l1 one when a queue is first
 * allocated to it.
 *
 * Related APIs: bf_tm_sched_l1_enable
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] l1_node         L1 node
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_l1_free(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_port_t port,
                                      p4_pd_tm_l1_node_t l1_node);

/**
 * @brief Set port shaping rate in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_get()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] pps             If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[in] burstsize       Burst size in packets or bytes.
 * @param[in] shaper          Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_port_shaping_rate(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              bool pps,
                                              uint32_t burstsize,
                                              uint32_t rate);
/**
 * @brief Set port shaping rate in units of kbps or pps using
 * provisioning type.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_get()
 *
 * @param[in] dev               ASIC device identifier.
 * @param[in] port              Port
 * @param[in] pps               If set to true, values are in terms of pps
 *                              and packets, else in terms of kbps and bytes.
 * @param[in] burstsize         Burst size in packets or bytes.
 * @param[in] rate              Shaper value in pps or kbps.
 * @param[in] provisioning_type Shaper provisioning type {UPPER, LOWER, MIN_ERR}
 * @return                      Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_port_shaping_rate_provisioning(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    bool pps,
    uint32_t burstsize,
    uint32_t rate,
    p4_pd_tm_sched_shaper_prov_type_t provisioning_type);

/**
 * @brief Set number of bytes added per packet to packet length by shaper
 *
 * Default: Zero bytes are added to packet to length.
 *
 * Related APIs: bf_tm_sched_pkt_ifg_compensation_get
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pipe            Pipe identifier.
 * @param[in] adjustment      Byte adjustment done on every packet.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_shaper_pkt_ifg_compensation(p4_pd_tm_dev_t dev,
                                                        p4_pd_tm_pipe_t pipe,
                                                        uint8_t adjustment);

/**
 * @brief Enable queue scheduling.
 * If disabled, queue will not participate in scheduling.
 *
 * Default: By deafult, queue is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_disable_q
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_q_sched(p4_pd_tm_dev_t dev,
                                       p4_pd_tm_port_t port,
                                       p4_pd_tm_queue_t queue);

/**
 * @brief Disable queue scheduling.
 * If disabled, queue will not participate in scheduling.
 *
 * Default: By deafult, queue is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_enable_q
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_q_sched(p4_pd_tm_dev_t dev,
                                        p4_pd_tm_port_t port,
                                        p4_pd_tm_queue_t queue);

/**
 * @brief Enable token bucket that assures port shaping rate (pps or kbps)
 *
 * Default: Port shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_port_shaping_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_enable_port_shaping(p4_pd_tm_dev_t dev,
                                            p4_pd_tm_port_t port);

/**
 * @brief Disable token bucket that assures port shaping rate (pps or kbps)
 *
 * Default: Port shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_port_shaping_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_port_shaping(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port);

/**
 * @brief Enable port into scheduler.
 * When enabled, the port will participate
 * in scheduling and also port shaping is enabled.
 * The scheduling speed will be set on all channels (TM Ports)
 * starting from the port given depending on how many channels
 * are needed to participate.
 *
 * Default: By deafult, port is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_sched_port_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] port_speed      Speed (10G/25G/40G/50G/100G/200G/400G)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  BF_IN_USE if the port is not the first one in a group of channels
 *    needed for the given port_speed.
 */

p4_pd_status_t p4_pd_tm_enable_port_sched(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          uint16_t port_speed);

/**
 * @brief Disable port from scheduler.
 * If disabled, the port will not participate
 * in scheduling and also port shaping is disabled.
 *
 * Default: By deafult, port is enabled to schedule its traffic
 *          towards egress pipe/MAC.
 *
 * Related APIs: bf_tm_sched_port_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_disable_port_sched(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port);
/**
 * @brief This API can be issued to complete any pending dma operation to asic.
 * API blocks until all dma operations are complete.
 *
 * @param dev           ASIC device identifier.
 * @return
 */

void p4_pd_tm_complete_operations(p4_pd_tm_dev_t dev);

/**
 * @brief Get queue scheduling priority.
 * Scheduling priority level when serving guaranteed bandwidth. Higher the
 * number, higher the  priority to select the queue for scheduling.
 *
 * Related APIs: bf_tm_sched_q_priority_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[out] priority             Scheduling priority of queue.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_sched_priority(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port,
                                             p4_pd_tm_queue_t queue,
                                             p4_pd_tm_sched_prio_t *priority);

/**
 * @brief Get queue DWRR weights.
 * These weights are used when queues at same
 * priority level are scheduled during excess bandwidth sharing.
 *
 * Related APIs: bf_tm_sched_q_dwrr_weight_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] weight         Weight value. Supported range [ 0.. 1023 ]
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_dwrr_weight(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          p4_pd_tm_queue_t queue,
                                          uint16_t *weight);

/**
 * @brief Get queue shaping rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burstsize      Burst size in packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_shaping_rate(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           p4_pd_tm_queue_t queue,
                                           bool *pps,
                                           uint32_t *burstsize,
                                           uint32_t *rate);

/**
 * @brief Get queue shaping rate.
 * Rate is in units of kbps or pps. Also get the rate provisioning type.
 *
 * Related APIs: bf_tm_sched_q_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burstsize      Burst size in packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @param[out] pd_prov_type   The rate provisioning type (OVER, UNDER,
 *                            MIN_ERROR)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_shaping_rate_provisioning(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    bool *pps,
    uint32_t *burstsize,
    uint32_t *rate,
    p4_pd_tm_sched_shaper_prov_type_t *pd_prov_type);

/**
 * @brief Get queue guaranteed rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burstsize      Burst size in packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_guaranteed_rate(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              p4_pd_tm_queue_t queue,
                                              bool *pps,
                                              uint32_t *burstsize,
                                              uint32_t *rate);

/**
 * @brief Get scheduling priority when serving remaining bandwidth.
 * Higher the number, higher the  priority to select the queue for scheduling.
 *
 * Related APIs: bf_tm_sched_q_remaining_bw_priority_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @param[out] priority       Scheduling priority of queue.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_remaining_bw_sched_priority(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    p4_pd_tm_sched_prio_t *priority);

/**
 * @brief Get port shaping rate. Rate is in units of kbps or pps.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burstsize      Burst size in packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_port_shaping_rate(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port,
                                              bool *pps,
                                              uint32_t *burstsize,
                                              uint32_t *rate);
/**
 * @brief Get port shaping rate. Rate is in units of kbps or pps.
 * Also get the rate provisioning type.
 *
 * Related APIs: bf_tm_sched_port_shaping_rate_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[out] pps            If set to true, values are in terms of pps
 *                            and packets, else in terms of kbps and bytes.
 * @param[out] burstsize      Burst size in packets or bytes.
 * @param[out] rate           Shaper value in pps or kbps.
 * @param[out] pd_prov_type   The rate provisioning type (OVER, UNDER,
 *                            MIN_ERROR)
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_port_shaping_rate_provisioning(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    bool *pps,
    uint32_t *burstsize,
    uint32_t *rate,
    p4_pd_tm_sched_shaper_prov_type_t *pd_prov_type);

/**
 * @brief Get per packet byte adjustment value
 *
 * Related APIs: bf_tm_sched_pkt_ifg_compensation_set ()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pipe            Pipe identifier.
 * @param[out] adjust         Byte adjustment done on every packet.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_shaper_pkt_ifg_compensation(p4_pd_tm_dev_t dev,
                                                        p4_pd_tm_pipe_t pipe,
                                                        uint8_t *adjust);

/**
 * @brief Get Egress pipe limit.
 * Default value of the pipe limit is set to maximum buffering capability
 * of the traffic manager.
 *
 * When admitting packet into Traffic manager, apart from other
 * checks, the packet has to also pass usage check on per egress pipe
 * usage limit. A packet destined to egress pipe whose limit  has
 * crossed, will not be admitted.
 *
 * Related API: bf_tm_pipe_egress_limit_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[out] cells     Limits in terms of number of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_egress_pipe_limit(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_pipe_t pipe,
                                              uint32_t *cells);

/**
 * @brief Get egress pipe hysteresis limit.
 * When usage of cells goes below the hysteresis
 * limit, pipe level drop condition  will be cleared.
 *
 * Related API: p4_pd_tm_set_egress_pipe_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier
 * @param[out] cells     Limits in terms of number of cells.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_egress_pipe_hysteresis(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_pipe_t pipe,
                                                   uint32_t *cells);

/**
 * @brief This API can be used to get queue count and mapping of a port.
 *
 * Related APIs: p4_pd_tm_set_port_q_mapping ()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle.
 * @param[out] q_count    Number of queues used for the port.
 * @param[out] q_map      Array of integer values specifying queue mapping
 *                        Mapping is indexed by ig_intr_md.qid.
 *                        Value q_mapping[ig_intr_md.qid] is port's QID
 *                        Caller has to provide array of size 32 (TOF1) or
 *                        128 (TOF2)
 *                        .
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_port_q_mapping(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           uint8_t *q_count,
                                           uint8_t *q_map);

/**
 * @brief This API can be used to get port and ingress qid for
 * particular physical queue
 *
 * @param[in] dev              ASIC device identifier.
 * @param[in] log_pipe         Logical pipe ID.
 * @param[in] pipe_queue       Physical pipe-related queue ID.
 * @param[out] port            Port handle {logical pipe id, port id}.
 * @param[out] qid_count       Number of ingress queues (first ingress_q_count
 *                             in array is significant).
 * @param[out] qid_list        Ingress queue array.
 *                             :
 *                             Caller has to provide array of size 32 for TOF or
 *                             128 for TOF2
 *                             if physical queue is not currently mapped, port
 *                             is set to the port's group first port and
 *                             qid_count is set to the 0
 *
 * @return                     Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_pipe_queue_qid_list(p4_pd_tm_dev_t dev,
                                                p4_pd_tm_pipe_t log_pipe,
                                                p4_pd_tm_queue_t pipe_queue,
                                                p4_pd_tm_port_t *port,
                                                uint32_t *qid_count,
                                                p4_pd_tm_queue_t *qid_list);

/**
 * @brief This API can be used to get physical queue for particular port and
 * ingress qid.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       port handle.
 * @param[in] ingress_qid  ingress qid.
 * @param[out] log_pipe  logical pipe ID.
 * @param[out] phys_q    physical queue ID.
 *
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_port_pipe_physical_queue(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_port_t port,
                                                     uint32_t ingress_qid,
                                                     p4_pd_tm_pipe_t *log_pipe,
                                                     p4_pd_tm_queue_t *phys_q);

/**
 * @brief Get Queue App pool, limit configuration
 * A queue can be optionally assigned to any application pool.
 * When assigned to application pool, get static or dynamic shared limit
 *
 * Related APIs: p4_pd_tm_set_q_app_pool_usage()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port handle.
 * @param[in] queue           Queue identifier. Valid range [ 0..31 ] TOF1
 *                            [ 0..127 ] TOF2
 * @param[out] pool           Application pool to which queue is assigned to.
 *                            Valid values are BF_TM_EG_POOL0..3.
 * @param[out] base_use_limit Limit to which PPG can grow inside application
 *                            pool. Once this limit is crossed, if queue burst
 *                            absroption factor (BAF) is non zero, depending
 *                            availability of buffer, queue is allowed to
 *                            use buffer upto BAF limit. If BAF limit is zero,
 *                            queue is treated as static and no dynamic
 *                            thresholding.
 * @param[out] dynamic_baf    One of the values listed in bf_tm_queue_baf_t.
 *                            When BF_TM_QUEUE_BAF_DISABLE is used, queue uses
 *                            static limit.
 * @param[out] hysteresis     Hysteresis value of queue.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_app_pool_usage(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port,
                                             p4_pd_tm_queue_t queue,
                                             p4_pd_pool_id_t *pool,
                                             uint32_t *base_use_limit,
                                             p4_pd_tm_queue_baf_t *dynamic_baf,
                                             uint32_t *hysteresis);

/**
 * @brief Get queue min limits.
 * Returned limits are accounted in terms of cells.
 *
 * Related APIs: p4_pd_tm_set_q_guaranteed_min_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue identifier. Valid range [ 0..31 ] TOF1
 *                       [ 0..127 ] TOF2
 * @param[out] cells          Queue limits specified in cell count
 * @return               Status of the API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_guaranteed_min_limit(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   p4_pd_tm_queue_t queue,
                                                   uint32_t *cells);

/**
 * @brief Get color drop limits for queue.
 *
 * Related APIs: p4_pd_tm_set_q_color_limit ()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose dynamic limits has to be adjusted.
 * @param[in] color      Color (RED, YELLOW)
 * @param[out] limit     Color Limit is specified in percentage of guaranteed
 *                       queue limit.
 *                       Green Color limit is equal to queue limit.
 *                       For yellow, red, limit obtained is percentage of
 *                       overall queue share limit. Once queue usage reaches
 *                       the limit, appropriate colored packets are tail
 *                       dropped.
 *                       To get GREEN Color limit use
 *                       p4_pd_tm_get_q_guaranteed_min_limit()
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_color_limit(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          p4_pd_tm_queue_t queue,
                                          p4_pd_color_t color,
                                          p4_pd_color_limit_t *limit);

/**
 * @brief Get color hysteresis for queue.
 *
 * Related APIs: p4_pd_tm_set_q_color_hysteresis ()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[in] queue      Queue whose dynamic limits has to be adjusted.
 * @param[in] color      Color (RED, YELLOW, GREEN)
 * @param[out] cells     Number of cells queue usage drops to
 *                       when drop condition is cleared.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_q_color_hysteresis(p4_pd_tm_dev_t dev,
                                               p4_pd_tm_port_t port,
                                               p4_pd_tm_queue_t queue,
                                               p4_pd_color_t color,
                                               p4_pd_tm_thres_t *cells);

/**
 * @brief Get Port Unicast Cut Through Limit
 * This API can be used to get cut through buffer size on per port basis.
 * The specified size is set aside for unicast traffic in cut through mode.
 *
 * Related APIs: p4_pd_tm_set_port_uc_cut_through_limit()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       Port handle.
 * @param[out] size      Size in terms of cells (upto 16). Valid value [1..15]
 *                       If size is set to zero, then cut through get disabled.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_port_uc_cut_through_limit(p4_pd_tm_dev_t dev,
                                                      p4_pd_tm_port_t port,
                                                      uint8_t *cells);
/**
 * @brief Get total number of supported PPGs.
 *
 * Related APIs: bf_tm_ppg_unusedppg_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe identifier.
 * @param[out] total_ppg Pointer to unsigned integer location where total
 *                       supported PPG count will be stored.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_total_ppg(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_pipe_t pipe,
                                      uint32_t *total_ppg);
/**
 * @brief Get total number of unused PPGs.
 *
 * Related APIs: bf_tm_ppg_totalppg_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe identifier.
 * @param[out] total_ppg Pointer to unsigned integer location where
 *                       current unused PPG count will be stored.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_unused_ppg_count(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_pipe_t pipe,
                                             uint32_t *unused_ppg);

/**
 * @brief Get PPG pool limit information
 * A non deafult PPG can be optionally assigned to any application pool.
 * When assigned to application pool, get static or dynamic shared limit
 *
 * Related APIs: bf_tm_ppg_app_pool_usage_set()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] ppg             Ppg handle.
 * @param[in] pool_id         Application pool to which PPG is assigned to.
 * @param[out] base_use_limit Limit to which PPG can grow inside application
 *                            pool. Once this limit is crossed, if PPG burst
 *                            absroption factor (BAF) is non zero, depending
 *                            availability of buffer, PPG is allowed to
 *                            use buffer upto BAF limit. If BAF limit is zero,
 *                            PPG is treated as static and no dynamic
 *                            thresholding.
 * @param[out] dynamic_baf    One of the values listed in bf_tm_ppg_baf_t.
 *                            When BF_TM_PPG_BAF_DISABLE is used, PPG uses
 *                            static limit.
 * @param[out] hysteresis     Hysteresis value.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ppg_app_pool_usage(p4_pd_tm_dev_t dev,
                                               p4_pd_tm_ppg_t ppg,
                                               p4_pd_pool_id_t pool_id,
                                               uint32_t *base_use_limit,
                                               p4_pd_tm_ppg_baf_t *dynamic_baf,
                                               uint32_t *hysteresis);

/**
 * @brief Get PPG miminum limits.
 * Returned limits are accounted in terms of cells.
 *
 * Related APIs: bf_tm_ppg_guaranteed_min_limit_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose limits has to be adjusted.
 * @param[out] cells     Number of cells set as minimum limit
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ppg_guaranteed_min_limit(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_ppg_t ppg,
                                                     uint32_t *cells);
/**
 * @brief Get ppg skid limits.
 *
 * Related APIs: bf_tm_ppg_skid_limit_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose skid limits has to be fetched.
 * @param[out] cells     Limits in terms of number of cells
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ppg_skid_limit(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_ppg_t ppg,
                                           uint32_t *cells);

/**
 * @brief Get ppg hysteresis limits.
 * Same hysteresis limits are applied to PPGs limits inside MIN pool
 * and PPGs mapped to Skid Pool. Hysterisis limits are numbers of cells
 * the ppg usage should fall by from its limit value. Once usage limits
 * are below hysteresis, appropriate condition is cleared. Example when
 * PPG's skid usage limit falls below its allowed limits limit by
 * hysteresis value, drop condition is cleared.
 *
 * Related APIs: bf_tm_ppg_skid_hysteresis_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        Ppg whose hysteresis limits has to be fetched.
 * @param[out] cells     Limits in terms of number of cells
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ppg_guaranteed_min_skid_hysteresis(
    p4_pd_tm_dev_t dev, p4_pd_tm_ppg_t ppg, uint32_t *cells);

/**
 * @brief Get application pool size.
 * Size in units of cell set aside for application pool.
 *
 * Related APIs: p4_pd_tm_set_app_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Pool identifier.
 * @param[out] cells          Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_app_pool_size(p4_pd_tm_dev_t dev,
                                          p4_pd_pool_id_t pool_id,
                                          uint32_t *cells);

/**
 * @brief Get Application pool color drop limits.
 *
 * Related APIs: p4_pd_tm_set_app_pool_color_drop_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool            Pool handle.
 * @param[in] color           Color (Green, Yellow, Red)
 * @param[out] limit          Limits in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_app_pool_color_drop_limit(p4_pd_tm_dev_t dev,
                                                      p4_pd_pool_id_t pool_id,
                                                      p4_pd_color_t color,
                                                      uint32_t *limit);
/**
 * @brief Get Color drop hysteresis.
 * The same hysteresis value is applied on all application pools.
 * Resume condition is triggered when pool usage drops
 * by hysteresis value.
 *
 * Related APIs: p4_pd_tm_set_app_pool_color_drop_hysteresis()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] color       Color (Green, Yellow, Red)
 * @param[out] limit      Limits in terms of cells.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_app_pool_color_drop_hysteresis(p4_pd_tm_dev_t dev,
                                                           p4_pd_color_t color,
                                                           uint32_t *limit);

/**
 * @brief Get per PFC level limit values.
 * PFC level limits are configurable on per application pool basis.
 * When usage numbers hit pfc limits, PAUSE is triggered
 * for lossless traffic or PFC enabled traffc.
 *
 * Related APIs: p4_pd_tm_set_app_pool_pfc_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool_id         Pool handle for which limits are configured.
 * @param[in] icos            Internal CoS (iCoS = ig_intr_md.ingress_cos) level
 *                            on which limits are applied.
 * @param[out] limit          Limit value in terms of cell count.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_app_pool_pfc_limit(p4_pd_tm_dev_t dev,
                                               p4_pd_pool_id_t pool_id,
                                               p4_pd_tm_icos_t icos,
                                               uint32_t *limit);

/**
 * @brief Get skid pool size.
 *
 * Related APIs: p4_pd_tm_set_skid_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[out] cells          Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_skid_pool_size(p4_pd_tm_dev_t dev, uint32_t *cells);

/**
 * @brief Get global skid pool hysteresis.
 *
 * Related APIs: p4_pd_tm_set_skid_pool_hysteresis()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[out] cells     Number of cells set as skid pool hysteresis.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_skid_pool_hysteresis(p4_pd_tm_dev_t dev,
                                                 uint32_t *limit);

/**
 * @brief Get negative mirror pool limit.
 * Returned limit are accounted in terms of cells.
 *
 * Related APIs: p4_pd_tm_set_negative_mirror_pool_size()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[out] cells      Size of pool.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_negative_mirror_pool_size(p4_pd_tm_dev_t dev,
                                                      uint32_t *cells);

/**
 * @brief Get cut through pool size for unicast traffic.
 *
 * Related APIs:  p4_pd_tm_set_uc_cut_through_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool_id         Pool handle.
 * @param[out] cells          Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_uc_cut_through_pool_size(p4_pd_tm_dev_t dev,
                                                     p4_pd_pool_id_t pool_id,
                                                     uint32_t *cells);
/**
 * @brief Get cut through pool size for Multicast traffic.
 * This size determines total buffer set aside for multicast
 * cut through traffic.
 *
 * Related APIs:  p4_pd_tm_set_mc_cut_through_pool_size()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] pool_id         Pool handle.
 * @param[out] cells          Size of pool in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_mc_cut_through_pool_size(p4_pd_tm_dev_t dev,
                                                     p4_pd_pool_id_t pool_id,
                                                     uint32_t *cells);

/**
 * @brief Get ingress global cell limit threshold.
 * This size determines total cells usage in ingress.
 *
 *
 * Related APIs:  p4_pd_tm_set_ingress_buffer_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] cells           Size in terms of cells.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ingress_buffer_limit(p4_pd_tm_dev_t dev,
                                                 uint32_t *cells);

/**
 * @brief Get status of ingress global threshold usage
 * This determines the usage of global threshold limit.
 *
 *
 * Related APIs:  p4_pd_tm_enable_ingress_buffer_limit(),
 *		  p4_pd_tm_disable_ingress_buffer_limit()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] state           enable/disble
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ingress_buffer_limit_state(p4_pd_tm_dev_t dev,
                                                       bool *state);

/**
 * @brief Get Ingress port limit.
 * When buffer usage accounted on port basis crosses the limit,
 * traffic is not admitted into traffic manager.
 *
 * Related APIs: p4_pd_tm_set_app_pool_color_drop_limit()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port Identifier
 * @param[out] cells      Limit in terms of number of cells.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ingress_port_drop_limit(p4_pd_tm_dev_t dev,
                                                    p4_pd_tm_port_t port,
                                                    uint32_t *cells);

/**
 * @brief Get Egress port limit.
 * When buffer usage accounted on port basis crosses the limit,
 * traffic will be droppped on QAC stage.
 *
 * Related APIs: p4_pd_tm_set_app_pool_color_drop_limit()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port Identifier
 * @param[out] cells      Limit in terms of number of cells.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_egress_port_drop_limit(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   uint32_t *cells);

/**
 * @brief Get port hysteresis limits.
 * When usage of cells goes below hysteresis value  port pause or drop
 * condition  will be cleared.
 *
 * Related APIs: bf_tm_port_ingress_hysteresis_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port Identifier
 * @param[out] cells      Offset Limit
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_ingress_port_hysteresis(p4_pd_tm_dev_t dev,
                                                    p4_pd_tm_port_t port,
                                                    uint32_t *cells);

/**
 * @brief Get port egress hysteresis limits.
 * When usage of cells goes below hysteresis value  port drop
 * condition  will be cleared.
 *
 * Related APIs: bf_tm_port_egress_hysteresis_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port Identifier
 * @param[out] cells      Offset Limit
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_egress_port_hysteresis(p4_pd_tm_dev_t dev,
                                                   p4_pd_tm_port_t port,
                                                   uint32_t *cells);

/**
 * @brief This API can be used to get pause type set on port.
 *
 * Related APIs: bf_tm_port_flowcontrol_mode_set()
 *
 * Default : No Pause or flow control.
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] port        Port handle.
 * @param[out] fctype       Pause type.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_get_port_flowcontrol_mode(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_flow_ctrl_type_t *fctype);

/**
 * @brief Get iCoS(iCoS = ig_intr_md.ingress_cos) to packet CoS.
 *
 * Default: No PFC
 *
 * Related APIs: bf_tm_port_pfc_cos_mapping_set()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port handle
 * @param[out] cos_to_icos  Array of 8 uint8_t values.
 *                          Array index is CoS and array value is iCoS.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 *
 */

p4_pd_status_t p4_pd_tm_get_port_pfc_cos_mapping(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 uint8_t *cos_map);
/**
 * @brief Total Traffic Manager buffer capability measured in byte count.
 *
 * Related API : bf_tm_convert_bytes_to_cell()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[out] buffer_size  TM buffering capacity measured in bytes.
 *                          Application SW can use this API to get TM
 *                          buffer size to plan buffer carving.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_total_buffer_size(p4_pd_tm_dev_t dev,
                                              uint64_t *buffer_size);

/**
 * @brief Get cells size in bytes.
 *
 * Related API : bf_tm_total_cell_count_get()
 *
 * @param[in] dev           ASIC device identifier.
 * @param[out] cell_size    Cell size in bytes.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_cell_size_in_bytes(p4_pd_tm_dev_t dev,
                                               uint32_t *cell_size);

/**
 * @brief Total Traffic Manager buffer capability measured in cell count.
 * Get total cell count supported by ASIC.
 *
 * Related API : bf_tm_convert_bytes_to_cell()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[out] total_cells    TM buffering capacity measured in cell count.
 *                            Application SW can use this API to get TM
 *                            buffer size to plan buffer carving.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_cell_count(p4_pd_tm_dev_t dev,
                                       uint32_t *total_cells);

/**
 * @briefEnable token bucket that assures queue shaping rate (pps or bps)
 *
 * Default: Queue shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_q_max_shaping_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_max_rate_shaper_enable(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_queue_t queue);

/**
 * @brief Disable token bucket that assures queue shaping rate (pps or bps)
 *
 * Default: Queue shaping rate is enabled
 *
 * Related APIs: bf_tm_sched_q_max_shaping_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_max_rate_shaper_disable(p4_pd_tm_dev_t dev,
                                                  p4_pd_tm_port_t port,
                                                  p4_pd_tm_queue_t queue);

/**
 * @brief Enable token bucket that assures queue guaranteed rate (pps or bps)
 *
 * Default: Queue guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_disable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_min_rate_shaper_enable(p4_pd_tm_dev_t dev,
                                                 p4_pd_tm_port_t port,
                                                 p4_pd_tm_queue_t queue);

/**
 * @brief Disable token bucket that assures queue guaranteed rate (pps or bps)
 *
 * Default: Queue guaranteed shaping rate is disabled
 *
 * Related APIs: bf_tm_sched_q_guaranteed_rate_enable()
 *
 * @param[in] dev             ASIC device identifier.
 * @param[in] port            Port
 * @param[in] queue           Queue
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_min_rate_shaper_disable(p4_pd_tm_dev_t dev,
                                                  p4_pd_tm_port_t port,
                                                  p4_pd_tm_queue_t queue);

/**
 * Set number of Global timestamp bits that is to be right shifted.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] shift         Number of Global timestamp bits that are
 *                          right shifted.
 *                          Upto 16bits can be right shifted. Any shift value
 *                          greater than 16 is capped to 16.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_set_timestamp_shift(p4_pd_tm_dev_t dev,
                                            uint8_t ts_shift);

/**
 * @brief Get number of Global timestamp bits that are right shifted.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[out] shift        Number of Global timestamp bits that are
 *                          right shifted.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_get_timestamp_shift(p4_pd_tm_dev_t dev,
                                            uint8_t *ts_shift);

/**
 * Get per ppg drop count cached 64 bit counter to account for the
 * counter wrap.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] pipe          Pipe Identifier.
 * @param[in] ppg           Ppg identifier.
 * @param[out] count        Counts number of dropped packet.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_ppg_drop_get(p4_pd_tm_dev_t dev,
                                     p4_pd_tm_pipe_t pipe,
                                     p4_pd_tm_ppg_t ppg,
                                     uint64_t *count);

/**
 * Clear per ppg drop count.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] ppg           PPG identifier.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_ppg_drop_count_clear(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_ppg_t ppg);

/**
 * Get per queue drop count cached 64 bit counter to account for the
 * counter wrap.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] pipe          Pipe Identifier.
 * @param[in] port          Port identifier.
 * @param[in] queue         Queue identifier behind port.
 * @param[out] count        Counts number of packet.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_drop_get(p4_pd_tm_dev_t dev,
                                   p4_pd_tm_pipe_t pipe,
                                   p4_pd_tm_port_t port,
                                   p4_pd_tm_queue_t queue,
                                   uint64_t *count);

/**
 * Clear per queue drop count
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port identifier.
 * @param[in] queue         Queue identifier behind port.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_drop_count_clear(p4_pd_tm_dev_t dev,
                                           p4_pd_tm_port_t port,
                                           p4_pd_tm_queue_t queue);

/**
 * Get Pool usage counters. Valid pools are application/shared pool,
 * negative mirror pool and skid pool. There is no water mark support
 * for negative mirror pool. In negative mirror pool case water mark
 * value will be zero all the time.
 * Water mark value indicates maximum usage ever reached
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pool_id     Pool identifier.
 * @param[out] count      Pool usage count in cells.
 * @param[out] wm         Water mark value in units of cell.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_pool_usage_get(p4_pd_tm_dev_t dev,
                                       p4_pd_pool_id_t pool_id,
                                       uint32_t *count,
                                       uint32_t *wm);

/**
 * Clear shared AP pool watermark counter
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pool_id     Pool identifier.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_pool_watermark_clear(p4_pd_tm_dev_t dev,
                                             p4_pd_pool_id_t pool_id);

/**
 * Get per port drop count 64 bit counter to account for the
 * counter wrap.
 * On Ingress, if packet is dropped when usage crosses PPG or
 * or Port drop limit, this counter gets incremented.
 * On Egress, queue tail drop are also accounted against port.
 *
 * @param[in] dev            ASIC device identifier.
 * @param[in] pipe           Pipe Identifier.
 * @param[in] port           Port identifier.
 * @param[out] ig_count      Per port Packet drops from Ingress TM perspective.
 * @param[out] eg_count      Per port Packet drops from Egress TM perspective.
 * @return                   Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_drop_get(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_pipe_t pipe,
                                      p4_pd_tm_port_t port,
                                      uint64_t *ig_count,
                                      uint64_t *eg_count);

/**
 * Clear per port drop count.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port identifier.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_drop_count_clear(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_port_t port);

/**
 * Clear per port ingress_drop count.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port identifier.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_ingress_drop_count_clear(p4_pd_tm_dev_t dev,
                                                      p4_pd_tm_port_t port);

/**
 * Clear per port egress_drop count.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port identifier.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_egress_drop_count_clear(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_port_t port);

/**
 * Get PPG usage count.
 * Water mark value indicates maximum usage ever reached.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] pipe          Pipe Identifier.
 * @param[in] ppg           PPG identifier.
 * @param[out] gmin_count   Cell inuse from gmin pool.
 * @param[out] shared_count Cell inuse from shared pool.
 * @param[out] skid_count   Cell inuse from skid pool.
 * @param[out] wm           Water mark value in units of cell.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_ppg_usage_get(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_pipe_t pipe,
                                      p4_pd_tm_ppg_t ppg,
                                      uint32_t *gmin_count,
                                      uint32_t *shared_count,
                                      uint32_t *skid_count,
                                      uint32_t *wm);

/**
 * Clear PPG watermark counter.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] ppg        PPG identifier.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_ppg_watermark_clear(p4_pd_tm_dev_t dev,
                                            p4_pd_tm_ppg_t ppg);

/**
 * Get queue usage count
 * Water mark value indicates maximum usage ever reached
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] pipe          Pipe Identifier.
 * @param[in] port          Port identifier.
 * @param[in] queue         Queue identifier behind port.
 * @param[out] count         Cell inuse .
 * @param[out] wm            Water mark value in units of cell.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_usage_get(p4_pd_tm_dev_t dev,
                                    p4_pd_tm_pipe_t pipe,
                                    p4_pd_tm_port_t port,
                                    p4_pd_tm_queue_t queue,
                                    uint32_t *count,
                                    uint32_t *wm);

/**
 * Clear queue watermark counter.
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] port          Port identifier.
 * @param[in] queue         Queue identifier behind port.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_watermark_clear(p4_pd_tm_dev_t dev,
                                          p4_pd_tm_port_t port,
                                          p4_pd_tm_queue_t queue);

/**
 * Get port current usage count in units of cells.
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pipe        Pipe Identifier.
 * @param[in] port        Port identifier.
 * @param[out] ig_count   Port usage count in cells from Ingress TM perspective.
 * @param[out] eg_count   Port usage count in cells from Egress TM perspective.
 * @param[out] ig_wm      Watermark of port in units of cell from
 *                        Ingress TM view point.
 * @param[out] eg_wm      Watermark of port in units of cell from
 *                        Egress TM view point.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_usage_get(p4_pd_tm_dev_t dev,
                                       p4_pd_tm_pipe_t pipe,
                                       p4_pd_tm_port_t port,
                                       uint32_t *ig_count,
                                       uint32_t *eg_count,
                                       uint32_t *ig_wm,
                                       uint32_t *eg_wm);

/**
 * Clear port watermark counter.
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] port       port identifier.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_port_watermark_clear(p4_pd_tm_dev_t dev,
                                             p4_pd_tm_port_t port);

/**
 * Get blocklevel drop counters for TM.
 * Blocklevel Drops, Error, or Discard Counters
 *
 * @param[in] dev           ASIC device identifier.
 * @param[in] pipe          Pipe Identifier.
 * @param[out] blk_cntrs    Block Level Counters
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_blklvl_drop_get(p4_pd_tm_dev_t dev,
                                        p4_pd_tm_pipe_t pipe,
                                        p4_pd_tm_blklvl_cntrs_t *blk_cntrs);

/**
 * Get PRE FIFO drop counters for TM.
 * PRE-FIFO Drops, Error, or Discard Counters
 *
 * @param[in] dev             ASIC device identifier.
 * @param[out] fifo_cntrs     PRE FIFO Level Counters.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_pre_fifo_drop_get(
    p4_pd_tm_dev_t dev, p4_pd_tm_pre_fifo_cntrs_t *fifo_cntrs);

/**
 * Stop timer to poll counters for TM.
 *
 * @param[in] dev       ASIC device identifier.
 * @return              Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_stop_cache_counters_timer(p4_pd_tm_dev_t dev);

/**
 * Start timer to poll counters for TM.
 *
 * @param[in] dev           ASIC device identifier.
 * @return                  Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_start_cache_counters_timer(p4_pd_tm_dev_t dev);

// TM Queue Stats reporting (QSTAT) API

/**
 * @brief Set Queue Stats reporting visibility.
 * "Queue Stats Reporting" (QSTAT) feature: Visible queues report its depth
 * changes to ingress MAU depending on what reporting mode is set at the queue's
 * egress pipe.
 *
 * Default : False
 *
 * Related APIs: bf_tm_q_visible_get()
 *               bf_tm_qstat_report_mode_get()
 *
 * param[in] dev        ASIC device identifier.
 * param[in] port       Port handle.
 * param[in] queue      Queue for port.
 * param[in] visible    QSTAT reporting visibility of the queue.
 * return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_visible_set(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_port_t port,
                                      p4_pd_tm_queue_t queue,
                                      bool visible);

/**
 * @brief Get Queue Stats reporting visibility.
 * "Queue Stats Reporting" (QSTAT) feature: Visible queues report its depth
 * changes to ingress MAU depending on what reporting mode is set at the queue's
 * egress pipe.
 *
 * Default : False
 *
 * Related APIs: bf_tm_q_visible_set()
 *               bf_tm_qstat_report_mode_set()
 *
 * param[in] dev            ASIC device identifier.
 * param[in] port           Port handle.
 * param[in] queue          Queue for port.
 * param[out] visible       QSTAT reporting visibility of the queue.
 * return                   Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_q_visible_get(p4_pd_tm_dev_t dev,
                                      p4_pd_tm_port_t port,
                                      p4_pd_tm_queue_t queue,
                                      bool *visible);

/**
 * @brief Set Egress Pipe Queue Stats Reporting (QSTAT) mode
 * Only QSTAT visible queues at the pipe are participating in reporting.
 * False: Trigger QSTAT reporting on Q color threshold crosses.
 * True: Trigger QSTAT reporting on any Q depth updates.
 *
 * Default: 0
 *
 * Related APIs: bf_tm_q_visible_set()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[in] mode       Queue statistics reporting mode.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_qstat_report_mode_set(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_pipe_t pipe,
                                              bool mode);

/**
 * @brief Get Egress Pipe Queue Stats Reporting (QSTAT) mode
 * Only QSTAT visible queues at the pipe are participating in reporting.
 * False: Trigger QSTAT reporting of Q color threshold crosses.
 * True: Trigger QSTAT reporting of any Q depth updates.
 *
 * Default: 0
 *
 * Related APIs: bf_tm_q_visible_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[out] mode      Queue statistics reporting mode.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_qstat_report_mode_get(p4_pd_tm_dev_t dev,
                                              p4_pd_tm_pipe_t pipe,
                                              bool *mode);

/**
 * @brief Set queue scheduler advanced flow control mode.
 * Scheduler Advanced Flow Control Mechanism, 0 = Credit 1 = Xoff
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_q_adv_fc_mode_get()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[in] mode                  Scheduler Advanced Flow Control Mode
 *                                  0 = credit 1 = xoff.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_q_adv_fc_mode_set(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    p4_pd_tm_sched_adv_fc_mode_t mode);

/**
 * @brief Get queue scheduler advanced flow control mode.
 * Scheduler Advanced Flow Control Mechanism, 0 = Credit 1 = Xoff
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_q_adv_fc_mode_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] port                  Port
 * @param[in] queue                 Queue
 * @param[out] mode                 Scheduler Advanced Flow Control Mode
 *                                  0 = credit 1 = xoff.
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_q_adv_fc_mode_get(
    p4_pd_tm_dev_t dev,
    p4_pd_tm_port_t port,
    p4_pd_tm_queue_t queue,
    p4_pd_tm_sched_adv_fc_mode_t *mode);

/**
 * @brief Sets scheduler advanced flow control mode enable/disable mode.
 * used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_adv_fc_mode_enable_get()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] pipe                  Logical PipeId index
 * @param[in] enable                Scheduler Advanced Flow Control Mode
 *                                  Enable/Disable
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_adv_fc_mode_enable_set(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_pipe_t pipe,
                                                     bool enable);

/**
 * @brief Get scheduler advanced flow control mode enable/disable mode.
 * Used for TM Visibility Implementation
 *
 * Related APIs: bf_tm_sched_adv_fc_mode_enable_set()
 *
 * @param[in] dev                   ASIC device identifier.
 * @param[in] pipe                  Logical PipeId index
 * @param[out] enable               Scheduler Advanced Flow Control Mode
 *                                  Enable/Disable
 * @return                          Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */

p4_pd_status_t p4_pd_tm_sched_adv_fc_mode_enable_get(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_pipe_t pipe,
                                                     bool *enable);
/**
 * @brief Get TM device-specific settings.
 *
 * @param[in]  dev            ASIC device identifier.
 * @param[out] cfg            Config.
 * @return                    Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
p4_pd_status_t p4_pd_tm_dev_config_get(p4_pd_tm_dev_t dev,
                                       p4_pd_tm_dev_cfg_t *cfg);

/**
 * Set egress Pipe deflection port enable mode
 *
 * Default: true
 *
 * Related APIs: bf_tm_pipe_deflection_port_enable_get()
 *
 * @param[in] dev        ASIC device identifier.
 * @param[in] pipe       Pipe Identifier.
 * @param[in] enable     Deflection mode enable status.
 * @return               Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
p4_pd_status_t pd_tm_pipe_deflection_port_enable_set(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_pipe_t pipe,
                                                     bool enable);

/**
 * Get egress Pipe deflection port enable mode
 *
 * Default: true
 *
 * Related APIs: bf_tm_pipe_deflection_port_enable_set()
 *
 * @param[in] dev         ASIC device identifier.
 * @param[in] pipe        Pipe Identifier.
 * @param[out] enable     Deflection mode enable status.
 * @return                Status of API call.
 *  BF_SUCCESS on success
 *  Non-Zero on error
 */
p4_pd_status_t pd_tm_pipe_deflection_port_enable_get(p4_pd_tm_dev_t dev,
                                                     p4_pd_tm_pipe_t pipe,
                                                     bool *enable);

/* @} */

#endif
