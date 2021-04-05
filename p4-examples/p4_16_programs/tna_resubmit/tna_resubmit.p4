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

/* Define values assigned to the intrinsic deparser metadata indicating which
 * type of resubmit is to be done.  Up to eight resubmit types can be used,
 * each type can resubmit a different set of fields.  The actual numeric value
 * is not important, it just needs to be consistent between what it set in the
 * ingress pipeline and what is checked in the ingress deparser. */
const bit<3> DPRSR_DIGEST_TYPE_A = 0;
const bit<3> DPRSR_DIGEST_TYPE_B = 7;
const bit<3> DPRSR_DIGEST_TYPE_C = 3;

/* Define magic numbers to carry in the resubmit data so that the ingress parser
 * can identify which type of resubmit data the packet has.  This will consume
 * one byte in each resubmit digest and must be at a fixed location as the
 * ingress parser will check one byte in the resubmit data and branch to the
 * appropriate parser state to extract the resubmit data differently for each
 * resubmit digest type.  Again, the exact numerical values are not important
 * but they must be consistent between what is packed into the digest and what
 * the parser checks against to decide on the resubmit type. */
const bit<8> RESUB_TYPE_A = 255;
const bit<8> RESUB_TYPE_B = 0;
const bit<8> RESUB_TYPE_C = 1;

/* Define a few different resubmit digests.  Note that each must be exactly 8
 * bytes on Tofino-1 and 16 bytes on Tofino-2.  In order to allow the ingress
 * parser to identify which resubmit digest type a packet has, one byte is used
 * to carry a type. */
header resubmit_type_a {
    bit<8>  type;
    bit<8>  f1;
    bit<16> f2;
    bit<32> f3;
#if __TARGET_TOFINO__ != 1
    bit<64> additional;
#endif
}
header resubmit_type_b {
    bit<8>  type;
    bit<8>  f1;
    bit<48> padding;
#if __TARGET_TOFINO__ != 1
    bit<64> additional;
#endif
}
header resubmit_type_c {
    bit<8>  type;
    bit<16> f1;
    bit<16> f2;
    bit<16> f3;
    bit<8> padding;
#if __TARGET_TOFINO__ != 1
    bit<64> additional;
#endif
}
@pa_container_size("ingress", "md.a.type", 8)
@pa_container_size("ingress", "md.a.f1", 8)
@pa_container_size("ingress", "md.a.f2", 16)
@pa_container_size("ingress", "md.a.f3", 32)
@pa_container_size("ingress", "md.b.type", 8)
@pa_container_size("ingress", "md.b.f1", 8)
@pa_container_size("ingress", "md.c.type", 8)
@pa_container_size("ingress", "md.c.f1", 16)
@pa_container_size("ingress", "md.c.f2", 16)
@pa_container_size("ingress", "md.c.f3", 16)
@pa_container_size("ingress", "md.c.padding", 8)

/* This program will also use the Port Metadata table, per-port data available
 * in the ingress parser, to highlight the fact that a packet has the Port
 * Metadata available during the first pass and the resubmit digest available
 * during the second pass.  A packet cannot have both Port Metadata and resubmit
 * data at the same time. */
header port_metadata {
  bit<32> f1;
  bit<32> f2;
}
struct metadata_t { 
    port_metadata   port_md;
    bit<8>          resub_type;
    resubmit_type_a a;
    resubmit_type_b b;
    resubmit_type_c c;
}

#include "common/headers.p4"
#include "common/util.p4"


// ---------------------------------------------------------------------------
// Ingress parser
// ---------------------------------------------------------------------------
parser SwitchIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        pkt.extract(ig_intr_md);
        /* The resubmit flag indicates if the packet has been resubmitted.  When
         * set the Port Metadata is not present and those bytes instead carry
         * the resubmit data. */
        transition select(ig_intr_md.resubmit_flag) {
            0 : parse_port_metadata;
            1 : parse_resubmit;
        }
    }

    state parse_port_metadata {
        md.port_md = port_metadata_unpack<port_metadata>(pkt);
#if __TARGET_TOFINO__ != 1
        pkt.advance(128);
#endif
        transition parse_ethernet;
    }

    state parse_resubmit {
        /* Since the program uses more than a single format for resubmit data
         * a type must be encoded in the resubmit digest.  The parser will now
         * branch to an appropriate parser state for the packet's resubmit data.
         */ 
        md.resub_type = pkt.lookahead<bit<8>>()[7:0];
        transition select(md.resub_type) {
            RESUB_TYPE_A : parse_resub_a;
            RESUB_TYPE_B : parse_resub_b;
            RESUB_TYPE_C : parse_resub_c;
            default : reject;
        }
    }
    state parse_resub_a {
        pkt.extract(md.a);
        transition parse_resub_end;
    }
    state parse_resub_b {
        pkt.extract(md.b);
        transition parse_resub_end;
    }
    state parse_resub_c {
        pkt.extract(md.c);
        transition parse_resub_end;
    }
    state parse_resub_end {
#if __TARGET_TOFINO__ != 1
        /* On Tofino-2 and later there are an additional 64 bits of padding
         * after the resubmit data but before the packet headers.  This is also
         * present for non-resubmit packets but the "port_metadata_unpack" call
         * will handle skipping over this padding for non-resubmit packets. */
        pkt.advance(64);
#endif
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition accept;
    }
}


control SwitchIngress(
        inout header_t hdr,
        inout metadata_t md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    Counter<bit<1>, bit<2>>(4, CounterType_t.PACKETS) cntr;

    action drop() { ig_dprsr_md.drop_ctl = 1; }
    action resub_a(bit<8> f1, bit<16> f2, bit<32> f3, bit<64> more_data) {
        md.a.type = RESUB_TYPE_A;
        md.a.f1 = f1;
        md.a.f2 = f2;
        md.a.f3 = f3;
#if __TARGET_TOFINO__ != 1
        md.a.additional = more_data;
#endif
        ig_dprsr_md.resubmit_type = DPRSR_DIGEST_TYPE_A;
        cntr.count(0);
    }
    action resub_b(bit<8> f1, bit<64> more_data) {
        md.b.type = RESUB_TYPE_B;
        md.b.f1 = f1;
#if __TARGET_TOFINO__ != 1
        md.b.additional = more_data;
#endif
        ig_dprsr_md.resubmit_type = DPRSR_DIGEST_TYPE_B;
        cntr.count(0);
    }
    action resub_c(bit<16> f1, bit<16> f2, bit<16> f3, bit<64> more_data) {
        md.c.type = RESUB_TYPE_C;
        md.c.f1 = f1;
        md.c.f2 = f2;
        md.c.f3 = f3;
#if __TARGET_TOFINO__ != 1
        md.c.additional = more_data;
#endif
        ig_dprsr_md.resubmit_type = DPRSR_DIGEST_TYPE_C;
        cntr.count(0);
    }
    action no_resub() { }

    table pass_one {
        key = {
            ig_intr_md.ingress_port : exact @name("port");
            md.port_md.f1      : exact @name("f1");
            md.port_md.f2      : exact @name("f2");
        }
        actions = {
            resub_a;
            resub_b;
            resub_c;
            no_resub;
            drop;
        }
        default_action = drop;
        size = 256;
    }

    action okay_a() { cntr.count(1); }
    action okay_b() { cntr.count(2); }
    action okay_c() { cntr.count(3); }

    table pass_two_type_a {
        key = {
            md.a.f1 : exact;
            md.a.f2 : exact;
            md.a.f3 : exact;
#if __TARGET_TOFINO__ != 1
            md.a.additional : exact;
#endif
        }
        actions = {
            okay_a;
            drop;
        }
        default_action = drop;
        size = 256;
    }
    table pass_two_type_b {
        key = {
            md.b.f1 : exact;
#if __TARGET_TOFINO__ != 1
            md.b.additional : exact;
#endif
        }
        actions = {
            okay_b;
            drop;
        }
        default_action = drop;
        size = 256;
    }
    table pass_two_type_c {
        key = {
            md.c.f1 : exact;
            md.c.f2 : exact;
            md.c.f3 : exact;
#if __TARGET_TOFINO__ != 1
            md.c.additional : exact;
#endif
        }
        actions = {
            okay_c;
            drop;
        }
        default_action = drop;
        size = 256;
    }

    apply {
        if (ig_intr_md.resubmit_flag == 0) {
            /* Modify the packet headers.  If the packet is resubmitted all
             * packet modifications, including this one, are discarded.  Only
             * the resubmit digest is carried across to the packet's second
             * pass. */
            hdr.ethernet.dst_addr = 1;
            hdr.ethernet.src_addr = 0;
            pass_one.apply();
        } else {
            if (md.resub_type == RESUB_TYPE_A) {
                pass_two_type_a.apply();
            } else if (md.resub_type == RESUB_TYPE_B) {
                pass_two_type_b.apply();
            } else if (md.resub_type == RESUB_TYPE_C) {
                pass_two_type_c.apply();
            } else {
                /* Not expected, drop the packet. */
                ig_dprsr_md.drop_ctl = 1;
            }
        }
        ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
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

    Resubmit() resubmit;

    apply {
        if (ig_intr_dprsr_md.resubmit_type == DPRSR_DIGEST_TYPE_A) {
            resubmit.emit(ig_md.a);
        } else if (ig_intr_dprsr_md.resubmit_type == DPRSR_DIGEST_TYPE_B) {
            resubmit.emit(ig_md.b);
        } else if (ig_intr_dprsr_md.resubmit_type == DPRSR_DIGEST_TYPE_C) {
            resubmit.emit(ig_md.c);
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
