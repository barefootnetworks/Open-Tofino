# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

##### Required for GRPC #####
from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client
#### ****************** #####

##### Required for Thrift #####
from ptf.thriftutils import *
import pd_base_tests
from res_pd_rpc.ttypes import *

##### ******************* #####


logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class TnaWithPdfixedThriftTest(BfRuntimeTest):
    class FixedInterface(pd_base_tests.ThriftInterfaceDataPlane):
        def __init__(self, p4names):
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self,
                                                            p4names)

        def setUp(self):
            pd_base_tests.ThriftInterfaceDataPlane.setUp(self)

        def runTest(self):
            pass

    def setUp(self):
        client_id = 0
        p4_name = "bri_with_pdfixed_thrift"
        self.fixedObject = self.FixedInterface([p4_name])
        # Initialize the fixed API thirft interface through setUp
        self.fixedObject.setUp()
        # Initialize the GRPC conn through BfRuntimeTest setUp
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'
        device_id = 0

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("bri_with_pdfixed_thrift")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        exp_pkt = pkt

        target = client.Target(device_id=0, pipe_id=0xffff)
        forward_table.entry_add(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
            [forward_table.make_data([client.DataTuple('port', eg_port)],
                                     'SwitchIngress.hit')]
        )

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        forward_table.entry_del(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)

        def portToPipe(port):
            return port >> 7

        def portToPipeLocalId(port):
            return port & 0x7F

        def portToBitIdx(port):
            pipe = portToPipe(port)
            index = portToPipeLocalId(port)
            return 72 * pipe + index

        def set_port_map(indicies):
            bit_map = [0] * ((288 + 7) // 8)
            for i in indicies:
                index = portToBitIdx(i)
                bit_map[index // 8] = (bit_map[index // 8]
                                       | (1 << (index % 8))) & 0xFF
            return bytes_to_string(bit_map)

        def make_port(pipe, local_port):
            assert pipe >= 0 and pipe < 4
            return pipe << 7 | local_port

        # Creating mc groups and adding nodes via thrift based fixed APIs
        # randomly take 20 MGIDs from 1000
        possible_ids = list(range(1000))
        random.shuffle(possible_ids)
        mgids = possible_ids[:20]

        mgrp_hdls = []
        mgid_to_hdl = {}
        mgrp_node_hdls = []

        mc_sess_hdl = self.fixedObject.mc.mc_create_session()
        for mgid in mgids:
            mgrp_hdl = self.fixedObject.mc.mc_mgrp_create(mc_sess_hdl, device_id, hex_to_i16(mgid))
            mgid_to_hdl[mgid] = mgrp_hdl
        # Add nodes to the MGIDs
        for mgid in mgids:
            mgrp_hdl = mgid_to_hdl[mgid]
            mgrp_hdls.append(mgrp_hdl)
            # Determine how many L1s per MGID
            copy_count = random.randint(1, 2)
            # Determine the membership for the group
            members = random.sample(swports, random.randint(0, len(swports)))
            port_map = set_port_map(members)
            lag_map = set_port_map([])
            # Create the nodes
            l1_hdls = []
            for rid in range(copy_count):
                l1_hdl = self.fixedObject.mc.mc_node_create(mc_sess_hdl, device_id, hex_to_i16(rid), port_map, lag_map)
                l1_hdls.append(l1_hdl)
            mgrp_node_hdls.append(list(l1_hdls))
            # Add the nodes to the group
            for l1_hdl in l1_hdls:
                status = self.fixedObject.mc.mc_associate_node(mc_sess_hdl, device_id, mgrp_hdl, l1_hdl, 0, 0)

        # Cleanup
        for mgrp, mbrs in zip(mgrp_hdls, mgrp_node_hdls):
            for mbr in mbrs:
                self.fixedObject.mc.mc_dissociate_node(mc_sess_hdl, device_id, mgrp, mbr)
                self.fixedObject.mc.mc_node_destroy(mc_sess_hdl, device_id, mbr)
        for mgrp in mgrp_hdls:
            self.fixedObject.mc.mc_mgrp_destroy(mc_sess_hdl, device_id, mgrp)

        self.fixedObject.mc.mc_complete_operations(mc_sess_hdl)
        self.fixedObject.mc.mc_destroy_session(mc_sess_hdl)
