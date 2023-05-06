/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _BF_TYPES_H
#define _BF_TYPES_H

#ifndef __KERNEL__
#include <stdint.h>
#include <stdbool.h>
#else
#include <bf_types/bf_kernel_types.h>
#endif

/**
 * @file bf_types.h
 * \brief Details bf specific types.
 */
/**
 * @addtogroup bf_types-bf_types
 * @{
 */

/*! Identifies an ASIC in the system. */
typedef int bf_dev_id_t;

/*! Identifies a specific die within an ASIC in the system. */
typedef int bf_subdev_id_t;

/** Identifies a port on an ASIC.  This is a 9-bit field where the upper two
 *  bits identify the pipeline and the lower 7 bits identify the port number
 *  local to that pipeline.  The valid range for the lower 7 bits is 0-71. */
typedef int bf_dev_port_t;

/** Identifies a pipe on an ASIC.  This is a 2-bit field where the bits identify
 * pipeline.
 */
typedef uint32_t bf_dev_pipe_t;

/** Identifies an invalid value for a pipeline on an ASIC. */
#define BF_INVALID_PIPE 0xFFFFFFFF

/** Identifies a pipeline on an ASIC.  Note dev_pipe_id can be set to
 *  BF_DEV_PIPE_ALL as a special value to indicate "all pipelines". */
#define BF_DEV_PIPE_ALL 0xFFFF

typedef struct bf_dev_target_t {
  bf_dev_id_t device_id;
  bf_dev_pipe_t dev_pipe_id;
} bf_dev_target_t;

/** Strcture for direction definition
 */
typedef enum {
  BF_DEV_DIR_INGRESS = 0,
  BF_DEV_DIR_EGRESS = 1,
  BF_DEV_DIR_ALL = 0xff
} bf_dev_direction_t;

/** Wildcard to specify all parser engines in a pipe */
#define BF_DEV_PIPE_PARSER_ALL 0xFF

/* Type definition for port speeds */
typedef enum bf_port_speed_e {
  BF_SPEED_NONE = 0,
  BF_SPEED_1G = (1 << 0),
  BF_SPEED_10G = (1 << 1),
  BF_SPEED_25G = (1 << 2),
  BF_SPEED_40G = (1 << 3),
  BF_SPEED_50G = (1 << 5),
  BF_SPEED_100G = (1 << 6),

  // Tof2 speeds
  BF_SPEED_200G = (1 << 7),
  BF_SPEED_400G = (1 << 8),
  BF_SPEED_40G_R2 = (1 << 9),    /* 40G 2x20G NRZ, Non-standard speed */
  BF_SPEED_50G_CONS = (1 << 10), /* 50G 2x25G NRZ, Consortium mode */
  BF_SPEED_800G = (1 << 11),
} bf_port_speed_t;
static inline const char *bf_port_speed_str(bf_port_speed_t e) {
  switch (e) {
    case BF_SPEED_NONE:
      return "None";
    case BF_SPEED_1G:
      return "1G";
    case BF_SPEED_10G:
      return "10G";
    case BF_SPEED_25G:
      return "25G";
    case BF_SPEED_40G:
      return "40G";
    case BF_SPEED_50G:
      return "50G";
    case BF_SPEED_100G:
      return "100G";
    case BF_SPEED_200G:
      return "200G";
    case BF_SPEED_400G:
      return "400G";
    case BF_SPEED_40G_R2:
      return "40G-R2";
    case BF_SPEED_50G_CONS:
      return "50G-Cons";
    case BF_SPEED_800G:
      return "800G";
  }
  return "Unknown";
}
typedef uint32_t bf_port_speeds_t;

/* Type definition for serdes mode */
typedef enum bf_port_serdes_mode_e {
  BF_SERDES_MODE_56G = 0,
  BF_SERDES_MODE_112G,
  BF_SERDES_MODE_MAX
} bf_port_serdes_mode_t;

typedef uint32_t bf_port_serdes_modes_t;

#define BF_ADV_SPEEDS_SET(x, speed) (x |= speed)
#define BF_ADV_SPEED_CLR(x, speed) (x &= ~speed)
#define BF_IS_ADV_SPEED_SET(x, speed) ((x & speed) ? true : false)

typedef enum bf_fec_type_e {
  BF_FEC_TYP_NONE = 0,
  BF_FEC_TYP_FIRECODE = (1 << 0),
  BF_FEC_TYP_REED_SOLOMON = (1 << 1),
  BF_FEC_TYP_REED_SOLOMON_INTERLEAVED = (1 << 2),
  BF_FEC_TYP_REED_SOLOMON_KL = (1 << 3),
  BF_FEC_TYP_FC = BF_FEC_TYP_FIRECODE,
  BF_FEC_TYP_RS = BF_FEC_TYP_REED_SOLOMON,
  BF_FEC_TYP_RS_IN = BF_FEC_TYP_REED_SOLOMON_INTERLEAVED,
  BF_FEC_TYP_RS_KL = BF_FEC_TYP_REED_SOLOMON_KL,
} bf_fec_type_t;
static inline const char *bf_fec_type_str(bf_fec_type_t fec) {
  if (fec == BF_FEC_TYP_NONE) return "BF_FEC_TYP_NONE";
  if (fec == BF_FEC_TYP_FIRECODE) return "BF_FEC_TYP_FIRECODE";
  if (fec == BF_FEC_TYP_REED_SOLOMON) return "BF_FEC_TYP_REED_SOLOMON";
  if (fec == BF_FEC_TYP_REED_SOLOMON_INTERLEAVED)
    return "BF_FEC_TYP_REED_SOLOMON_INTERLEAVED";
  if (fec == BF_FEC_TYP_REED_SOLOMON_KL) return "BF_FEC_TYP_REED_SOLOMON_KL";
  return "UNKNOWN FEC TYPE";
}
typedef uint32_t bf_fec_types_t;

#define BF_ADV_FECS_SET(x, fec) (x |= fec)
#define BF_ADV_FECS_CLR(x, fec) (x &= ~fec)
#define BF_IS_ADV_FECS_SET(x, fec) ((x & fec) ? true : false)

typedef enum bf_pause_cfg_e {
  BF_PAUSE_CFG_ENABLE = 0,
  BF_PAUSE_CFG_DISABLE,
} bf_pause_cfg_t;

/*Indentifies the Media type inserted/detected on a port*/
typedef enum bf_media_type_e {
  BF_MEDIA_TYPE_COPPER = 0,
  BF_MEDIA_TYPE_OPTICAL,
  BF_MEDIA_TYPE_UNKNOWN
} bf_media_type_t;

/** Identifies an error code. */
typedef int bf_status_t;

#define BF_STATUS_VALUES                                                    \
  BF_STATUS_(BF_SUCCESS, "Success"), BF_STATUS_(BF_NOT_READY, "Not ready"), \
      BF_STATUS_(BF_NO_SYS_RESOURCES, "No system resources"),               \
      BF_STATUS_(BF_INVALID_ARG, "Invalid arguments"),                      \
      BF_STATUS_(BF_ALREADY_EXISTS, "Already exists"),                      \
      BF_STATUS_(BF_HW_COMM_FAIL, "HW access fails"),                       \
      BF_STATUS_(BF_OBJECT_NOT_FOUND, "Object not found"),                  \
      BF_STATUS_(BF_MAX_SESSIONS_EXCEEDED, "Max sessions exceeded"),        \
      BF_STATUS_(BF_SESSION_NOT_FOUND, "Session not found"),                \
      BF_STATUS_(BF_NO_SPACE, "Not enough space"),                          \
      BF_STATUS_(BF_EAGAIN,                                                 \
                 "Resource temporarily not available, try again later"),    \
      BF_STATUS_(BF_INIT_ERROR, "Initialization error"),                    \
      BF_STATUS_(BF_TXN_NOT_SUPPORTED, "Not supported in transaction"),     \
      BF_STATUS_(BF_TABLE_LOCKED, "Resource held by another session"),      \
      BF_STATUS_(BF_IO, "IO error"),                                        \
      BF_STATUS_(BF_UNEXPECTED, "Unexpected error"),                        \
      BF_STATUS_(BF_ENTRY_REFERENCES_EXIST,                                 \
                 "Action data entry is being referenced by match entries"), \
      BF_STATUS_(BF_NOT_SUPPORTED, "Operation not supported"),              \
      BF_STATUS_(BF_HW_UPDATE_FAILED, "Updating hardware failed"),          \
      BF_STATUS_(BF_NO_LEARN_CLIENTS, "No learning clients registered"),    \
      BF_STATUS_(BF_IDLE_UPDATE_IN_PROGRESS,                                \
                 "Idle time update state already in progress"),             \
      BF_STATUS_(BF_DEVICE_LOCKED, "Device locked"),                        \
      BF_STATUS_(BF_INTERNAL_ERROR, "Internal error"),                      \
      BF_STATUS_(BF_TABLE_NOT_FOUND, "Table not found"),                    \
      BF_STATUS_(BF_IN_USE, "In use"),                                      \
      BF_STATUS_(BF_NOT_IMPLEMENTED, "Object not implemented"),             \
      BF_STATUS_(BF_NO_HW_ENTRY, "Hardware entry not found")
enum bf_status_enum {
#define BF_STATUS_(x, y) x
  BF_STATUS_VALUES,
  BF_STS_MAX
#undef BF_STATUS_
};
static const char *bf_err_strings[BF_STS_MAX + 1] = {
#define BF_STATUS_(x, y) y
    BF_STATUS_VALUES, "Unknown error"
#undef BF_STATUS_
};
static inline const char *bf_err_str(bf_status_t sts) {
  if (BF_STS_MAX <= sts || 0 > sts) {
    return bf_err_strings[BF_STS_MAX];
  } else {
    return bf_err_strings[sts];
  }
}

/** The max number of devices in the domain of the driver */
#define BF_MAX_DEV_COUNT 8
/** The max number of sub devices in the device */
#define BF_MAX_SUBDEV_COUNT 2
/** The number of pipes in the ASIC per subdev. */
#define BF_SUBDEV_PIPE_COUNT 4
/** The number of pipes in the ASIC. */
#define BF_PIPE_COUNT (2 * BF_SUBDEV_PIPE_COUNT)
/** The number of ports per pipe in the ASIC. */
#define BF_PIPE_PORT_COUNT 72
/** The number of ports per subdev */
#define BF_SUBDEV_PORT_COUNT (BF_PIPE_PORT_COUNT * BF_SUBDEV_PIPE_COUNT)
/** The number of ports in the ASIC. */
#define BF_PORT_COUNT (BF_PIPE_PORT_COUNT * BF_PIPE_COUNT)
/** The number of LAGs in the ASIC. */
#define BF_LAG_COUNT 256
/** The number of Multicast Group Ids in the ASIC. */
#define BF_MGID_COUNT (64 * 1024)
/** The number of fifos per PRE in the ASIC. */
#define BF_PRE_FIFO_COUNT 4

static inline bool bf_dev_id_validate(bf_dev_id_t dev_id) {
  return dev_id >= 0 && dev_id < BF_MAX_DEV_COUNT;
}

#define DEV_PORT_TO_PIPE(x) (((x) >> 7) & 7)
#define DEV_PORT_TO_LOCAL_PORT(x) ((x)&0x7F)
#define MAKE_DEV_PORT(pipe, port) (((pipe) << 7) | (port))
#define DEV_PORT_VALIDATE(x) ((((x)&0x7F) < 72) && !((x) & ~0x3FF))
#define LOCAL_PORT_VALIDATE(x) ((x) < 72)
#define TM_IS_PRE_FIFO_INVALID(fifo) (fifo >= BF_PRE_FIFO_COUNT)
/**
 * @}
 */

/* RMT device type */
typedef enum bf_dev_type_t {
  BF_DEV_UNKNOWN,
  BF_DEV_BFNT10064Q,  // TOF1
  BF_DEV_BFNT10032Q,
  BF_DEV_BFNT10032D,
  BF_DEV_BFNT10032D012,
  BF_DEV_BFNT10032D018,
  BF_DEV_BFNT10032D020,
  BF_DEV_BFNT20128Q,  // TOF2
  BF_DEV_BFNT20128QM,
  BF_DEV_BFNT20080T,
  BF_DEV_BFNT20080TM,
  BF_DEV_BFNT20064Q,
  BF_DEV_BFNT20064D,
  BF_DEV_BFNT31_12Q,  // TOF3
  BF_DEV_BFNT31_112_12Q,
  BF_DEV_BFNT31_12QH,
  BF_DEV_BFNT32_25Q,
  BF_DEV_BFNT32_112_25Q,
  BF_DEV_BFNT32_25QH,



  BF_DEV_TYPE_MAX
} bf_dev_type_t;

static inline const char *pipe_mgr_dev_type2str(bf_dev_type_t dev_type) {
  switch (dev_type) {
    case BF_DEV_BFNT10064Q:
      return "BFN-T10-064Q";
    case BF_DEV_BFNT10032Q:
      return "BFN-T10-032Q";
    case BF_DEV_BFNT10032D:
      return "BFN-T10-032D";
    case BF_DEV_BFNT10032D018:
      return "BFN-T10-032D-018";
    case BF_DEV_BFNT10032D020:
      return "BFN-T10-032D-020";
    case BF_DEV_BFNT10032D012:
      return "BFN-T10-032D-012";
    case BF_DEV_BFNT20128Q:
      return "BFN-T20-128Q";
    case BF_DEV_BFNT20128QM:
      return "BFN-T20-128QM";
    case BF_DEV_BFNT20080T:
      return "BFN-T20-080T";
    case BF_DEV_BFNT20080TM:
      return "BFN-T20-080TM";
    case BF_DEV_BFNT20064Q:
      return "BFN-T20-064Q";
    case BF_DEV_BFNT20064D:
      return "BFN-T20-064D";
    case BF_DEV_BFNT31_12Q:
      return "BFN-T31-12Q";
    case BF_DEV_BFNT31_112_12Q:
      return "BFN-T31-112-12Q";
    case BF_DEV_BFNT31_12QH:
      return "BFN-T31-12QH";
    case BF_DEV_BFNT32_25Q:
      return "BFN-T32-25Q";
    case BF_DEV_BFNT32_112_25Q:
      return "BFN-T32-112-25Q";
    case BF_DEV_BFNT32_25QH:
      return "BFN-T32-25QH";




    default:
      return "UNKNOWN";
  }
}

typedef enum bf_dev_family_t {
  BF_DEV_FAMILY_TOFINO,
  BF_DEV_FAMILY_TOFINO2,
  BF_DEV_FAMILY_TOFINO3,

  BF_DEV_FAMILY_UNKNOWN,
} bf_dev_family_t;

static inline const char *bf_dev_family_str(bf_dev_family_t fam) {
  switch (fam) {
    case BF_DEV_FAMILY_TOFINO:
      return "Tofino";
    case BF_DEV_FAMILY_TOFINO2:
      return "Tofino2";
    case BF_DEV_FAMILY_TOFINO3:
      return "Tofino3";


    case BF_DEV_FAMILY_UNKNOWN:
      return "Unknown";
  }
  return "Unknown";
}

static inline bool bf_is_dev_type_family_tofino(bf_dev_type_t t) {
  return t >= BF_DEV_BFNT10064Q && t <= BF_DEV_BFNT10032D020;
}

static inline bool bf_is_dev_type_family_tofino2(bf_dev_type_t t) {
  return t >= BF_DEV_BFNT20128Q && t <= BF_DEV_BFNT20064D;
}

static inline bool bf_is_dev_type_family_tofino3(bf_dev_type_t t) {
  return t >= BF_DEV_BFNT31_12Q && t <= BF_DEV_BFNT32_25QH;
}





static inline bf_dev_family_t bf_dev_type_to_family(bf_dev_type_t t) {
  if (bf_is_dev_type_family_tofino(t)) return BF_DEV_FAMILY_TOFINO;
  if (bf_is_dev_type_family_tofino2(t)) return BF_DEV_FAMILY_TOFINO2;
  if (bf_is_dev_type_family_tofino3(t)) return BF_DEV_FAMILY_TOFINO3;

  return BF_DEV_FAMILY_UNKNOWN;
}

/* Two macros to convert between bf_dev_port_t and a bit index in the range of
 * 0-287. */
#define DEV_PORT_TO_BIT_IDX(dp) \
  (72 * DEV_PORT_TO_PIPE(dp) + DEV_PORT_TO_LOCAL_PORT(dp))
#define BIT_IDX_TO_DEV_PORT(bi) (MAKE_DEV_PORT((bi) / 72, (bi) % 72))
#define BIT_IDX_VALIDATE(bi) (bi < BF_PORT_COUNT)

typedef uint16_t bf_mirror_id_t;
#define BIT_IDX_TO_PIPE(x) (DEV_PORT_TO_PIPE(BIT_IDX_TO_DEV_PORT((x))))
#define BIT_IDX_TO_LOCAL_PORT(x) \
  (DEV_PORT_TO_LOCAL_PORT(BIT_IDX_TO_DEV_PORT((x))))

typedef enum bf_dev_flag_e {
  BF_DEV_IS_SW_MODEL = (1 << 0),
  BF_DEV_IS_VIRTUAL_DEV_SLAVE = (1 << 1),
} bf_dev_flag_t;

typedef uint32_t bf_dev_flags_t;

#define BF_DEV_IS_SW_MODEL_GET(flags) \
  ((flags & BF_DEV_IS_SW_MODEL) ? true : false)
#define BF_DEV_IS_SW_MODEL_SET(flags, value) \
  (flags |= (value & BF_DEV_IS_SW_MODEL))

#define BF_DEV_IS_VIRTUAL_DEV_SLAVE_GET(flags) \
  ((flags & BF_DEV_IS_VIRTUAL_DEV_SLAVE) ? true : false)
#define BF_DEV_IS_VIRTUAL_DEV_SLAVE_SET(flags) \
  (flags |= BF_DEV_IS_VIRTUAL_DEV_SLAVE)

#endif
