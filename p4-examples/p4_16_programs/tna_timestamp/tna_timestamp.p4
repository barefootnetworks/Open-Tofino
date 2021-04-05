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


#if __TARGET_TOFINO__ == 2
header tna_timestamps_h {
    bit<16> pad_1;
    bit<48> ingress_mac;
    bit<16> pad_2;
    bit<48> ingress_global;
    bit<32> enqueue;
    bit<32> dequeue_delta;
    bit<16> pad_5;
    bit<48> egress_global;
    bit<16> pad_6;
    bit<48> egress_tx;
}
#else
header tna_timestamps_h {
    bit<16> pad_1;
    bit<48> ingress_mac;
    bit<16> pad_2;
    bit<48> ingress_global;
    bit<14> pad_3;
    bit<18> enqueue;
    bit<14> pad_4;
    bit<18> dequeue_delta;
    bit<16> pad_5;
    bit<48> egress_global;
    bit<16> pad_6;
    bit<48> egress_tx;
}
#endif

struct metadata_t {
    tna_timestamps_h tna_timestamps_hdr;
    ptp_metadata_t tx_ptp_md_hdr;
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
        transition parse_tna_timestamp;
    }

    state parse_tna_timestamp {
        pkt.extract(ig_md.tna_timestamps_hdr);
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
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    action set_output_port(PortId_t port_id) {
        ig_intr_tm_md.ucast_egress_port = port_id;
    }

    table output_port {
        actions = {
            set_output_port;
        }
    }

    apply {
        ig_md.tna_timestamps_hdr.ingress_mac = ig_intr_md.ingress_mac_tstamp;
        ig_md.tna_timestamps_hdr.ingress_global = ig_intr_prsr_md.global_tstamp;

        output_port.apply();
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
        pkt.emit(ig_md.tna_timestamps_hdr);
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
        transition parse_tna_timestamp;
    }

    state parse_tna_timestamp {
        pkt.extract(eg_md.tna_timestamps_hdr);
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
        eg_md.tna_timestamps_hdr.enqueue = eg_intr_md.enq_tstamp;
        eg_md.tna_timestamps_hdr.dequeue_delta = eg_intr_md.deq_timedelta;
        eg_md.tna_timestamps_hdr.egress_global = eg_intr_from_prsr.global_tstamp;

        // tx timestamping is only available on hardware

        // request tx ptp correction timestamp insertion
        // eg_intr_md_for_oport.update_delay_on_tx = true;

        // Instructions for the ptp correction timestamp writer
        // eg_md.tx_ptp_md_hdr.setValid();
        // eg_md.tx_ptp_md_hdr.cf_byte_offset = 8w76;
        // eg_md.tx_ptp_md_hdr.udp_cksum_byte_offset = 8w34;
        // eg_md.tx_ptp_md_hdr.updated_cf = 0;
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
        // tx timestamping is only available on hardware
        // pkt.emit(eg_md.tx_ptp_md_hdr);
        pkt.emit(hdr);
        pkt.emit(eg_md.tna_timestamps_hdr);
    }
}


Pipeline(SwitchIngressParser(),
         SwitchIngress(),
         SwitchIngressDeparser(),
         SwitchEgressParser(),
         SwitchEgress(),
         SwitchEgressDeparser()) pipe;

Switch(pipe) main;
