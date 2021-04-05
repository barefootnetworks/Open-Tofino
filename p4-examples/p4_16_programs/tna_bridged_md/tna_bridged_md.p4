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

enum bit<8> internal_header_t {
    NONE               = 0x0,
    IG_INTR_MD         = 0x1,
    EXAMPLE_BRIDGE_HDR = 0x2
}

header internal_h {
    internal_header_t header_type;
}

/* Example bridge metadata */

// This header is only used inside of our pipeline and will never leave the
// chip. Therefore, we let the compiler decide how to layout the fields by using 
// the @flexible pragma.
@flexible
header example_bridge_h {
    bit<32> dst_mac_addr_low;
    bit<32> src_mac_addr_low;
}

struct metadata_t { 
    internal_h internal_hdr;
    ingress_intrinsic_metadata_t ig_intr_md;
    example_bridge_h example_bridge_hdr;
}

#include "common/headers.p4"
#include "common/util.p4"

// ---------------------------------------------------------------------------
// Ingress Parser
// ---------------------------------------------------------------------------
parser SwitchIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t ig_md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    TofinoIngressParser() tofino_parser;

    state start {
        ig_md.internal_hdr.setValid();
        ig_md.internal_hdr.header_type = internal_header_t.NONE;
        ig_md.example_bridge_hdr.setInvalid();
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
// Ingress 
// ---------------------------------------------------------------------------
control SwitchIngress(
        inout header_t hdr,
        inout metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    action bridge_add_ig_intr_md() {
        ig_md.internal_hdr.header_type = internal_header_t.IG_INTR_MD;
        ig_md.ig_intr_md = ig_intr_md;
    }

    action bridge_add_example_hdr(bit<32> dst_mac_addr_low,
                                  bit<32> src_mac_addr_low) {
        ig_md.internal_hdr.header_type = internal_header_t.EXAMPLE_BRIDGE_HDR;
        ig_md.example_bridge_hdr.setValid();
        ig_md.example_bridge_hdr.dst_mac_addr_low = dst_mac_addr_low;
        ig_md.example_bridge_hdr.src_mac_addr_low = src_mac_addr_low;
    }

    table bridge_md_ctrl {
        actions = {
            NoAction;
            bridge_add_ig_intr_md;
            bridge_add_example_hdr;
        }

        default_action = NoAction;
    }

    action set_output_port(PortId_t port_id) {
        ig_tm_md.ucast_egress_port = port_id;
    }

    table output_port {
        actions = {
            set_output_port;
        }
    }

    apply {
        bridge_md_ctrl.apply();

        output_port.apply();
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
        pkt.emit(ig_md.internal_hdr);
        pkt.emit(ig_md.ig_intr_md);
        pkt.emit(ig_md.example_bridge_hdr);
        pkt.emit(hdr);
    }
}

// ---------------------------------------------------------------------------
// Egress Parser
// ---------------------------------------------------------------------------
parser SwitchEgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t eg_md,
        out egress_intrinsic_metadata_t eg_intr_md) {

    TofinoEgressParser() tofino_parser;

    state start {
        tofino_parser.apply(pkt, eg_intr_md);
        transition parse_internal_hdr;
    }

    state parse_internal_hdr {
        pkt.extract(eg_md.internal_hdr);
        eg_md.ig_intr_md.setInvalid();
        eg_md.example_bridge_hdr.setInvalid();
        transition select(eg_md.internal_hdr.header_type) {
            internal_header_t.NONE: parse_ethernet;
            internal_header_t.IG_INTR_MD: parse_ig_intr_md;
            internal_header_t.EXAMPLE_BRIDGE_HDR: parse_example_bridge_hdr;
            default: reject;
        }
    }
    
    state parse_ig_intr_md {
        pkt.extract(eg_md.ig_intr_md);
        transition parse_ethernet;
    }

    state parse_example_bridge_hdr {
        pkt.extract(eg_md.example_bridge_hdr);
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
// Egress 
// ---------------------------------------------------------------------------
control SwitchEgress(
        inout header_t hdr,
        inout metadata_t eg_md,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_intr_from_prsr,
        inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
        inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport) {

    apply {
        if (eg_md.internal_hdr.header_type == internal_header_t.IG_INTR_MD) {
            hdr.ethernet.dst_addr = (bit<48>) eg_md.ig_intr_md.ingress_port;
        } if (eg_md.internal_hdr.header_type == internal_header_t.EXAMPLE_BRIDGE_HDR) {
            hdr.ethernet.dst_addr = (bit<48>) eg_md.example_bridge_hdr.dst_mac_addr_low;
            hdr.ethernet.src_addr = (bit<48>) eg_md.example_bridge_hdr.src_mac_addr_low;
        }
    }
}

// ---------------------------------------------------------------------------
// Egress Deparser
// ---------------------------------------------------------------------------
control SwitchEgressDeparser(packet_out pkt,
                              inout header_t hdr,
                              in metadata_t eg_md,
                              in egress_intrinsic_metadata_for_deparser_t 
                                eg_intr_dprsr_md
                              ) {

    apply {
        pkt.emit(hdr);
    }
}


Pipeline(SwitchIngressParser(),
       SwitchIngress(),
       SwitchIngressDeparser(),
       SwitchEgressParser(),
       SwitchEgress(),
       SwitchEgressDeparser()) pipe;

Switch(pipe) main;
