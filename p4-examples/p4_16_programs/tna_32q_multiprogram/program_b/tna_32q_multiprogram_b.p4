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
#include "custom_headers.p4"

#define FORWARD_TABLE_SIZE 1024
struct port_md_b_t {
    bit<8> f1;
}
struct metadata_b_t {
  port_md_b_t port_md;
}


// ---------------------------------------------------------------------------
// Ingress parser for pipeline b
// ---------------------------------------------------------------------------
parser SwitchIngressParser_b(
        packet_in pkt,
        out custom_header_t hdr,
        out metadata_b_t ig_md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        pkt.extract(ig_intr_md);
        ig_md.port_md = port_metadata_unpack<port_md_b_t>(pkt);
        pkt.extract(hdr.ethernet);
        transition select (hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : reject;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition parse_custom_metadata;
    }

    state parse_custom_metadata {
        pkt.extract(hdr.custom_metadata);
        transition accept;
    }
}

// ---------------------------------------------------------------------------
// Egress parser for pipeline b
// ---------------------------------------------------------------------------
parser SwitchEgressParser_b(
        packet_in pkt,
        out custom_header_t hdr,
        out metadata_b_t eg_md,
        out egress_intrinsic_metadata_t eg_intr_md) {

    state start {
        pkt.extract(eg_intr_md);
        pkt.extract(hdr.ethernet);
        transition select (hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : reject;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition parse_custom_metadata;
    }

    state parse_custom_metadata {
        pkt.extract(hdr.custom_metadata);
        transition accept;
    }
}

// ---------------------------------------------------------------------------
// Ingress Deparser for pipeline b
// ---------------------------------------------------------------------------
control SwitchIngressDeparser_b(
        packet_out pkt,
        inout custom_header_t hdr,
        in metadata_b_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
    Digest<digest_t>() digest;
    Checksum() ipv4_checksum;

    apply {
        if (ig_dprsr_md.digest_type == 0) {
            digest.pack({hdr.ethernet.dst_addr, hdr.ethernet.src_addr});
        }
        hdr.ipv4.hdr_checksum = ipv4_checksum.update(
                {hdr.ipv4.version,
                 hdr.ipv4.ihl,
                 hdr.ipv4.diffserv,
                 hdr.ipv4.total_len,
                 hdr.ipv4.identification,
                 hdr.ipv4.flags,
                 hdr.ipv4.frag_offset,
                 hdr.ipv4.ttl,
                 hdr.ipv4.protocol,
                 hdr.ipv4.src_addr,
                 hdr.ipv4.dst_addr});

        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.custom_metadata);
    }
}


// ---------------------------------------------------------------------------
// Egress Deparser for pipeline b
// ---------------------------------------------------------------------------
control SwitchEgressDeparser_b(packet_out pkt,
                              inout custom_header_t hdr,
                              in metadata_b_t eg_md,
                              in egress_intrinsic_metadata_for_deparser_t eg_intr_dprsr_md) {
    Checksum() ipv4_checksum;

    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update(
                {hdr.ipv4.version,
                 hdr.ipv4.ihl,
                 hdr.ipv4.diffserv,
                 hdr.ipv4.total_len,
                 hdr.ipv4.identification,
                 hdr.ipv4.flags,
                 hdr.ipv4.frag_offset,
                 hdr.ipv4.ttl,
                 hdr.ipv4.protocol,
                 hdr.ipv4.src_addr,
                 hdr.ipv4.dst_addr});

        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.custom_metadata);
    }
}


// ---------------------------------------------------------------------------
// P4 Pipeline b
// Packet travels through different table types (exm, alpm), each of which
// decrement the ipv4 ttl on a table hit.
// Mac Learning is also done thorugh one table.
// ---------------------------------------------------------------------------
control SwitchIngress_b(
        inout custom_header_t hdr,
        inout metadata_b_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    action hit() {
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
        hdr.custom_metadata.custom_tag = hdr.custom_metadata.custom_tag + 1;
    }

    action miss() {
        ig_dprsr_md.drop_ctl = 0x1; // Drop packet.
    }

    table forward {
        key = {
            hdr.ipv4.dst_addr : exact;
            hdr.ipv4.ttl : exact;
            hdr.custom_metadata.custom_tag: exact;
            ig_md.port_md.f1 : exact;
        }

        actions = {
            hit;
            miss;
        }

        const default_action = miss;
        size = FORWARD_TABLE_SIZE;
    }

    action dmac_hit() {
    }

    action dmac_miss() {
        ig_dprsr_md.digest_type = 0;
    }

    table learning {
        key = {
            hdr.ethernet.dst_addr : exact;
        }

        actions = {
            dmac_hit;
            dmac_miss;
        }

        const default_action = dmac_miss;
        size = 2048;
    }

    action modify_eg_port(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
    }

    table pinning {
        key = {
            ig_intr_md.ingress_port : exact;
        }

        actions = {
            NoAction;
            modify_eg_port;
        }

        const default_action = NoAction;
        size = 512;
    }

    apply {
        forward.apply();
        learning.apply();
        pinning.apply();
    }
}

control SwitchEgress_b(
    inout custom_header_t hdr,
    inout metadata_b_t eg_md,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprs,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport) {


    action hit() {
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
        hdr.custom_metadata.custom_tag = hdr.custom_metadata.custom_tag + 1;
    }

    action miss() {
        eg_intr_md_for_dprs.drop_ctl = 0x1; // Drop packet.
    }

    @alpm(1)
    @alpm_partitions(1024)
    @alpm_subtrees_per_partition(2)
    table forward {
        key = {
            hdr.ipv4.dst_addr : lpm;
            hdr.ipv4.ttl : exact;
            hdr.custom_metadata.custom_tag: exact;
        }

        actions = {
            hit;
            miss;
        }

        const default_action = miss;
        size = FORWARD_TABLE_SIZE;
    }

    apply {
        forward.apply();
    }

}

Pipeline(SwitchIngressParser_b(),
         SwitchIngress_b(),
         SwitchIngressDeparser_b(),
         SwitchEgressParser_b(),
         SwitchEgress_b(),
         SwitchEgressDeparser_b()) profile_b;

Switch(profile_b) main;
