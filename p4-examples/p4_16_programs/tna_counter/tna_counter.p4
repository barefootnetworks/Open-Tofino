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


struct metadata_t {}

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
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    // Create indirect counter
    Counter<bit<32>, PortId_t>(
        512, CounterType_t.PACKETS_AND_BYTES) indirect_counter;

    // Create direct counters
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) direct_counter;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) direct_counter_2;

    action hit(PortId_t port) {
        // Call direct counter. Note that no index parameter is required because
        // the index is implicitely generated from the entry of the the 
        // associated match table.
        direct_counter.count();
        ig_intr_tm_md.ucast_egress_port = port;
    }

    action hit_dst(PortId_t port) {
        // Call indirect counter. Note that for indirect counters an index 
        // parameter must be provided.
        indirect_counter.count(port);
        ig_intr_tm_md.ucast_egress_port = port;
        ig_intr_dprsr_md.drop_ctl = 0x0; // clear drop packet
    }

    action miss() {
        ig_intr_dprsr_md.drop_ctl = 0x1; // Drop packet.
    }
    action nop() {
    }

    table forward {
        key = {
            hdr.ethernet.src_addr : ternary;
        }

        actions = {
            hit;
            @defaultonly nop;
        }

        size = 1024;
        const default_action = nop;
        // Associate this table with a direct counter
        counters = direct_counter;
    }

    action hit_forward_exact(PortId_t port) {
        // Call direct counter. Note that no index parameter is required because
        // the index is implicitely generated from the entry of the the 
        // associated match table.
        direct_counter_2.count();
        ig_intr_tm_md.ucast_egress_port = port;
    }

    table forward_exact {
        key = {
            hdr.ethernet.src_addr : exact;
        }

        actions = {
            hit_forward_exact;
            @defaultonly nop;
        }

        size = 1024;
        const default_action = nop;
        // Associate this table with a direct counter
        counters = direct_counter_2;
    }

    table forward_dst {
        key = {
            hdr.ethernet.dst_addr : exact;
        }

        actions = {
            hit_dst;
            @defaultonly nop;
        }

        const default_action = nop;
        size = 1024;
        // No association a counter required, since an indirect counter is used.
    }

    apply {
        forward.apply();
        forward_exact.apply();
        forward_dst.apply();

        // No need for egress processing, skip it and use empty controls for egress.
        ig_intr_tm_md.bypass_egress = 1w1;
    }
}

Pipeline(SwitchIngressParser(),
         SwitchIngress(),
         SwitchIngressDeparser(),
         EmptyEgressParser(),
         EmptyEgress(),
         EmptyEgressDeparser()) pipe;

Switch(pipe) main;
