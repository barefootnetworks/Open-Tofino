# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config, mask
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

dev_id = 0
p4_program_name = "tna_symmetric_hash"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()


def int2ip(int_val):
    b0 = (int_val >> 24) & 0xff
    b1 = (int_val >> 16) & 0xff
    b2 = (int_val >> 8) & 0xff
    b3 = (int_val >> 0) & 0xff
    return "{}.{}.{}.{}".format(b0, b1, b2, b3)


class TestSymmetricHash(BfRuntimeTest):
    """@brief This example shows that packets with pair-wise swapped fields 
    generate the same hash when the @symmetric pragma is used for those pairs.

    We send four packets, each using the same ports and IPv4 addresses but in 
    different positions, either as src or dst. These for packets cover all
    combinations of the two addresses and ports in all compatible positions in
    the packet. After receiving the packets, we verify that all hashes 
    calculated by the switch are identical.
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
            random.seed(1)
            num_trials = 20

            for i in range(num_trials):
                ip_src = int2ip(random.randint(0, 2 ** 32 - 1))
                ip_dst = int2ip(random.randint(0, 2 ** 32 - 1))
                udp_sport = random.randint(0, 2 ** 16 - 1)
                udp_dport = random.randint(0, 2 ** 16 - 1)
                print(("  Testing IPv4 and UDP port values: {}, {}, {}, {}".
                       format(ip_src, ip_dst, udp_sport, udp_dport)))
                ipkt_1 = testutils.simple_udp_packet(eth_dst="22:22:22:22:22:22",
                                                     eth_src='00:00:00:00:00:00',
                                                     ip_src=ip_src,
                                                     ip_dst=ip_dst,
                                                     ip_id=101,
                                                     ip_ttl=64,
                                                     udp_sport=udp_sport,
                                                     udp_dport=udp_dport)

                testutils.send_packet(self, swports[0], ipkt_1)
                (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                    testutils.dp_poll(self, dev_id, swports[1], timeout=2)
                rpkt_1 = ipkt_1.__class__(rcv_pkt)

                ipkt_2 = testutils.simple_udp_packet(eth_dst="22:22:22:22:22:22",
                                                     eth_src='00:00:00:00:00:00',
                                                     ip_src=ip_dst,
                                                     ip_dst=ip_src,
                                                     ip_id=101,
                                                     ip_ttl=64,
                                                     udp_sport=udp_dport,
                                                     udp_dport=udp_sport)

                testutils.send_packet(self, swports[0], ipkt_2)
                (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                    testutils.dp_poll(self, dev_id, swports[1], timeout=2)
                rpkt_2 = ipkt_2.__class__(rcv_pkt)

                ipkt_3 = testutils.simple_udp_packet(eth_dst="22:22:22:22:22:22",
                                                     eth_src='00:00:00:00:00:00',
                                                     ip_src=ip_src,
                                                     ip_dst=ip_dst,
                                                     ip_id=101,
                                                     ip_ttl=64,
                                                     udp_sport=udp_dport,
                                                     udp_dport=udp_sport)

                testutils.send_packet(self, swports[0], ipkt_3)
                (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                    testutils.dp_poll(self, dev_id, swports[1], timeout=2)
                rpkt_3 = ipkt_3.__class__(rcv_pkt)

                ipkt_4 = testutils.simple_udp_packet(eth_dst="22:22:22:22:22:22",
                                                     eth_src='00:00:00:00:00:00',
                                                     ip_src=ip_src,
                                                     ip_dst=ip_dst,
                                                     ip_id=101,
                                                     ip_ttl=64,
                                                     udp_sport=udp_dport,
                                                     udp_dport=udp_sport)

                testutils.send_packet(self, swports[0], ipkt_4)
                (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                    testutils.dp_poll(self, dev_id, swports[1], timeout=2)
                rpkt_4 = ipkt_4.__class__(rcv_pkt)

                assert rpkt_1[Ether].src != "00:00:00:00:00:00"
                assert rpkt_1[Ether].src == rpkt_2[Ether].src == rpkt_3[Ether].src == rpkt_4[Ether].src

        finally:
            table_output_port.default_entry_reset(target)
