/* -*- P4_16 -*- */

/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */



parser TofinoIngressParser(
        packet_in pkt,
        out ingress_intrinsic_metadata_t ig_intr_md) {
    state start {
        pkt.extract(ig_intr_md);
        transition select(ig_intr_md.resubmit_flag) {
            1 : parse_resubmit;
            0 : parse_port_metadata;
        }
    }

    state parse_resubmit {
        // Parse resubmitted packet here.
        transition reject;
    }

    state parse_port_metadata {
        pkt.advance(PORT_METADATA_SIZE);
        transition accept;
    }
}

parser TofinoEgressParser(
        packet_in pkt,
        out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }
}


// ---------------------------------------------------------------------------
// Ingress parser
// ---------------------------------------------------------------------------
parser SwitchIngressParser(
        packet_in pkt,
        out header_t hdr,
        out ingress_metadata_t ig_md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    Checksum() ipv4_checksum;
    TofinoIngressParser() tofino_parser;

    state start {
        tofino_parser.apply(pkt, ig_intr_md);
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_IPV6 : parse_ipv6;
            ETHERTYPE_VLAN : parse_vlan;
            default : accept;
        }
    }

    state parse_vlan {
        pkt.extract(hdr.vlan_tag);
        transition select(hdr.vlan_tag.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_IPV6 : parse_ipv6;
            default : accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        ipv4_checksum.add(hdr.ipv4);
        ig_md.checksum_err = ipv4_checksum.verify();
        transition select(hdr.ipv4.protocol) {
            IP_PROTOCOLS_TCP : parse_tcp;
            IP_PROTOCOLS_UDP : parse_udp;
            default : accept;
        }
    }

    state parse_ipv6 {
        pkt.extract(hdr.ipv6);
        transition select(hdr.ipv6.next_hdr) {
            IP_PROTOCOLS_TCP : parse_tcp;
            IP_PROTOCOLS_UDP : parse_udp;
            IP_PROTOCOLS_SRV6 : parse_srh;
            IP_PROTOCOLS_IPV6 : parse_inner_ipv6;
            IP_PROTOCOLS_IPV4 : parse_inner_ipv4;
            default : accept;
        }
    }

    state parse_srh {
        pkt.extract(hdr.srh);
        transition parse_srh_segment_0;
    }

//FIXME(msharif): ig_md.srv6.sid is NOT set correctly.
#define IG_PARSE_SRH_SEGMENT(curr, next)                                    \
    state parse_srh_segment_##curr {                                        \
        pkt.extract(hdr.srh_segment_list[curr]);                            \
        transition select(hdr.srh.last_entry) {                             \
            curr : parse_srh_next_header;                                   \
            /* (_, next) :  set_active_segment_##curr;*/                    \
            default : parse_srh_segment_##next;                             \
        }                                                                   \
    }                                                                       \
                                                                            \
    state set_active_segment_##curr {                                       \
        /* ig_md.srv6.sid = hdr.srh_segment_list[curr].sid; */              \
        transition parse_srh_segment_##next;                                \
    }

IG_PARSE_SRH_SEGMENT(0, 1)
IG_PARSE_SRH_SEGMENT(1, 2)
IG_PARSE_SRH_SEGMENT(2, 3)
IG_PARSE_SRH_SEGMENT(3, 4)

    state parse_srh_segment_4 {
        pkt.extract(hdr.srh_segment_list[4]);
        transition parse_srh_next_header;
    }

    state set_active_segment_4 {
        ig_md.srv6.sid = hdr.srh_segment_list[4].sid;
        transition parse_srh_next_header;
    }

    state parse_srh_next_header {
        transition select(hdr.srh.next_hdr) {
            IP_PROTOCOLS_IPV6 : parse_inner_ipv6;
            IP_PROTOCOLS_IPV4 : parse_inner_ipv4;
            IP_PROTOCOLS_SRV6 : parse_inner_srh;
            IP_PROTOCOLS_NONXT : accept;
            default : reject;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        transition select(hdr.udp.dst_port) {
            UDP_PORT_GTPU: parse_gtpu;
            default: accept;
        }
    }

    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }


    state parse_gtpu {
        pkt.extract(hdr.gtpu);
        bit<4> version = pkt.lookahead<bit<4>>();
        transition select(version) {
            4w4 : parse_inner_ipv4;
            4w6 : parse_inner_ipv6;
        }
    }


    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        transition select(hdr.inner_ipv4.protocol) {
            IP_PROTOCOLS_TCP : parse_inner_tcp;
            IP_PROTOCOLS_UDP : parse_inner_udp;
            default : accept;
        }
    }

    state parse_inner_ipv6 {
        pkt.extract(hdr.inner_ipv6);
        transition select(hdr.inner_ipv6.next_hdr) {
            IP_PROTOCOLS_TCP : parse_inner_tcp;
            IP_PROTOCOLS_UDP : parse_inner_udp;
            IP_PROTOCOLS_SRV6 : parse_inner_srh;
            default : accept;
        }
    }

    state parse_inner_srh {
        pkt.extract(hdr.inner_srh);
        transition accept;
    }

    state parse_inner_udp {
        pkt.extract(hdr.inner_udp);
        transition select(hdr.inner_udp.dst_port) {
            default: accept;
        }
    }

    state parse_inner_tcp {
        pkt.extract(hdr.inner_tcp);
        transition accept;
    }
}

// ---------------------------------------------------------------------------
// Ingress Deparser
// ---------------------------------------------------------------------------
control SwitchIngressDeparser(
        packet_out pkt,
        inout header_t hdr,
        in ingress_metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
    apply {
        pkt.emit(hdr.bridged_md);
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.ipv6);
        pkt.emit(hdr.srh);
        pkt.emit(hdr.srh_segment_list);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.tcp);
        pkt.emit(hdr.gtpu);
        pkt.emit(hdr.inner_ipv4);
        pkt.emit(hdr.inner_ipv6);
        pkt.emit(hdr.inner_srh);
        pkt.emit(hdr.inner_srh_segment_list);
        pkt.emit(hdr.inner_udp);
        pkt.emit(hdr.inner_tcp);
    }
}

// ---------------------------------------------------------------------------
// Egress parser
// ---------------------------------------------------------------------------
parser SwitchEgressParser(
        packet_in pkt,
        out header_t hdr,
        out egress_metadata_t eg_md,
        out egress_intrinsic_metadata_t eg_intr_md) {

    TofinoEgressParser() tofino_parser;

    state start {
        tofino_parser.apply(pkt, eg_intr_md);
        transition parse_bridged_metadata;
    }

    state parse_bridged_metadata {
        pkt.extract(hdr.bridged_md);
        eg_md.srv6.rewrite = hdr.bridged_md.rewrite;
        eg_md.srv6.psp = (bool) hdr.bridged_md.psp;
        eg_md.srv6.usp = (bool) hdr.bridged_md.psp;
        eg_md.srv6.encap = (bool) hdr.bridged_md.encap;
        eg_md.srv6.decap = (bool) hdr.bridged_md.decap;
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_IPV6 : parse_ipv6;
            ETHERTYPE_VLAN : parse_vlan;
            default : accept;
        }
    }

    state parse_vlan {
        pkt.extract(hdr.vlan_tag);
        transition select(hdr.vlan_tag.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_IPV6 : parse_ipv6;
            default : accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            IP_PROTOCOLS_UDP : parse_udp;
            default : accept;
        }
    }

    state parse_ipv6 {
        pkt.extract(hdr.ipv6);
        transition select(hdr.ipv6.next_hdr) {
            IP_PROTOCOLS_UDP : parse_udp;
            IP_PROTOCOLS_SRV6 : parse_srh;
            IP_PROTOCOLS_IPV6 : parse_inner_ipv6;
            IP_PROTOCOLS_IPV4 : parse_inner_ipv4;
            default : accept;
        }
    }

    state parse_srh {
        pkt.extract(hdr.srh);
        transition parse_srh_segment_0;
    }

#define EG_PARSE_SRH_SEGMENT(curr, next)                                    \
    state parse_srh_segment_##curr {                                        \
        pkt.extract(hdr.srh_segment_list[curr]);                            \
        transition select(hdr.srh.last_entry) {                             \
            curr : parse_srh_next_header;                                   \
            default : parse_srh_segment_##next;                             \
        }                                                                   \
    }

EG_PARSE_SRH_SEGMENT(0, 1)
EG_PARSE_SRH_SEGMENT(1, 2)
EG_PARSE_SRH_SEGMENT(2, 3)
EG_PARSE_SRH_SEGMENT(3, 4)

    state parse_srh_segment_4 {
        pkt.extract(hdr.srh_segment_list[4]);
        transition parse_srh_next_header;
    }

    state parse_srh_next_header {
        transition select(hdr.srh.next_hdr) {
            IP_PROTOCOLS_IPV6 : parse_inner_ipv6;
            IP_PROTOCOLS_IPV4 : parse_inner_ipv4;
            IP_PROTOCOLS_SRV6 : parse_inner_srh;
            IP_PROTOCOLS_NONXT : accept;
            default : reject;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        transition select(hdr.udp.dst_port) {
            UDP_PORT_GTPU: parse_gtpu;
            default: accept;
        }
    }

    state parse_gtpu {
        pkt.extract(hdr.gtpu);
        bit<4> version = pkt.lookahead<bit<4>>();
        transition select(version) {
            4w4 : parse_inner_ipv4;
            4w6 : parse_inner_ipv6;
        }
    }


    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        transition select(hdr.inner_ipv4.protocol) {
            IP_PROTOCOLS_UDP : parse_inner_udp;
            default : accept;
        }
    }

    state parse_inner_ipv6 {
        pkt.extract(hdr.inner_ipv6);
        transition select(hdr.inner_ipv6.next_hdr) {
            IP_PROTOCOLS_UDP : parse_inner_udp;
            IP_PROTOCOLS_SRV6 : parse_inner_srh;
            default : accept;
        }
    }

    state parse_inner_srh {
        pkt.extract(hdr.inner_srh);
        transition accept;
    }

    state parse_inner_udp {
        pkt.extract(hdr.inner_udp);
        transition select(hdr.inner_udp.dst_port) {
            default: accept;
        }
    }
}

// ---------------------------------------------------------------------------
// Egress Deparser
// ---------------------------------------------------------------------------
control SwitchEgressDeparser(
        packet_out pkt,
        inout header_t hdr,
        in egress_metadata_t eg_md,
        in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
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
        pkt.emit(hdr.ipv6);
        pkt.emit(hdr.srh);
        pkt.emit(hdr.srh_segment_list);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.gtpu);
        pkt.emit(hdr.inner_ipv4);
        pkt.emit(hdr.inner_ipv6);
        pkt.emit(hdr.inner_srh);
        pkt.emit(hdr.inner_srh_segment_list);
        pkt.emit(hdr.inner_udp);
    }
}
