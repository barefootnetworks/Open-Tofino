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

        pkt.emit(hdr);
    }
}

control SwitchIngress(
        inout header_t hdr,
        inout metadata_t md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    // Instantiate an action profile with 1024 entries.
    ActionProfile(size=1024) action_profile;

    // A single instance of ActionProfile can be shared across multiple match 
    // action tables that are mutually exclusive.
    ActionProfile(size=1024) shared_action_profile;

    bit<12> vid;
    bit<10> vrf;
    const PortId_t CPU_PORT = 64;

    // Set the output port only
    action set_port(PortId_t port) {
        ig_intr_tm_md.ucast_egress_port = port;
    }

    // Set the output port and the next hop
    action set_nexthop(PortId_t port, bit<48> smac, bit<48> dmac) {
        ig_intr_tm_md.ucast_egress_port = port;
        hdr.ethernet.src_addr = smac;
        hdr.ethernet.dst_addr = dmac;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    // Set the output port to the CPU port
    action send_to_cpu() {
        ig_intr_tm_md.ucast_egress_port = CPU_PORT;
    }

    table forward {
        key = {
            ig_intr_md.ingress_port : exact;
            vid : exact;
        }

        actions = {
            set_port;
            NoAction;
        }

        size = 8192;
        default_action = NoAction;
        // Bind the action profile to the match action table.
        implementation = action_profile;
    }

    table fib {
        key = {
            vrf : exact;
            hdr.ipv4.dst_addr : lpm @name("dst_addr");
        }

        // All the tables sharing the same instance of ActionProfile require to 
        // have the same set of actions.
        actions = {
            set_nexthop;
            send_to_cpu;
            NoAction;
        }

        size = 8192;
        const default_action = NoAction;
        implementation = shared_action_profile;
    }

    table fallback {
        key = { hdr.ipv4.dst_addr : lpm @name("dst_addr"); }
        actions = {
            set_nexthop;
            send_to_cpu;
            NoAction;
        }

        size = 4096;
        const default_action = NoAction;
        implementation = shared_action_profile;
    }


    apply {
        // Packets with invalid port numbers will be dropped.
        ig_intr_tm_md.ucast_egress_port = 0x1ff;
        vrf = 0;

        if (hdr.vlan_tag.isValid()) {
            vid = hdr.vlan_tag.vid;
        } else {
            // Set the default vlan
            vid = 0;
        }

        forward.apply();

        if (!fib.apply().hit) {
            fallback.apply();
        }

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
