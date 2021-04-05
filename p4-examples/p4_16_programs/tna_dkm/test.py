# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config
from ptf.thriftutils import *
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


def port_to_pipe(port):
    local_port = port & 0x7F
    assert (local_port < 72)
    pipe = (port >> 7) & 0x3
    assert (port == ((pipe << 7) | local_port))
    return pipe


swports_0 = []
swports_1 = []
swports_2 = []
swports_3 = []
# the following method categorizes the ports in ports.json file as belonging to either of the pipes (0, 1, 2, 3)
for port in swports:
    pipe = port_to_pipe(port)
    if pipe == 0:
        swports_0.append(port)
    elif pipe == 1:
        swports_1.append(port)
    elif pipe == 2:
        swports_2.append(port)
    elif pipe == 3:
        swports_3.append(port)


class DKMTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def send_and_verify_packet(self, ingress_port, egress_port, pkt, exp_pkt):
        logger.info("Sending packet on port %d", ingress_port)
        testutils.send_packet(self, ingress_port, pkt)
        logger.info("Expecting packet on port %d", egress_port)
        testutils.verify_packet(self, exp_pkt, egress_port)

    def runTest(self):
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'
        dkey = '22:22:22:22:22:23'
        dmask = 'ff:ff:ff:ff:ff:f0'
        port_mask = 0
        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        pkt2 = testutils.simple_tcp_packet(eth_dst=dkey)
        exp_pkt = pkt
        exp_pkt2 = pkt2

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_dkm")

        # Set the scope of the table to ALL_PIPES
        logger.info("=============== Testing Dyn Key Mask ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        logger.info("set dyn key mask")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr",
                                                    "mac")
        key_mask = forward_table.make_key(
            [gc.KeyTuple('hdr.ethernet.dst_addr', dmask),
             gc.KeyTuple('ig_intr_md.ingress_port', port_mask)])
        forward_table.attribute_dyn_key_mask_set(target, key_mask)
        resp = forward_table.attribute_get(target, "DynamicKeyMask")
        for d in resp:
            assert d["fields"].to_dict()["ig_intr_md.ingress_port"]["value"] == port_mask
            assert d["fields"].to_dict()["hdr.ethernet.dst_addr"]["value"] == dmask

        logger.info("Add entry")
        key_list = [forward_table.make_key(
            [gc.KeyTuple('hdr.ethernet.dst_addr', dmac),
             gc.KeyTuple('ig_intr_md.ingress_port', swports_0[0])])]
        data_list = [forward_table.make_data([gc.DataTuple('port', eg_port)],
                                             "SwitchIngress.hit")]
        forward_table.entry_add(target, key_list, data_list)

        self.send_and_verify_packet(swports_0[0], eg_port, pkt2, exp_pkt2)
        self.send_and_verify_packet(swports_1[0], eg_port, pkt2, exp_pkt2)
        if int(testutils.test_param_get('num_pipes')) > 2:
            self.send_and_verify_packet(swports_2[0], eg_port, pkt2, exp_pkt2)
        if int(testutils.test_param_get('num_pipes')) > 3:
            self.send_and_verify_packet(swports_3[0], eg_port, pkt2, exp_pkt2)
        self.send_and_verify_packet(swports_0[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_1[0], eg_port, pkt, exp_pkt)
        if int(testutils.test_param_get('num_pipes')) > 2:
            self.send_and_verify_packet(swports_2[0], eg_port, pkt, exp_pkt)
        if int(testutils.test_param_get('num_pipes')) > 3:
            self.send_and_verify_packet(swports_3[0], eg_port, pkt, exp_pkt)
        testutils.verify_no_other_packets(self, timeout=2)
        logger.info("Delete the entry")
        forward_table.entry_del(target, key_list)
