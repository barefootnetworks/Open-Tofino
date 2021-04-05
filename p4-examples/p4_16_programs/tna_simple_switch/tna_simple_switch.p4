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
#include "headers.p4"

// Common types
typedef bit<16> bd_t;
typedef bit<16> vrf_t;
typedef bit<16> nexthop_t;
typedef bit<16> ifindex_t;

typedef bit<8> bypass_t;
const bypass_t BYPASS_L2 = 8w0x01;
const bypass_t BYPASS_L3 = 8w0x02;
const bypass_t BYPASS_ACL = 8w0x04;
// Add more bypass flags here.
const bypass_t BYPASS_ALL = 8w0xff;
#define BYPASS(t) (ig_md.bypass & BYPASS_##t != 0)

typedef bit<128> srv6_sid_t;
struct srv6_metadata_t {
    srv6_sid_t sid; // SRH[SL]
    bit<16> rewrite; // Rewrite index
    bool psp; // Penultimate Segment Pop
    bool usp; // Ultimate Segment Pop
    bool decap;
    bool encap;
}

struct ingress_metadata_t {
    bool checksum_err;
    bd_t bd;
    vrf_t vrf;
    nexthop_t nexthop;
    ifindex_t ifindex;
    ifindex_t egress_ifindex;
    bypass_t bypass;
    srv6_metadata_t srv6;
}

struct egress_metadata_t {
    srv6_metadata_t srv6;
}

header bridged_metadata_t {
    // user-defined metadata carried over from ingress to egress.
    bit<16> rewrite;
    bit<1> psp; // Penultimate Segment Pop
    bit<1> usp; // Ultimate Segment Pop
    bit<1> decap;
    bit<1> encap;
    bit<4> pad;
}

struct lookup_fields_t {
    mac_addr_t mac_src_addr;
    mac_addr_t mac_dst_addr;
    bit<16> mac_type;

    bit<4> ip_version;
    bit<8> ip_proto;
    bit<8> ip_ttl;
    bit<8> ip_dscp;

    bit<20> ipv6_flow_label;
    ipv4_addr_t ipv4_src_addr;
    ipv4_addr_t ipv4_dst_addr;
    ipv6_addr_t ipv6_src_addr;
    ipv6_addr_t ipv6_dst_addr;
}

struct header_t {
    bridged_metadata_t bridged_md;
    ethernet_h ethernet;
    vlan_tag_h vlan_tag;
    ipv4_h ipv4;
    ipv6_h ipv6;
    srh_h srh;
    srh_segment_list_t[5] srh_segment_list;
    tcp_h tcp;
    udp_h udp;
    gtpu_h gtpu;
    ethernet_h inner_ethernet;
    ipv6_h inner_ipv6;
    srh_h inner_srh;
    srh_segment_list_t[5] inner_srh_segment_list;
    ipv4_h inner_ipv4;
    tcp_h inner_tcp;
    udp_h inner_udp;

    // Add more headers here.
}


#include "parde.p4"
#include "srv6.p4"


//-----------------------------------------------------------------------------
// Packet validaion
// Validate ethernet, Ipv4 or Ipv6 headers and set the common lookup fields.
//-----------------------------------------------------------------------------
control PktValidation(
        in header_t hdr, out lookup_fields_t lkp) {

    const bit<32> table_size = 512;

    action malformed_pkt() {
        // drop.
    }

    action valid_pkt_untagged() {
        lkp.mac_src_addr = hdr.ethernet.src_addr;
        lkp.mac_dst_addr = hdr.ethernet.dst_addr;
        lkp.mac_type = hdr.ethernet.ether_type;
    }

    action valid_pkt_tagged() {
        lkp.mac_src_addr = hdr.ethernet.src_addr;
        lkp.mac_dst_addr = hdr.ethernet.dst_addr;
        lkp.mac_type = hdr.vlan_tag.ether_type;
    }

    table validate_ethernet {
        key = {
            hdr.ethernet.src_addr : ternary;
            hdr.ethernet.dst_addr : ternary;
            hdr.vlan_tag.isValid() : ternary;
        }

        actions = {
            malformed_pkt;
            valid_pkt_untagged;
            valid_pkt_tagged;
        }

        size = table_size;
    }

//-----------------------------------------------------------------------------
// Validate outer IPv4 header and set the lookup fields.
// - Drop the packet if ttl is zero, ihl is invalid, or version is invalid.
//-----------------------------------------------------------------------------
    action valid_ipv4_pkt() {
        // Set common lookup fields
        lkp.ip_version = 4w4;
        lkp.ip_dscp = hdr.ipv4.diffserv;
        lkp.ip_proto = hdr.ipv4.protocol;
        lkp.ip_ttl = hdr.ipv4.ttl;
        lkp.ipv4_src_addr = hdr.ipv4.src_addr;
        lkp.ipv4_dst_addr = hdr.ipv4.dst_addr;
    }

    table validate_ipv4 {
        key = {
            //ig_md.checksum_err : ternary;
            hdr.ipv4.version : ternary;
            hdr.ipv4.ihl : ternary;
            hdr.ipv4.ttl : ternary;
        }

        actions = {
            valid_ipv4_pkt;
            malformed_pkt;
        }

        size = table_size;
    }

//-----------------------------------------------------------------------------
// Validate outer IPv6 header and set the lookup fields.
// - Drop the packet if hop_limit is zero or version is invalid.
//-----------------------------------------------------------------------------
    action valid_ipv6_pkt() {
        // Set common lookup fields
        lkp.ip_version = 4w6;
        lkp.ip_dscp = hdr.ipv6.traffic_class;
        lkp.ip_proto = hdr.ipv6.next_hdr;
        lkp.ip_ttl = hdr.ipv6.hop_limit;
        lkp.ipv6_src_addr = hdr.ipv6.src_addr;
        lkp.ipv6_dst_addr = hdr.ipv6.dst_addr;
    }

    table validate_ipv6 {
        key = {
            hdr.ipv6.version : ternary;
            hdr.ipv6.hop_limit : ternary;
        }

        actions = {
            valid_ipv6_pkt;
            malformed_pkt;
        }

        size = table_size;
    }

    apply {
        validate_ethernet.apply();
        if (hdr.ipv4.isValid()) {
            validate_ipv4.apply();
        } else if (hdr.ipv6.isValid()) {
            validate_ipv6.apply();
        }
    }
}

control PortMapping(
        in PortId_t port,
        in vlan_tag_h vlan_tag,
        inout ingress_metadata_t ig_md)(
        bit<32> port_vlan_table_size,
        bit<32> bd_table_size) {

    ActionProfile(bd_table_size) bd_action_profile;

    action set_port_attributes(ifindex_t ifindex) {
        ig_md.ifindex = ifindex;

        // Add more port attributes here.
    }

    table port_mapping {
        key = { port : exact; }
        actions = { set_port_attributes; }
    }

    action set_bd_attributes(bd_t bd, vrf_t vrf) {
        ig_md.bd = bd;
        ig_md.vrf = vrf;
    }

    table port_vlan_to_bd_mapping {
        key = {
            ig_md.ifindex : exact;
            vlan_tag.isValid() : ternary;
            vlan_tag.vid : ternary;
        }

        actions = {
            NoAction;
            set_bd_attributes;
        }

        const default_action = NoAction;
        implementation = bd_action_profile;
        size = port_vlan_table_size;
    }

    apply {
        port_mapping.apply();
        port_vlan_to_bd_mapping.apply();
    }
}

//-----------------------------------------------------------------------------
// Destination MAC lookup
// - Bridge out the packet of the interface in the MAC entry.
// - Flood the packet out of all ports within the ingress BD.
//-----------------------------------------------------------------------------
control MAC(
    in mac_addr_t dst_addr,
    in bd_t bd,
    out ifindex_t egress_ifindex)(
    bit<32> mac_table_size) {

    action dmac_miss() {
        egress_ifindex = 16w0xffff;
    }

    action dmac_hit(ifindex_t ifindex) {
        egress_ifindex = ifindex;
    }

    table dmac {
        key = {
            bd : exact;
            dst_addr : exact;
        }

        actions = {
            dmac_miss;
            dmac_hit;
        }

        const default_action = dmac_miss;
        size = mac_table_size;
    }

    apply {
        dmac.apply();
    }
}

control FIB(in ipv4_addr_t dst_addr,
            in vrf_t vrf,
            out nexthop_t nexthop)(
            bit<32> host_table_size,
            bit<32> lpm_table_size) {

    action fib_hit(nexthop_t nexthop_index) {
        nexthop = nexthop_index;
    }

    action fib_miss() { }

    table fib {
        key = {
            vrf : exact;
            dst_addr : exact;
        }

        actions = {
            fib_miss;
            fib_hit;
        }

        const default_action = fib_miss;
        size = host_table_size;
    }

    table fib_lpm {
        key = {
            vrf : exact;
            dst_addr : lpm;
        }

        actions = {
            fib_miss;
            fib_hit;
        }

        const default_action = fib_miss;
        size = lpm_table_size;
    }

    apply {
        if (!fib.apply().hit) {
            fib_lpm.apply();
        }
    }
}

control FIBv6(in ipv6_addr_t dst_addr,
              in vrf_t vrf,
              out nexthop_t nexthop)(
              bit<32> host_table_size,
              bit<32> lpm_table_size) {

    action fib_hit(nexthop_t nexthop_index) {
        nexthop = nexthop_index;
    }

    action fib_miss() { }

    table fib {
        key = {
            vrf : exact;
            dst_addr : exact;
        }

        actions = {
            fib_miss;
            fib_hit;
        }

        const default_action = fib_miss;
        size = host_table_size;
    }

    table fib_lpm {
        key = {
            vrf : exact;
            dst_addr : lpm;
        }

        actions = {
            fib_miss;
            fib_hit;
        }

        const default_action = fib_miss;
        size = lpm_table_size;
    }

    apply {
        if (!fib.apply().hit) {
            fib_lpm.apply();
        }
    }
}

// ----------------------------------------------------------------------------
// Nexthop/ECMP resolution
// ----------------------------------------------------------------------------
control Nexthop(in lookup_fields_t lkp,
                inout header_t hdr,
                inout ingress_metadata_t ig_md)(
                bit<32> table_size) {
    bool routed;
    Hash<bit<32>>(HashAlgorithm_t.CRC32) sel_hash;
    ActionSelector(
        1024, sel_hash, SelectorMode_t.FAIR) ecmp_selector;

    action set_nexthop_attributes(bd_t bd, mac_addr_t dmac) {
        routed = true;
        ig_md.bd = bd;
        hdr.ethernet.dst_addr = dmac;
    }

    table ecmp {
        key = {
            ig_md.nexthop : exact;
            lkp.ipv6_src_addr : selector;
            lkp.ipv6_dst_addr : selector;
            lkp.ipv6_flow_label : selector;
            lkp.ipv4_src_addr : selector;
            lkp.ipv4_dst_addr : selector;
        }

        actions = {
            NoAction;
            set_nexthop_attributes;
        }

        const default_action = NoAction;
        size = table_size;
        implementation = ecmp_selector;
    }

    table nexthop {
        key = {
            ig_md.nexthop : exact;
        }

        actions = {
            NoAction;
            set_nexthop_attributes;
        }

        const default_action = NoAction;
        size = table_size;
    }

    action rewrite_ipv4() {
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    action rewrite_ipv6() {
        hdr.ipv6.hop_limit = hdr.ipv6.hop_limit - 1;
    }

    table ip_rewrite {
        key = {
            hdr.ipv4.isValid() : exact;
            hdr.ipv6.isValid() : exact;
        }

        actions = {
            rewrite_ipv4;
            rewrite_ipv6;
        }

        const entries = {
            (true, false) : rewrite_ipv4();
            (false, true) : rewrite_ipv6();
        }
    }


    action rewrite_smac(mac_addr_t smac) {
        hdr.ethernet.src_addr = smac;
    }

    table smac_rewrite {
        key = { ig_md.bd : exact; }
        actions = {
            NoAction;
            rewrite_smac;
        }

        const default_action = NoAction;
    }

    apply {
        routed = false;
        switch(nexthop.apply().action_run) {
            NoAction : { ecmp.apply(); }
        }

        // Decrement TTL and rewrite ethernet src addr if the packet is routed.
        if (routed) {
            ip_rewrite.apply();
            smac_rewrite.apply();
        }
    }
}

// ----------------------------------------------------------------------------
// Link Aggregation
// ----------------------------------------------------------------------------
control LAG(in lookup_fields_t lkp,
            in ifindex_t ifindex,
            out PortId_t egress_port) {
    Hash<bit<32>>(HashAlgorithm_t.CRC32) sel_hash;
    ActionSelector(1024, sel_hash, SelectorMode_t.FAIR) lag_selector;

    action set_port(PortId_t port) {
        egress_port = port;
    }

    action lag_miss() { }

    table lag {
        key = {
            ifindex : exact;
            lkp.ipv6_src_addr : selector;
            lkp.ipv6_dst_addr : selector;
            lkp.ipv6_flow_label : selector;
            lkp.ipv4_src_addr : selector;
            lkp.ipv4_dst_addr : selector;
        }

        actions = {
            lag_miss;
            set_port;
        }

        const default_action = lag_miss;
        size = 1024;
        implementation = lag_selector;
    }

    apply {
        lag.apply();
    }
}

control SwitchIngress(
        inout header_t hdr,
        inout ingress_metadata_t ig_md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    PortMapping(1024, 1024) port_mapping;
    PktValidation() pkt_validation;
    SRv6() srv6;
    MAC(1024) mac;
    FIB(1024, 1024) fib;
    FIBv6(1024, 1024) fibv6;
    Nexthop(1024) nexthop;
    LAG() lag;

    lookup_fields_t lkp;

    action add_bridged_md() {
        hdr.bridged_md.setValid();
        hdr.bridged_md.rewrite = ig_md.srv6.rewrite;
        hdr.bridged_md.psp = (bit<1>) ig_md.srv6.psp;
        hdr.bridged_md.usp = (bit<1>) ig_md.srv6.usp;
        hdr.bridged_md.decap = (bit<1>) ig_md.srv6.decap;
        hdr.bridged_md.encap = (bit<1>) ig_md.srv6.encap;
    }

//-----------------------------------------------------------------------------
// Destination MAC lookup
// - Route the packet if the destination MAC address is owned by the switch.
//-----------------------------------------------------------------------------
    action rmac_hit() { }
    table rmac {
        key = {
            lkp.mac_dst_addr : exact;
        }

        actions = {
            NoAction;
            rmac_hit;
        }

        const default_action = NoAction;
        size = 1024;
    }

    apply {
        port_mapping.apply(ig_intr_md.ingress_port, hdr.vlan_tag, ig_md);
        pkt_validation.apply(hdr, lkp);
        switch (rmac.apply().action_run) {
            rmac_hit : {
                // initialize ig_md.bypass to 0 here.
                // srv6 block will decide what to set it to finally
                ig_md.bypass = 0;
                srv6.apply(hdr, ig_md, lkp);
                if (!BYPASS(L3)) {
                    if (lkp.ip_version == 4w4) {
                        fib.apply(lkp.ipv4_dst_addr, ig_md.vrf, ig_md.nexthop);
                    } else {
                        fibv6.apply(lkp.ipv6_dst_addr, ig_md.vrf, ig_md.nexthop);
                    }
                }
            }
        }

        nexthop.apply(lkp, hdr, ig_md);
        if (!BYPASS(L2)) {
            mac.apply(hdr.ethernet.dst_addr, ig_md.bd, ig_md.egress_ifindex);
        }

        lag.apply(lkp, ig_md.egress_ifindex, ig_tm_md.ucast_egress_port);

        add_bridged_md();
    }
}

control SwitchEgress(
        inout header_t hdr,
        inout egress_metadata_t eg_md,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_intr_from_prsr,
        inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
        inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport) {
    SRv6Decap() decap;
    SRv6Encap() encap;

    apply {
        decap.apply(eg_md.srv6, hdr);
        encap.apply(eg_md.srv6, hdr);
    }
}

Pipeline(SwitchIngressParser(),
         SwitchIngress(),
         SwitchIngressDeparser(),
         SwitchEgressParser(),
         SwitchEgress(),
         SwitchEgressDeparser()) pipe;

Switch(pipe) main;
