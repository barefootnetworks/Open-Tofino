# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class DigestTest(BfRuntimeTest):
    """@brief Send packet packet and expect mac addresses and port as digest.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_digest"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ''' Simple test to check if a digest is received after sending a packet. '''
        ig_port = swports[2]
        smac = '00:01:02:03:04:05'
        dmac = '00:06:07:08:09:0a'

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_digest")

        # The learn object can be retrieved using a lesser qualified name on the condition
        # that it is unique
        learn_filter = bfrt_info.learn_get("digest_a")
        learn_filter.info.data_field_annotation_add("src_addr", "mac")
        learn_filter.info.data_field_annotation_add("dst_addr", "mac")

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)

        digest = self.interface.digest_get()

        recv_target = digest.target
        self.assertTrue(
            recv_target.device_id == self.device_id,
            "Error! Recv device id = %d does not match expected = %d" % (recv_target.device_id, self.device_id))
        exp_pipe_id = (ig_port >> 7) & 0x3
        self.assertTrue(
            recv_target.pipe_id == exp_pipe_id,
            "Error! Recv pipe id = %d does not match expected = %d" % (recv_target.pipe_id, exp_pipe_id))

        data_list = learn_filter.make_data_list(digest)
        data_dict = data_list[0].to_dict()

        recv_src_addr = data_dict["src_addr"]
        recv_port = data_dict["port"]
        recv_dst_addr = data_dict["dst_addr"]

        self.assertTrue(smac == recv_src_addr,
                        "Error! smac = %s received smac = %s" % (str(smac), str(recv_src_addr)))
        self.assertTrue(dmac == recv_dst_addr,
                        "Error! dmac = %s received dmac = %s" % (str(dmac), str(recv_dst_addr)))
        self.assertTrue(ig_port == recv_port,
                        "Error! port = %s received port = %s" % (str(ig_port), str(recv_port)))

class DigestTest2(BfRuntimeTest):
    """@brief Example of using the second digest defined in the P4 program.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_digest"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        port = swports[0]
        dmac1 = '00:11:22:33:44:55'
        dmac2 = 'ab:cd:ef:01:23:45'
        md1 = 0xABCDEF
        md2 = 0x123456
        pkt1 = testutils.simple_tcp_packet(eth_dst=dmac1)
        pkt2 = testutils.simple_tcp_packet(eth_dst=dmac2)
        target = gc.Target(device_id=0)

        # First get the bfrt info, it is needed to get the match table and digest.
        bfrt_info = self.interface.bfrt_info_get("tna_digest")

        # The learn object and match table can be retrieved using a lesser qualified name, not
        # specifying the pipeline name or control name, as long as it is unique.
        learn_filter = bfrt_info.learn_get("digest_b")
        learn_filter.info.data_field_annotation_add("dst_addr", "mac")
        table = bfrt_info.table_get("dmac")
        table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        # Add entries to the dmac table to generate digests.
        table.entry_add(
            target,
            [table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac1)])],
            [table.make_data([gc.DataTuple('port', port),
                              gc.DataTuple('f', md1)],
                             'SwitchIngress.dmac_hit_with_digest')])
        table.entry_add(
            target,
            [table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac2)])],
            [table.make_data([gc.DataTuple('port', port),
                              gc.DataTuple('f', md2)],
                             'SwitchIngress.dmac_hit_with_digest')])

        logger.info("Sending packets on port %d", port)
        testutils.send_packet(self, port, pkt1)
        testutils.send_packet(self, port, pkt2)

        logger.info("Packets are expected to come back on port %d", port)
        testutils.verify_packet(self, pkt1, port)
        testutils.verify_packets(self, pkt2, [port])

        # Clean up the table entries as they are no longer needed.
        table.entry_del(target)

        digest = self.interface.digest_get()

        # The digest(s) should have been generated for the correct device id and pipe id.
        recv_target = digest.target
        self.assertTrue(
            recv_target.device_id == self.device_id,
            "Error! Recv device id = %d does not match expected = %d" % (recv_target.device_id, self.device_id))
        exp_pipe_id = port >> 7
        self.assertTrue(
            recv_target.pipe_id == exp_pipe_id,
            "Error! Recv pipe id = %d does not match expected = %d" % (recv_target.pipe_id, exp_pipe_id))

        # We expect two digests generated.  However, the two digests may come in
        # a single message or separate messages.
        data_list = learn_filter.make_data_list(digest)

        data_dict = data_list[0].to_dict()
        recv_dst_addr = data_dict["dst_addr"]
        recv_md = data_dict["f1"]
        self.assertTrue(dmac1 == recv_dst_addr,
                        "Error! dmac = %s received dmac = %s" % (str(dmac1), str(recv_dst_addr)))
        self.assertTrue(md1 == recv_md,
                        "Error! md = 0x%x received port = 0x%x" % (md1, recv_md))

        if len(data_list) == 1:
            # Get the second digest message for the second learn event.
            digest = self.interface.digest_get()
            data_list += learn_filter.make_data_list(digest)
        self.assertEqual(2, len(data_list))

        data_dict = data_list[1].to_dict()
        recv_dst_addr = data_dict["dst_addr"]
        recv_md = data_dict["f1"]
        self.assertTrue(dmac2 == recv_dst_addr,
                        "Error! dmac = %s received dmac = %s" % (str(dmac2), str(recv_dst_addr)))
        self.assertTrue(md2 == recv_md,
                        "Error! md = 0x%x received port = 0x%x" % (md2, recv_md))
