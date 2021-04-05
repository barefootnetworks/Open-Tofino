/* -*- P4_16 -*- */

/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */



#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "pipeline_profile_a.p4"
#include "pipeline_profile_b.p4"




// Packet comes into ingress profile_a which adds an extra custom_metadata_h
// header to the packet. The packet travels to egress profile_b, then to
// ingress profile_b and finally to egress profile_a. The custom_metadata_h
// header is striped off by egress profile_b. Value of custom_tag is modified
// as the packet travels.

Pipeline(SwitchIngressParser_a(),
         SwitchIngress_a(),
         SwitchIngressDeparser_a(),
         SwitchEgressParser_a(),
         SwitchEgress_a(),
         SwitchEgressDeparser_a()) pipeline_profile_a;

Pipeline(SwitchIngressParser_b(),
         SwitchIngress_b(),
         SwitchIngressDeparser_b(),
         SwitchEgressParser_b(),
         SwitchEgress_b(),
         SwitchEgressDeparser_b()) pipeline_profile_b;

Switch(pipeline_profile_a, pipeline_profile_b) main;
