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


struct user_metadata_t {
    bit<1> bf_tmp;
}

header ethernet_t {
    bit<48> dmac;
    bit<48> smac;
    bit<16> ethertype;
}

header ipv4_t {
    bit<4>  ver;
    bit<4>  len;
    bit<8>  diffserv;
    bit<16> totalLen;
    bit<16> id;
    bit<3>  flags;
    bit<13> offset;
    bit<8>  ttl;
    bit<8>  proto;
    bit<16> csum;
    bit<32> sip;
    bit<32> dip;
}

header tcp_t {
    bit<16> sPort;
    bit<16> dPort;
    bit<32> seqNo;
    bit<32> ackNo;
    bit<4>  dataOffset;
    bit<3>  res;
    bit<3>  ecn;
    bit<6>  ctrl;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgentPtr;
}

struct headers {
    ethernet_t ethernet;
    ipv4_t ipv4;
    tcp_t tcp;
}

struct metadata {
    user_metadata_t md;
}

parser ParserI(packet_in packet,
               out headers hdr,
               out metadata meta,
               out ingress_intrinsic_metadata_t ig_intr_md) {
    @name(".parse_ethernet") state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.ethertype) {
            16w0x800: parse_ipv4;
        }
    }
    @name(".parse_ipv4") state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.proto) {
            8w6: parse_tcp;
        }
    }
    @name(".parse_tcp") state parse_tcp {
        packet.extract(hdr.tcp);
        transition accept;
    }
    @name(".start") state start {
        transition parse_ethernet;
    }
}

Register<bit<1>, bit<32>>(32w262144) bloom_filter_1;

control IngressP(
        inout headers hdr,
        inout metadata meta,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    @name(".bloom_filter_alu_1") RegisterAction<bit<1>, bit<32>, bit<1>>(bloom_filter_1) bloom_filter_alu_1 = {
        void apply(inout bit<1> value, out bit<1> rv) {
            rv = 1w0;
            bit<1> in_value;
            in_value = value;
            value = 1w1;
            rv = ~value;
        }
    };
    Hash<bit<32>>(HashAlgorithm_t.RANDOM) hash_1;
    @name(".check_bloom_filter_1") action check_bloom_filter_1() {
        {
            bit<32> temp = (bit<32>)(hash_1.get({ hdr.ipv4.proto, 
                                                  hdr.ipv4.sip, 
                                                  hdr.ipv4.dip, 
                                                  hdr.tcp.sPort, 
                                                  hdr.tcp.dPort })[17:0]);
            meta.md.bf_tmp = meta.md.bf_tmp | bloom_filter_alu_1.execute(temp);
        }
    }
    @name(".bloom_filter_1") table bloom_filter_1_0 {
        actions = {
            check_bloom_filter_1;
        }
        default_action = check_bloom_filter_1;
        size = 1;
    }
    apply {
        bloom_filter_1_0.apply();
    }
}

control DeparserI(
        packet_out b,
        inout headers hdr,
        in metadata meta,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    apply {
        b.emit(hdr.ethernet);
        b.emit(hdr.ipv4);
        b.emit(hdr.tcp);
    }
}

parser ParserE(packet_in b,
               out headers hdr,
               out metadata meta,
               out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        b.extract(hdr.ethernet);
        transition accept;
    }
}

control EgressP(
        inout headers hdr,
        inout metadata meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_intr_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    apply { }
}

control DeparserE(packet_out b,
                  inout headers hdr,
                  in metadata meta,
                  in egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md) {
    apply {
        b.emit(hdr.ethernet);
    }
}

Pipeline(ParserI(), IngressP(), DeparserI(), ParserE(), EgressP(), DeparserE()) pipe;
Switch(pipe) main;

