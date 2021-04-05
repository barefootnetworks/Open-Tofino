# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import pdb
import socket
import logging
from ptf import config
import ptf.testutils as testutils
import scapy.layers.inet6 as scapy

from bfruntime_client_base_tests import BfRuntimeTest
from collections import namedtuple

import grpc
from ptf.thriftutils import *
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import google.rpc.code_pb2 as code_pb2
from functools import partial


logger = logging.getLogger('SimpleSwitch')
logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    if len(swports) >= 6:
        break
swports.sort()

IP_PROTOCOLS_IPV4 = 4
IP_PROTOCOLS_TCP = 6
IP_PROTOCOLS_UDP = 17
IP_PROTOCOLS_IPV6 = 41
IP_PROTOCOLS_SR = 43
IP_PROTOCOLS_NONXT = 59
IP_PROTOCOLS_ETHERIP = 97


class IpAddr():
    def __init__(self, addr, type_='Ipv6', prefix=128):
        self.addr = addr
        self.type_ = type_
        self.prefix = prefix

        if type_ == 'Ipv6':
            try:
                socket.inet_pton(socket.AF_INET6, self.addr)
            except socket.error:  # not a valid address
                logger.error("Invalid IPv6 address.")

    def netmask(self):
        ''' CIDR to netmask. '''
        return (1 << 128) - (1 << (128 - self.prefix))

    def __str__(self):
        return self.addr


class SRv6Segment():
    def __init__(self, addr, func):
        self.addr = addr
        self.func = func


class SRv6Transit():
    def __init__(self, addr, behavior):
        self.addr = addr
        self.behavior = behavior


class SRv6Test(BfRuntimeTest):
    Interface = namedtuple('Interface', 'type_ port bd')
    Interface.__new__.__defaults__ = (None, None, None)
    Entry = namedtuple('Entry', 'table key_fields')

    def __init__(self):
        super(SRv6Test, self).__init__()

    def setUp(self):
        self.dev = 0
        self.ports = {}
        self.intfs = {}
        self.handle = 0
        self.cpu_port = 0
        self.entries = []
        self.client_id = 0
        self.p4_name = "tna_simple_switch"
        self.target = gc.Target(self.dev, pipe_id=0xffff)
        BfRuntimeTest.setUp(self, self.client_id, self.p4_name)
        self.bfrt_info = self.interface.bfrt_info_get("tna_simple_switch")
        self.tables = []
        self.add_init_entries()

    def tearDown(self):
        self.cleanup()
        super(SRv6Test, self).tearDown()

    def _get_handle(self):
        """ Generate globally unique handle. """
        self.handle += 1
        return self.handle

    def add_tables(self, table):
        if table in self.tables:
            pass
        else:
            self.tables.append(table)

    def cleanup(self):
        """ Delete all the stored entries. """
        print("\n")
        print("Table Cleanup:")
        print("==============")
        try:
            for t in reversed(self.tables):
                print(("  Clearing Table {}".format(t.info.name_get())))
                t.entry_del(self.target)
        except Exception as e:
            print(("Error cleaning up: {}".format(e)))

    def add_init_entries(self):
        self.add_pkt_validation_entries()

    def add_pkt_validation_entries(self):
        self.add_ethernet_validation_entries()
        self.add_ipv4_validation_entries()
        self.add_ipv6_validation_entries()

    def add_ethernet_validation_entries(self):
        hexa = 0x00
        hexb = 0x01
        table_valid_ether_obj = self.bfrt_info.table_get("SwitchIngress.pkt_validation.validate_ethernet")
        self.add_tables(table_valid_ether_obj)
        #table_valid_ether_obj.info.key_field_annotation_add('hdr.vlan_tag.$valid', "bytes")
        keys = [table_valid_ether_obj.make_key([
                gc.KeyTuple('hdr.vlan_tag.$valid', hexa, hexb)]
        )]
        data = [table_valid_ether_obj.make_data([], 'SwitchIngress.pkt_validation.valid_pkt_untagged')]
        table_valid_ether_obj.entry_add(self.target, keys, data)
        keys = [table_valid_ether_obj.make_key([gc.KeyTuple('hdr.vlan_tag.$valid', hexb, hexb)]
                                               )]
        data = [table_valid_ether_obj.make_data([], 'SwitchIngress.pkt_validation.valid_pkt_tagged')]
        table_valid_ether_obj.entry_add(self.target, keys, data)

    def add_ipv4_validation_entries(self):
        table_valid_ipv4_obj = self.bfrt_info.table_get("SwitchIngress.pkt_validation.validate_ipv4")
        self.add_tables(table_valid_ipv4_obj)
        table_valid_ipv4_obj.entry_add(
            self.target,
            [table_valid_ipv4_obj.make_key([
                gc.KeyTuple('hdr.ipv4.version', 0x04, 0x0f)]
            )],
            [table_valid_ipv4_obj.make_data([],  'SwitchIngress.pkt_validation.valid_ipv4_pkt')]
        )

    def add_ipv6_validation_entries(self):
        table_valid_ipv6_obj = self.bfrt_info.table_get("SwitchIngress.pkt_validation.validate_ipv6")
        self.add_tables(table_valid_ipv6_obj)
        table_valid_ipv6_obj.entry_add(
            self.target,
            [table_valid_ipv6_obj.make_key([
                gc.KeyTuple('hdr.ipv6.version', 0x06, 0x0f)]
            )],
            [table_valid_ipv6_obj.make_data([],  'SwitchIngress.pkt_validation.valid_ipv6_pkt')]
        )

    def add_port(self, port):
        table_lag_selector_obj = self.bfrt_info.table_get("SwitchIngress.lag.lag_selector")
        self.add_tables(table_lag_selector_obj)
        self.table_lag_name = "SwitchIngress.lag.lag"
        table_lag_obj = self.bfrt_info.table_get("SwitchIngress.lag.lag")
        self.add_tables(table_lag_obj)
        port_index = self._get_handle()
        table_lag_selector_obj.entry_add(
            self.target,
            [table_lag_selector_obj.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', port_index)])],
            [table_lag_selector_obj.make_data([gc.DataTuple('port', port)], 'SwitchIngress.lag.set_port')]
        )

        table_lag_obj.entry_add(
            self.target,
            [table_lag_obj.make_key([gc.KeyTuple('ifindex', port_index)])],
            [table_lag_obj.make_data([gc.DataTuple('$ACTION_MEMBER_ID', port_index)])]
        )

        self.add_ingress_port_mapping(port, port_index)
        #self.add_egress_port_mapping(port, port_index)
        self.ports[port_index] = [port]
        return port_index

    def add_bd(self, bd, vrf=0):
        """ Program bd action profile """
        self.table_port_mapping_bda_name = "SwitchIngress.port_mapping.bd_action_profile"
        table_port_mapping_bda_obj = self.bfrt_info.table_get(self.table_port_mapping_bda_name)
        self.add_tables(table_port_mapping_bda_obj)

        table_port_mapping_bda_obj.entry_add(
            self.target,
            [table_port_mapping_bda_obj.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', bd)])],
            [table_port_mapping_bda_obj.make_data(
             [gc.DataTuple('bd', bd), gc.DataTuple('vrf', vrf)], 'SwitchIngress.port_mapping.set_bd_attributes')]
        )
        return bd

    def add_vrf(self, vrf):
        return vrf

    def add_interface(self, port_index, type_):
        ifindex = port_index
        self.intfs[ifindex] = self.Interface(type_=type_, port=port_index)
        return ifindex

    def add_vlan(self, vid):
        bd = self.add_bd(vid)
        return bd

    def add_vlan_member(self, vlan, ifindex):
        if ifindex not in list(self.intfs.keys()):
            logging.error("interface is not found.")

        intf = self.intfs[ifindex]
        if intf.type_ == 'access' or intf.type_ == 'port':
            # (port, 1, vlan) : Access port + packets tagged with access-vlan.
            self.add_port_vlan_to_bd_mapping(intf.port, vlan, vlan)
            # (port, 0, *) : Access port + untagged packet.
            self.add_port_vlan_to_bd_mapping(intf.port, None, vlan)
            #self.add_port_bd_to_vlan_mapping(intf.port, vlan, None)

        elif self.intfs[intf].type_ == 'trunk':
            self.add_port_vlan_to_bd_mapping(intf.port, vlan, vlan)
            #self.add_port_bd_to_vlan_mapping(intf.port, vlan, vlan)

    def add_router_interface(self, vrf, rmac, port_index, rif_ip):
        bd = self._get_handle() + 1 << 12
        print("add_router_interface", vrf, rmac, port_index, rif_ip, bd)
        ifindex = port_index
        #vlan = 2
        self.add_bd(bd, vrf)
        # (port, 0, *) : L3 interface
        self.add_port_vlan_to_bd_mapping(ifindex, None, bd)
        self.add_smac_rewrite_entry(bd, rmac)
        self.intfs[ifindex] = self.Interface(type_='port', port=port_index, bd=bd)
        return ifindex

    def add_port_vlan_to_bd_mapping(self, ifindex, vlan, bd):
        self.table_port_vlan_bd_name = "SwitchIngress.port_mapping.port_vlan_to_bd_mapping"
        table_port_vlan_bd_obj = self.bfrt_info.table_get(self.table_port_vlan_bd_name)
        self.add_tables(table_port_vlan_bd_obj)
        if vlan is None:
            keys = [table_port_vlan_bd_obj.make_key([
                    gc.KeyTuple('ig_md.ifindex', ifindex)]
            )]
            data = [table_port_vlan_bd_obj.make_data([gc.DataTuple('$ACTION_MEMBER_ID', bd)])]
            table_port_vlan_bd_obj.entry_add(self.target, keys, data)
        else:
            table_port_vlan_bd_obj.entry_add(
                self.target,
                [table_port_vlan_bd_obj.make_key([gc.KeyTuple('ig_md.ifindex', ifindex),
                                                  gc.KeyTuple('vlan_tag.$valid', 0x01, 0x01),
                                                  gc.KeyTuple('vlan_tag.vid', vlan, mask=0x0fff)]
                                                 )],
                [table_port_vlan_bd_obj.make_data([gc.DataTuple('$ACTION_MEMBER_ID', bd)])]
            )

    def add_ingress_port_mapping(self, port, ifindex):
        self.table_port_mapping_name = "SwitchIngress.port_mapping.port_mapping"
        table_port_mapping_obj = self.bfrt_info.table_get(self.table_port_mapping_name)
        self.add_tables(table_port_mapping_obj)
        table_port_mapping_obj.entry_add(
            self.target,
            [table_port_mapping_obj.make_key([gc.KeyTuple('port', port)])],
            [table_port_mapping_obj.make_data([gc.DataTuple('ifindex', ifindex)],
                                              'SwitchIngress.port_mapping.set_port_attributes')]
        )

    def add_egress_port_mapping(self, port, port_lag_index, port_lag_label=0):
        """ Program egress port mapping. """
        table_eg_port_mapping_obj = self.bfrt_info.table_get('SwitchIngress.egress_port_mapping.port_mapping')
        self.add_tables(table_eg_port_mapping_obj)
        table_eg_port_mapping_obj.entry_add(
            self.target,
            [table_eg_port_mapping_obj.make_key([gc.KeyTuple('port', port)])],
            [table_eg_port_mapping_obj.make_data([gc.DataTuple('port_lag_index', port_lag_index),
                                                  gc.DataTuple('port_lag_label', port_lag_label)], 'SwitchIngress.egress_port_mapping.port_normal')]
        )

    def add_mac_entry(self, mac, bd, ifindex):
        """ Program smac and dmac tables. """
        self.add_dmac_entry(mac, bd, ifindex)

    def add_smac_entry(self, smac, bd, ifindex):
        table_mac_smac_obj = self.bfrt_info.table_get('SwitchIngress.mac.smac')
        self.add_tables(table_mac_smac_obj)
        table_mac_smac_obj.entry_add(
            self.target,
            [table_mac_smac_obj.make_key(
                [gc.KeyTuple('ig_md.bd', bd), gc.KeyTuple('src_addr', gc.mac_to_bytes(smac))])],
            [table_mac_smac_obj.make_data([gc.DataTuple('ifindex', ifindex)],
                                          'SwitchIngress.mac.smac_hit')]
        )

    def add_dmac_entry(self, dmac, bd, ifindex):
        """ Program dmac table. """
        self.table_mac_dmac_name = "SwitchIngress.mac.dmac"
        table_mac_dmac_obj = self.bfrt_info.table_get(self.table_mac_dmac_name)
        self.add_tables(table_mac_dmac_obj)
        table_mac_dmac_obj.entry_add(
            self.target,
            [table_mac_dmac_obj.make_key([gc.KeyTuple('bd', bd), gc.KeyTuple('dst_addr', gc.mac_to_bytes(dmac))])],
            [table_mac_dmac_obj.make_data(data_field_list_in=[gc.DataTuple(
                'ifindex', ifindex)], action_name='SwitchIngress.mac.dmac_hit')]
        )

    def add_rmac_entry(self, rmac, rmac_group=0):
        """ Program router mac table. """
        self.table_name = "SwitchIngress.rmac"
        table_obj = self.bfrt_info.table_get(self.table_name)
        self.add_tables(table_obj)
        table_obj.entry_add(
            self.target,
            [table_obj.make_key([gc.KeyTuple('lkp.mac_dst_addr', gc.mac_to_bytes(rmac))])],
            [table_obj.make_data([], 'SwitchIngress.rmac_hit')]
        )
        return rmac

    def add_smac_rewrite_entry(self, bd, smac):
        self.table_nh_smac_name = "SwitchIngress.nexthop.smac_rewrite"
        table_nh_smac_obj = self.bfrt_info.table_get(self.table_nh_smac_name)
        self.add_tables(table_nh_smac_obj)
        table_nh_smac_obj.entry_add(
            self.target,
            [table_nh_smac_obj.make_key([gc.KeyTuple('ig_md.bd', bd)])],
            [table_nh_smac_obj.make_data([gc.DataTuple('smac',
                                                       gc.mac_to_bytes(smac))],
                                         'SwitchIngress.nexthop.rewrite_smac')]
        )

    def add_nexthop(self, rif, dmac):
        """ Program nexthop and rewrite tables. """
        self.table_nh_name = "SwitchIngress.nexthop.nexthop"
        table_nh_obj = self.bfrt_info.table_get(self.table_nh_name)
        self.add_tables(table_nh_obj)
        if rif not in list(self.intfs.keys()):
            logging.error("router interface is not found.")

        bd = self.intfs[rif].bd
        handle = self._get_handle()
        table_nh_obj.entry_add(
            self.target,
            [table_nh_obj.make_key([gc.KeyTuple('ig_md.nexthop', handle)])],
            [table_nh_obj.make_data([gc.DataTuple('dmac', gc.mac_to_bytes(dmac)), gc.DataTuple(
                'bd', bd)], 'SwitchIngress.nexthop.set_nexthop_attributes')]
        )
        self.add_mac_entry(dmac, bd, rif)
        return handle

    def add_fib_entry(self, vrf, ip, nexthop):
        """ Program Ipv4 or Ipv6 FIB table. """
        self.table_fib_name = "SwitchIngress.fib.fib"
        table_fib_obj = self.bfrt_info.table_get(self.table_fib_name)
        self.add_tables(table_fib_obj)
        self.table_fib_lpm_name = "SwitchIngress.fib.fib_lpm"
        table_fib_lpm_obj = self.bfrt_info.table_get(self.table_fib_lpm_name)
        self.add_tables(table_fib_lpm_obj)
        self.table_fibv6_name = "SwitchIngress.fibv6.fib"
        table_fibv6_obj = self.bfrt_info.table_get(self.table_fibv6_name)
        self.add_tables(table_fibv6_obj)
        self.table_fibv6_lpm_name = "SwitchIngress.fibv6.fib_lpm"
        table_fibv6_lpm_obj = self.bfrt_info.table_get(self.table_fibv6_lpm_name)
        self.add_tables(table_fibv6_lpm_obj)
        if ip.type_ == 'Ipv4':
            if ip.prefix == 32:
                # Ipv4 Host table
                table_fib_obj.entry_add(
                    self.target,
                    [table_fib_obj.make_key([gc.KeyTuple('vrf', vrf), gc.KeyTuple(
                        'dst_addr', gc.ipv4_to_bytes(ip.addr))])],
                    [table_fib_obj.make_data([gc.DataTuple('nexthop_index', nexthop)], 'SwitchIngress.fib.fib_hit')]
                )
            else:
                table_fib_lpm_obj.entry_add(
                    self.target,
                    [table_fib_lpm_obj.make_key([gc.KeyTuple('vrf', vrf), gc.KeyTuple(
                        'dst_addr', gc.ipv4_to_bytes(ip.addr), prefix_len=ip.prefix)])],
                    [table_fib_lpm_obj.make_data([gc.DataTuple('nexthop_index', nexthop)],
                                                 'SwitchIngress.fib.fib_hit')]
                )
        elif ip.type_ == 'Ipv6':
            if ip.prefix == 128:
                # Ipv4 Host table
                keys = [table_fibv6_obj.make_key([gc.KeyTuple('vrf', vrf),
                                                  gc.KeyTuple('dst_addr', gc.ipv6_to_bytes(ip.addr))])]
                data = [table_fibv6_obj.make_data(
                    [gc.DataTuple('nexthop_index', nexthop)], 'SwitchIngress.fibv6.fib_hit')]
                table_fibv6_obj.entry_add(self.target, keys, data)
            else:
                table_fibv6_lpm_obj.entry_add(
                    self.target,
                    [table_fibv6_lpm_obj.make_key([gc.KeyTuple('vrf', vrf), gc.KeyTuple(
                        'dst_addr', gc.ipv6_to_bytes(ip.addr), prefix_len=ip.prefix)])],
                    [table_fibv6_lpm_obj.make_data(
                        [gc.DataTuple('nexthop_index', nexthop)], 'SwitchIngress.fibv6.fib_hit')]
                )
        else:
            logger.error("Invalid IP type")

    def add_local_sid(self, segment, **kwargs):
        """ Program the local sid table. """
        if segment.addr.type_ != 'Ipv6':
            logger.error("Invalid segment")
        func = segment.func

        if func == 'end':
            self.add_local_sid_end(segment)
        elif func == 'end.dx2':
            self.add_local_sid_end_dx2(segment, **kwargs)
        elif func == 'end.dt2u':
            self.add_local_sid_end_dt2u(segment, **kwargs)
        elif func == 'end.dx6':
            self.add_local_sid_end_dx6(segment, **kwargs)
        elif func == 'end.dx4':
            self.add_local_sid_end_dx4(segment, **kwargs)
        elif func == 'end.dt6':
            self.add_local_sid_end_dt6(segment, **kwargs)
        elif func == 'end.dt4':
            self.add_local_sid_end_dt4(segment, **kwargs)
        elif func == 'end.b6':
            self.add_local_sid_end_b6(segment, **kwargs)
        elif func == 'end.b6.red':
            self.add_local_sid_end_b6_red(segment, **kwargs)
        elif func == 'end.b6.encaps':
            self.add_local_sid_end_b6(segment, **kwargs)
        elif func == 'end.b6.encaps.red':
            self.add_local_sid_end_b6_red(segment, **kwargs)
        elif func == 'end.map':
            self.add_local_sid_end_map(segment, **kwargs)
        elif func == 'end.m.gtp6.d':
            self.add_local_sid_end_m_gtp6_d(segment, **kwargs)
        elif func == 'end.as':
            self.add_local_sid_end_as(segment, **kwargs)
        elif func == 'end.ad':
            self.add_local_sid_end_ad(segment, **kwargs)
        else:
            logger.error("Undefined endpoint functionality")

    def add_transit(self, transit, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")

        if transit.behavior == 't.insert':
            self.add_transit_insert(transit, **kwargs)
        elif transit.behavior == 't.insert.red':
            self.add_transit_insert_red(transit, **kwargs)
        elif transit.behavior == 't.encaps':
            self.add_transit_encaps(transit, **kwargs)
        elif transit.behavior == 't.encaps.red':
            self.add_transit_encaps_red(transit, **kwargs)
        elif transit.behavior == 't.encaps.l2':
            pass
        elif transit.behavior == 't.encaps.l2.red':
            pass

    def add_local_sid_end(self, segment):
        addr = segment.addr.addr
        mask = segment.addr.netmask()
        # Drop the packet if NH=SRH and SL>0
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 1, 1),
                gc.KeyTuple('hdr.srh.segment_left', 0x00, 0xff),
            ])],
            [table_srv6_lsid_obj.make_data([],
                                           'SwitchIngress.srv6.drop')],
        )
        # Drop the packet if NH!=SRH
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 0, 1)])],
            [table_srv6_lsid_obj.make_data([],
                                           'SwitchIngress.srv6.drop')],
        )
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('$MATCH_PRIORITY', 1)])],
            [table_srv6_lsid_obj.make_data(None,
                                           'SwitchIngress.srv6.end')],
        )

    def add_local_sid_end_dx2(self, segment, **kwargs):
        if 'ifindex' not in list(kwargs.keys()):
            logger.error("Interface is not found.")
        data_fields = [gc.DataTuple('ifindex', kwargs['ifindex'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_NONXT, data_fields)

    def add_local_sid_end_dt2u(self, segment, **kwargs):
        if 'bd' not in list(kwargs.keys()):
            logger.error("Bridge domain is not found.")
        data_fields = [gc.DataTuple('bd', kwargs['bd'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_NONXT, data_fields)

    def add_local_sid_end_dx6(self, segment, **kwargs):
        if 'nexthop' not in list(kwargs.keys()):
            logger.error("Nexthop is not found.")
        data_fields = [gc.DataTuple('nexthop', kwargs['nexthop'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_IPV6, data_fields)

    def add_local_sid_end_dx4(self, segment, **kwargs):
        if 'nexthop' not in list(kwargs.keys()):
            logger.error("Nexthop is not found.")
        data_fields = [gc.DataTuple('nexthop', kwargs['nexthop'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_IPV4, data_fields)

    def add_local_sid_end_dt6(self, segment, **kwargs):
        if 'vrf' not in list(kwargs.keys()):
            logger.error("vrf is not found.")
        data_fields = [gc.DataTuple('vrf', kwargs['vrf'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_IPV6, data_fields)

    def add_local_sid_end_dt4(self, segment, **kwargs):
        if 'vrf' not in list(kwargs.keys()):
            logger.error("vrf is not found.")
        data_fields = [gc.DataTuple('vrf', kwargs['vrf'])]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_IPV4, data_fields)

    def add_local_sid_end_b6(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list, len(seg_list) - 1)
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[len(seg_list) - 1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_srv6_encap_common_entries(segment, data_fields)

    def add_local_sid_end_b6_red(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list[:-1], len(seg_list) - 1)
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_srv6_encap_common_entries(segment, data_fields)

    def add_local_sid_end_b6_encaps(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list, len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_srv6_encap_common_entries(segment, data_fields)

    def add_local_sid_end_b6_encaps_red(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list[:-1], len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_srv6_encap_common_entries(segment, data_fields)

    def add_local_sid_end_map(self, segment, **kwargs):
        if 'sid' not in list(kwargs.keys()):
            logger.error("Mapped sid is not found")
        addr = segment.addr.addr
        mask = segment.addr.netmask()
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
            ])],
            [table_srv6_lsid_obj.make_data([
                gc.DataTuple('sid', gc.ipv6_to_bytes(kwargs['sid'])),
            ],
                'SwitchIngress.srv6.end_map')]
        )

    def add_local_sid_end_m_gtp6_d(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list[:-1], len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_srv6_decap_common_entries(segment, IP_PROTOCOLS_UDP, data_fields)

    def add_local_sid_end_as(self, segment, **kwargs):
        if 'seg_list' not in list(kwargs.keys()):
            logger.error("Segment list is not found.")
        if 'nexthop' not in list(kwargs.keys()):
            logger.error("Nexthop is not found.")
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list, len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])

        self.add_srv6_proxy_common_entries(
            segment, [gc.DataTuple('nexthop', kwargs['nexthop'])])

        data_fields = [gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
                       gc.DataTuple('rewrite_index', rewrite_index)]
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('ig_md.ifindex', kwargs['ifindex'], 0xffff),
            ])],
            [table_srv6_lsid_obj.make_data(data_fields, 'SwitchIngress.srv6.static_proxy')]
        )

    def add_transit_insert(self, transit, **kwargs):
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list, len(seg_list) - 1)
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_transit_common_entries(transit, data_fields)

    def add_tranist_insert_red(self, transit, **kwargs):
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list[:-1], len(seg_list) - 1)
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_transit_common_entries(transit, data_fields)

    def add_transit_encaps(self, transit, **kwargs):
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list, len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[len(seg_list) - 1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_transit_common_entries(transit, data_fields)

    def add_transit_encaps_red(self, transit, **kwargs):
        seg_list = kwargs['seg_list']
        rewrite_index = self._get_handle()
        self.add_srh_rewrite_entries(rewrite_index, seg_list[:-1], len(seg_list) - 1)
        self.add_ipv6_rewrite_entries(rewrite_index, kwargs['src_addr'], seg_list[-1])
        data_fields = [
            gc.DataTuple('s1', gc.ipv6_to_bytes(seg_list[-1])),
            gc.DataTuple('rewrite_index', rewrite_index)]
        self.add_transit_common_entries(transit, data_fields)

    def add_transit_common_entries(self, transit, data_fields):
        addr = transit.addr.addr
        action = 'SwitchIngress.srv6.' + transit.behavior.replace('.', '_')
        table_srv6_trans_obj = self.bfrt_info.table_get("SwitchIngress.srv6.transit_")
        self.add_tables(table_srv6_trans_obj)
        table_srv6_trans_obj.entry_add(
            self.target,
            [table_srv6_trans_obj.make_key(
                [gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), prefix_len=transit.addr.prefix)])],
            [table_srv6_trans_obj.make_data(data_fields, action)])

    def add_srv6_proxy_common_entries(self, segment, data_fields):
        addr = segment.addr.addr
        mask = segment.addr.netmask()
        action = 'SwitchIngress.srv6.' + segment.func.replace('.', '_')
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        # SL>0
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 1, 1),
                gc.KeyTuple('$MATCH_PRIORITY', 1)])],
            [table_srv6_lsid_obj.make_data(data_fields, action)])

    def add_srv6_encap_common_entries(self, segment, data_fields):
        addr = segment.addr.addr
        mask = segment.addr.netmask()
        action = 'SwitchIngress.srv6.' + segment.func.replace('.', '_')
        # SL>0
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 1, 1),
                gc.KeyTuple('$MATCH_PRIORITY', 1)])],
            [table_srv6_lsid_obj.make_data(data_fields,
                                           action)]
        )
        # Drop the packet if SL=0
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 1, 1),
                gc.KeyTuple('hdr.srh.segment_left', 0, 0xff)])],
            [table_srv6_lsid_obj.make_data([],
                                           'SwitchIngress.srv6.drop')]
        )

    def add_srv6_decap_common_entries(self, segment, next_hdr, data_fields):
        addr = segment.addr.addr
        mask = segment.addr.netmask()
        action = 'SwitchIngress.srv6.' + segment.func.replace('.', '_')
        # ENH=next_hdr and SL=0
        table_srv6_lsid_obj = self.bfrt_info.table_get("SwitchIngress.srv6.local_sid")
        self.add_tables(table_srv6_lsid_obj)
        table_srv6_lsid_obj.info.key_field_annotation_add('$MATCH_PRIORITY', "bytes")
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.srh.$valid', 1, 1),
                gc.KeyTuple('hdr.srh.segment_left', 0, 0xff),
                gc.KeyTuple('hdr.srh.next_hdr', next_hdr, 0xff)
            ])],
            [table_srv6_lsid_obj.make_data(data_fields, action)]
        )
        # NH=next_hdr (SRH is invalid)
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('hdr.ipv6.next_hdr', next_hdr, 0xff),
                gc.KeyTuple('hdr.srh.$valid', 0, 1)
            ])],
            [table_srv6_lsid_obj.make_data(data_fields, action)]
        )
        # Drop the packet if (NH=SRH and SL>0) or ENH!=next_hdr
        table_srv6_lsid_obj.entry_add(
            self.target,
            [table_srv6_lsid_obj.make_key([
                gc.KeyTuple('hdr.ipv6.dst_addr', gc.ipv6_to_bytes(addr), mask=mask),
                gc.KeyTuple('$MATCH_PRIORITY', 1)])],
            [table_srv6_lsid_obj.make_data([], 'SwitchIngress.srv6.drop')]
        )

    def add_srh_rewrite_entries(self, rewrite_index, seg_list, seg_left):
        action = 'SwitchEgress.encap.rewrite_srh_' + str(len(seg_list))
        data_fields = []
        if len(seg_list) != 0:
            data_fields = [gc.DataTuple('segment_left', seg_left)]
            for idx, sid in enumerate(reversed(seg_list), 1):
                data_fields.append(
                    gc.DataTuple('s' + str(idx), gc.ipv6_to_bytes(sid)))
        table_srv6_srh_rew_obj = self.bfrt_info.table_get("SwitchEgress.encap.srh_rewrite")
        self.add_tables(table_srv6_srh_rew_obj)
        table_srv6_srh_rew_obj.entry_add(
            self.target,
            [table_srv6_srh_rew_obj.make_key([gc.KeyTuple('srv6_md.rewrite', rewrite_index)])],
            [table_srv6_srh_rew_obj.make_data(data_fields,
                                              action)]
        )

    def add_ipv6_rewrite_entries(self, rewrite_index, src_addr, dst_addr, next_hdr=IP_PROTOCOLS_SR):
        table_srv6_rew_obj = self.bfrt_info.table_get("SwitchEgress.encap.ipv6_rewrite")
        self.add_tables(table_srv6_rew_obj)
        table_srv6_rew_obj.entry_add(
            self.target,
            [table_srv6_rew_obj.make_key([gc.KeyTuple('srv6_md.rewrite', rewrite_index)])],
            [table_srv6_rew_obj.make_data([gc.DataTuple('src_addr', gc.ipv6_to_bytes(src_addr)),
                                           gc.DataTuple('dst_addr', gc.ipv6_to_bytes(dst_addr)), gc.DataTuple('next_hdr', next_hdr)],
                                          'SwitchEgress.encap.rewrite_ipv6')]
        )


class SRv6TransitTest(SRv6Test):

    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '3000::1']
        host_ip = IpAddr(addr='2000::1', type_='Ipv6', prefix=128)

        self.bfrt_info = self.interface.bfrt_info_get("tna_simple_switch")

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(vrf, host_ip, nexthop)

        pkt = testutils.simple_tcpv6_packet(
            eth_dst=rmac, ipv6_dst=host_ip.addr, ipv6_hlim=64)
        exp_pkt = testutils.simple_tcpv6_packet(
            eth_dst=dmac, eth_src=rmac, ipv6_dst=host_ip.addr, ipv6_hlim=63)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6TransitInsertTest(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '3000::2']
        dst_ip = IpAddr(addr='2000::1', type_='Ipv6', prefix=128)
        seg_list = ['3000::4', '3000::3', '3000::2']
        self.bfrt_info = self.interface.bfrt_info_get("tna_simple_switch")

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(
            vrf,
            IpAddr(addr=seg_list[len(seg_list)-1], type_='Ipv6', prefix=128),
            nexthop)

        transit = SRv6Transit(dst_ip, 't.insert')
        self.add_transit(transit, seg_list=seg_list)

        pkt = testutils.simple_udpv6_packet(
            eth_dst=rmac, ipv6_dst=dst_ip.addr, ipv6_hlim=64, with_udp_chksum=False)
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=dmac,
            eth_src=rmac,
            ipv6_dst=seg_list[-1],
            ipv6_src=pkt['IPv6'].src,
            ipv6_hlim=63,
            srh_seg_left=len(seg_list) - 1,
            srh_first_seg=len(seg_list) - 1,
            srh_nh=IP_PROTOCOLS_UDP,
            srh_seg_list=seg_list,
            inner_frame=pkt['UDP'])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6TransitEncapsRedTest(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '3000::2']
        dst_ip = IpAddr(addr='2000::1', type_='Ipv6', prefix=128)
        seg_list = ['3000::4', '3000::3', '3000::2']

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(
            vrf,
            IpAddr(addr=seg_list[len(seg_list)-1], type_='Ipv6', prefix=128),
            nexthop)

        transit = SRv6Transit(dst_ip, 't.encaps.red')
        self.add_transit(transit, seg_list=seg_list, src_addr='4000::1')

        pkt = testutils.simple_tcpv6_packet(eth_dst=rmac, ipv6_dst=dst_ip.addr)
        inner_pkt = pkt.copy()
        inner_pkt['IPv6'].hlim = 63
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=dmac,
            eth_src=rmac,
            ipv6_dst=seg_list[-1],
            ipv6_src='4000::1',
            ipv6_hlim=64,
            srh_seg_left=len(seg_list) - 1,
            srh_first_seg=len(seg_list) - 2,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list[:-1],
            inner_frame=inner_pkt['IPv6'])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6EndTest(SRv6Test):
    def runTest(self):
        # TODO(msharif): ig_md.srv6.sid is NOT set correctly.
        return


class SRv6EndDX2Test(SRv6Test):
    def runTest(self):
        ig_port = swports[2]
        eg_port = swports[3]
        rmac = '00:11:11:11:11:11'
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        last_entry = len(seg_list) - 1

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        port_idx = self.add_port(ig_port)
        rif_ip = IpAddr(addr='3000::1', type_='Ipv6', prefix=64)
        rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        vlan = self.add_vlan(10)
        port_idx = self.add_port(eg_port)
        intf = self.add_interface(port_idx, 'port')
        self.add_vlan_member(vlan, intf)

        sid = IpAddr(addr=seg_list[0], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.dx2')
        self.add_local_sid(segment, ifindex=intf)
        inner_pkt = testutils.simple_tcp_packet(eth_dst=rmac)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[0],
            ipv6_hlim=64,
            srh_seg_left=0,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_NONXT,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt)
        exp_pkt = inner_pkt

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        pkt['Ethernet']['IPv6'].segleft = 1
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self, self.dev)


class SRv6EndDT2UTest(SRv6Test):
    # XXX This is expected to fail
    def runTest(self):
        return
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        last_entry = len(seg_list) - 1

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        port_idx = self.add_port(ig_port)
        rif_ip = IpAddr(addr='3000::1', type_='Ipv6', prefix=64)
        rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        vlan = self.add_vlan(10)
        port_idx = self.add_port(eg_port)
        intf = self.add_interface(port_idx, 'port')
        self.add_vlan_member(vlan, intf)
        self.add_mac_entry(dmac, vlan, port_idx)

        sid = IpAddr(addr=seg_list[0], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.dt2u')
        self.add_local_sid(segment, bd=vlan)

        inner_pkt = testutils.simple_tcp_packet(eth_dst=rmac)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[0],
            ipv6_hlim=64,
            srh_seg_left=0,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_NONXT,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt)
        exp_pkt = inner_pkt

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        pkt['Ethernet']['IPv6'].segleft = 1
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self, device)


class SRv6EndDX6Test(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        last_entry = len(seg_list) - 1

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        sid = IpAddr(addr=seg_list[0], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.dx6')
        self.add_local_sid(segment, nexthop=nexthop)

        inner_pkt = testutils.simple_tcpv6_packet(eth_dst=rmac)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[0],
            ipv6_hlim=64,
            srh_seg_left=0,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt['IPv6'])
        exp_pkt = testutils.simple_tcpv6_packet(
            eth_src=rmac, eth_dst=dmac, ipv6_hlim=64)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6EndDT6Test(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']
        host_ip = IpAddr(addr='2000::1', type_='Ipv6', prefix=128)
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        last_entry = len(seg_list) - 1

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(vrf, host_ip, nexthop)
        sid = IpAddr(addr=seg_list[0], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.dt6')

        self.add_local_sid(segment, vrf=vrf)

        inner_pkt = testutils.simple_tcpv6_packet(
            eth_dst=rmac, ipv6_dst=host_ip.addr)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[0],
            ipv6_hlim=64,
            srh_seg_left=0,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt['IPv6'])
        exp_pkt = testutils.simple_tcpv6_packet(
            eth_src=rmac, eth_dst=dmac, ipv6_dst=host_ip.addr, ipv6_hlim=64)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6EndB6Test(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        new_seg_list = ['3000::4', '3000::3', '3000::2']
        last_entry = len(seg_list) - 1
        seg_left = last_entry - 1
        dst_ip = IpAddr(
            addr=new_seg_list[len(new_seg_list) - 1], type_='Ipv6', prefix=128)

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(vrf, dst_ip, nexthop)

        sid = IpAddr(addr=seg_list[seg_left], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.b6')

        self.add_local_sid(segment, seg_list=new_seg_list)

        udp_hdr = scapy.UDP(sport=1234, dport=80, chksum=0)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[seg_left],
            ipv6_hlim=64,
            srh_seg_left=seg_left,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_UDP,
            srh_seg_list=seg_list,
            inner_frame=udp_hdr)

        inner_pkt = scapy.IPv6ExtHdrRouting(
            nh=IP_PROTOCOLS_UDP,
            type=4,
            segleft=seg_left,
            reserved=last_entry << 24,
            addresses=seg_list) / udp_hdr

        last_entry = len(new_seg_list)  # Number of segments
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_src=rmac,
            eth_dst=dmac,
            ipv6_dst=new_seg_list[last_entry - 1],
            ipv6_hlim=63,
            srh_seg_left=last_entry - 1,
            srh_first_seg=last_entry - 1,
            srh_nh=IP_PROTOCOLS_SR,
            srh_seg_list=new_seg_list,
            inner_frame=inner_pkt)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6EndB6RedTest(SRv6Test):
    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        new_seg_list = ['3000::4', '3000::3', '3000::2']
        last_entry = len(seg_list) - 1
        seg_left = last_entry - 1
        dst_ip = IpAddr(
            addr=new_seg_list[len(new_seg_list) - 1], type_='Ipv6', prefix=128)

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(vrf, dst_ip, nexthop)

        sid = IpAddr(addr=seg_list[seg_left], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.b6.red')

        self.add_local_sid(segment, seg_list=new_seg_list)

        udp_hdr = scapy.UDP(sport=1234, dport=80, chksum=0)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[seg_left],
            ipv6_hlim=64,
            srh_seg_left=seg_left,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_UDP,
            srh_seg_list=seg_list,
            inner_frame=udp_hdr)

        inner_pkt = scapy.IPv6ExtHdrRouting(
            nh=IP_PROTOCOLS_UDP,
            type=4,
            segleft=seg_left,
            reserved=last_entry << 24,
            addresses=seg_list) / udp_hdr

        last_entry = len(new_seg_list)  # Number of segments
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_src=rmac,
            eth_dst=dmac,
            ipv6_dst=new_seg_list[last_entry - 1],
            ipv6_hlim=63,
            srh_seg_left=last_entry - 1,
            srh_first_seg=last_entry - 2,
            srh_nh=IP_PROTOCOLS_SR,
            srh_seg_list=new_seg_list[:-1],
            inner_frame=inner_pkt)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6EndB6EncapsTest(SRv6Test):
    def runTest(self):
        # Expected to fail
        return
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']
        seg_list = ['2000::5', '2000::4', '2000::3', '2000::2']
        new_seg_list = ['3000::4', '3000::3', '3000::2']
        last_entry = len(seg_list) - 1
        seg_left = last_entry - 1
        dst_ip = IpAddr(
            addr=new_seg_list[len(new_seg_list) - 1], type_='Ipv6', prefix=128)

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)
        for idx, port in enumerate([ig_port, eg_port]):
            port_idx = self.add_port(port)
            rif_ip = IpAddr(addr=rif_ips[idx], type_='Ipv6', prefix=64)
            rif = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)

        nexthop = self.add_nexthop(rif, dmac)
        self.add_fib_entry(vrf, dst_ip, nexthop)

        sid = IpAddr(addr=seg_list[0], type_='Ipv6', prefix=128)
        segment = SRv6Segment(sid, 'end.b6')

        self.add_local_sid(segment, seg_list=new_seg_list)

        udp_hdr = scapy.UDP(sport=1234, dport=80, chksum=0)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=seg_list[seg_left],
            ipv6_hlim=64,
            srh_seg_left=seg_left,
            srh_first_seg=last_entry,
            srh_nh=IP_PROTOCOLS_UDP,
            srh_seg_list=seg_list,
            inner_frame=udp_hdr)

        inner_pkt = scapy.IPv6ExtHdrRouting(
            nh=IP_PROTOCOLS_UDP,
            type=4,
            segleft=seg_left,
            reserved=last_entry << 24,
            addresses=seg_list) / udp_hdr

        last_entry = len(new_seg_list)  # Number of segments
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_src=rmac,
            eth_dst=dmac,
            ipv6_dst=new_seg_list[last_entry - 1],
            ipv6_hlim=63,
            srh_seg_left=last_entry - 1,
            srh_first_seg=last_entry - 1,
            srh_nh=IP_PROTOCOLS_SR,
            srh_seg_list=new_seg_list,
            inner_frame=inner_pkt)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])


class SRv6MobileTraditionalDownlinkTest(SRv6Test):
    def setUp(self):
        SRv6Test.setUp(self)
        self.vrf = self.add_vrf(1)
        self.mac = {}
        self.intf = {}
        self.nexthop = {}
        for idx, port in enumerate(swports):
            self.mac[idx] = '00' + ':0%d' % (idx+1) * 5
            rmac_idx = self.add_rmac_entry(self.mac[idx])
            port_idx = self.add_port(port)
            self.intf[idx] = self.add_router_interface(
                self.vrf, rmac_idx, port_idx, None)

    def runTest(self):
        ''' Packet flow - Uplink (5.1.1) '''
        A = IpAddr('1000::1')
        Z = IpAddr(addr='2000::1')
        U1 = IpAddr(addr='3000::1')
        U2 = IpAddr(addr='4000::1')
        gNB = IpAddr('5000::1')

        logger.info("Packet flow - Downlink (5.1.2)\n")

        logger.info("Added transit behavior :  T.Encaps.Reduced <U1::1>")
        nexthop = self.add_nexthop(self.intf[4], self.mac[3])
        self.add_fib_entry(self.vrf, U1, nexthop)
        transit = SRv6Transit(A, 't.encaps.red')
        self.add_transit(transit, seg_list=[U1.addr], src_addr=U2.addr)

        logger.info("Sending packet from UPF2_in : (%s, %s)", Z.addr, A.addr)
        pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[5], ipv6_src=Z.addr, ipv6_dst=A.addr, ipv6_hlim=64)
        testutils.send_packet(self, swports[5], pkt)

        logger.info(
            "Expecting packet on port on UPF2_out : (%s, %s)(%s, %s)",
            U1.addr, U2.addr, Z.addr, A.addr)
        inner_pkt = pkt.copy()
        inner_pkt['IPv6'].hlim = 63
        exp_pkt = testutils.simple_ipv6ip_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[4],
            ipv6_src=U2.addr,
            ipv6_dst=U1.addr,
            ipv6_hlim=64,
            inner_frame=inner_pkt['IPv6'])
        testutils.verify_packets(self, exp_pkt, [swports[4]])

        logger.info("Added endpoint functionality :  -> End.MAP")
        nexthop = self.add_nexthop(self.intf[2], self.mac[1])
        self.add_fib_entry(self.vrf, gNB, nexthop)
        segment = SRv6Segment(U1, 'end.map')
        self.add_local_sid(segment, sid=gNB.addr)

        logger.info("Sending packet from gNB_out : (%s, %s)(%s, %s)", U2.addr, U1.addr, Z.addr, A.addr)
        testutils.send_packet(self, swports[3], exp_pkt)

        logger.info(
            "Expecting packet on port on UPF2_out : (%s, %s)(%s, %s)", U2.addr, gNB.addr, Z.addr, A.addr)
        exp_pkt = testutils.simple_ipv6ip_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[2],
            ipv6_src=U2.addr,
            ipv6_dst=gNB.addr,
            ipv6_hlim=63,
            inner_frame=inner_pkt['IPv6'])
        testutils.verify_packets(self, exp_pkt, [swports[2]])

        logger.info("Added endpoint functionality :  -> End.DT6")
        nexthop = self.add_nexthop(self.intf[0], self.mac[5])
        self.add_fib_entry(self.vrf, A, nexthop)
        segment = SRv6Segment(gNB, 'end.dt6')
        self.add_local_sid(segment, vrf=self.vrf)

        logger.info("Sending packet from UPF1_out : (%s, %s)(%s, %s)",
                    gNB.addr, U2.addr, A.addr, Z.addr)
        testutils.send_packet(self, swports[1], exp_pkt)

        logger.info("Expecting packet on port on UPF2_out : (%s, %s)", A, Z.addr)
        pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[5],
            eth_src=self.mac[0],
            ipv6_src=Z.addr,
            ipv6_dst=A.addr,
            ipv6_hlim=63)
        testutils.verify_packets(self, pkt, [swports[0]])


class SRv6MobileTraditionalUplinkTest(SRv6Test):
    ''' This is based on the "Segment Routing IPv6 for Mobile User Plane.
        see https://tools.ietf.org/html/draft-ietf-dmm-srv6-mobile-uplane-02
    '''

    def setUp(self):
        SRv6Test.setUp(self)
        self.vrf = self.add_vrf(1)
        self.mac = {}
        self.intf = {}
        self.nexthop = {}
        for idx, port in enumerate(swports):
            self.mac[idx] = '01' + ':0%d' % (idx+0) * 5
            rmac_idx = self.add_rmac_entry(self.mac[idx])
            port_idx = self.add_port(port)
            self.intf[idx] = self.add_router_interface(
                self.vrf, rmac_idx, port_idx, None)

    def runTest(self):
        ''' Packet flow - Uplink (5.1.1) '''
        A = IpAddr('1000::1')
        Z = IpAddr(addr='2000::1')
        U1 = IpAddr(addr='3000::1')
        U2 = IpAddr(addr='4000::1')
        gNB = IpAddr('5000::1')

        logger.info("Packet flow - Uplink (5.1.1)\n")

        logger.info("Added transit behavior :  T.Encaps.Reduced <U1::1>")
        nexthop = self.add_nexthop(self.intf[1], self.mac[2])
        self.add_fib_entry(self.vrf, U1, nexthop)
        transit = SRv6Transit(Z, 't.encaps.red')
        self.add_transit(transit, seg_list=[U1.addr], src_addr=gNB.addr)

        logger.info("Sending packet from UE_out : (%s, %s)", A.addr, Z.addr)
        pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[0], ipv6_src=A.addr, ipv6_dst=Z.addr, ipv6_hlim=64)
        testutils.send_packet(self, swports[0], pkt)

        logger.info(
            "Expecting packet on port on gNB_out : (%s, %s)(%s, %s)", gNB, U1.addr, A.addr, Z.addr)
        inner_pkt = pkt.copy()
        inner_pkt['IPv6'].hlim = 63
        exp_pkt = testutils.simple_ipv6ip_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ipv6_src=gNB.addr,
            ipv6_dst=U1.addr,
            ipv6_hlim=64,
            inner_frame=inner_pkt['IPv6'])
        testutils.verify_packets(self, exp_pkt, [swports[1]])

        logger.info("Added endpoint functionality :  -> End.MAP")
        nexthop = self.add_nexthop(self.intf[3], self.mac[4])
        self.add_fib_entry(self.vrf, U2, nexthop)
        segment = SRv6Segment(U1, 'end.map')
        self.add_local_sid(segment, sid=U2.addr)

        logger.info("Sending packet from gNB_out : (%s, %s)(%s, %s)", gNB, U1.addr, A.addr, Z.addr)
        testutils.send_packet(self, swports[2], exp_pkt)

        logger.info(
            "Expecting packet on port on UPF1_out : (%s, %s)(%s, %s)", gNB, U2.addr, A.addr, Z.addr)
        exp_pkt = testutils.simple_ipv6ip_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ipv6_src=gNB.addr,
            ipv6_dst=U2.addr,
            ipv6_hlim=63,
            inner_frame=inner_pkt['IPv6'])
        testutils.verify_packets(self, exp_pkt, [swports[3]])

        logger.info("Added endpoint functionality :  -> End.DT6")
        nexthop = self.add_nexthop(self.intf[5], self.mac[0])
        self.add_fib_entry(self.vrf, Z, nexthop)
        segment = SRv6Segment(U2, 'end.dt6')
        self.add_local_sid(segment, vrf=self.vrf)

        logger.info("Sending packet from UPF1_out : (%s, %s)(%s, %s)", gNB, U2.addr, A, Z.addr)
        testutils.send_packet(self, swports[4], exp_pkt)

        logger.info("Expecting packet on port on UPF2_out : (%s, %s)", A, Z.addr)
        pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[0],
            eth_src=self.mac[5],
            ipv6_src=A.addr,
            ipv6_dst=Z.addr,
            ipv6_hlim=63)
        testutils.verify_packets(self, pkt, [swports[5]])


class GTPU(Packet):
    name = "GTPU Header"
    fields_desc = [
        BitField("verstion", 0, 3),
        BitField("pt", 0, 1),
        BitField("reserved", 0, 1),
        BitField("e", 0, 1),
        BitField("s", 0, 1),
        BitField("pn", 0, 1),
        ByteField("message_type", 0),
        ShortField("message_len", 0),
        BitField("teid", 0, 32),
    ]


class SRv6MobileEnhancedUplinkTest(SRv6Test):
    ''' This is based on the "Segment Routing IPv6 for Mobile User Plane".
        see https://tools.ietf.org/html/draft-ietf-dmm-srv6-mobile-uplane-02
    '''

    def setUp(self):
        SRv6Test.setUp(self)
        self.vrf = self.add_vrf(1)
        self.mac = {}
        self.intf = {}
        self.nexthop = {}
        for idx, port in enumerate(swports):
            self.mac[idx] = '00' + ':0%d' % (idx+1) * 5
            rmac_idx = self.add_rmac_entry(self.mac[idx])
            port_idx = self.add_port(port)
            self.intf[idx] = self.add_router_interface(
                self.vrf, rmac_idx, port_idx, None)

    def runTest(self):
        ''' Packet flow - Uplink (5.3.1.1) '''
        A = IpAddr('1000::1')
        B = IpAddr('2000::1')
        Z = IpAddr('3000::1')
        TEID = 0x1234
        U1 = IpAddr('4000::1')
        U2 = IpAddr('5000::1')
        C1 = IpAddr('6000::1')
        S1 = IpAddr('7000::1')
        gNB = IpAddr('8000::1')
        SRGW = IpAddr('9000::1')

        logger.info("Packet flow - Uplink (5.3.1.1)\n")

        logger.info("Added endpoint functionality :  End.M.GTP6.D")
        nexthop = self.add_nexthop(self.intf[1], self.mac[2])
        self.add_fib_entry(self.vrf, S1, nexthop)
        segment = SRv6Segment(B, 'end.m.gtp6.d')
        seg_list = [U2.addr, C1.addr, S1.addr]
        self.add_local_sid(segment, seg_list=seg_list, src_addr=SRGW.addr)

        logger.info("Sending packet from gNB_out : (%s, %s)(GTP: TEID: %d)(%s, %s)",
                    gNB, B, TEID, A, Z)

        inner_pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[0], ipv6_src=A.addr, ipv6_dst=Z.addr, ipv6_hlim=64)
        pkt = testutils.simple_udpv6_packet(
            eth_dst=self.mac[0],
            ipv6_src=gNB.addr,
            ipv6_dst=B.addr,
            ipv6_hlim=64,
            udp_dport=2152,
            udp_payload=GTPU(teid=TEID) / inner_pkt['IPv6'])
        testutils.send_packet(self, swports[0], pkt)

        logger.info(
            "Expecting packet on port on gNB_out : (%s, %s)(%s, %s; SL=%d)(%s, %s)",
            SRGW, S1, U2, C1, len(seg_list) - 1, A, Z)
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ipv6_dst=S1.addr,
            ipv6_src=SRGW.addr,
            ipv6_hlim=64,
            srh_seg_left=len(seg_list) - 1,
            srh_first_seg=len(seg_list) - 2,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list[:-1],
            inner_frame=inner_pkt['IPv6'])
        testutils.verify_packets(self, exp_pkt, [swports[1]])


class SRv6StaticProxyTest(SRv6Test):
    ''' Based on "Service Programming with Segment Routing".
        See https://tools.ietf.org/html/draft-xuclad-spring-sr-service-programming-01.
    '''

    def runTest(self):
        ig_port = swports[0]
        eg_port = swports[1]
        rmac = '00:11:11:11:11:11'
        dmac = '00:22:22:22:22:22'
        rif_ips = ['3000::1', '2000::1']

        vrf = self.add_vrf(1)
        rmac_idx = self.add_rmac_entry(rmac)

        port_idx = self.add_port(ig_port)
        rif_ip = IpAddr(addr=rif_ips[0], type_='Ipv6', prefix=64)
        rif1 = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)
        nexthop1 = self.add_nexthop(rif1, dmac)

        port_idx = self.add_port(eg_port)
        rif_ip = IpAddr(addr=rif_ips[1], type_='Ipv6', prefix=64)
        rif2 = self.add_router_interface(vrf, rmac_idx, port_idx, rif_ip)
        nexthop2 = self.add_nexthop(rif2, dmac)

        seg_list = ['1000::4']
        sid = IpAddr('1000::3')
        segment = SRv6Segment(sid, 'end.as')
        src_addr = '1000::2'
        self.add_local_sid(
            segment, seg_list=seg_list, src_addr=src_addr, nexthop=nexthop2, ifindex=rif2)
        self.add_fib_entry(vrf, IpAddr(seg_list[0]), nexthop1)

        inner_pkt = testutils.simple_tcpv6_packet(eth_dst=rmac)
        pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=rmac,
            ipv6_dst=sid.addr,
            ipv6_hlim=64,
            srh_seg_left=len(seg_list),
            srh_first_seg=0,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt['IPv6'])
        exp_pkt = testutils.simple_tcpv6_packet(
            eth_src=rmac, eth_dst=dmac, ipv6_hlim=64)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d (IFACE-OUT)", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        pkt = testutils.simple_tcpv6_packet(
            eth_src=dmac, eth_dst=rmac, ipv6_hlim=64)
        inner_pkt['IPv6'].hlim = 63
        exp_pkt = testutils.simple_ipv6_sr_packet(
            eth_dst=dmac,
            eth_src=rmac,
            ipv6_src=src_addr,
            ipv6_dst=seg_list[-1],
            ipv6_hlim=64,
            srh_seg_left=0,
            srh_first_seg=0,
            srh_nh=IP_PROTOCOLS_IPV6,
            srh_seg_list=seg_list,
            inner_frame=inner_pkt['IPv6'])

        logger.info("Sending packet on port %d (IFACE-IN)", eg_port)
        testutils.send_packet(self, eg_port, pkt)

        logger.info("Expecting packet on port %d", ig_port)
        testutils.verify_packets(self, exp_pkt, [ig_port])
