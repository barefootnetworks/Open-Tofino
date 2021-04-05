# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
from collections import namedtuple
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class RangeMatchTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = [swports[5], swports[3]]

        num_entries = 5
        seed = random.randint(1, 65535)
        logger.info("Seed used %d", seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        # Make sure the table starts off empty
        target = client.Target(device_id=0, pipe_id=0xFFFF)
        resp = forward_table.entry_get(target, None, {"from_hw": False})
        for data, key in resp:
            assert 0, "Shouldn't have hit here since table is supposed to be empty"

        key_random_tuple = namedtuple('key_random', 'dst_ip pkt_length_start pkt_length_end range_size')
        tuple_list = []
        for i in range(0, num_entries):
            vrf = 0
            range_size = random.randint(1, 511)
            dst_ip = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            pkt_length_start = random.randint(60, 511)
            tuple_list.append(key_random_tuple(dst_ip, pkt_length_start, pkt_length_start + range_size, range_size))
            forward_table.entry_add(
                target,
                [forward_table.make_key([client.KeyTuple('$MATCH_PRIORITY', 1),
                                         client.KeyTuple('hdr.ipv4.dst_addr', dst_ip),
                                         client.KeyTuple('hdr.ipv4.total_len',
                                                         low=pkt_length_start,
                                                         high=pkt_length_start + range_size)])],
                [forward_table.make_data([client.DataTuple('port', eg_ports[0])],
                                         'SwitchIngress.hit')]
            )

        for i, (dip, low, high, _) in enumerate(tuple_list):
            logger.info("Rule %d:  %s total_len %d to %d forwards to port %d", i, dip, low, high, eg_ports[0])

        # send pkt and verify sent
        for i, item in enumerate(tuple_list):
            # select a random length between the range
            eth_hdr_size = 14
            dst_ip, pkt_length_start, pkt_length_end, range_size = item[0], item[1], item[2], item[3]
            pkt_len = random.randint(pkt_length_start, pkt_length_end) + eth_hdr_size
            pkt = testutils.simple_tcp_packet(pktlen=pkt_len, ip_dst=dst_ip)
            exp_pkt = pkt
            logger.info("Sending packet on port %d for rule %d with total_len %d", ig_port, i, pkt_len - eth_hdr_size)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_ports[0])
            testutils.verify_packet(self, exp_pkt, eg_ports[0])

        for i, item in enumerate(tuple_list):
            # select a length more than the range, it should be dropped
            pkt_len = pkt_length_end + eth_hdr_size + 2
            pkt = testutils.simple_tcp_packet(pktlen=pkt_len, ip_dst=dst_ip)
            exp_pkt = pkt
            logger.info("Sending packet on port %d to miss rule %d with total_len %d", ig_port, i,
                        pkt_len - eth_hdr_size)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)

        # check get
        for item in tuple_list:
            dst_ip, pkt_length_start, pkt_length_end, range_size = item[0], item[1], item[2], item[3]
            resp = forward_table.entry_get(
                target,
                [forward_table.make_key([client.KeyTuple('$MATCH_PRIORITY', 1),
                                         client.KeyTuple('hdr.ipv4.dst_addr',
                                                         dst_ip),
                                         client.KeyTuple('hdr.ipv4.total_len',
                                                         low=pkt_length_start,
                                                         high=pkt_length_start + range_size)])],
                {"from_hw": True})

            data_dict = next(resp)[0].to_dict()
            recv_port = data_dict["port"]
            if (recv_port != eg_ports[0]):
                logger.error("Error! port sent = %s received port = %s", str(eg_ports[0]), str(recv_port))
                assert 0

        # send pkt and verify sent
        for i, item in enumerate(tuple_list):
            eth_hdr_size = 14
            dst_ip, pkt_length_start, pkt_length_end, range_size = item[0], item[1], item[2], item[3]
            pkt_len = random.randint(pkt_length_start, pkt_length_end) + eth_hdr_size
            pkt = testutils.simple_tcp_packet(pktlen=pkt_len, ip_dst=dst_ip)
            exp_pkt = pkt
            logger.info("Sending packet on port %d for rule %d with total_len %d", ig_port, i, pkt_len - eth_hdr_size)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_ports[0])
            testutils.verify_packet(self, exp_pkt, eg_ports[0])

            forward_table.entry_del(
                target,
                [forward_table.make_key([client.KeyTuple('$MATCH_PRIORITY', 1),
                                         client.KeyTuple('hdr.ipv4.dst_addr',
                                                         dst_ip),
                                         client.KeyTuple('hdr.ipv4.total_len',
                                                         low=pkt_length_start,
                                                         high=pkt_length_start + range_size)])])

        # send pkt and verify dropped
        for item in tuple_list:
            eth_hdr_size = 14
            dst_ip, pkt_length_start, pkt_length_end, range_size = item[0], item[1], item[2], item[3]
            pkt_len = random.randint(pkt_length_start, pkt_length_end) + eth_hdr_size
            pkt = testutils.simple_tcp_packet(pktlen=pkt_len, ip_dst=dst_ip)
            logger.info("Sending packet on port %d with total_len %d", ig_port, pkt_len - eth_hdr_size)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self, timeout=2)
