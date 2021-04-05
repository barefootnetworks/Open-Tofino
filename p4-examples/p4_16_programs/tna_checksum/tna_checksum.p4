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
    bit<16> checksum_ipv4_tmp;
    bit<16> checksum_tcp_tmp;
    bit<16> checksum_udp_tmp;

    bool checksum_upd_ipv4;
    bool checksum_upd_tcp;
    bool checksum_upd_udp;

    bool checksum_err_ipv4_igprs;
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
    Checksum() ipv4_checksum;
    Checksum() tcp_checksum;
    Checksum() udp_checksum;

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
        ipv4_checksum.add(hdr.ipv4);
        ig_md.checksum_err_ipv4_igprs = ipv4_checksum.verify();

        tcp_checksum.subtract({hdr.ipv4.src_addr});
        udp_checksum.subtract({hdr.ipv4.src_addr});

        transition select(hdr.ipv4.protocol) {
            IP_PROTOCOLS_TCP : parse_tcp;
            IP_PROTOCOLS_UDP : parse_udp;
            default : accept;
        }
    }

    state parse_tcp {
        // The tcp checksum cannot be verified, since we cannot compute
        // the payload's checksum.
        pkt.extract(hdr.tcp);

        tcp_checksum.subtract({hdr.tcp.checksum});
        tcp_checksum.subtract({hdr.tcp.src_port});
        ig_md.checksum_tcp_tmp = tcp_checksum.get();

        transition accept;
    }

    state parse_udp {
        // The tcp checksum cannot be verified, since we cannot compute
        // the payload's checksum.
        pkt.extract(hdr.udp);

        udp_checksum.subtract({hdr.udp.checksum});
        udp_checksum.subtract({hdr.udp.src_port});
        ig_md.checksum_udp_tmp = udp_checksum.get();

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

    action nop_() { }

    action checksum_upd_ipv4(bool update) {
        ig_md.checksum_upd_ipv4 = update; 
    }
    
    action checksum_upd_tcp(bool update) {
        ig_md.checksum_upd_tcp = update; 
    }

    action checksum_upd_udp(bool update) {
        ig_md.checksum_upd_udp = update; 
    }

    action checksum_upd_ipv4_tcp_udp(bool update) {
        checksum_upd_ipv4(update);
        checksum_upd_tcp(update);
        checksum_upd_udp(update);
    }

    action snat(ipv4_addr_t src_addr,
                bool update) {
        hdr.ipv4.src_addr = src_addr;
        checksum_upd_ipv4_tcp_udp(update);
    }

    action stpat(bit<16> src_port,
                bool update) {
        checksum_upd_tcp(update);
        hdr.tcp.src_port = src_port;
    }

    action sntpat(ipv4_addr_t src_addr,
                 bit<16> src_port,
                 bool update) {
        checksum_upd_ipv4(update);
        checksum_upd_tcp(update);
        hdr.ipv4.src_addr = src_addr;
        hdr.tcp.src_port = src_port;
    }

    action supat(bit<16> src_port,
                bool update) {
        checksum_upd_udp(update);
        hdr.udp.src_port = src_port;
    }

    action snupat(ipv4_addr_t src_addr,
                 bit<16> src_port,
                 bool update) {
        checksum_upd_ipv4(update);
        checksum_upd_udp(update);
        hdr.ipv4.src_addr = src_addr;
        hdr.udp.src_port = src_port;
    }

    table translate {
        key = { hdr.ipv4.src_addr : exact; }
        actions = {
            nop_;
            snat;
            stpat;
            sntpat;
            supat;
            snupat;
            checksum_upd_ipv4;
            checksum_upd_tcp;
            checksum_upd_udp;
            checksum_upd_ipv4_tcp_udp;
        }

        default_action = nop_;
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
        // Apply address and port translations
        translate.apply();

        output_port.apply();

        // Ensure the UDP checksum is only checked if it was set in the incoming
        // packet
        if (hdr.udp.checksum == 0 && ig_md.checksum_upd_udp) {
            checksum_upd_udp(false);
        }

        // Detect checksum errors in the ingress parser and tag the packets
        if (ig_md.checksum_err_ipv4_igprs) {
            hdr.ethernet.dst_addr = 0x0000deadbeef;
        }

        // No need for egress processing, skip it and use empty controls for egress.
        ig_tm_md.bypass_egress = 1w1;
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

    Checksum() ipv4_checksum;
    Checksum() tcp_checksum;
    Checksum() udp_checksum;

    apply {
        // Updating and checking of the checksum is done in the deparser.
        // Checksumming units are only available in the parser sections of 
        // the program.
        if (ig_md.checksum_upd_ipv4) {
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
        }
        if (ig_md.checksum_upd_tcp) {
            hdr.tcp.checksum = tcp_checksum.update({
                hdr.ipv4.src_addr,
                hdr.tcp.src_port,
                ig_md.checksum_tcp_tmp
            });
        }
        if (ig_md.checksum_upd_udp) {
            hdr.udp.checksum = udp_checksum.update(data = {
                hdr.ipv4.src_addr,
                hdr.udp.src_port,
                ig_md.checksum_udp_tmp
            }, zeros_as_ones = true);
            // UDP specific checksum handling
        }

        pkt.emit(hdr);
    }
}



Pipeline(SwitchIngressParser(),
       SwitchIngress(),
       SwitchIngressDeparser(),
       EmptyEgressParser(),
       EmptyEgress(),
       EmptyEgressDeparser()) pipe;

Switch(pipe) main;
