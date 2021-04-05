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


struct partition_t {
    bit<10> partition_index;
}

struct metadata_t {
    partition_t partition;
}

struct pair {
    bit<32>     first;
    bit<32>     second;
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

control SwitchIngress(
        inout header_t hdr,
        inout metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    bit<16> vrf;

    action hit(PortId_t port) {
        ig_intr_tm_md.ucast_egress_port = port;
        ig_intr_dprsr_md.drop_ctl = 0x0;  // clear drop packet
    }

    action miss() {
        ig_intr_dprsr_md.drop_ctl = 0x1; // Drop packet.
    }

    action init_index(bit<10> p_index) {
        ig_md.partition.partition_index = p_index;
    }

    table forward {
        key = {
            vrf : exact;
            hdr.ipv4.dst_addr : ternary;
        }

        actions = {
            hit;
            miss;
        }

        const default_action = miss;
        size = 1024;
    }

    table set_partition {
        actions = {
          init_index;
          NoAction;
        }
        key = {
          hdr.ipv4.protocol : exact;
        }
        default_action = NoAction;
        size = 256;
    }

    Atcam(1024) atm;
    table forward_atcam {
        key = {
            ig_md.partition.partition_index : atcam_partition_index;
            hdr.ipv4.dst_addr : ternary;
        }

        actions = {
            hit;
            NoAction;
        }

        const default_action = NoAction;
        size = 32768;
        atcam = atm;
    }

    DirectRegister<pair>() direct_reg;
    DirectRegisterAction<pair, bit<32>>(direct_reg) direct_reg_action = {
        void apply(inout pair value, out bit<32> read_value){
            read_value = value.second;
            value.first = value.first + 1;
            value.second = value.second + 100;
        }
    };

    action change_smac(mac_addr_t srcMac, PortId_t dst_port) {
        ig_intr_tm_md.ucast_egress_port = dst_port;
        hdr.ethernet.src_addr = srcMac;
        direct_reg_action.execute();
        ig_intr_dprsr_md.drop_ctl = 0;
    }

    action change_dmac(mac_addr_t dstMac, PortId_t dst_port) {
        ig_intr_tm_md.ucast_egress_port = dst_port;
        hdr.ethernet.dst_addr = dstMac;
        direct_reg_action.execute();
        ig_intr_dprsr_md.drop_ctl = 0;
    }

    table tcam_table {
        key = {
            hdr.ipv4.dst_addr : ternary;
            hdr.ipv4.src_addr : ternary;
        }

        actions = {
            change_smac;
            change_dmac;
        }

        size = 1024;
        registers = direct_reg;
    }

    ActionProfile(1024) action_profile;

    DirectLpf<bit<8>>() direct_lpf;
    bit<8> lpf_output;

    action change_ipsrc(ipv4_addr_t srcAddr, PortId_t dst_port) {
        ig_intr_tm_md.ucast_egress_port = dst_port;
        hdr.ipv4.src_addr = srcAddr;
        ig_intr_dprsr_md.drop_ctl = 0;
        lpf_output = direct_lpf.execute(hdr.ipv4.diffserv);
    }

    action change_ipdst(ipv4_addr_t dstAddr, PortId_t dst_port) {
        ig_intr_tm_md.ucast_egress_port = dst_port;
        hdr.ipv4.dst_addr = dstAddr;
        ig_intr_dprsr_md.drop_ctl = 0;
        lpf_output = direct_lpf.execute(hdr.ipv4.diffserv);
    }


    table tcam_direct_lpf {
        key = {
            hdr.ethernet.dst_addr : ternary;
            hdr.ethernet.src_addr : ternary;
        }

        actions = {
            change_ipsrc;
            change_ipdst;
        }
        size = 2048;
        implementation = action_profile;
        filters = direct_lpf;
    }

    apply {
        vrf = 16w0;
        forward.apply();
        set_partition.apply();
        forward_atcam.apply();
        tcam_table.apply();
        tcam_direct_lpf.apply();

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
