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
    bit<32>  ifid;  // Logical Interface ID
    bit<16>  brid;  // Bridging Domain ID
    bit<16>  vrf;   // VRF ID
    bit<1>   l3;    // Set if routed
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
            ETHERTYPE_VLAN : parse_vlan;
            default : accept;
        }

    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }

    state parse_vlan {
        pkt.extract(hdr.vlan_tag);
        transition select(hdr.vlan_tag.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : accept;
        }
    }

}

// ---------------------------------------------------------------------------
// Ingress Deparser
// ---------------------------------------------------------------------------
control SwitchIngressDeparser(
        packet_out pkt,
        inout header_t hdr,
        in metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {

    Checksum() ipv4_checksum;

    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
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

        pkt.emit(hdr);
    }
}

// ---------------------------------------------------------------------------
// Switch Ingress MAU
// ---------------------------------------------------------------------------
control SwitchIngress(
        inout header_t hdr,
        inout metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    action set_ifid(bit<32> ifid) {
        ig_md.ifid = ifid;
        // Set the destination port to an invalid value
        ig_tm_md.ucast_egress_port = 9w0x1ff;
    }

    table  ing_port {
        key = {
            ig_intr_md.ingress_port  : exact;
            hdr.vlan_tag.isValid()   : exact;
            hdr.vlan_tag.vid     : exact;
        }

        actions = {
            set_ifid;
        }

        size = 1024;
    }

    action set_src_ifid_md(ReplicationId_t rid, bit<9> yid, bit<16> brid, bit<13> hash1, bit<13> hash2) {
        ig_tm_md.rid = rid;
        ig_tm_md.level2_exclusion_id = yid;
        ig_md.brid = brid;
        ig_tm_md.level1_mcast_hash = hash1;
        ig_tm_md.level2_mcast_hash = hash2;
    }

    table  ing_src_ifid {
        key = {
            ig_md.ifid : exact;
        }

        actions = {
            set_src_ifid_md;
        }

        size = 1024;
    }

    action flood() {
        ig_tm_md.mcast_grp_a = ig_md.brid;
    }

    action l2_switch(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
    }

    action route(bit<16> vrf) {
        ig_md.l3 = 1;
        ig_md.vrf = vrf;
    }

    table ing_dmac {
        key = {
            ig_md.brid   : exact;
            hdr.ethernet.dst_addr : exact;
        }

        actions = {
            l2_switch;
            route;
            flood;
        }

        const default_action = flood;
        size = 1024;
    }

    action mcast_route(bit<16> xid, MulticastGroupId_t mgid1, MulticastGroupId_t mgid2) {
        ig_tm_md.level1_exclusion_id = xid;
        ig_tm_md.mcast_grp_a = mgid1;
        ig_tm_md.mcast_grp_b = mgid2;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    table ing_ipv4_mcast {
        key = {
            ig_md.vrf   : exact;
            hdr.ipv4.src_addr : ternary;
            hdr.ipv4.dst_addr : ternary;
        }

        actions = {
            mcast_route;
        }

        size = 1024;
    }

    apply {
        ing_port.apply();
        ing_src_ifid.apply();
        ing_dmac.apply();
        if (ig_md.l3 == 1) {
            ing_ipv4_mcast.apply();
        }

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
