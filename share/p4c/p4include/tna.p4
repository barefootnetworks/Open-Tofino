/* -*- P4_16 -*- */

/**
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#if __TARGET_TOFINO__ == 1
#include "tofino1arch.p4"
#else
#error Target does not support tofino native architecture
#endif
