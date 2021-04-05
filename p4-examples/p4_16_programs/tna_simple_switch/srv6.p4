/* -*- P4_16 -*- */

/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */



// ----------------------------------------------------------------------------
// Segment Routing v6 (SRv6).
// Based on SRv6 Network Programming
// See https://tools.ietf.org/html/draft-filsfils-spring-srv6-network-programming-05
//
//
// SRv6 SID lookup
//
// @param hdr: Parsed headers.
// @param ig_md.srv6.sid: SID pointed by the SL field in the first SRH.
// @param ig_md: Ingress metadata.
// @param lkp: Lookup fields.

control SRv6(inout header_t hdr,
             inout ingress_metadata_t ig_md,
             inout lookup_fields_t lkp) {

    // Per "Local SID" table entry stats for traffic that matched a SID and was
    // processed correctly.
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) cnt1;

    action t() {
        // T -- Transit behavior
    }

    action t_insert(srv6_sid_t s1, bit<16> rewrite_index) {
        // T.Insert -- Transit with insertion of an SRv6 Policy.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        // Update IPv6.DA with the first segment of the SRv6 Policy.
        hdr.ipv6.dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
    }

    action t_insert_red(srv6_sid_t s1, bit<16> rewrite_index) {
        // T.Insert.Red -- Transit with reduced insertion of an SRv6 Policy.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        // Update IPv6.DA with the first segment of the SRv6 Policy.
        hdr.ipv6.dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
    }

    action t_encaps(srv6_sid_t s1, bit<20> flow_label, bit<16> rewrite_index) {
        // T.Encaps -- Transit with encapsulation in an SRv6 Policy.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        lkp.ipv6_flow_label = flow_label;
        ig_md.srv6.encap = true;
        ig_md.srv6.rewrite = rewrite_index;
    }

    action t_encaps_red(
        srv6_sid_t s1, bit<20> flow_label, bit<16> rewrite_index) {
        // T.Encaps.Red: Transit with reduce encaps in an SRv6 Policy.
        // If there is only one SID, there is no need to push an SRH
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        lkp.ipv6_flow_label = flow_label;
        ig_md.srv6.encap = true;
        ig_md.srv6.rewrite = rewrite_index;
    }

    action t_encaps_l2() {
        // T.Encaps.L2: Transit with encapsulation of L2 frames.
        //TODO(msharif)
    }

    action t_encaps_l2_red() {
        //T.Encaps.L2.Red: Transit with reduce encaps of L2 frames in an SRv6 Policy.
        //TODO(msharif)
    }

    action drop() {
        cnt1.count();
    }


    action end() {
        // End -- Endpoint
        // Set the FIB lookup field.
        lkp.ipv6_dst_addr = ig_md.srv6.sid;
        hdr.srh.segment_left = hdr.srh.segment_left - 1;
        // Update IPv6.DA with SRH[SL]
        hdr.ipv6.dst_addr = ig_md.srv6.sid;
        cnt1.count();
    }

    action end_x(nexthop_t nexthop) {
        // End.X -- Endpoint with Layer-3 cross-connect.
        ig_md.nexthop = nexthop;
        // Bypass the FIB lookup.
        ig_md.bypass = BYPASS_L3;
        hdr.srh.segment_left = hdr.srh.segment_left - 1;
        // Update IPv6.DA with SRH[SL]
        hdr.ipv6.dst_addr = ig_md.srv6.sid;
        cnt1.count();
    }

    action end_t() {
        // End.T -- Endpoint with specific IPv6 table lookup.
        //TODO(msharif)
        cnt1.count();
    }

    action end_dx2(ifindex_t ifindex) {
        // End.DX2 -- Endpoint with decapsulation and Layer-2 cross-connect.
        ig_md.egress_ifindex = ifindex;
        ig_md.bypass = BYPASS_ALL;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dx2v() {
        // End.DX2V -- Endpoint with decapsulation and VLAN L2 table lookup.
        //TODO(msharif): Lookup the exposed inner VLAN in L2 table.
        cnt1.count();
    }

    action end_dt2u(bd_t bd) {
        // End.DT2U -- Endpoint with decapsulation and unicast MAC L2 table lookup.
        //XXX Based on RFC8200, the value 59 in the Next Header field of an IPv6
        // header or any extension header indicates that there is nothing following
        // that header. So inner ethernet header is NOT parsed and hence cannot
        // forward (or learn) based on inner ethernet src/dst addresses.
        lkp.mac_src_addr = hdr.inner_ethernet.src_addr;
        lkp.mac_dst_addr = hdr.inner_ethernet.dst_addr;
        ig_md.srv6.decap = true;
        ig_md.bd = bd;
        cnt1.count();
    }

    action end_dt2m() {
        // End.DT2M -- Endpoint with decapsulation and L2 table flooding.
        //TODO(msharif)
        lkp.mac_src_addr = hdr.inner_ethernet.src_addr;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dx6(nexthop_t nexthop) {
        // End.DX6 -- Endpoint with decapsulation and IPv6 cross-connect.
        ig_md.nexthop = nexthop;
        ig_md.bypass = BYPASS_L3;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dx4(nexthop_t nexthop) {
        // End.DX4 -- Endpoint with decapsulation and IPv4 cross-connect.
        ig_md.nexthop = nexthop;
        ig_md.bypass = BYPASS_L3;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dt4(vrf_t vrf) {
        // End.DT4 -- Endpoint with decapsulation and specific IPv4 table lookup.
        ig_md.vrf = vrf;
        lkp.ipv4_src_addr = hdr.inner_ipv4.src_addr;
        lkp.ipv4_dst_addr = hdr.inner_ipv4.dst_addr;
        lkp.ip_proto = hdr.inner_ipv4.protocol;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dt6(vrf_t vrf) {
        // End.DT6 -- Endpoint with decapsulation and specific IPv6 table lookup.
        ig_md.vrf = vrf;
        lkp.ipv6_src_addr = hdr.inner_ipv6.src_addr;
        lkp.ipv6_dst_addr = hdr.inner_ipv6.dst_addr;
        lkp.ip_proto = hdr.inner_ipv6.next_hdr;
        lkp.ipv6_flow_label = hdr.inner_ipv6.flow_label;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_dt46() {
        // End.DT46 -- Endpoint with decapsulation and specific IP table lookup.
        // This functionality is implementing by using end.dt4 and end_dt6
        // actions and it does NOT need a new action.
    }

    action end_b6(srv6_sid_t s1, bit<16> rewrite_index) {
        // End.B6 -- Endpoint bound to an SRv6 Policy.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        // Update IPv6.DA with the first segment of the SRv6 Policy.
        hdr.ipv6.dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
        cnt1.count();
    }

    action end_b6_red(srv6_sid_t s1, bit<16> rewrite_index) {
        // End.B6.Red -- Endpoint bound to an SRv6 reduced policy.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        // Update IPv6.DA with the first segment of the SRv6 Policy.
        hdr.ipv6.dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
        cnt1.count();
    }

    action end_b6_encaps(bit<16> rewrite_index) {
        // End.B6.Encaps -- Endpoint bound to an SRv6 encapsulation policy.
        // Set the FIB lookup based on SRH[SL]
        lkp.ipv6_dst_addr = ig_md.srv6.sid;
        hdr.srh.segment_left = hdr.srh.segment_left - 1;
        // Update IPv6.DA with SRH[SL]
        hdr.ipv6.dst_addr = ig_md.srv6.sid;
        ig_md.srv6.rewrite = rewrite_index;
        ig_md.srv6.encap = true;
        cnt1.count();
    }

    action end_b6_encaps_red(bit<16> rewrite_index) {
        // End.B6.Encaps.Red -- Endpoint bound to an SRv6 reduced encapsulation policy.
        // Set the FIB lookup based on SRH[SL]
        lkp.ipv6_dst_addr = ig_md.srv6.sid;
        hdr.srh.segment_left = hdr.srh.segment_left - 1;
        // Update IPv6.DA with SRH[SL]
        hdr.ipv6.dst_addr = ig_md.srv6.sid;
        ig_md.srv6.rewrite = rewrite_index;
        ig_md.srv6.encap = true;
        cnt1.count();
    }

    action end_bm() {
        // End.BM: Endpoint bound to an SR-MPLS policy.
        // MPLS is NOT supported.
    }

    action end_s() {
        // NOT supported.
    }

// SRv6 Mobility Functions
// See https://tools.ietf.org/html/draft-ietf-dmm-srv6-mobile-uplane-02

#define SRv6_mobility_functions  \
    end_map;                     \
    end_m_gtp6_d;                \
    end_m_gtp6_e;                \
    end_m_gtp4_d;                \
    end_limit;

    action end_map(srv6_sid_t sid) {
        // XXX(msharif): Separate mapping table?
        // End.MAP -- Endpoint function with SID mapping.
        lkp.ipv6_dst_addr = sid;
        // Update IPv6.DA with the new mapped SID.
        hdr.ipv6.dst_addr = sid;
        cnt1.count();
    }

    action end_m_gtp6_d(srv6_sid_t s1, bit<16> rewrite_index) {
        // End.M.GTP6.D -- Endpoint function with decapsulation from IPv6/GTP tunnel.
        // Set the FIB lookup field based on first segment of the SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
        ig_md.srv6.decap = true;
        ig_md.srv6.encap = true;
        cnt1.count();
    }

    action end_m_gtp6_e(bit<16> rewrite_index) {
        // End.M.GTP6.D -- Endpoint function with encapsulation for IPv6/GTP tunnel.
        hdr.srh.segment_left = hdr.srh.segment_left - 1;
        // Set the FIB lookup field based on SRH[SL]
        lkp.ipv6_dst_addr = ig_md.srv6.sid;
        // TEID = ig_md.srv6.sid[31:0]
        ig_md.srv6.rewrite = rewrite_index;
        ig_md.srv6.decap = true;
        ig_md.srv6.encap = true;
        cnt1.count();
    }

    action end_m_gtp4_d() {
        // End.M.GTP4.E -- Endpoint function with encapsulation for IPv4/GTP tunnel.
        //TODO(msharif)
        cnt1.count();
    }

    action end_limit() {
        // End.Limit: Rate Limiting function.
        //TODO(msharif)
        cnt1.count();
    }

    action t_map() {
        // T.M.Tmap: Transit behavior with IPv4/GTP decapsulation and mapping into an SRv6 Policy.
        //TODO(msharif)
    }

// SRv6 Proxy Functions
// See https://tools.ietf.org/html/draft-xuclad-spring-sr-service-programming-00

#define SRv6_proxy_functions  \
    end_as;                   \
    end_ad;                   \
    static_proxy;             \
    dynamic_proxy;

    action end_as(nexthop_t nexthop) {
        // End.AS: Static proxy
        ig_md.nexthop = nexthop;
        // Bypass the FIB lookup.
        ig_md.bypass = BYPASS_L3;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

    action end_ad(nexthop_t nexthop) {
        // End.AS: Static proxy
        ig_md.nexthop = nexthop;
        // Bypass the FIB lookup.
        ig_md.bypass = BYPASS_L3;
        ig_md.srv6.decap = true;
        cnt1.count();
    }

// Since the returning traffic from the service is classified based on the incoming interface, an
// interface can be used as receiving interface only for a single SR proxy segment.
    action static_proxy(srv6_sid_t s1, bit<16> rewrite_index) {
        // Set the FIB lookup field based on first segment of the cached SRv6 Policy
        lkp.ipv6_dst_addr = s1;
        ig_md.srv6.rewrite = rewrite_index;
        ig_md.srv6.encap = true;
        cnt1.count();
    }

    action dynamic_proxy() {
        //TODO(msharif)
        cnt1.count();
    }

    table local_sid {
        key = {
            hdr.ipv6.dst_addr : ternary;
            hdr.ipv6.next_hdr : ternary;
            hdr.srh.isValid() : ternary;
            hdr.srh.segment_left : ternary;
            hdr.srh.next_hdr : ternary;
            ig_md.ifindex : ternary; // for SRv6 proxy
        }

        actions = {
            @defaultonly NoAction;
            drop;
            end;               // END
            end_x;             // END.X
            end_t;             // END.T
            end_dx2;           // END.DX2
            end_dx2v;          // END.DX2V
            end_dt2u;          // END.DT2U
            end_dt2m;          // END.DT2M
            end_dx4;           // END.DX4
            end_dx6;           // END.DX6
            end_dt4;           // END.DT4
            // end_dt46;       // END.DT4
            end_dt6;           // END.DT6
            end_b6;            // END.B6
            end_b6_red;        // END.B6.Red
            end_b6_encaps;     // END.B6.Encaps
            end_b6_encaps_red; // END.B6.Encaps.Red
            // end_bm;         // END.BM
            // end_s;          // END.S

            SRv6_mobility_functions
            SRv6_proxy_functions
        }

        const default_action = NoAction;
        counters = cnt1;
    }

    table transit_ {
        key = { hdr.ipv6.dst_addr : lpm; }
        actions = {
            t;
            t_insert;
            t_insert_red;
            t_encaps;
            t_encaps_red;
            t_encaps_l2;
            t_encaps_l2_red;
        }
    }

    apply {
        if (hdr.ipv6.isValid()) {
            if(!local_sid.apply().hit) {
                transit_.apply();
            }
        }
    }
}


//------------------------------------------------------------------------------
// SRv6 Decapsulation
//------------------------------------------------------------------------------
control SRv6Decap(in srv6_metadata_t srv6_md, inout header_t hdr) {
    action decap_inner_udp() {
        hdr.udp = hdr.inner_udp;
        hdr.inner_udp.setInvalid();
        hdr.gtpu.setInvalid();
    }

    action decap_inner_tcp() {
        // hdr.tcp = hdr.inner_tcp;
        // hdr.inner_tcp.setInvalid();
        hdr.udp.setInvalid();
        hdr.gtpu.setInvalid();
    }

    action decap_inner_unknown() {
        hdr.udp.setInvalid();
        hdr.gtpu.setInvalid();
    }

    table decap_inner_l4 {
        key = {
            hdr.inner_udp.isValid() : exact;
            // hdr.inner_tcp.isValid() : exact;
            hdr.tcp.isValid() : exact;
        }

        actions = {
            decap_inner_udp;
            decap_inner_tcp;
            decap_inner_unknown;
        }

        const default_action = decap_inner_unknown;
        const entries = {
            (true, false) : decap_inner_udp();
            (false, true) : decap_inner_tcp();
            (false, false) : decap_inner_unknown();
        }
    }

    action remove_srh_header() {
        hdr.srh.setInvalid();
        hdr.srh_segment_list[0].setInvalid();
        hdr.srh_segment_list[1].setInvalid();
        hdr.srh_segment_list[2].setInvalid();
        hdr.srh_segment_list[3].setInvalid();
        hdr.srh_segment_list[4].setInvalid();
    }

    action remove_inner_srh_header() {
        hdr.inner_srh.setInvalid();
        hdr.inner_srh_segment_list[0].setInvalid();
        hdr.inner_srh_segment_list[1].setInvalid();
        hdr.inner_srh_segment_list[2].setInvalid();
        hdr.inner_srh_segment_list[3].setInvalid();
        hdr.inner_srh_segment_list[4].setInvalid();
    }

    action copy_srh_header() {
        hdr.srh = hdr.inner_srh;
        hdr.srh_segment_list[0] = hdr.inner_srh_segment_list[0];
        hdr.srh_segment_list[1] = hdr.inner_srh_segment_list[1];
        hdr.srh_segment_list[2] = hdr.inner_srh_segment_list[2];
        hdr.srh_segment_list[3] = hdr.inner_srh_segment_list[3];
        // hdr.srh_segment_list[4] = hdr.inner_srh_segment_list[4];
    }

    action decap_inner_non_ip() {
        hdr.ethernet = hdr.inner_ethernet;
        hdr.inner_ethernet.setInvalid();
        hdr.ipv6.setInvalid();
        remove_srh_header();
    }

    action decap_inner_ipv4() {
        hdr.ethernet.ether_type = ETHERTYPE_IPV4;
        hdr.ipv4 = hdr.inner_ipv4;
        hdr.ipv6.setInvalid();
        hdr.inner_ipv4.setInvalid();
        remove_srh_header();
    }

    action decap_inner_ipv6() {
        hdr.ethernet.ether_type = ETHERTYPE_IPV6;
        hdr.ipv6 = hdr.inner_ipv6;
        hdr.inner_ipv6.setInvalid();
        remove_srh_header();
    }

    action decap_inner_ipv6_srh() {
        hdr.ethernet.ether_type = ETHERTYPE_IPV6;
        hdr.ipv6 = hdr.inner_ipv6;
        hdr.inner_ipv6.setInvalid();
        copy_srh_header();
        remove_inner_srh_header();
    }

    table decap_inner_ip {
        key = {
            hdr.inner_ipv4.isValid() : exact;
            hdr.inner_ipv6.isValid() : exact;
            hdr.inner_srh.isValid() : exact;
        }

        actions = {
            NoAction;
            decap_inner_non_ip;
            decap_inner_ipv4;
            decap_inner_ipv6;
            decap_inner_ipv6_srh;
        }

        const default_action = NoAction;
        const entries = {
            (false, false, false) : decap_inner_non_ip();
            (true, false, false) : decap_inner_ipv4();
            (false, true, false) : decap_inner_ipv6();
            (false, true, true) : decap_inner_ipv6_srh();
        }
    }

    apply {
        if (srv6_md.decap) {
            decap_inner_l4.apply();
            decap_inner_ip.apply();
        }

        //TODO(msharif): Implement segment popping (PSP, USP)
    }
}

//------------------------------------------------------------------------------
// SRv6 Encapsulation
//------------------------------------------------------------------------------
control SRv6Encap(in srv6_metadata_t srv6_md,  inout header_t hdr) {
    bit<8> proto;
    bit<16> len;

    // Per SRv6 Policy stats for traffic steered into it and processed correctly.
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) cnt2;

    action encap_outer_udp() {
        hdr.inner_udp = hdr.udp;
        hdr.udp.setInvalid();
    }

    action encap_outer_unknown() {
        // NoAction
    }

    table encap_outer_l4 {
        key = {
            hdr.udp.isValid() : exact;
        }

        actions = {
            encap_outer_udp;
            encap_outer_unknown;
        }

        const default_action = encap_outer_unknown;
        const entries = {
            true : encap_outer_udp();
        }
    }

    action remove_srh_header() {
        hdr.srh.setInvalid();
        hdr.srh_segment_list[0].setInvalid();
        hdr.srh_segment_list[1].setInvalid();
        hdr.srh_segment_list[2].setInvalid();
        hdr.srh_segment_list[3].setInvalid();
        // hdr.srh_segment_list[4].setInvalid();
    }


    action copy_srh_header() {
        hdr.inner_srh = hdr.srh;
        hdr.inner_srh_segment_list[0] = hdr.srh_segment_list[0];
        hdr.inner_srh_segment_list[1] = hdr.srh_segment_list[1];
        hdr.inner_srh_segment_list[2] = hdr.srh_segment_list[2];
        hdr.inner_srh_segment_list[3] = hdr.srh_segment_list[3];
        // hdr.inner_srh_segment_list[4] = hdr.srh_segment_list[4];
    }

    action encap_outer_ipv4() {
        hdr.inner_ipv4 = hdr.ipv4;
        hdr.ipv4.setInvalid();
        len = hdr.ipv4.total_len;
        proto = IP_PROTOCOLS_IPV4;
    }

    action encap_outer_ipv6() {
        hdr.inner_ipv6 = hdr.ipv6;
        hdr.ipv6.setInvalid();
        len = hdr.ipv6.payload_len + 16w40;
        proto = IP_PROTOCOLS_IPV6;
    }

    action encap_outer_ipv6_srh() {
        hdr.inner_ipv6 = hdr.ipv6;
        copy_srh_header();
        hdr.ipv6.setInvalid();
        len = hdr.ipv6.payload_len + 16w40;
        proto = IP_PROTOCOLS_IPV6;
    }

    action push_outer_srh() {
        // Copy outer SRH to inner SRH
        copy_srh_header();
        // Remove outer SRH
        remove_srh_header();
        proto = IP_PROTOCOLS_SRV6;
    }

    action no_encap() {
        proto = hdr.ipv6.next_hdr;
    }

    table encap_outer_ip {
        key = {
            srv6_md.encap : exact;
            hdr.ipv4.isValid() : exact;
            hdr.ipv6.isValid() : exact;
            hdr.srh.isValid() : exact;
        }

        actions = {
            no_encap;
            encap_outer_ipv4;
            encap_outer_ipv6;
            encap_outer_ipv6_srh;
            push_outer_srh;
        }

        const entries = {
            (false, false, true, true) : push_outer_srh();
            (true, true, false, false) : encap_outer_ipv4();
            (true, false, true, false) : encap_outer_ipv6();
            (true, false, true, true) : encap_outer_ipv6_srh();
        }

        const default_action = no_encap;
    }

    action insert_srh_header(in bit<8> next_hdr,
                             in bit<8> hdr_ext_len,
                             in bit<8> last_entry,
                             in bit<8> segment_left) {
        hdr.srh.setValid();
        hdr.srh.next_hdr = next_hdr;
        hdr.srh.hdr_ext_len =  hdr_ext_len;
        hdr.srh.routing_type = 0x4;
        hdr.srh.segment_left = segment_left;
        hdr.srh.last_entry = last_entry;
        hdr.srh.flags = 0;
        hdr.srh.tag = 0;
    }

   action rewrite_ipv6(ipv6_addr_t src_addr, ipv6_addr_t dst_addr, bit<8> next_hdr) {
        // Insert IPv6 header.
        hdr.ethernet.ether_type = ETHERTYPE_IPV6;
        hdr.ipv6.setValid();
        hdr.ipv6.version = 4w6;
        hdr.ipv6.traffic_class = 0;
        hdr.ipv6.flow_label = 0;
        hdr.ipv6.payload_len = len;
        hdr.ipv6.next_hdr = next_hdr;
        hdr.ipv6.hop_limit = 8w64;
        hdr.ipv6.src_addr = src_addr;
        hdr.ipv6.dst_addr = dst_addr;
    }

    table ipv6_rewrite {
        key = { srv6_md.rewrite : exact; }
        actions = {
            NoAction;
            rewrite_ipv6;
        }

        const default_action = NoAction;
    }

    action rewrite_srh_0() {
        hdr.ipv6.next_hdr = proto;
        cnt2.count();
    }

    action rewrite_srh_1(bit<8> segment_left, srv6_sid_t s1) {
        // Insert SRH with SID list <S1>.
        hdr.ipv6.payload_len = hdr.ipv6.payload_len + 16w24;
        hdr.ipv6.next_hdr = IP_PROTOCOLS_SRV6;
        insert_srh_header(proto, 8w2, 8w0, segment_left);
        hdr.srh_segment_list[0].setValid();
        hdr.srh_segment_list[0].sid = s1;
        cnt2.count();
    }

    action rewrite_srh_2(bit<8> segment_left, srv6_sid_t s1, srv6_sid_t s2) {
        // Insert SRH with SID list <S1, S2>.
        hdr.ipv6.payload_len = hdr.ipv6.payload_len + 16w40;
        hdr.ipv6.next_hdr = IP_PROTOCOLS_SRV6;
        insert_srh_header(proto, 8w4, 8w1, segment_left);
        hdr.srh_segment_list[0].setValid();
        hdr.srh_segment_list[1].setValid();
        hdr.srh_segment_list[0].sid = s2;
        hdr.srh_segment_list[1].sid = s1;
        cnt2.count();
    }

    action rewrite_srh_3(
            bit<8> segment_left, srv6_sid_t s1, srv6_sid_t s2, srv6_sid_t s3) {
        // Insert SRH with SID list <S1, S2, S3>.
        hdr.ipv6.payload_len = hdr.ipv6.payload_len + 16w56;
        hdr.ipv6.next_hdr = IP_PROTOCOLS_SRV6;
        insert_srh_header(proto, 8w6, 8w2, segment_left);
        hdr.srh_segment_list[0].setValid();
        hdr.srh_segment_list[1].setValid();
        hdr.srh_segment_list[2].setValid();
        hdr.srh_segment_list[0].sid = s3;
        hdr.srh_segment_list[1].sid = s2;
        hdr.srh_segment_list[2].sid = s1;
        cnt2.count();
    }

    action rewrite_srh_4(bit<8> segment_left, srv6_sid_t s1, srv6_sid_t s2,
                         srv6_sid_t s3, srv6_sid_t s4) {
        // Insert SRH with SID list <S1, S2, S3, S4>.
        hdr.ipv6.payload_len = hdr.ipv6.payload_len + 16w72;
        hdr.ipv6.next_hdr = IP_PROTOCOLS_SRV6;
        insert_srh_header(proto, 8w8, 8w3, segment_left);
        hdr.srh_segment_list[0].setValid();
        hdr.srh_segment_list[1].setValid();
        hdr.srh_segment_list[2].setValid();
        hdr.srh_segment_list[3].setValid();
        hdr.srh_segment_list[0].sid = s4;
        hdr.srh_segment_list[1].sid = s3;
        hdr.srh_segment_list[2].sid = s2;
        hdr.srh_segment_list[3].sid = s1;
        cnt2.count();
    }

    action rewrite_srh_5(bit<8> segment_left, srv6_sid_t s1, srv6_sid_t s2,
                         srv6_sid_t s3, srv6_sid_t s4, srv6_sid_t s5) {
        // Insert SRH with SID list <S1, S2, S3, S4, S5>.
        hdr.ipv6.payload_len = hdr.ipv6.payload_len + 16w88;
        hdr.ipv6.next_hdr = IP_PROTOCOLS_SRV6;
        insert_srh_header(proto, 8w10, 8w4, segment_left);
        hdr.srh_segment_list[0].setValid();
        hdr.srh_segment_list[1].setValid();
        hdr.srh_segment_list[2].setValid();
        hdr.srh_segment_list[3].setValid();
        hdr.srh_segment_list[4].setValid();
        hdr.srh_segment_list[0].sid = s5;
        hdr.srh_segment_list[1].sid = s4;
        hdr.srh_segment_list[2].sid = s3;
        hdr.srh_segment_list[3].sid = s2;
        hdr.srh_segment_list[4].sid = s1;
        cnt2.count();
    }

    table srh_rewrite {
        key = { srv6_md.rewrite : exact; }
        actions = {
            @defaultonly NoAction;
            rewrite_srh_0; // No SRH
            rewrite_srh_1; // SRH with 1 segment
            rewrite_srh_2; // SRH with 2 segments
            rewrite_srh_3; // SRH with 3 segments
            // rewrite_srh_4; // SRH with 4 segments
            // rewrite_srh_5; // SRH with 5 segments
        }

        const default_action = NoAction;
        counters = cnt2;
    }

    apply {
        if (srv6_md.rewrite != 0) {
            encap_outer_l4.apply();
            encap_outer_ip.apply();
        }

        ipv6_rewrite.apply();
        srh_rewrite.apply();
    }
}
