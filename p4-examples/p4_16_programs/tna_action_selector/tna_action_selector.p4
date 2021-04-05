/* -*- P4_16 -*- */

/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */



#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "common/headers.p4"
#include "common/util.p4"


struct metadata_t {
    bit<16> md;
}

// ---------------------------------------------------------------------------
// Ingress parser
// ---------------------------------------------------------------------------
parser SwitchIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t ig_md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    TofinoIngressParser() tofino_parser;

    state start {
        tofino_parser.apply(pkt, ig_intr_md);
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select (hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : reject;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }
}

// ---------------------------------------------------------------------------
// Ingress Deparser
// ---------------------------------------------------------------------------
control SwitchIngressDeparser(
         packet_out pkt,
         inout header_t hdr,
         in metadata_t ig_md,
         in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {

    apply {
        pkt.emit(hdr);
    }
}

control SwitchIngress(
        inout header_t hdr,
        inout metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    Hash<bit<16>>(HashAlgorithm_t.CRC16) sel_hash;
    ActionProfile(2048) example_action_selector_ap;
    ActionSelector(example_action_selector_ap, // action profile
                   sel_hash, // hash extern
                   SelectorMode_t.FAIR, // Selector algorithm
                   200, // max group size
                   100 // max number of groups
                   ) example_action_selector;

    action hit(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
    }

    action set_md(bit<16> idx) {
        ig_md.md = idx;
    }

    action miss() {
        ig_dprsr_md.drop_ctl = 0x1; // Drop packet.
    }

    table forward {
        key = {
            ig_intr_md.ingress_port : exact;
            hdr.ethernet.src_addr : selector;
            hdr.ethernet.dst_addr : selector;
            hdr.ipv4.src_addr : selector;
            hdr.ipv4.dst_addr : selector;
        }

        actions = {
            hit;
            set_md;
            miss;
        }

        const default_action = miss;
        size = 512;
        implementation = example_action_selector;
    }

    table set_dest {
        key = { ig_md.md : exact; }
        actions = { hit; }
        size = 4096;
    }

    apply {
        forward.apply();
        set_dest.apply();

        // No need for egress processing, skip it and use empty controls for egress.
        ig_tm_md.bypass_egress = 1w1;
    }
}

Pipeline(SwitchIngressParser(),
         SwitchIngress(),
         SwitchIngressDeparser(),
         EmptyEgressParser(),
         EmptyEgress(),
         EmptyEgressDeparser()) pipe;

Switch(pipe) main;
