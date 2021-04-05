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
        transition select(hdr.ipv4.protocol) {
            IP_PROTOCOLS_UDP : parse_udp;
            default : reject;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
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
    
    BypassEgress() bypass_egress;

    // Define which input field pairs for the hash should be made symmetric.
    // Multiple pairs can be defined using multiple pragma calls.
    @symmetric("hdr.ipv4.src_addr", "hdr.ipv4.dst_addr")
    @symmetric("hdr.udp.src_port", "hdr.udp.dst_port")
    Hash<bit<32>>(HashAlgorithm_t.CRC32) my_symmetric_hash;

    action set_output_port(PortId_t port_id) {
        ig_tm_md.ucast_egress_port = port_id;
    }

    table output_port {
        actions = {
            set_output_port;
        }
    }

    apply {
        output_port.apply();

        // compute a hash from the destination MAC address and write it into
        // the source mac address field
        hdr.ethernet.src_addr[31:0] = my_symmetric_hash.get({
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr,
            hdr.udp.src_port,
            hdr.udp.dst_port
            });

            // No need for egress processing, skip it and use empty controls for egress.
            // Demonstrate how to use a control for that.
            bypass_egress.apply(ig_tm_md);
    }
}

Pipeline(SwitchIngressParser(),
       SwitchIngress(),
       SwitchIngressDeparser(),
       EmptyEgressParser(),
       EmptyEgress(),
       EmptyEgressDeparser()) pipe;

Switch(pipe) main;
