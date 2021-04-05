# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
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


class DirectCounterSyncTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_operations"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_operations")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        target = client.Target(device_id=0, pipe_id=0xffff)
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")
        forward_table.entry_add(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.src_addr', smac,
                                                     smac_mask),
                                     client.KeyTuple('$MATCH_PRIORITY', 0)])],
            [forward_table.make_data([client.DataTuple('port', eg_port),
                                      client.DataTuple('$COUNTER_SPEC_BYTES', 0),
                                      client.DataTuple('$COUNTER_SPEC_PKTS', 0)],
                                     'SwitchIngress.hit')]
        )

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 3
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            testutils.verify_packets(self, exp_pkt, [eg_port])

        logger.info("Expecting packet on port %d", eg_port)

        # Get count from sw. It should be 0
        resp = forward_table.entry_get(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.src_addr', smac,
                                                     smac_mask),
                                     client.KeyTuple('$MATCH_PRIORITY', 0)])],
            {"from_hw": False})

        # parse resp to get the counter
        data_dict = next(resp)[0].to_dict()
        recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
        recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

        # should be 0
        if (0 != recv_pkts):
            logger.error("Error! expected 0 received count = %s", str(recv_pkts))
            assert 0

        if (0 != recv_bytes):
            logger.error("Error! expected 0 received count = %s", str(recv_bytes))
            assert 0

        # apply table op to sync counters on the direct table
        forward_table.operations_execute(target, 'SyncCounters')

        # Get count from sw. It should be correct
        resp = forward_table.entry_get(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.src_addr', smac,
                                                     smac_mask),
                                     client.KeyTuple('$MATCH_PRIORITY', 0)])],
            {"from_hw": False},
            forward_table.make_data(
                [client.DataTuple("$COUNTER_SPEC_BYTES"),
                 client.DataTuple("$COUNTER_SPEC_PKTS")],
                'SwitchIngress.hit')
        )

        # parse resp to get the counter
        data_dict = next(resp)[0].to_dict()
        recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
        recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

        if (num_pkts != recv_pkts):
            logger.error("Error! packets sent = %s received count = %s", str(num_pkts), str(recv_pkts))
            assert 0

        if (num_bytes != recv_bytes):
            logger.error("Error! bytes sent = %s received count = %s", str(num_bytes), str(recv_bytes))
            assert 0

        forward_table.entry_del(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.src_addr', smac,
                                                     smac_mask),
                                     client.KeyTuple('$MATCH_PRIORITY', 0)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class IndirectCounterSyncTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_operations"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_operations")

        forward_dst_table = bfrt_info.table_get("SwitchIngress.forward_dst")
        forward_dst_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        indirect_counter_table = bfrt_info.table_get("SwitchIngress.indirect_counter")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        target = client.Target(device_id=0, pipe_id=0xffff)
        # insert entry in MAT and the indirect counter table both
        forward_dst_table.entry_add(
            target,
            [forward_dst_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
            [forward_dst_table.make_data([client.DataTuple('port', eg_port)],
                                         'SwitchIngress.hit_dst')]
        )

        indirect_counter_table.entry_add(
            target,
            [indirect_counter_table.make_key([client.KeyTuple('$COUNTER_INDEX', eg_port)])],
            [indirect_counter_table.make_data([client.DataTuple('$COUNTER_SPEC_BYTES', 0),
                                               client.DataTuple('$COUNTER_SPEC_PKTS', 0)])])

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 6
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            testutils.verify_packets(self, exp_pkt, [eg_port])

        logger.info("Expecting packet on port %d", eg_port)

        # Get from sw and check its value. They should be 0
        resp = indirect_counter_table.entry_get(
            target,
            [indirect_counter_table.make_key([client.KeyTuple('$COUNTER_INDEX', eg_port)])],
            {"from_hw": False})
        # parse resp to get the counter
        data_dict = next(resp)[0].to_dict()
        recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
        recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

        if (0 != recv_pkts):
            logger.error("Error! expected 0 received count = %s", str(recv_pkts))
            assert 0

        if (0 != recv_bytes):
            logger.error("Error! expected 0 received count = %s", str(recv_bytes))
            assert 0

        # apply table op to sync counters on the indirect table
        indirect_counter_table.operations_execute(target, 'Sync')

        # Get from sw and check its value. They should be the correct values now
        resp = indirect_counter_table.entry_get(
            target,
            [indirect_counter_table.make_key([client.KeyTuple('$COUNTER_INDEX', eg_port)])],
            {"from_hw": False})

        # parse resp to get the counter
        data_dict = next(resp)[0].to_dict()
        recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
        recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

        if (num_pkts != recv_pkts):
            logger.error("Error! packets sent = %s received count = %s", str(num_pkts), str(recv_pkts))
            assert 0

        if (num_bytes != recv_bytes):
            logger.error("Error! bytes sent = %s received count = %s", str(num_bytes), str(recv_bytes))
            assert 0

        forward_dst_table.entry_del(
            target,
            [forward_dst_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])
