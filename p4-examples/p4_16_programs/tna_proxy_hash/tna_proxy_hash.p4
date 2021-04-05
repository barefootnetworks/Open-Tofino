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

struct metadata_t { 
}

#include "common/headers.p4"
#include "common/util.p4"


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
        transition select(hdr.ethernet.ether_type) {
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
control SwitchIngressDeparser(packet_out pkt,
                              inout header_t hdr,
                              in metadata_t ig_md,
                              in ingress_intrinsic_metadata_for_deparser_t 
                                ig_intr_dprsr_md
                              ) {

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

    action set_output_port(PortId_t port_id) {
        ig_tm_md.ucast_egress_port = port_id;
    }

    // match table without proxy hash
    table ipv4_match_regular {
        key = {
            hdr.ethernet.dst_addr: exact;
            hdr.ethernet.src_addr: exact;
            hdr.ipv4.dst_addr: exact;
            hdr.ipv4.src_addr: exact;
        }

        actions = {
            NoAction;
            set_output_port;
        }

        default_action = NoAction;
        size = 1024;
    }

    // Match table with proxy hash
    table ipv4_match_proxy_hash {
        key = {
            hdr.ethernet.dst_addr: exact;
            hdr.ethernet.src_addr: exact;
            hdr.ipv4.dst_addr: exact;
            hdr.ipv4.src_addr: exact;
        }

        actions = {
            NoAction;
            set_output_port;
        }

        const default_action = NoAction;
        size = 1024;

        proxy_hash = Hash<bit<32>>(HashAlgorithm_t.CRC32);
    }

    apply {
        // Both, ipv4_match_regular and _proxy_hash are functionally equivalent.
        // The difference is that ipv4_match_proxy_hash trades a lower RAM usage
        // for a small probability of false positives.
        ipv4_match_regular.apply();
        ipv4_match_proxy_hash.apply();

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
