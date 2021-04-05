# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import struct

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc
import scapy.all

dev_id = 0
p4_program_name = "tna_checksum"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()


class Ipv4ChksumTest(BfRuntimeTest):
    """@brief Base test for the program: verify that a correct checksum is not
    altered.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = ipkt

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)


class Ipv4ChksumErrNoUpdTest(BfRuntimeTest):
    """@brief Verify that the program identifies a bad IPv4 checksum.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)
            # Set incorrect checksum
            ipkt[scapy.all.IP].chksum = 0x4321

            # The default behavior of the pipeline is to detect incorrect
            # checksums, but not to correct them. Therefore, we expect
            # to receive a packet with an incorrect checkum.
            epkt = testutils.simple_tcp_packet(eth_dst='00:00:de:ad:be:ef',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)
            epkt[scapy.all.IP].chksum = 0x4321

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)


class Ipv4ChksumErrUpdTest(BfRuntimeTest):
    """@brief Check if the program identifies a bad IPv4 checksum and is able
    to correct it.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.checksum_upd_ipv4_tcp_udp",
            data_field_list_in=[gc.DataTuple(name="update", val=0x1)]
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)
            # Set incorrect checksum
            ipkt[scapy.all.IP].chksum = 0x4321

            # We expect the program to correct the IPv4 checksum.
            epkt = testutils.simple_tcp_packet(eth_dst='00:00:de:ad:be:ef',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4TranslateNoUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address translation (snat).
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.snat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="update", val=0x0)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=False)
            # We expect the packet to have the same checksum as the packet
            # we sent in. Calling str forces scapy to calculate the checksum
            # for the given layer.
            epkt[scapy.all.IP].chksum = ipkt[scapy.all.IP].__class__(
                str(ipkt[scapy.all.IP])).chksum
            epkt[scapy.all.TCP].chksum = ipkt[scapy.all.TCP].__class__(
                str(ipkt[scapy.all.TCP])).chksum

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4TranslateUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address translation (snat) and check if 
    the program corrects the bad checksum for both, IPv4 and TCP.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.snat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class TcpTranslateNoUpdTest(BfRuntimeTest):
    """@brief Apply source TCP port translation (stpat).
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.stpat",
            data_field_list_in=[
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x0)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x4321,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=False)
            # We expect the packet to have the same TCP checksum as the packet
            # we sent in. Calling str forces scapy to calculate the checksum
            # for the given layer.
            epkt[scapy.all.TCP].chksum = ipkt[scapy.all.TCP].__class__(
                str(ipkt[scapy.all.TCP])).chksum

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class TcpTranslateUpdTest(BfRuntimeTest):
    """@brief Apply source TCP port translation (stpat) and check if the program 
    corrects the bad TCP checksum.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.stpat",
            data_field_list_in=[
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x4321,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4TcpTranslateNoUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address and TCP port translation (sntpat).
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.sntpat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x0)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x4321,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=False)
            # We expect the packet to have the same TCP checksum as the packet
            # we sent in. Calling str forces scapy to calculate the checksum
            # for the given layer.
            epkt[scapy.all.IP].chksum = ipkt[scapy.all.IP].__class__(
                str(ipkt[scapy.all.IP])).chksum
            epkt[scapy.all.TCP].chksum = ipkt[scapy.all.TCP].__class__(
                str(ipkt[scapy.all.TCP])).chksum

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4TcpTranslateUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address and TCP port translation (sntpat) 
    and check if the program corrects the bad IPv4 and TCP checksum.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.sntpat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x4321,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class UdpTranslateNoUpdTest(BfRuntimeTest):
    """@brief Apply source UDP port translation (supat).
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.supat",
            data_field_list_in=[
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x0)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=False)
            # We expect the packet to have the same TCP checksum as the packet
            # we sent in. Calling str forces scapy to calculate the checksum
            # for the given layer.
            epkt[scapy.all.UDP].chksum = ipkt[scapy.all.UDP].__class__(
                str(ipkt[scapy.all.UDP])).chksum

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class UdpTranslateUpdTest(BfRuntimeTest):
    """@brief Apply source UDP port translation (supat) and check if the program 
    corrects the bad UDP checksum.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.supat",
            data_field_list_in=[
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4UdpTranslateNoUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address and UDP port translation (snupat).
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.snupat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x0)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=False)
            # We expect the packet to have the same TCP checksum as the packet
            # we sent in. Calling str forces scapy to calculate the checksum
            # for the given layer.
            epkt[scapy.all.IP].chksum = ipkt[scapy.all.IP].__class__(
                str(ipkt[scapy.all.IP])).chksum
            epkt[scapy.all.UDP].chksum = ipkt[scapy.all.UDP].__class__(
                str(ipkt[scapy.all.UDP])).chksum

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4UdpTranslateUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address and UDP port translation (snupat) 
    and check if the program corrects the bad IPv4 and UDP checksum.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.snupat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            # Send packet with checksum, should be updated
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])

            # Send packet without checksum, should not be updated
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=False)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=False)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)


class Ipv4UdpTranslateSpecialUpdTest(BfRuntimeTest):
    """@brief Apply source IPv4 network address and UDP port translation (snupat) 
    and check if the program corrects the bad IPv4 and UDP checksum for special
    cases when the checksum is 0x0000 before or after the translation.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        # Set a default entry in the rewrite table so all packets get
        # their checksum updated.
        table_translate = bfrt_info.table_get("SwitchIngress.translate")

        action_data = table_translate.make_data(
            action_name="SwitchIngress.snupat",
            data_field_list_in=[
                gc.DataTuple(name="src_addr", val=gc.ipv4_to_bytes("4.3.2.1")),
                gc.DataTuple(name="src_port", val=gc.to_bytes(0x4321, 2)),
                gc.DataTuple(name="update", val=0x1)
            ],
        )

        table_translate.default_entry_set(
            target=target,
            data=action_data)

        try:
            # Test special cases
            # The UDP checksum value 0x0000 denotes the absence of the optional
            # checksum. Therefore, if the actual checksum value is 0x0000, the
            # checksum value is flipped to be 0xffff.

            # Find payload value for UDP checksum 0xffff before translation
            # Uncomment if to find new payload value when the input values have
            # changed.
            # for i in range(2**16):
            #     tpkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
            #                                 eth_src='00:00:00:00:00:00',
            #                                 ip_src='1.2.3.4',
            #                                 ip_dst='100.99.98.97',
            #                                 ip_id=101,
            #                                 ip_ttl=64,
            #                                 udp_sport=0x1234,
            #                                 udp_dport=0xabcd,
            #                                 udp_payload = struct.pack("!I", i),
            #                                 with_udp_chksum = True)
            #     # Force checksum calculation
            #     checksum = tpkt[scapy.all.UDP].__class__(str(tpkt[scapy.all.UDP])).chksum
            #     print("Packet: {}, payload: {}, checksum: {}".format(hash(tpkt),
            #                                                          hex(i),
            #                                                          hex(checksum)))
            #     if hex(checksum) == '0xffff':
            #         print("Found payload with hash value 0xffff: {}".format(i))
            #         break
            # Payload for UDP checksum 0xffff: 46530, actual UDP csum value: 0x0

            # Find payload value for UDP checksum 0xffff after translation
            # Uncomment if to find new payload value when the input values have
            # changed.
            # for i in range(2**16):
            #     tpkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
            #                                 eth_src='00:00:00:00:00:00',
            #                                 ip_src='4.3.2.1',
            #                                 ip_dst='100.99.98.97',
            #                                 ip_id=101,
            #                                 ip_ttl=64,
            #                                 udp_sport=0x4321,
            #                                 udp_dport=0xabcd,
            #                                 udp_payload = struct.pack("!I", i),
            #                                 with_udp_chksum = True)
            #     # Force checksum calculation
            #     checksum = tpkt[scapy.all.UDP].__class__(str(tpkt[scapy.all.UDP])).chksum
            #     print("Packet: {}, payload: {}, checksum: {}".format(hash(tpkt),
            #                                                          hex(i),
            #                                                          hex(checksum)))
            #     if hex(checksum) == '0xffff':
            #         print("Found payload with hash value 0xffff: {}".format(i))
            #         break
            # Payload for UDP checksum 0xffff: 33495, actual UDP csum value: 0x0

            # Send packet with checksum 0xffff, should be updated
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               udp_payload=struct.pack("!I", 46530),
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               udp_payload=struct.pack("!I", 46530),
                                               with_udp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])

            # Send packet with checksum, that should be updated to be 0x0000,
            # and therefore requires the P4 program to flips the bits to be 0xffff
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               udp_payload=struct.pack("!I", 33495),
                                               with_udp_chksum=True)

            epkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='00:00:00:00:00:00',
                                               ip_src='4.3.2.1',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x4321,
                                               udp_dport=0xabcd,
                                               udp_payload=struct.pack("!I", 33495),
                                               with_udp_chksum=True)

            testutils.send_packet(self, swports[0], ipkt)
            testutils.verify_packet(self, epkt, swports[1])
        finally:
            table_output_port.default_entry_reset(target)
            table_translate.default_entry_reset(target)
