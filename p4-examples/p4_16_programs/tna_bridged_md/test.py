# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config, mask
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc
import scapy.all

dev_id = 0
p4_program_name = "tna_bridged_md"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()


class NoBridgedMdTest(BfRuntimeTest):
    """@brief Verify that the program forwards packets as expected.
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
                                               eth_src='22:33:44:55:66:77',
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


class BridgedMdIgIntrMdTest(BfRuntimeTest):
    """@brief Forward ingress_intrinsic_md to the egress pipeline using bridged
    metadata.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info
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

        # Add bridge_md to ig_intr_md
        table_bridge_md_ctl = bfrt_info.table_get("SwitchIngress.bridge_md_ctrl")

        action_data = table_bridge_md_ctl.make_data(
            [],
            action_name="SwitchIngress.bridge_add_ig_intr_md"
        )

        table_bridge_md_ctl.default_entry_set(
            target=target,
            data=action_data)

        try:
            input_ports = list(range(6))

            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='22:33:44:55:66:77',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt_tmpl = testutils.simple_tcp_packet(eth_dst='00:00:00:00:00:02',
                                                    eth_src='22:33:44:55:66:77',
                                                    ip_src='1.2.3.4',
                                                    ip_dst='100.99.98.97',
                                                    ip_id=101,
                                                    ip_ttl=64,
                                                    tcp_sport=0x1234,
                                                    tcp_dport=0xabcd,
                                                    with_tcp_chksum=True)

            epkts = []
            for p in input_ports:
                epkt = epkt_tmpl.copy()[scapy.all.Ether]
                dmac = "00:00:00:00:{:02x}:{:02x}".format(swports[p] >> 8, swports[p] & 0xFF)
                epkt.dst = dmac
                epkts.append(epkt)

            for p in input_ports:
                testutils.send_packet(self, swports[p], ipkt)
                testutils.verify_packet(self, epkts[p], swports[1])

        finally:
            table_output_port.default_entry_reset(target)
            table_bridge_md_ctl.default_entry_reset(target)


class BridgedMdExampleHdrTest(BfRuntimeTest):
    """@brief Forward a custom struct to the egress pipeline using bridged
    metadata.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info
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

        # Add bridge_md to ig_intr_md
        table_bridge_md_ctl = bfrt_info.table_get(
            "SwitchIngress.bridge_md_ctrl")

        action_data = table_bridge_md_ctl.make_data(
            [
                gc.DataTuple(name="dst_mac_addr_low",
                             val=0xffeeddcc),
                gc.DataTuple(name="src_mac_addr_low",
                             val=0x11223344),
            ],
            action_name="SwitchIngress.bridge_add_example_hdr"
        )

        table_bridge_md_ctl.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt = testutils.simple_tcp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='22:22:22:22:22:22',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               tcp_sport=0x1234,
                                               tcp_dport=0xabcd,
                                               with_tcp_chksum=True)

            epkt = testutils.simple_tcp_packet(eth_dst='00:00:ff:ee:dd:cc',
                                               eth_src='00:00:11:22:33:44',
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
            table_bridge_md_ctl.default_entry_reset(target)
