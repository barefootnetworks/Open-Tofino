# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import grpc
import time
import random

from ptf import config
from ptf.thriftutils import *
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import google.rpc.code_pb2 as code_pb2
from functools import partial

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


class ExactMatchTest(BfRuntimeTest):
    '''@brief This test adds an exact match entry and runs traffic test
        1. Add an entry to forward_table
        2. Send and verify packet
        3. Delete entry
        4. verify dropped packet
    '''

    def setUp(self):
        client_id = 0
        p4_name = "tna_exact_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()

        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        exp_pkt = pkt

        target = gc.Target(device_id=0, pipe_id=0xffff)

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        key_list = [forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)])]
        data_list = [forward_table.make_data([gc.DataTuple('port', eg_port)],
                                             "SwitchIngress.hit")]

        forward_table.entry_add(target, key_list, data_list)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)
        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        forward_table.entry_del(target)

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)
        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class EntryScopeTest(BfRuntimeTest):
    '''@brief This test adds an exact match entry to a table in different pipe scopes
        1. Set entry scope attributes of the table to create 2 pipe_scopes (pipe0, pipe1)
            and (pipe2, pipe3) using user defined entry scope attribtues
        2. Add entry to inidividual scopes and test packet behavior
        3. Set entry scope to ALL_PIPES, add entry and test packet behavior
        4. Set Entry scope to Single pipe, add entry and test packet behavior
    '''

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def send_and_verify_packet(self, ingress_port, egress_port, pkt, exp_pkt):
        logger.info("Sending packet on port %d", ingress_port)
        testutils.send_packet(self, ingress_port, pkt)
        logger.info("Expecting packet on port %d", egress_port)
        testutils.verify_packets(self, exp_pkt, [egress_port])

    def send_and_verify_no_other_packet(self, ingress_port, pkt):
        logger.info("Sending packet on port %d (negative test); expecting no packet", ingress_port)
        testutils.send_packet(self, ingress_port, pkt)
        testutils.verify_no_other_packets(self)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'

        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        exp_pkt = pkt

        # Run this test only for 4 pipe devices
        if int(testutils.test_param_get('num_pipes')) != 4:
            logger.info("Skipping Entry scope test for a non 4 pipe device")
            return

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_exact_match")

        # Set pipes 0 and 1 in scope 1 and pipes 2 and 3 in scope 2
        logger.info(
            "=============== Tesing User Defined Scope: Scope 1 (pipe 0 and 1), Scope 2 (pipe 2 and 3) ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.attribute_entry_scope_set(target, predefined_pipe_scope=False, user_defined_pipe_scope_val=0xc03)
        resp = forward_table.attribute_get(target, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert "predef" not in d["pipe_scope"]
            assert d["pipe_scope"]["user_defined"] == 0xc03
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL

        target = gc.Target(device_id=0, pipe_id=0x0)
        # Install entry in pipes 0 and 1
        key_list = [forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', gc.mac_to_bytes(dmac))])]
        data_list = [forward_table.make_data([gc.DataTuple('port', eg_port)], "SwitchIngress.hit")]
        forward_table.entry_add(target, key_list, data_list)

        # Since we have installed the entry in scope 1 (pipe 0 and 1) only, we expect the packet to hit the entries in pipes 0 and 1
        # and miss in pipes 2 and 3
        self.send_and_verify_packet(swports_0[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_1[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_no_other_packet(swports_2[0], pkt)
        self.send_and_verify_no_other_packet(swports_3[0], pkt)

        logger.info("Delete the entry from the first scope (pipe 0, 1)")
        forward_table.entry_del(target, key_list)

        logger.info("Add the entry in the other scope (pipe 2, 3) as well")
        target = gc.Target(device_id=0, pipe_id=0x2)
        # install entry in pipes 2 and 3
        forward_table.entry_add(target, key_list, data_list)

        # Since we have deleted the entry from pipes 0 and 1 and installed the entry in scope 2,
        # we expect the packet to hit the entries in pipe 2 and 3
        self.send_and_verify_packet(swports_2[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_3[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_no_other_packet(swports_0[0], pkt)
        self.send_and_verify_no_other_packet(swports_1[0], pkt)

        logger.info("Delete the entry from the second scope (pipe 2, 3)")
        forward_table.entry_del(target, key_list)

        # Since we have deleted the entry from all the pipes, we expect the packet to miss in all pipes
        self.send_and_verify_no_other_packet(swports_0[0], pkt)
        self.send_and_verify_no_other_packet(swports_1[0], pkt)
        self.send_and_verify_no_other_packet(swports_2[0], pkt)
        self.send_and_verify_no_other_packet(swports_3[0], pkt)

        # Set the scope of the table to ALL_PIPES
        logger.info("=============== Testing All Pipes Scope ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        forward_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)
        resp = forward_table.attribute_get(target, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL
        logger.info("Add entry")
        # Install entry in all the pipes
        forward_table.entry_add(target, key_list, data_list)
        # Since we have installed the entry in all the pipes, we expect all the pipes to hit
        self.send_and_verify_packet(swports_0[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_1[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_2[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_packet(swports_3[0], eg_port, pkt, exp_pkt)
        logger.info("Delete the entry")
        forward_table.entry_del(target, key_list)

        # Set the scope of the table to SINGLE_PIPE
        logger.info("=============== Testing Single Pipe Scope ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        forward_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE)
        resp = forward_table.attribute_get(target, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.SINGLE
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL

        logger.info("Adding entry in pipe 0")
        target = gc.Target(device_id=0, pipe_id=0x0)
        # Install entry in pipe 0 only
        forward_table.entry_add(target, key_list, data_list)
        # Since the entry is installed in pipe 0 only, we expect that to hit and all other pipes to fail
        self.send_and_verify_packet(swports_0[0], eg_port, pkt, exp_pkt)
        self.send_and_verify_no_other_packet(swports_1[0], pkt)
        logger.info("Adding entry in pipe 1")
        target = gc.Target(device_id=0, pipe_id=0x1)
        # Install entry in pipe 1 as well
        forward_table.entry_add(target, key_list, data_list)
        # Now the packet should hit entries in pipe 1 as well
        self.send_and_verify_packet(swports_1[0], eg_port, pkt, exp_pkt)

        # Finally delete entries from both pipe 0 and 1
        logger.info("Delete entries from both pipe 0 and 1")
        target = gc.Target(device_id=0, pipe_id=0x0)
        forward_table.entry_del(target, key_list)
        target = gc.Target(device_id=0, pipe_id=0x1)
        forward_table.entry_del(target, key_list)

        # Reset the enry scope of the table to ALL_PIPES
        logger.info("Reset the Scope to All pipes")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        forward_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)
        resp = forward_table.attribute_get(target, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL


class ExactMatchGetTest(BfRuntimeTest):
    '''@brief This test adds multiple exact match entries to ip_route table and performs entry_get
        1. Enter some entries as part of action "route" and some as part of "nat"
        2. Get the entries and verify.
        3. Delete the entries
    '''

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        ig_port = swports[1]
        num_entries = 100
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_exact_match")

        vrfs = [x for x in range(num_entries)]
        ipDstAddrs = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]

        nat_action_data = {}
        nat_action_data['ipSrcAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        nat_action_data['ipDstAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        nat_action_data['ports'] = [swports[random.randint(1, 5)] for x in range(num_entries)]

        route_action_data = {}
        route_action_data['srcMacAddrs'] = [":".join("%02x" % i for i in [random.randint(0, 255) for j in range(6)]) for
                                            k in range(num_entries)]
        route_action_data['dstMacAddrs'] = [":".join("%02x" % i for i in [random.randint(0, 255) for j in range(6)]) for
                                            k in range(num_entries)]
        route_action_data['ports'] = [swports[random.randint(1, 5)] for x in range(num_entries)]

        action_choices = ['SwitchIngress.route', 'SwitchIngress.nat']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        target = gc.Target(device_id=0, pipe_id=0xffff)

        logger.info("Adding %d entries on ipRoute Table", num_entries)

        iproute_table = bfrt_info.table_get("SwitchIngress.ipRoute")
        iproute_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
        key_list = []
        data_list = []
        meter_data = {}
        meter_data['cir'] = [1000 * random.randint(1, 1000) for i in range(num_entries)]
        meter_data['pir'] = [meter_data['cir'][i] * random.randint(1, 5) for i in range(num_entries)]
        meter_data['cbs'] = [1000 * random.randint(1, 100) for i in range(num_entries)]
        meter_data['pbs'] = [meter_data['cbs'][i] * random.randint(1, 5) for i in range(num_entries)]
        for x in range(num_entries):
            dip = ipDstAddrs[x]
            vrf = vrfs[x]
            if action[x] == 'SwitchIngress.route':
                data_list.append(iproute_table.make_data(
                    [gc.DataTuple('srcMac', route_action_data['srcMacAddrs'][x]),
                     gc.DataTuple('dstMac', route_action_data['dstMacAddrs'][x]),
                     gc.DataTuple('dst_port', route_action_data['ports'][x]),
                     gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                     gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                     gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                     gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                    "SwitchIngress.route"))
            elif action[x] == 'SwitchIngress.nat':
                data_list.append(iproute_table.make_data(
                    [gc.DataTuple('srcAddr', nat_action_data['ipSrcAddrs'][x]),
                     gc.DataTuple('dstAddr', nat_action_data['ipDstAddrs'][x]),
                     gc.DataTuple('dst_port', nat_action_data['ports'][x]),
                     gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                     gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                     gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                     gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                    "SwitchIngress.nat"))
            key_list.append(iproute_table.make_key([gc.KeyTuple('vrf', vrf),
                                                    gc.KeyTuple('hdr.ipv4.dst_addr', dip)]))

        iproute_table.entry_add(target, key_list, data_list)

        logger.info("DONE adding %d entries on ipRoute Table", num_entries)
        logger.info("Reading %d entries on ipRoute Table", num_entries)

        resp = iproute_table.entry_get(target, key_list, {"from_hw": True})
        x = 0
        for data, key in resp:
            data_fields = data.to_dict()
            key_fields = key.to_dict()
            logger.info("Verifying entry %d for action %s", x, action[x])
            assert data_fields["action_name"] == action[x]
            if action[x] == 'SwitchIngress.route':
                assert data_fields['srcMac'] == route_action_data['srcMacAddrs'][x]
                assert data_fields['dstMac'] == route_action_data['dstMacAddrs'][x]
                assert data_fields['dst_port'] == route_action_data['ports'][x]
            elif action[x] == 'SwitchIngress.nat':
                assert data_fields['srcAddr'] == nat_action_data['ipSrcAddrs'][x]
                assert data_fields['dstAddr'] == nat_action_data['ipDstAddrs'][x]
                assert data_fields['dst_port'] == nat_action_data['ports'][x]
            x += 1

        logger.info("Deleting entries")
        iproute_table.entry_del(target, key_list)

class ExactMatchEntryIteratorTest(BfRuntimeTest):
    '''@brief This test adds multiple exact match entries to ip_route table and performs entry_get all. This test
        also tests out usage_get.
        1. Enter some entries as part of action "route" and some as part of "nat"
        2. Get table usage
        3. Get table All by passing in key_list as None and verify them
        4. Delete a few entries and verify table usage.
        5. Delete the rest of the entries
    '''

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        num_entries = 2
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_exact_match")

        vrfs = [x for x in range(num_entries)]
        ipDstAddrs = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]

        nat_action_data = {}
        nat_action_data['ipSrcAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        nat_action_data['ipDstAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        nat_action_data['ports'] = [random.randint(1, 5) for x in range(num_entries)]

        route_action_data = {}
        route_action_data['srcMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        route_action_data['dstMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        route_action_data['ports'] = [random.randint(1, 5) for x in range(num_entries)]

        action_choices = ['SwitchIngress.route', 'SwitchIngress.nat']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        target = gc.Target(device_id=0, pipe_id=0xffff)

        default_action = action_choices[random.randint(0, 1)]
        nat_default_action_data = {}
        nat_default_action_data['ipSrcAddr'] = "%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        nat_default_action_data['ipDstAddr'] = "%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        nat_default_action_data['port'] = random.randint(1, 5)

        route_default_action_data = {}
        route_default_action_data['srcMacAddr'] = "%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255))
        route_default_action_data['dstMacAddr'] = "%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255))
        route_default_action_data['port'] = random.randint(1, 5)

        logger.info("Adding default entry with %s action", default_action)
        iproute_table = bfrt_info.table_get("SwitchIngress.ipRoute")

        iproute_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        meter_data = {}
        meter_data['cir'] = [1000 * random.randint(1, 1000) for i in range(num_entries + 1)]
        meter_data['pir'] = [meter_data['cir'][i] * random.randint(1, 5) for i in range(num_entries + 1)]
        meter_data['cbs'] = [1000 * random.randint(1, 100) for i in range(num_entries + 1)]
        meter_data['pbs'] = [meter_data['cbs'][i] * random.randint(1, 5) for i in range(num_entries + 1)]
        if default_action == 'SwitchIngress.route':
            iproute_table.default_entry_set(
                target,
                iproute_table.make_data(
                    [gc.DataTuple('srcMac', route_default_action_data['srcMacAddr']),
                     gc.DataTuple('dstMac', route_default_action_data['dstMacAddr']),
                     gc.DataTuple('dst_port', route_default_action_data['port']),
                     gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][num_entries]),
                     gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][num_entries]),
                     gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][num_entries]),
                     gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][num_entries])],
                    'SwitchIngress.route'))
        else:
            iproute_table.default_entry_set(
                target,
                iproute_table.make_data(
                    [gc.DataTuple('srcAddr', nat_default_action_data['ipSrcAddr']),
                     gc.DataTuple('dstAddr', nat_default_action_data['ipDstAddr']),
                     gc.DataTuple('dst_port', nat_default_action_data['port']),
                     gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][num_entries]),
                     gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][num_entries]),
                     gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][num_entries]),
                     gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][num_entries])],
                    'SwitchIngress.nat'))

        logger.info("Adding %d entries on ipRoute Table", num_entries)

        key_list = []
        data_list = []
        for x in range(num_entries):
            dip = ipDstAddrs[x]
            vrf = vrfs[x]
            if action[x] == 'SwitchIngress.route':
                srcMac = route_action_data['srcMacAddrs'][x]
                dstMac = route_action_data['dstMacAddrs'][x]
                port = route_action_data['ports'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcMac', srcMac),
                                                          gc.DataTuple('dstMac', dstMac),
                                                          gc.DataTuple('dst_port', port),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "SwitchIngress.route"))
            elif action[x] == 'SwitchIngress.nat':
                srcIp = nat_action_data['ipSrcAddrs'][x]
                dstIp = nat_action_data['ipDstAddrs'][x]
                port = nat_action_data['ports'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcAddr', srcIp),
                                                          gc.DataTuple('dstAddr', dstIp),
                                                          gc.DataTuple('dst_port', port),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "SwitchIngress.nat"))
            key_list.append(iproute_table.make_key([gc.KeyTuple('vrf', vrf),
                                                    gc.KeyTuple('hdr.ipv4.dst_addr', dip)]))

        iproute_table.entry_add(target, key_list, data_list)

        logger.info("DONE adding %d entries on ipRoute Table", num_entries)
        # Get Table Usage
        usage = next(iproute_table.usage_get(target))
        assert usage == num_entries, "Usage = %d num_entries = %d" % (usage, num_entries)
        logger.info("Current entries = %d as expected", usage)

        # Get all the entries
        logger.info("Getting All %d entries", num_entries)
        resp = iproute_table.entry_get(target,
                                       None,
                                       {"from_hw": random.choice([True, False])})

        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            assert key_dict["vrf"]['value'] == vrfs[i]
            assert key_dict["hdr.ipv4.dst_addr"]['value'] == ipDstAddrs[i]
            assert data_dict["action_name"] == action[i]
            if action[i] == 'SwitchIngress.route':
                assert data_dict['srcMac'] == route_action_data['srcMacAddrs'][i]
                assert data_dict['dstMac'] == route_action_data['dstMacAddrs'][i]
                assert data_dict['dst_port'] == route_action_data['ports'][i]
            elif action[i] == 'SwitchIngress.nat':
                assert data_dict['srcAddr'] == nat_action_data['ipSrcAddrs'][i]
                assert data_dict['dstAddr'] == nat_action_data['ipDstAddrs'][i]
                assert data_dict['dst_port'] == nat_action_data['ports'][i]
            logger.info("SUCCESS : Entry %d matched", i)
            i += 1

        resp = iproute_table.default_entry_get(target)
        for data, _ in resp:
            data_dict = data.to_dict()
            assert data_dict["is_default_entry"] == True
            assert data_dict["action_name"] == default_action, \
                "expected %s, received %s" % (default_action, data_dict["action_name"])
            if default_action == 'SwitchIngress.route':
                assert data_dict['srcMac'] == route_default_action_data['srcMacAddr']
                assert data_dict['dstMac'] == route_default_action_data['dstMacAddr']
                assert data_dict['dst_port'] == route_default_action_data['port']
            else:
                assert data_dict['srcAddr'] == nat_default_action_data['ipSrcAddr']
                assert data_dict['dstAddr'] == nat_default_action_data['ipDstAddr']
                assert data_dict['dst_port'] == nat_default_action_data['port']
            logger.info("SUCCESS : Default entry matched")

        logger.info("SUCCESS : All entries read matched")

        logger.info("Deleting entries")

        checkpoint = random.randint(1, num_entries)
        logger.info("Checkpoint = %d", checkpoint)
        for x in range(num_entries):
            iproute_table.entry_del(target, [key_list[x]])

            # Get Table Usage
            if (x == checkpoint - 1):
                usage_resp = next(iproute_table.usage_get(target))
                assert usage_resp == num_entries - checkpoint
                logger.info("Current entries = %d as expected", usage_resp)

        # Reset the default entry
        iproute_table.default_entry_reset(target)


class BfruntimeClientKeyDataTest(BfRuntimeTest):
    '''@brief This test adds creates and adds _Key and _Data pairs to a table and tests the
    following
        1. Test _Key and _Data object dictionaries
        2. Test _Key and _Data object comparison with entries added and entries get
        3. Test _Key and _Data list sorting by field or action_name
        4. Test _Key and _Data __getitem__ and __setitem__
    '''

    def setUp(self):
        client_id = 0
        p4_name = "tna_exact_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        num_entries = 20
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_exact_match")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        iproute_table = bfrt_info.table_get("SwitchIngress.ipRoute")

        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        iproute_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        vrfs = [x for x in range(num_entries)]
        ipDstAddrs = ["%d.%d.%d.%d" % (random.randint(1, 255), random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]

        nat_action_data = {}
        nat_action_data['ipSrcAddrs'] = ["%d.%d.%d.%d" % (random.randint(1, 255), random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        nat_action_data['ipDstAddrs'] = ["%d.%d.%d.%d" % (random.randint(1, 255), random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        nat_action_data['ports'] = [random.randint(1, 5) for x in range(num_entries)]

        route_action_data = {}
        route_action_data['srcMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (random.randint(0, 255), random.randint(0, 255), random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        route_action_data['dstMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (random.randint(0, 255), random.randint(0, 255), random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        route_action_data['ports'] = [random.randint(1, 5) for x in range(num_entries)]

        action_choices = ['SwitchIngress.route', 'SwitchIngress.nat']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        target = gc.Target(device_id=0, pipe_id=0xffff)

        meter_data = {}
        meter_data['cir'] = [1000*random.randint(1, 1000) for i in range(num_entries+1)]
        meter_data['pir'] = [meter_data['cir'][i]*random.randint(1, 5) for i in range(num_entries+1)]
        meter_data['cbs'] = [1000*random.randint(1, 100) for i in range(num_entries+1)]
        meter_data['pbs'] = [meter_data['cbs'][i]*random.randint(1, 5) for i in range(num_entries+1)]

        logger.info("Adding %d entries on ipRoute Table", num_entries)
        # 1. Test _Key and _Data object dictionaries addition
        key_list = []
        pair_dict = dict()
        reverse_pair_dict = dict()
        data_list = []
        key_to_test_hash = None
        for x in range(num_entries):
            dip = ipDstAddrs[x]
            vrf = vrfs[x]
            if action[x] == 'SwitchIngress.route':
                srcMac = route_action_data['srcMacAddrs'][x]
                dstMac = route_action_data['dstMacAddrs'][x]
                port = route_action_data['ports'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcMac', srcMac),
                                                          gc.DataTuple('dstMac', dstMac),
                                                          gc.DataTuple('dst_port', port),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "SwitchIngress.route"))
            elif action[x] == 'SwitchIngress.nat':
                srcIp = nat_action_data['ipSrcAddrs'][x]
                dstIp = nat_action_data['ipDstAddrs'][x]
                port = nat_action_data['ports'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcAddr', srcIp),
                                                          gc.DataTuple('dstAddr', dstIp),
                                                          gc.DataTuple('dst_port', port),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "SwitchIngress.nat"))
            key_list.append(iproute_table.make_key([gc.KeyTuple('vrf', vrf),
                                                    gc.KeyTuple('hdr.ipv4.dst_addr', dip)]))
            if x == num_entries-1:
                key_to_test_hash = iproute_table.make_key([gc.KeyTuple('vrf', vrf),
                                                           gc.KeyTuple('hdr.ipv4.dst_addr', dip)])
            pair_dict[key_list[-1]] = data_list[-1]
            reverse_pair_dict[data_list[-1]] = key_list[-1]
        assert key_to_test_hash in pair_dict

        logger.info("Printing dict of _Key:_Data")
        for key, data in list(pair_dict.items()):
            logger.info("%s --> %s", str(key), str(data))
        logger.info("Printing dict of _Data:_Key")
        for data, key in list(reverse_pair_dict.items()):
            logger.info("%s --> %s", str(data), str(key))

        iproute_table.entry_add(target, key_list, data_list)

        logger.info("DONE adding %d entries on ipRoute Table", num_entries)
        # Get Table Usage
        usage = next(iproute_table.usage_get(target))
        assert usage == num_entries, "Usage = %d num_entries = %d" % (usage, num_entries)
        logger.info("Current entries = %d as expected", usage)

        # 2. Test _Key and _Data object comparison with entries added and entries get
        # Get all the entries
        logger.info("Getting All %d entries", num_entries)
        resp = iproute_table.entry_get(target,
                                       None,
                                       {"from_hw": random.choice([True, False])})
        sent_key_list = list(pair_dict.keys())
        sent_data_list = list(pair_dict.values())
        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            assert key_dict["vrf"]['value'] == vrfs[i]
            assert key_dict["hdr.ipv4.dst_addr"]['value'] == ipDstAddrs[i]
            assert data_dict["action_name"] == action[i]
            if action[i] == 'SwitchIngress.route':
                assert data_dict['srcMac'] == route_action_data['srcMacAddrs'][i]
                assert data_dict['dstMac'] == route_action_data['dstMacAddrs'][i]
                assert data_dict['dst_port'] == route_action_data['ports'][i]
            elif action[i] == 'SwitchIngress.nat':
                assert data_dict['srcAddr'] == nat_action_data['ipSrcAddrs'][i]
                assert data_dict['dstAddr'] == nat_action_data['ipDstAddrs'][i]
                assert data_dict['dst_port'] == nat_action_data['ports'][i]
            # remove the received _Key and _Data objects from the sent lists.
            # The meter data received won't be equal so we need a filter operation
            sent_key_list.remove(key)
            sent_data_list = [x for x in sent_data_list if (
                x.action_name == data.action_name == "SwitchIngress.route" and
                x["srcMac"] == data["srcMac"] and
                x["dstMac"] == data["dstMac"]) or
                (x.action_name == data.action_name == "SwitchIngress.nat" and
                 x["srcAddr"] == data["srcAddr"] and
                 x["dstAddr"] == data["dstAddr"])]
            logger.info("SUCCESS : Entry %d matched and removed from sent list", i)
            i += 1
        assert len(sent_key_list) == 0
        assert len(sent_data_list) == 0
        iproute_table.entry_del(target)

        forward_key_list = [forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', "22:22:22:22:22:22")]),
                            forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', "22:22:22:22:22:33")])
                            ]
        forward_data_list = [forward_table.make_data([gc.DataTuple('port', 1)],
                                                     "SwitchIngress.hit"),
                             forward_table.make_data([gc.DataTuple('port', 2)],
                                                     "SwitchIngress.hit"),
                             ]
        forward_table.entry_add(target, forward_key_list, forward_data_list)
        resp = forward_table.entry_get(target)
        for data, key in resp:
            assert data in forward_data_list,\
                "%s not in forward_data_list" % (str(data))
            assert key in forward_key_list,\
                "%s not in forward_key_list" % (str(key))
        forward_table.entry_del(target)

        # 3. Test _Key and _Data list sorting by field or action_name
        test_key_list = key_list[:]
        test_data_list = data_list[:]

        logger.info("\Before Sorting--")
        for item in test_data_list:
            logger.info(item)

        test_data_list.sort(key=lambda x: x.action_name)
        sorted_actions = sorted(action)
        i = 0
        logger.info("\nAfter Sorting with action name--")
        for item in test_data_list:
            logger.info(item)
            assert item.action_name == sorted_actions[i]
            i += 1
        # Sort using srcMac. If not applicable, then use srcAddr to sort
        test_data_list.sort(key=lambda x: x["srcMac"] if "srcMac" in x else x["srcAddr"])
        logger.info("\nAfter Sorting with srcMac and srcAddr --")
        for item in test_data_list:
            logger.info(item)

        try:
            data_list[0]["$METER_SPEC_CIR_KBPS"] = gc.DataTuple("$METER_SPEC_PIR_KBPS", 200)
            assert 0, "The above statement should have raised exception"
        except KeyError as e:
            pass

        try:
            data_list[0]["$METER_SPEC"] = gc.DataTuple("$METER_SPEC_CIR_KBPS", 200)
            assert 0, "The above statement should have raised exception"
        except KeyError as e:
            pass
        try:
            data_list[0]["$METER_SPEC_CIR_KBPS"] = gc.DataTuple("$METER_SPEC", 200)
            assert 0, "The above statement should have raised exception"
        except KeyError as e:
            pass

        # 4. Test _Key and _Data __getitem__ and __setitem__
        data0 = test_data_list[0]
        logger.info("before = %s" % (str(data0)))
        data0["$METER_SPEC_CIR_KBPS"] = gc.DataTuple("$METER_SPEC_CIR_KBPS", 200)
        logger.info("after = %s" % (str(data0)))

        key0 = test_key_list[0]
        logger.info("before = %s" % (str(key0)))
        key0["vrf"] = gc.KeyTuple("vrf", 30)
        logger.info("after = %s" % (str(key0)))


class ExactMatchModifyTest(BfRuntimeTest):
    '''@brief This test adds multiple exact match entries to iproute table and modifies the entries.
    It tests out modify in various scenarios like modifying only direct resource entries.
       1. Adds 100 exact match entries
       2. Sends packets to all 100 entries and verifies.
       3. Modifies all 100 match entries.
       4. Sends packets to all modified entries and verifies.
       5. Modifies just the direct counter spec and direct meter spec of entries
       6. Reads back the direct counter spec and direct meter spec and verify.
    '''

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def _create_get_req_data(self, iproute_table, action_list):
        data_list = []
        for x in range(len(action_list)):
            if action_list[x] == 'SwitchIngress.route':
                # Yes, semi qualified names work for action anmes, data fields and key-fields.
                # So just "route" is acceptable. Even if there was a SwitchEgress.route,
                # it wouldn't have been present in the scope of this table since this table
                # is in the control block SwitchIngress itself, so just "route" would have
                # worked even then
                data_list.append(iproute_table.make_data([gc.DataTuple('$COUNTER_SPEC_BYTES'),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS'),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS'),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS'),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS'),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS')],
                                                         "route", get=True))
            elif action_list[x] == 'SwitchIngress.nat':
                data_list.append(iproute_table.make_data([gc.DataTuple('$COUNTER_SPEC_BYTES'),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS'),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS'),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS'),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS'),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS')],
                                                         "nat", get=True))
        return data_list

    def _create_data_cntrs_mtrs(self, iproute_table, cntr_data, meter_data, action_list):
        data_list = []
        for x in range(len(action_list)):
            if action_list[x] == 'SwitchIngress.route':
                data_list.append(iproute_table.make_data([gc.DataTuple('$COUNTER_SPEC_BYTES', cntr_data['bytes'][x]),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS', cntr_data['packets'][x]),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "route"))
            elif action_list[x] == 'SwitchIngress.nat':
                data_list.append(iproute_table.make_data([gc.DataTuple('$COUNTER_SPEC_BYTES', cntr_data['bytes'][x]),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS', cntr_data['packets'][x]),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "nat"))
        return data_list

    def _create_data(self, iproute_table, route_action_data, nat_action_data, eg_ports, cntr_data, meter_data,
                     action_list):
        data_list = []
        for x in range(len(action_list)):
            if action_list[x] == 'SwitchIngress.route':
                srcMac = route_action_data['srcMacAddrs'][x]
                dstMac = route_action_data['dstMacAddrs'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcMac', srcMac),
                                                          gc.DataTuple('dstMac', dstMac),
                                                          gc.DataTuple('dst_port', eg_ports[x]),
                                                          gc.DataTuple('$COUNTER_SPEC_BYTES', cntr_data['bytes'][x]),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS', cntr_data['packets'][x]),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "route"))
            elif action_list[x] == 'SwitchIngress.nat':
                srcIp = nat_action_data['ipSrcAddrs'][x]
                dstIp = nat_action_data['ipDstAddrs'][x]
                data_list.append(iproute_table.make_data([gc.DataTuple('srcAddr', srcIp),
                                                          gc.DataTuple('dstAddr', dstIp),
                                                          gc.DataTuple('dst_port', eg_ports[x]),
                                                          gc.DataTuple('$COUNTER_SPEC_BYTES', cntr_data['bytes'][x]),
                                                          gc.DataTuple('$COUNTER_SPEC_PKTS', cntr_data['packets'][x]),
                                                          gc.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                                                          gc.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                                                          gc.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                                                          gc.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])],
                                                         "nat"))
        return data_list

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        num_entries = 100
        ig_ports = [random.choice(swports) for x in range(num_entries)]
        all_ports = swports_0 + swports_1 + swports_2 + swports_3
        eg_ports = [random.choice(all_ports) for x in range(num_entries)]

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()

        target = gc.Target(device_id=0, pipe_id=0xffff)

        iproute_table = bfrt_info.table_get("SwitchIngress.ipRoute")
        ipDstAddrs = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]

        nat_action_data = {}
        nat_action_data['ipSrcAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        nat_action_data['ipDstAddrs'] = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]

        route_action_data = {}
        route_action_data['srcMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        route_action_data['dstMacAddrs'] = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]

        cntr_data = {}
        cntr_data['packets'] = [random.randint(500, 2000) for i in range(num_entries)]
        cntr_data['bytes'] = [random.randint(500, 2000) * 64 for i in range(num_entries)]

        meter_data = {}
        meter_data['cir'] = [1000 * random.randint(1, 1000) for i in range(num_entries)]
        meter_data['pir'] = [meter_data['cir'][i] * random.randint(1, 5) for i in range(num_entries)]
        meter_data['cbs'] = [1000 * random.randint(1, 100) for i in range(num_entries)]
        meter_data['pbs'] = [meter_data['cbs'][i] * random.randint(1, 5) for i in range(num_entries)]

        action_choices = ['SwitchIngress.route', 'SwitchIngress.nat']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        target = gc.Target(device_id=0, pipe_id=0xffff)

        iproute_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
        # Add entries
        logger.info("Adding %d entries to SwitchIngress.ipRoute table", num_entries)
        key_list = []
        data_list = []
        for x in range(num_entries):
            dip = ipDstAddrs[x]
            key_list.append(iproute_table.make_key([gc.KeyTuple('vrf', 0),
                                                    gc.KeyTuple('hdr.ipv4.dst_addr', dip)]))

        data_list = self._create_data(iproute_table, route_action_data, nat_action_data, eg_ports, cntr_data,
                                      meter_data, action)

        iproute_table.entry_add(target, key_list, data_list)

        logger.info("DONE adding %d entries to SwitchIngress.ipRoute table", num_entries)

        logger.info("Sending packets to all %d entries of SwitchIngress.ipRoute table", num_entries)
        # Send packets
        for x in range(num_entries):
            pkt = testutils.simple_tcp_packet(ip_dst=ipDstAddrs[x], with_tcp_chksum=False)
            if action[x] == 'SwitchIngress.route':
                exp_pkt = testutils.simple_tcp_packet(eth_dst=route_action_data['dstMacAddrs'][x],
                                                      eth_src=route_action_data['srcMacAddrs'][x],
                                                      ip_dst=ipDstAddrs[x],
                                                      with_tcp_chksum=False)
            elif action[x] == 'SwitchIngress.nat':
                exp_pkt = testutils.simple_tcp_packet(ip_dst=nat_action_data['ipDstAddrs'][x],
                                                      ip_src=nat_action_data['ipSrcAddrs'][x],
                                                      with_tcp_chksum=False)

            logger.info("Sending packet on port %d", ig_ports[x])
            testutils.send_packet(self, ig_ports[x], pkt)
            logger.info("Expecting packet on port %d", eg_ports[x])
            testutils.verify_packet(self, exp_pkt, eg_ports[x])

        testutils.verify_no_other_packets(self, timeout=2)

        logger.info("DONE sending packets to all %d entries of SwitchIngress.ipRoute table", num_entries)

        # Entry modify
        logger.info("Modifying %d entries of SwitchIngress.ipRoute table", num_entries)
        random.shuffle(action)

        random.shuffle(route_action_data['srcMacAddrs'])
        random.shuffle(route_action_data['dstMacAddrs'])

        random.shuffle(nat_action_data['ipSrcAddrs'])
        random.shuffle(nat_action_data['ipDstAddrs'])

        random.shuffle(eg_ports)
        data_list = self._create_data(iproute_table, route_action_data, nat_action_data, eg_ports, cntr_data,
                                      meter_data, action)
        iproute_table.entry_mod(target, key_list, data_list)

        logger.info("DONE Modifying %d entries of SwitchIngress.ipRoute table", num_entries)

        logger.info("Sending packets to all %d modified entries of SwitchIngress.ipRoute table", num_entries)
        # Send packets
        for x in range(num_entries):
            pkt = testutils.simple_tcp_packet(ip_dst=ipDstAddrs[x], with_tcp_chksum=False)
            if action[x] == 'SwitchIngress.route':
                exp_pkt = testutils.simple_tcp_packet(eth_dst=route_action_data['dstMacAddrs'][x],
                                                      eth_src=route_action_data['srcMacAddrs'][x],
                                                      ip_dst=ipDstAddrs[x],
                                                      with_tcp_chksum=False)
            elif action[x] == 'SwitchIngress.nat':
                exp_pkt = testutils.simple_tcp_packet(ip_dst=nat_action_data['ipDstAddrs'][x],
                                                      ip_src=nat_action_data['ipSrcAddrs'][x],
                                                      with_tcp_chksum=False)

            logger.info("Sending packet on port %d", ig_ports[x])
            testutils.send_packet(self, ig_ports[x], pkt)
            logger.info("Expecting packet on port %d", eg_ports[x])
            testutils.verify_packet(self, exp_pkt, eg_ports[x])

        testutils.verify_no_other_packets(self, timeout=2)
        logger.info("DONE Sending packets to all %d modified entries of SwitchIngress.ipRoute table", num_entries)
        # Direct resource modify
        logger.info("Modifying direct counter and direct meter for  %d entries of SwitchIngress.ipRoute table",
                    num_entries)
        random.shuffle(cntr_data['packets'])
        random.shuffle(cntr_data['bytes'])

        random.shuffle(meter_data['cir'])
        meter_data['pir'] = [meter_data['cir'][i] * random.randint(1, 5) for i in range(num_entries)]
        random.shuffle(meter_data['cbs'])
        meter_data['pbs'] = [meter_data['cbs'][i] * random.randint(1, 5) for i in range(num_entries)]

        data_list = self._create_data_cntrs_mtrs(iproute_table, cntr_data, meter_data, action)
        iproute_table.entry_mod(target, key_list, data_list)

        logger.info("DONE Modifying direct counter and direct meter for  %d entries of SwitchIngress.ipRoute table",
                    num_entries)
        # Now read back the entries to verify the resource update has happened

        logger.info("Reading direct counter and direct meter for  %d entries of SwitchIngress.ipRoute table",
                    num_entries)

        try:
            get_data_list = self._create_get_req_data(iproute_table, action)
            for x in range(num_entries):
                logger.info("Trying to read entry %d", x)
                dip = ipDstAddrs[x]
                from_hw = random.choice([True, False])
                resp = iproute_table.entry_get(target,
                                               [key_list[x]],
                                               {"from_hw": from_hw},
                                               get_data_list[x])
                data, _ = next(resp)
                fields = data.to_dict()
                recv_cir = fields["$METER_SPEC_CIR_KBPS"]
                recv_pir = fields["$METER_SPEC_PIR_KBPS"]
                recv_cbs = fields["$METER_SPEC_CBS_KBITS"]
                recv_pbs = fields["$METER_SPEC_PBS_KBITS"]

                # Read back meter values are not always the same. It should be within a 2% error rate

                if abs(recv_cir - meter_data['cir'][x]) > meter_data['cir'][x] * 0.02:
                    assert 0
                if abs(recv_pir - meter_data['pir'][x]) > meter_data['pir'][x] * 0.02:
                    assert 0
                if abs(recv_cbs - meter_data['cbs'][x]) > meter_data['cbs'][x] * 0.02:
                    assert 0
                if abs(recv_pbs - meter_data['pbs'][x]) > meter_data['pbs'][x] * 0.02:
                    assert 0

                expected_bytes = cntr_data['bytes'][x]
                expected_pkts = cntr_data['packets'][x]

                assert expected_bytes == fields["$COUNTER_SPEC_BYTES"]
                assert expected_pkts == fields["$COUNTER_SPEC_PKTS"]
                # assert fields["action_name"] == action[x]

                logger.info("Entry %d matched", x)

            logger.info("DONE Reading direct counter and direct meter for  %d entries of SwitchIngress.ipRoute table",
                        num_entries)

        except gc.BfruntimeRpcException as e:
            gc.print_grpc_error(e)
            raise e

        finally:
            logger.info("Clearing all entries of SwitchIngress.ipRoute table")
            iproute_table.entry_del(target)
            logger.info("Verifying whether all have been deleted")
            assert next(iproute_table.usage_get(target)) == 0, \
                "usage = %s expected = 0" % (iproute_table.usage_get)
            for data, key in iproute_table.entry_get(target):
                assert 0, "Not expecting any entries here"


class ExactMatchDefaultEntryTest(BfRuntimeTest):
    '''@brief This test sets default entry with action route, then sets default with
        action nat and then resets it
    '''

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        ig_port = swports[1]
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_exact_match")

        iproute_table = bfrt_info.table_get("SwitchIngress.ipRoute")

        nat_action_data = {}
        nat_action_data['ipSrcAddr'] = "%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        nat_action_data['ipDstAddr'] = "%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        nat_action_data['port'] = swports[random.randint(1, 5)]

        route_action_data = {}
        route_action_data['srcMacAddr'] = "%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255))
        route_action_data['dstMacAddr'] = "%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255))
        route_action_data['port'] = swports[random.randint(1, 5)]

        target = gc.Target(device_id=0, pipe_id=0xffff)

        logger.info("Adding default entry with route action on ipRoute Table")
        iproute_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")
        iproute_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.nat", "ipv4")
        iproute_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        iproute_table.default_entry_set(
            target,
            iproute_table.make_data(
                [gc.DataTuple('srcMac', route_action_data['srcMacAddr']),
                 gc.DataTuple('dstMac', route_action_data['dstMacAddr']),
                 gc.DataTuple('dst_port', route_action_data['port'])],
                "SwitchIngress.route"))

        logger.info("Sending a packet to match the default entry with route action")
        pkt = testutils.simple_tcp_packet()
        exp_pkt = testutils.simple_tcp_packet(eth_dst=route_action_data['dstMacAddr'],
                                              eth_src=route_action_data['srcMacAddr'])

        testutils.send_packet(self, ig_port, pkt)
        testutils.verify_packet(self, exp_pkt, route_action_data['port'])
        logger.info("Received expected packet for default entry with route action")

        logger.info("Reading default entry with route action")

        resp = iproute_table.default_entry_get(target)

        data, _ = next(resp)

        fields = data.to_dict()
        logger.info("Verifying default entry with route action")
        assert fields["action_name"] == "SwitchIngress.route"
        assert fields['srcMac'] == route_action_data['srcMacAddr']
        assert fields['dstMac'] == route_action_data['dstMacAddr']
        assert fields['dst_port'] == route_action_data['port']

        iproute_table.default_entry_set(
            target,
            iproute_table.make_data(
                [gc.DataTuple('srcAddr', nat_action_data['ipSrcAddr']),
                 gc.DataTuple('dstAddr', nat_action_data['ipDstAddr']),
                 gc.DataTuple('dst_port', nat_action_data['port'])],
                'SwitchIngress.nat'))

        logger.info("Sending a packet to match the default entry with nat action")
        pkt = testutils.simple_tcp_packet(with_tcp_chksum=False)
        exp_pkt = testutils.simple_tcp_packet(ip_dst=nat_action_data['ipDstAddr'],
                                              ip_src=nat_action_data['ipSrcAddr'],
                                              with_tcp_chksum=False)
        testutils.send_packet(self, ig_port, pkt)
        testutils.verify_packet(self, exp_pkt, nat_action_data['port'])
        logger.info("Received expected packet for default entry with nat action")

        logger.info("Reading default entry with nat action")

        resp = iproute_table.default_entry_get(target)

        data, _ = next(resp)
        fields = data.to_dict()
        assert fields["action_name"] == "SwitchIngress.nat"
        assert fields['srcAddr'] == nat_action_data['ipSrcAddr']
        assert fields['dstAddr'] == nat_action_data['ipDstAddr']
        assert fields['dst_port'] == nat_action_data['port']
        # Reset the default entry
        logger.info("Resetting the default entry")
        iproute_table.default_entry_reset(target)
        logger.info("Sending a packet to ensure the default entry has been reset")
        testutils.send_packet(self, ig_port, pkt)
        testutils.verify_no_other_packets(self)

        # Reset the default entry
        logger.info("Resetting the default entry as part of cleanup")
        iproute_table.default_entry_reset(target)

class ExactMatchConstDefaultEntryTest(BfRuntimeTest):
    '''@brief forward table has a const default action as miss(0x1).
        This test does the following
        1. Try and set the value of default_entry with miss(0x0). Should fail (const defaultAction)
        2. Try and set the value of default_entry with hit(0x0). Should fail (const defaultAction)
        3. Try and set action miss(0x1) for a regular entry. Should fail (miss is defaultOnly)
    '''

    @staticmethod
    def try_and_check_func(func, err_str, error_expected):
        error_received = False
        try:
            func()
        except gc.BfruntimeRpcException as e:
            # The error list should only have one error since the write
            # request should have failed
            error_list = e.sub_errors_get()
            assert len(error_list) == 1
            assert error_list[0][1].canonical_code == code_pb2.INVALID_ARGUMENT
            error_received = True
        assert error_received == error_expected, err_str

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        ig_port = swports[1]
        dmac = '22:22:22:22:22:22'
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        # 1. Try and set/reset the value of default_entry with miss(0x0). Should fail (const)
        bound_f = partial(forward_table.default_entry_set,
                          target,
                          forward_table.make_data(
                              [gc.DataTuple('drop', 0)],
                              "SwitchIngress.miss")
                          )
        self.try_and_check_func(bound_f, "Error was expected while trying to set "
                "default entry since it is a const default action", True)

        # reset_default_action is a no-op if default action is const
        bound_f = partial(forward_table.default_entry_reset, target)
        self.try_and_check_func(bound_f, "Error was NOT expected while trying to reset "
                "default entry since it is a const default action", False)

        # 2. Try and set/reset the value of default_entry with hit(0x0). Should fail (const)
        bound_f = partial(forward_table.default_entry_set,
                          target,
                          forward_table.make_data(
                              [gc.DataTuple('port', 0)],
                              "SwitchIngress.hit")
                          )
        self.try_and_check_func(bound_f, "Error was expected while trying to set default "
                "entry to another action since it is a const default action", True)
        # reset_default_action is a no-op if default action is const
        bound_f = partial(forward_table.default_entry_reset, target)
        self.try_and_check_func(bound_f, "Error was NOT expected while trying to reset "
                "default entry since it is a const default action", False)

        # 3. Try and set action miss(0x1) for a regular entry. Should fail (defaultOnly)
        key_list = [forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)])]
        data_list = [forward_table.make_data([gc.DataTuple('drop', 0)],
                                             "SwitchIngress.miss")]

        bound_f = partial(forward_table.entry_add, target, key_list, data_list)
        self.try_and_check_func(bound_f, "Error was expected while trying to set a "
                "usual entry with a defaultonly action", True)
        forward_table.entry_del(target)


class ExactMatchTimeoutTest(BfRuntimeTest):
    '''@brief This test tries to add entries such that the RPC request will timeout before the server can service all the entries
    '''

    def setUp(self):
        self.c_id = 0
        BfRuntimeTest.setUp(self, self.c_id)

    def clearTable(self, forward_table, target, num_entries):
        key_list = list()
        read_success = False
        max_tries = 20
        attempt_no = 0
        # we need to spin here to read back the successfully installed entries from the server
        # we can't simply attempt to read it once because there is a probability that that request
        # might fail because, even though the client has got back control after the timed out
        # entry_add request, the server might still be busy processing one of the entries (from the
        # timed out entry_add request) and hence might not know about the RPC being cancelled just
        # yet. Thus the entry_get request might error out as the server would not have closed the
        # batch yet. Hence spin on this hoping that eventually the server will eventually check
        # the status of isCancelled() in the entry_add RPC and close the batch
        while read_success == False and attempt_no < max_tries:
            try:
                resp = forward_table.entry_get(target, None, {"from_hw": True})
                for data, key in resp:
                    key_list.append(key)
                read_success = True
            except gc.BfruntimeRpcException as e:
                logger.error("Unable to read the entries from the table during attempt number %d, Try again...",
                             attempt_no)
                time.sleep(1)
                attempt_no = attempt_no + 1
        if attempt_no == max_tries:
            logger.info("Unable to read any entries from the table even after %d attempts. Hence assert", max_tries)
            assert (0)
        logger.info("Successfully read entries from the table")
        if len(key_list) == 0 or len(key_list) == num_entries:
            # This indicates that not even a single entry got added in the table or all entries were added to
            # the table. We don't want this. We want the request to timeout somewhere mid-way. Hence tune
            # the timeout that is being passed to the write request
            logger.error(
                "Adjust the timeout of entry add RPC so that number of entries successfully added (%d) is not 0 or %d",
                len(key_list), num_entries)
            assert (0)
        logger.info("Deleting %d out of the %d total entries that got added successfully", len(key_list), num_entries)
        forward_table.entry_del(target, key_list)

    def runTest(self):
        if testutils.test_param_get("target") == "hw":
            logger.info(
                "This test is meant to be run only on the model as the timeout for the entry_add RPC is tuned for it. So simply returning")
            return
        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'
        num_entries = 200000
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()

        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        exp_pkt = pkt

        target = gc.Target(device_id=0, pipe_id=0xffff)

        forward_table = bfrt_info.table_get("SwitchIngress.forward_timeout")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        key_list = list()
        data_list = list()

        logger.info("Forming entries")
        for i in range(num_entries):
            # come up with the worst possible distribution of keys
            # A linear sequence of addresse will yeild the worst possible distribution because
            # all the significant bits (~30) will be the same (0s) as we are only exercising 200000
            # out of the possible 2^48 - 1 addresses and the change between the successive addresses
            # is minimal.
            dmac = "%x:%x:%x:%x:%x:%x" % (
                i & 0xff, (i >> 8) & 0xff, (i >> 16) & 0xff, (i >> 24) & 0xff, (i >> 32) & 0xff, (i >> 40) & 0xff)
            key_list.append(forward_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)]))
            data_list.append(forward_table.make_data([gc.DataTuple('port', eg_port)],
                                                     "SwitchIngress.hit"))
        logger.info("Done forming entries")

        client_metadata = (("deadline_sec", "1"), ("deadline_nsec", "499999999"))
        try:
            forward_table.entry_add(target, key_list, data_list,  metadata=client_metadata)
        except gc.BfruntimeRpcException as e:
            if e.grpc_error_get().code() != grpc.StatusCode.DEADLINE_EXCEEDED:
                logger.error("We expected the write request to timeout, but it didn't. Hence assert")
                assert (0)
            else:
                logger.info("Write request timed out as expected")
        finally:
            self.clearTable(forward_table, target, num_entries)
