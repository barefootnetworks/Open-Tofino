/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/*!
 * @file bf_pkt.h
 * \brief bf_pkt management
 * @date
 *
 */

#ifndef _BF_PKT_H
#define _BF_PKT_H

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup pkt_mgr-pkt
 * @{
 */

#define BF_PKT_MAX_SIZE 16000

/*!
 * Enum bf packet status
 */
typedef enum {
  PKT_SUCCESS = 0,
  PKT_NO_SYS_RESOURCES,
} pkt_status_t;

/*!
 * Structure bf packet
 */
typedef struct bf_pkt_s {
  // fields of interest to applications
  struct bf_pkt_s *next;         // pointer to a chained bf_pkt
  uint16_t pkt_size;             // current size of bf_pkt payload
  uint8_t *pkt_data;             // current pointer to bf_pkt payload
  bf_phys_addr_t phys_pkt_data;  // physical address of the bf_pkt payload
  bf_sys_dma_pool_handle_t
      hndl;  // handle of the DMA pool of the bf_pkt payload
  // fields for pkt_mgr?s internal house keeping
  int ref_cnt;      // for multi consumer of bf_pkt (future use)
  bf_dev_id_t dev;  // device the packet is allocated for
  bf_subdev_id_t
      subdev;  // subdevice (within device) the packet is allocated for
  bf_dma_type_t src_dr;  // DR pool to which its buffer belongs to
  bool bypass_padding;   // TRUE to indicate to pkt mgr to bypass padding
} bf_pkt;

/**
 * allocate a packet
 *
 * @param id
 *   chip id
 * @param pkt
 *   bf packet pointer
 * @param size
 *   buffer size
 * @param dr
 *   DR pool to use to allocate buffer (bf dma type)
 * @return
 *   0 on success, -1 on failure
 */
int bf_pkt_alloc(bf_dev_id_t id, bf_pkt **pkt, size_t size, bf_dma_type_t dr);

/**
 * copy payload data to packet
 *
 * @param pkt
 *   bf packet pointer
 * @param pkt_buf
 *   payload data
 * @param size
 *   payload data size
 * @return
 *   0 on success, -1 on failure
 */
int bf_pkt_data_copy(bf_pkt *pkt, const uint8_t *pkt_buf, uint16_t size);

/**
 * free a packet
 *
 * @param id
 *   chip id
 * @param pkt
 *   bf packet
 * @return
 *   0 on success, -1 on failure
 */
int bf_pkt_free(bf_dev_id_t id, bf_pkt *pkt);

/**
 * free all packets allocated for a device
 *
 * @param id
 *   chip id
 * @return
 *   0 on success, -1 on failure
 */
int pkt_mgr_free_dev_pkts(bf_dev_id_t id);

/**
 * Get the pkt data size of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   pkt data size of the current segment of bf packet
 */
static inline uint16_t bf_pkt_get_pkt_size(bf_pkt *p) { return (p->pkt_size); }

/**
 * Set the pkt size of the bf packet.
 *
 * @param p
 *   bf packet.
 * @param size
 *   size of buffer pointed by the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_size(bf_pkt *p, uint16_t size) {
  p->pkt_size = size;
}

/**
 * Get the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   pkt data of the current segment of bf packet
 */
static inline uint8_t *bf_pkt_get_pkt_data(bf_pkt *p) { return (p->pkt_data); }

/**
 * Set the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @param buf
 *   buffer pointed by the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_data(bf_pkt *p, uint8_t *buf) {
  p->pkt_data = buf;
}

/**
 * Get the physical addr of the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   phys address of the pkt data of the current segment of bf packet
 */
static inline bf_phys_addr_t bf_pkt_get_phys_pkt_data(bf_pkt *p) {
  return (p->phys_pkt_data);
}

/**
 * Set the physical address of the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @param phys_addr
 *   physical address of the buffer pointed by the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_phys_pkt_data(bf_pkt *p,
                                            bf_phys_addr_t phys_addr) {
  p->phys_pkt_data = phys_addr;
}

/**
 * Get the pool handle of the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   pkt data of the current segment of bf packet
 */
static inline bf_sys_dma_pool_handle_t bf_pkt_get_pkt_dma_pool_handle(
    bf_pkt *p) {
  return (p->hndl);
}

/**
 * Set the pool handle of the pkt data of the bf packet.
 *
 * @param p
 *   bf packet.
 * @param hndl
 *   pool handle of the dma pool of the pkt data of the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_dma_pool_handle(
    bf_pkt *p, bf_sys_dma_pool_handle_t hndl) {
  p->hndl = hndl;
}

/**
 * Get the  DR associated with the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   src DR of the current segment of bf packet
 */
static inline int bf_pkt_get_pkt_dr(bf_pkt *p) { return (p->src_dr); }

/**
 * Set the dev_id associated with the bf packet.
 *
 * @param p
 *   bf packet.
 * @param id
 *   dev_id associated with the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_dev(bf_pkt *p, int id) { p->dev = id; }

/**
 * Set the subdev_id associated with the bf packet.
 *
 * @param p
 *   bf packet.
 * @param subdev_id
 *   subdev_id associated with the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_subdev(bf_pkt *p, int subdev_id) {
  p->subdev = subdev_id;
}

/**
 * Get the subdev_id associated with the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   none
 */
static inline bf_subdev_id_t bf_pkt_get_pkt_subdev(bf_pkt *p) {
  return (p->subdev);
}

/**
 * Get the pkt dev of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   pkt dev_id of the current segment of bf packet
 */
static inline bf_dev_id_t bf_pkt_get_pkt_dev(bf_pkt *p) { return (p->dev); }

/**
 * Set the  DR associated with the bf packet.
 *
 * @param p
 *   bf packet.
 * @param dr
 *   DR associated with the bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_pkt_dr(bf_pkt *p, bf_dma_type_t dr) {
  p->src_dr = dr;
}

/**
 * Get the last segment of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   last segment of the given bf packet.
 */
static inline bf_pkt *bf_pkt_lastseg(bf_pkt *p) {
  bf_pkt *p2 = p;

  while (p2->next != NULL) {
    p2 = p2->next;
  }
  return p2;
}

/**
 * Get the next segment of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   next segment of the given bf packet.
 */
static inline bf_pkt *bf_pkt_get_nextseg(bf_pkt *p) {
  bf_pkt *p2 = NULL;

  if (p) {
    p2 = p->next;
  }
  return p2;
}

/**
 * Set the next segment of the bf packet.
 *
 * @param p
 *   bf packet.
 * @param next
 *   next bf packet.
 * @return
 *   none
 */
static inline void bf_pkt_set_nextseg(bf_pkt *p, bf_pkt *next) {
  if (p) {
    p->next = next;
  }
}

/**
 * Get the bypass padding state of the bf packet.
 *
 * @param p
 *   bf packet.
 * @return
 *   bypass padding state of the bf packet. True indicates to bypass padding.
 */
static inline bool bf_pkt_get_bypass_padding(bf_pkt *p) {
  return (p->bypass_padding);
}

/**
 * initialize pkt queue
 *
 * @param pkt_count
 *   number of packets to allocate in queue
 * @return
 *   success or failure
 */
pkt_status_t pkt_mgr_init_pkt_queue(int pkt_count);

/**
 * un-initialize pkt queue
 *
 * @param pkt_count
 *   number of packets to allocate in queue
 * @return
 *   none
 */
void pkt_mgr_uninit_pkt_queue(void);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_PKT_H */
