# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import socket
import struct

from ptf import config, mask
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

dev_id = 0
p4_program_name = "tna_proxy_hash"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()


def ip2int(addr):
    return struct.unpack("!I", socket.inet_aton(addr))[0]


def int2ip(int_val):
    b0 = (int_val >> 24) & 0xff
    b1 = (int_val >> 16) & 0xff
    b2 = (int_val >> 8) & 0xff
    b3 = (int_val >> 0) & 0xff
    return "{}.{}.{}.{}".format(b0, b1, b2, b3)


def crc8_value(val):
    import struct
    import crcmod
    crc8 = crcmod.predefined.Crc("crc-8")
    crc8.update(struct.pack(">I", val))
    return crc8.crcValue


class TestNoop(BfRuntimeTest):
    """@brief Verify packet forwarding functionality.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        ipv4_match_regular = bfrt_info.table_get("SwitchIngress.ipv4_match_regular")
        action_data = ipv4_match_regular.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[0])]
        )
        ipv4_match_regular.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='22:22:22:22:22:22',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, ipkt, swports[0])

        finally:
            ipv4_match_regular.default_entry_reset(target)


class TestMatchRegular(BfRuntimeTest):
    """@brief Test regular match table without proxy hash feature.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        ipv4_match_regular = bfrt_info.table_get("SwitchIngress.ipv4_match_regular")
        ipv4_match_regular.info.key_field_annotation_add('hdr.ethernet.dst_addr',
                                                         'mac')
        ipv4_match_regular.info.key_field_annotation_add('hdr.ethernet.src_addr',
                                                         'mac')
        ipv4_match_regular.info.key_field_annotation_add('hdr.ipv4.dst_addr',
                                                         'ipv4')
        ipv4_match_regular.info.key_field_annotation_add('hdr.ipv4.src_addr',
                                                         'ipv4')

        key_data_regular = ipv4_match_regular.make_key([
            gc.KeyTuple(name='hdr.ethernet.dst_addr',
                        value='11:11:11:11:11:11'),
            gc.KeyTuple(name='hdr.ethernet.src_addr',
                        value='22:22:22:22:22:22'),
            gc.KeyTuple(name='hdr.ipv4.dst_addr',
                        value='100.99.98.97'),
            gc.KeyTuple(name='hdr.ipv4.src_addr',
                        value='1.2.3.4'),
        ])

        action_data_regular = ipv4_match_regular.make_data(
            [gc.DataTuple(name='port_id', val=swports[3])], 'SwitchIngress.set_output_port'
        )

        ipv4_match_regular.entry_add(
            target,
            [key_data_regular],
            [action_data_regular])

        try:
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='22:22:22:22:22:22',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, ipkt, swports[3])

        finally:
            ipv4_match_regular.default_entry_reset(target)
            ipv4_match_regular.entry_del(target, [key_data_regular])


class TestMatchProxyHash(BfRuntimeTest):
    """@brief Test match table with proxy hash feature.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        ipv4_match_regular = bfrt_info.table_get("SwitchIngress.ipv4_match_regular")
        action_data = ipv4_match_regular.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[0])]
        )
        ipv4_match_regular.default_entry_set(
            target=target,
            data=action_data)

        ipv4_match_proxy_hash = bfrt_info.table_get("SwitchIngress.ipv4_match_proxy_hash")
        ipv4_match_proxy_hash.info.key_field_annotation_add('hdr.ethernet.dst_addr',
                                                            'mac')
        ipv4_match_proxy_hash.info.key_field_annotation_add('hdr.ethernet.src_addr',
                                                            'mac')
        ipv4_match_proxy_hash.info.key_field_annotation_add('hdr.ipv4.dst_addr',
                                                            'ipv4')
        ipv4_match_proxy_hash.info.key_field_annotation_add('hdr.ipv4.src_addr',
                                                            'ipv4')

        key_data_proxy_hash = ipv4_match_proxy_hash.make_key([
            gc.KeyTuple(name='hdr.ethernet.dst_addr',
                        value='11:11:11:11:11:11'),
            gc.KeyTuple(name='hdr.ethernet.src_addr',
                        value='22:22:22:22:22:22'),
            gc.KeyTuple(name='hdr.ipv4.dst_addr',
                        value='100.99.98.97'),
            gc.KeyTuple(name='hdr.ipv4.src_addr',
                        value='1.2.3.4')
        ])

        action_data_proxy_hash = ipv4_match_proxy_hash.make_data(
            [gc.DataTuple(name='port_id', val=swports[5])], 'SwitchIngress.set_output_port'
        )

        ipv4_match_proxy_hash.entry_add(
            target,
            [key_data_proxy_hash],
            [action_data_proxy_hash])

        try:
            ipkt1 = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                                eth_src='22:22:22:22:22:22',
                                                ip_src='1.2.3.4',
                                                ip_dst='100.99.98.97',
                                                ip_id=101,
                                                ip_ttl=64,
                                                udp_sport=0x1234,
                                                udp_dport=0xabcd)

            testutils.send_packet(self, swports[0], ipkt1)
            testutils.verify_packet(self, ipkt1, swports[5])

            ipkt2 = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                                eth_src='22:22:22:22:22:22',
                                                ip_src='1.2.3.4',
                                                ip_dst='98.97.100.99',
                                                ip_id=101,
                                                ip_ttl=64,
                                                udp_sport=0x1234,
                                                udp_dport=0xabcd)

            testutils.send_packet(self, swports[0], ipkt2)
            testutils.verify_packet(self, ipkt2, swports[0])

        finally:
            ipv4_match_regular.default_entry_reset(target)
            ipv4_match_proxy_hash.entry_del(target, [key_data_proxy_hash])
