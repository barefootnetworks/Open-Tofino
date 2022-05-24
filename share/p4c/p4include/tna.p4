/* -*- P4_16 -*- */

/**
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


#ifndef _TOFINO1_NATIVE_ARCHITECTURE_
#define _TOFINO1_NATIVE_ARCHITECTURE_

#if   __TARGET_TOFINO__ == 1
#include "tofino1_specs.p4"
#elif __TARGET_TOFINO__ == 2
#include "tofino2_specs.p4"
#else
#error Target does not support tofino native architecture
#endif

#include "tofino1_arch.p4"

#endif /* _TOFINO1_NATIVE_ARCHITECTURE_ */
