/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

#if __TARGET_TOFINO__ == 1
#include "tofino1arch.p4"
#else
#error Target does not support Tofino Native Architecture
#endif
