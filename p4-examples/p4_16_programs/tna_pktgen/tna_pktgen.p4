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

#include "common/headers.p4"
#include "common/util.p4"


struct headers {
    pktgen_timer_header_t timer;
    pktgen_port_down_header_t port_down;
}

parser SwitchIngressParser(
       packet_in packet, 
       out headers hdr, 
       out empty_metadata_t md,
       out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        packet.extract(ig_intr_md);
        packet.advance(PORT_METADATA_SIZE);

        pktgen_port_down_header_t pktgen_pd_hdr = packet.lookahead<pktgen_port_down_header_t>();
        transition select(pktgen_pd_hdr.app_id) {
            1 : parse_pktgen_timer;
            2 : parse_pktgen_port_down;
            default : reject;
        }
    }

    state parse_pktgen_timer {
        packet.extract(hdr.timer);
        transition accept;
    }

    state parse_pktgen_port_down {
        packet.extract(hdr.port_down);
        transition accept;
    }
}


control SwitchIngressDeparser(
        packet_out pkt,
        inout headers hdr,
        in empty_metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {

    apply {
        pkt.emit(hdr);
    }
}


control SwitchIngress(
        inout headers hdr, 
        inout empty_metadata_t md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    action drop() {
        ig_intr_dprsr_md.drop_ctl = 0x1;
    }

    action match(PortId_t port) {
        ig_intr_tm_md.ucast_egress_port = port;
        ig_intr_tm_md.bypass_egress = 1w1;
    }

    table t {
        key = {
            hdr.timer.pipe_id : exact;
            hdr.timer.app_id  : exact;
            hdr.timer.batch_id : exact;
            hdr.timer.packet_id : exact;
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            match;
            @defaultonly drop;
        }
        const default_action = drop();
        size = 1024;
    }
    table p {
        key = {
            hdr.port_down.pipe_id   : exact;
            hdr.port_down.app_id    : exact;
            hdr.port_down.port_num  : exact;
            hdr.port_down.packet_id : exact;
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            match;
            @defaultonly drop;
        }
        const default_action = drop();
        size = 1024;
    }

    apply {
        if (hdr.timer.isValid()) {
            t.apply();
        } else if (hdr.port_down.isValid()) {
            p.apply();
        } else {
            drop();
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
