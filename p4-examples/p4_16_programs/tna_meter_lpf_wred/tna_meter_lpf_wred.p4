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
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {

    apply {
        pkt.emit(hdr);
    }
}

control SwitchIngress(
        inout header_t hdr,
        inout metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    Meter<bit<10>>(500, MeterType_t.BYTES) meter;
    DirectMeter(MeterType_t.BYTES) direct_meter;

    action set_color(bit<10> meter_idx) {
        // Execute the meter and write the color to the ipv4 header diffserv 
        // field
        hdr.ipv4.diffserv = meter.execute(meter_idx);
    }
    action set_color_direct() {
        // Execute the Direct meter and write the color to the ipv4 header 
        // diffserv field
        hdr.ipv4.diffserv = direct_meter.execute();
    }

    table meter_color {
        key = {
            hdr.ethernet.dst_addr : exact;
        }

        actions = {
            set_color;
        }

        size = 1024;
    }
    table direct_meter_color {
        key = {
            hdr.ethernet.src_addr : exact;
        }

        actions = {
            set_color_direct;
        }
        meters = direct_meter;
        size = 1024;
    }

    // Sample LPF, which takes outputs a 8 bit value and has a 16 bit index
    Lpf<bit<8>, bit<16>>(500) simple_lpf;
    DirectLpf<bit<8>>() direct_lpf;

    action set_rate(bit<16> lpf_idx) {
      // Execute the LPF and write the result to the ipv4 header ttl field
      hdr.ipv4.ttl = simple_lpf.execute(hdr.ipv4.diffserv, lpf_idx);
    }
    action set_rate_direct() {
      // Execute the LPF and write the result to the ipv4 header ttl field
      hdr.ipv4.ttl = direct_lpf.execute(hdr.ipv4.diffserv);
    }

    table lpf_match_tbl {
       key = {
         hdr.ipv4.src_addr : exact;
       }
       actions = { set_rate; }
       // Associate the LPF with the table
       filters = simple_lpf;
       size = 1024;
    }

    table direct_lpf_match_tbl {
       key = {
         hdr.ipv4.dst_addr : exact;
       }
       actions = {
         set_rate_direct;
       }
       // Associate the LPF with the table
       filters = direct_lpf;
       size = 1024;
    }

    // Sample WRED, which outputs a 8 bit value with drop indicator value as 0
    // and no-drop indicator value as 255, with a size of 500 entries
    Wred<bit<8>, bit<16>>(500, 0, 255) simple_wred;

    action mark_wred(bit<16> wred_idx) {
      // Execute the WRED to take in the ipv4 diffserve field as input and
      // collect output at ipv4 hdr ttl
      hdr.ipv4.ttl = simple_wred.execute(hdr.ipv4.diffserv, wred_idx);
    }

    table wred_match_tbl {
       key = {
         hdr.ipv4.src_addr : exact;
       }
       actions = { mark_wred; }
       filters = simple_wred;
       size = 1024;
    }

    DirectWred<bit<8>>(0, 255) simple_direct_wred;

    action mark_direct_wred() {
      // Execute the WRED to take in the ipv4 diffserve field as input and
      // collect output at ipv4 hdr ttl
      hdr.ipv4.ttl = simple_direct_wred.execute(hdr.ipv4.diffserv);
    }

    table wred_direct_match_tbl {
       key = {
         hdr.ipv4.src_addr : exact;
       }
       actions = { mark_direct_wred; }
       filters = simple_direct_wred;
       size = 1024;
    }

    apply {
        // Apply meter by MAC dst address
        meter_color.apply();
        // Apply meter by MAC src address
        direct_meter_color.apply();

        // Apply lpf by MAC dst address
        lpf_match_tbl.apply();
        // Apply lpf by MAC src address
        direct_lpf_match_tbl.apply();

        // Apply wred by MAC src address
        wred_match_tbl.apply();
        // Apply wred by MAC src address
        wred_direct_match_tbl.apply();

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
