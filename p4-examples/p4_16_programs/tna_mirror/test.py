# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import grpc
import time
import sys

from ptf import config
from ptf.thriftutils import *
import ptf.testutils as testutils
from ptf.testutils import *
from bfruntime_client_base_tests import BfRuntimeTest, BaseTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import google.rpc.code_pb2 as code_pb2
from functools import partial

import random

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

dev_id = 0
if test_param_get("arch") == "tofino":
    MIR_SESS_COUNT = 1024
    MAX_SID_NORM = 1015
    MAX_SID_COAL = 1023
    BASE_SID_NORM = 1
    BASE_SID_COAL = 1016
    EXP_LEN1 = 127
    EXP_LEN2 = 63
elif test_param_get("arch") == "tofino2":
    MIR_SESS_COUNT = 256
    MAX_SID_NORM = 255
    MAX_SID_COAL = 255
    BASE_SID_NORM = 0
    BASE_SID_COAL = 0
    EXP_LEN1 = 127
    EXP_LEN2 = 59
else:
    assert False, "Unsupported arch %s" % test_param_get("arch")

EGRESS_PORT_INVALID = 511

MCID1 = 1
MCID2 = 2
HASH1 = 1
HASH2 = 2

base_pick_path = testutils.test_param_get("base_pick_path")
binary_name = testutils.test_param_get("arch")
if binary_name  is not "tofino2" and binary_name is not "tofino":
    assert 0, "%s is unknown arch" %(binary_name)

if not base_pick_path:
    base_pick_path = "install/share/" + binary_name + "pd/"

base_put_path = testutils.test_param_get("base_put_path")
if not base_put_path:
    base_put_path = "install/share/"+ binary_name + "pd/forwarding"

logger.info("\nbase_put_path=%s \nbase_pick_path=%s", base_pick_path, base_put_path)

def create_path_bf_rt(base_path, p4_name_to_use):
    return base_path + "/" + p4_name_to_use + "/bf-rt.json"

def create_path_context(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/context.json"

def create_path_tofino(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/" + binary_name +".bin"


def portToBitIdx(port):
    pipe = port_to_pipe(port)
    index = port_to_pipe_local_port(port)
    return 72 * pipe + index


def set_port_map(indicies):
    bit_map = [0] * ((288 + 7) // 8)
    for i in indicies:
        index = portToBitIdx(i)
        bit_map[index // 8] = (bit_map[index // 8] | (1 << (index % 8))) & 0xFF
    return bytes_to_string(bit_map)


def set_lag_map(indicies):
    bit_map = [0] * ((256 + 7) // 8)
    for i in indicies:
        bit_map[i // 8] = (bit_map[i // 8] | (1 << (i % 8))) & 0xFF
    return bytes_to_string(bit_map)


def verify_coal_pkt(self, pkt, port):
    logging.debug("Checking for pkt on port %r", port)
    (_, rcv_port, rcv_pkt, pkt_time) = self.dataplane.poll(port_number=port, timeout=2, exp_pkt=None)
    self.assertTrue(rcv_pkt != None, "Did not receive pkt on %r" % port)
    print()
    hexdump(rcv_pkt)
    sys.stdout.flush()
    # only compare slices
    print()
    hexdump(pkt)
    sys.stdout.flush()


def setup_random(seed_val=0):
    if 0 == seed_val:
        seed_val = int(time.time())
    logger.info("Seed is: %d", seed_val)
    sys.stdout.flush()
    random.seed(seed_val)


def make_port(pipe, local_port):
    assert (pipe >= 0 and pipe < 4)
    assert (local_port >= 0 and local_port < 72)
    return (pipe << 7) | local_port


def port_to_pipe(port):
    local_port = port & 0x7F
    assert (local_port < 72)
    pipe = (port >> 7) & 0x3
    assert (port == ((pipe << 7) | local_port))
    return pipe


def port_to_pipe_local_port(port):
    return port & 0x7F


swports = []
swports_by_pipe = {}
for device, port, ifname in config["interfaces"]:
    pipe = port_to_pipe(port)
    if pipe not in list(range(int(test_param_get('num_pipes')))):
        continue
    if pipe not in swports_by_pipe:
        swports_by_pipe[pipe] = []
    swports.append(port)
    swports.sort()
    swports_by_pipe[pipe].append(port)
    swports_by_pipe[pipe].sort()

if swports == []:
    for pipe in range(int(test_param_get('num_pipes'))):
        for port in range(2):
            swports.append(make_port(pipe, port))
print()
"Using ports:", swports
sys.stdout.flush()


def mirror_session(mir_type, mir_dir, sid, egr_port=0, egr_port_v=False,
                   egr_port_queue=0, packet_color=0, mcast_grp_a=0,
                   mcast_grp_a_v=False, mcast_grp_b=0, mcast_grp_b_v=False,
                   max_pkt_len=0, level1_mcast_hash=0, level2_mcast_hash=0,
                   mcast_l1_xid=0, mcast_l2_xid=0, mcast_rid=0, cos=0, c2c=False, extract_len=0, timeout=0,
                   int_hdr=[], hdr_len=0):
    return MirrorSessionInfo_t(mir_type,
                               mir_dir,
                               sid,
                               egr_port,
                               egr_port_v,
                               egr_port_queue,
                               packet_color,
                               mcast_grp_a,
                               mcast_grp_a_v,
                               mcast_grp_b,
                               mcast_grp_b_v,
                               max_pkt_len,
                               level1_mcast_hash,
                               level2_mcast_hash,
                               mcast_l1_xid,
                               mcast_l2_xid,
                               mcast_rid,
                               cos,
                               c2c,
                               extract_len,
                               timeout,
                               int_hdr,
                               hdr_len)


def program_mirror_fwd_table(self, target, port, egr_port=None, ing_mir=0, ing_sid=0, egr_mir=0, egr_sid=0):
    logger.info("Programming mirror forwarding table for the test...")
    if egr_port is None:
        egr_port = port

    self.mirror_fwd_table.entry_add(
        target,
        [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])],
        [self.mirror_fwd_table.make_data([gc.DataTuple('dest_port', egr_port),
                                          gc.DataTuple('ing_mir', ing_mir),
                                          gc.DataTuple('ing_ses', ing_sid),
                                          gc.DataTuple('egr_mir', egr_mir),
                                          gc.DataTuple('egr_ses', egr_sid)],
                                         'SwitchIngress.set_md')]
    )


class TestNegativeDuplicate(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        Check that adding duplicate keys to the mirror_cfg table will report an error.
        The mirror_cfg key concept is equivalent to mirror id in PD API.
        '''
        p4_name = "tna_mirror"
        logger.info("=============== Testing Duplicated Mirror Session with Same Mirror ID ===============")

        bfrt_info = self.interface.bfrt_info_get(p4_name)
        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=dev_id)
        setup_random()

        duplicate_add_error = False
        duplicate_add_error_p4_table = False
        try:
            ports = random.sample(set(swports), len(swports)//2)
            logger.info("swports: %s", swports)
            logger.info("selected ports: %s ", ports)
            sid = BASE_SID_NORM
            port_0 = ports[0]
            port_0_recv = ports[1]
            # The mirror_cfg table controls what a specific mirror session id does to a packet.
            # This is programming the mirror block in hardware.
            # mirror_cfg_bfrt_key is equivalent to old "mirror_id" in PD term
            mirror_cfg_bfrt_key  = mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])
            mirror_cfg_bfrt_data = mirror_cfg_table.make_data([
                gc.DataTuple('$direction', str_val="INGRESS"),
                gc.DataTuple('$ucast_egress_port', port_0_recv),
                gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                gc.DataTuple('$session_enable', bool_val=True),
            ], "$normal")
            mirror_cfg_table.entry_add(target, [ mirror_cfg_bfrt_key ], [ mirror_cfg_bfrt_data ])
            try:
                mirror_cfg_table.entry_add(target, [ mirror_cfg_bfrt_key ], [ mirror_cfg_bfrt_data ])
            except gc.BfruntimeRpcException as e:
                logger.info("Expected Error: %s", e)
                duplicate_add_error = True

            # The mirror_fwd table is a P4 table in the ingress MAU that controls
            # what metadata is assigned to an ingress packet.
            # It will assign metadata to cause the packet to be mirrored to a specific session-id.
            mirror_fwd_table_bfrt_key  = mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port_0)])
            mirror_fwd_table_bfrt_data = mirror_fwd_table.make_data([
                gc.DataTuple('dest_port', EGRESS_PORT_INVALID),
                gc.DataTuple('ing_mir', 1),
                gc.DataTuple('ing_ses', sid),
                gc.DataTuple('egr_mir', 0),
                gc.DataTuple('egr_ses', 0)
                ],
                'SwitchIngress.set_md'
            )
            mirror_fwd_table.entry_add(target, [mirror_fwd_table_bfrt_key], [mirror_fwd_table_bfrt_data])
            try:
                mirror_fwd_table.entry_add(target, [mirror_fwd_table_bfrt_key], [mirror_fwd_table_bfrt_data])
            except gc.BfruntimeRpcException as e:
                logger.info("Expected Error: %s", e)
                duplicate_add_error_p4_table = True

            # make sure packets are still passing after duplicated entry_add
            logger.info("Using session %d, Sending Packet to port %d", sid, port_0)
            logger.info("Expect Packets are to be received on these ports %s", port_0_recv)
            sys.stdout.flush()

            pkt = simple_eth_packet(pktlen=128)
            send_packet(self, port_0, pkt)
            verify_packet(self, pkt, port_0_recv)
            verify_no_other_packets(self)

        finally:
            # Cleanup all the config
            mirror_cfg_table.entry_del(target, [mirror_cfg_bfrt_key])
            mirror_fwd_table.entry_del(target, [mirror_fwd_table_bfrt_key])

        if duplicate_add_error is not True or duplicate_add_error_p4_table is not True:
            raise RuntimeError("Duplicated Entry Add must raise Error to user")

class TestMCast(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        Check that we can ingress mirror a packet to a multicast group.
        '''
        p4_name = "tna_mirror"
        logger.info("=============== Testing Mirror to Multicast Group ===============")

        bfrt_info = self.interface.bfrt_info_get(p4_name)
        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        pre_node_table = bfrt_info.table_get("$pre.node")
        pre_mgid_table = bfrt_info.table_get("$pre.mgid")
        pre_port_table = bfrt_info.table_get("$pre.port")
        mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=dev_id)
        setup_random()

        try:
            # programming the multicast group
            # Group a will use at most half of the swports
            ports_a = random.sample(set(swports), len(swports)//2)
            logger.info("swports: %s ", swports)
            logger.info("ports_a: %s ", ports_a)

            sid = random.choice(list(range(BASE_SID_NORM, MAX_SID_NORM)))
            rid = 1
            node_id_a = 0xA
            mgid_a = 0xAAA

            # create node A without $MULTICAST_LAG_ID
            node_a_bfrt_key = pre_node_table.make_key([gc.KeyTuple('$MULTICAST_NODE_ID', node_id_a)])
            node_a_bfrt_data = pre_node_table.make_data([
                gc.DataTuple('$MULTICAST_RID', rid),
                gc.DataTuple('$DEV_PORT',int_arr_val=ports_a[1:])
            ])
            # create multicast group A
            mgid_a_bfrt_key  = pre_mgid_table.make_key([gc.KeyTuple('$MGID', mgid_a)])
            mgid_a_bfrt_data = pre_mgid_table.make_data([
                gc.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[node_id_a]),
                gc.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[False]),
                gc.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=ports_a[:1]),
            ])
            pre_node_table.entry_add(target, [node_a_bfrt_key],[node_a_bfrt_data])
            pre_mgid_table.entry_add(target, [mgid_a_bfrt_key], [mgid_a_bfrt_data])

            # The mirror_fwd table is a P4 table in the ingress MAU that controls
            # what metadata is assigned to an ingress packet.
            # It will assign metadata to cause the packet to be mirrored to a specific session-id.
            mirror_fwd_table_bfrt_key  = mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', ports_a[0])])
            mirror_fwd_table_bfrt_data = mirror_fwd_table.make_data([
                gc.DataTuple('dest_port', EGRESS_PORT_INVALID),
                gc.DataTuple('ing_mir', 1),
                gc.DataTuple('ing_ses', sid),
                gc.DataTuple('egr_mir', 0),
                gc.DataTuple('egr_ses', 0)
                ],
                'SwitchIngress.set_md'
            )
            mirror_fwd_table.entry_add(target, [mirror_fwd_table_bfrt_key], [mirror_fwd_table_bfrt_data])

            # programing the mirror session for group A
            # The mirror_cfg table controls what a specific mirror session id does to a packet.
            # This is programming the mirror block in hardware.
            mirror_session_bfrt_key  = mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])
            mirror_session_bfrt_data = mirror_cfg_table.make_data([
                gc.DataTuple('$direction', str_val="INGRESS"),
                gc.DataTuple('$session_enable', bool_val=True),
                gc.DataTuple('$mcast_grp_a', mgid_a),
                gc.DataTuple('$mcast_grp_a_valid', bool_val=True),
                gc.DataTuple('$mcast_rid', rid),
            ], "$normal")
            mirror_cfg_table.entry_add(target, [ mirror_session_bfrt_key ], [ mirror_session_bfrt_data ])

            logger.info("Using session %d, Sending Packet to port %d", sid, ports_a[0])
            logger.info("Expect Packets are to be received on these ports %s", ports_a[1:])
            sys.stdout.flush()

            pkt = simple_eth_packet(pktlen=128)
            send_packet(self, ports_a[0], pkt)
            for port in ports_a[1:]:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)
        finally:
            # Cleanup all the config
            # Delete all mutlicast setup
            pre_node_table.entry_del(target, [node_a_bfrt_key])
            pre_mgid_table.entry_del(target, [mgid_a_bfrt_key])
            mirror_cfg_table.entry_del(target, [mirror_session_bfrt_key])
            mirror_fwd_table.entry_del(target, [mirror_fwd_table_bfrt_key])


class TestTruncMir(BfRuntimeTest):
    """
    Check that the mirrored packet is truncated based on the
    max pkt len config for the mirror session
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        @brief This test does the following:
          - Randomly selects a mirror session id for each port in the test
          - Adds a mirror forwarding table entry with egress port set to an
            invalid port for normal packet forwarding and enables ingress
            mirroring for the port's mirror session id with max length of
            mirrored packet set to 128 or 64 depending on the port number
            is odd or even
          - Verifies the mirror session config programming using entry get
            for each mirror session
          - Sends a packet to each port and verifies that the mirrored copy
            is truncated as expected and also normal packet gets dropped
          - Disables each mirror session and sends a packet to each port
            and verifies that no packet is received on any port
          - Cleans up the test by deleting all mirror forwarding table
            entries and mirror session tables config
        '''
        logger.info("=============== Testing Mirror Truncation ===============")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        setup_random()
        self.sids = []
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent to a dummy port that drops them.
            sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), len(swports))
            for port, sid in zip(swports, sids):
                program_mirror_fwd_table(self,
                                         target,
                                         port,
                                         egr_port=EGRESS_PORT_INVALID,
                                         ing_mir=1,
                                         ing_sid=sid)
                if port % 2 == 0:
                    max_len = 128
                else:
                    max_len = 64

                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True),
                                                 gc.DataTuple('$max_pkt_len', max_len)],
                                                '$normal')]
                )
                self.sids.append(sid)
                logger.info("Using session %d for port %d", sid, port)
                sys.stdout.flush()

                # Verify mirror session config using entry get
                logger.info("Verifying entry get for session %d for port %d", sid, port)
                resp = mirror_cfg_table.entry_get(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    {"from_hw": True},
                    mirror_cfg_table.make_data([gc.DataTuple('$direction'),
                                                gc.DataTuple('$ucast_egress_port'),
                                                gc.DataTuple('$ucast_egress_port_valid'),
                                                gc.DataTuple('$max_pkt_len')],
                                               '$normal')
                )
                data_dict = next(resp)[0].to_dict()

                assert data_dict["$direction"] == "INGRESS"
                logger.info("Verified $direction in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port"] == port
                logger.info("Verified $ucast_egress_port in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port_valid"] == True
                logger.info("Verified $ucast_egress_port_valid in entry get for session %d for port %d", sid, port)
                assert data_dict["$max_pkt_len"] == max_len
                logger.info("Verified $max_pkt_len in entry get for session %d for port %d", sid, port)

            pkt = simple_eth_packet(pktlen=79)
            pkt = simple_eth_packet(pktlen=200)
            rec_pkt1 = simple_eth_packet(pktlen=EXP_LEN1)
            rec_pkt2 = simple_eth_packet(pktlen=EXP_LEN2)
            for port in swports:
                send_packet(self, port, pkt)
                if port % 2 == 0:
                    verify_packet(self, rec_pkt1, port)
                else:
                    verify_packet(self, rec_pkt2, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for sid in self.sids:
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False),
                                                 gc.DataTuple('$max_pkt_len', max_len)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=200)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

        finally:
            # Cleanup all the config
            # Delete the mirror forwarding table entries
            for port in swports:
                self.mirror_fwd_table.entry_del(
                    target,
                    [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])])
            # Delete all mirror sessions
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])


class TestBasicIngMir(BfRuntimeTest):
    """
    Check that we can ingress mirror a packet out the port it came in on.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        @brief This test does the following:
          - Randomly selects a mirror session id for each port in the test
          - Adds a mirror forwarding table entry with egress port set to an
            invalid port for normal packet forwarding and enables ingress
            mirroring for the port's mirror session id
          - Sends a packet to each port and verifies that the mirrored copy
            is received as expected and normal packet gets dropped
          - Disables each mirror session and sends a packet to each port
            and verifies that no packet is received on any port
          - Enables each mirror session and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Disables each mirror session again and sends a packet to each port
            and verifies that no packet is received on any port
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Deletes all mirror sessions
          - Creates each mirror session in disabled state and sends a packet
            to each port and verifies that no packet is received on any port
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Cleans up the test by deleting all mirror forwarding table
            entries and mirror session tables config
        '''
        logger.info("=============== Testing Basic Ingress Mirroring ===============")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        setup_random()
        self.sids = []
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent to a dummy port that drops them.
            sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), len(swports))
            for port, sid in zip(swports, sids):
                program_mirror_fwd_table(self,
                                         target,
                                         port,
                                         egr_port=EGRESS_PORT_INVALID,
                                         ing_mir=1,
                                         ing_sid=sid)

                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )
                self.sids.append(sid)
                logger.info("Using session %d for port %d", sid, port)
                sys.stdout.flush()

            pkt = simple_eth_packet(pktlen=79)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=80)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=81)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=82)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=83)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Delete the sessions, add them again in a disabled state.
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

            pkt = simple_eth_packet(pktlen=89)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)
            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )
                self.sids.append(sid)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions in all pipes and check again
            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=90)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

        finally:
            # Cleanup all the config
            # Delete the mirror forwarding table entries
            for port in swports:
                self.mirror_fwd_table.entry_del(
                    target,
                    [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])])
            # Delete all mirror sessions
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])


class TestBasicEgrMir(BfRuntimeTest):
    """
    Check that we can ingress mirror a packet out the port it came in on.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        @brief This test does the following:
          - Randomly selects a mirror session id for each port in the test
          - Adds a mirror forwarding table entry with egress port set to own
            ingress port for normal packet forwarding and enables egress
            mirroring for the port's mirror session id
          - Sends a packet to each port and verifies that the mirrored copy
            is received as expected and normal packet also gets received
            as expected
          - Disables each mirror session and sends a packet to each port
            and verifies that only one packet is received on any port and
            mirrored copy is not received
          - Enables each mirror session and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            normal packet also gets received
          - Disables each mirror session again and sends a packet to each port
            and verifies that only one packet is received on any port and
            mirrored copy is not received
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            normal packet also gets received
          - Deletes all mirror sessions
          - Creates each mirror session in disabled state and sends a packet
            to each port and verifies that only normal packet is received
            on each port
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            normal packet also gets received
          - Cleans up the test by deleting all mirror forwarding table
            entries and mirror session tables config
        '''
        logger.info("=============== Testing Basic Egress Mirroring ===============")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        setup_random()
        self.sids = []
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent back out the ingress ports.
            sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), len(swports))
            for port, sid in zip(swports, sids):
                program_mirror_fwd_table(self,
                                         target,
                                         port,
                                         egr_port=port,
                                         egr_mir=1,
                                         egr_sid=sid)

                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

                self.sids.append(sid)
                logger.info("Using session %d for port %d", sid, port)
                sys.stdout.flush()

            pkt = simple_eth_packet(pktlen=79)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=80)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=81)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=82)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=83)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Delete the sessions, add them again in a disabled state.
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

            pkt = simple_eth_packet(pktlen=89)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )
                self.sids.append(sid)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Enable the sessions in all pipes and check again
            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=90)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

        finally:
            # Cleanup all the config
            # Delete the mirror forwarding table entries
            for port in swports:
                self.mirror_fwd_table.entry_del(
                    target,
                    [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])])
            # Delete all mirror sessions
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])


class TestMirInfoGet(BfRuntimeTest):
    """
    Check Mirror info
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        @brief This test does the following:
          - Randomly selects a mirror session id for each port in the test
          - Adds a mirror forwarding table entry with egress port set to an
            invalid port for normal packet forwarding and enables ingress
            mirroring for the port's mirror session id
          - Verifies the mirror session programming using entry get
          - Verify mirror session config using wildcard entry get
          - Sends a packet to each port and verifies that the mirrored copy
            is received as expected and normal packet gets dropped
          - Modifies the mirror session programming and verifies the same
            using entry get
          - Reprograms the mirror session config back to original config
          - Disables each mirror session and sends a packet to each port
            and verifies that no packet is received on any port
          - Enables each mirror session and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Disables each mirror session again and sends a packet to each port
            and verifies that no packet is received on any port
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Deletes all mirror sessions
          - Creates each mirror session in disabled state and sends a packet
            to each port and verifies that no packet is received on any port
          - Enables each mirror session again and sends a packet to each port
            and verifies that mirrored copy is received on each port and
            no other copy is received
          - Cleans up the test by deleting all mirror forwarding table
            entries and mirror session tables config
        '''
        logger.info("=============== Testing Entry Get for Mirror table ===============")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        setup_random()
        self.sids = []
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent to a dummy port that drops them.
            sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), len(swports))
            for port, sid in zip(swports, sids):
                program_mirror_fwd_table(self,
                                         target,
                                         port,
                                         egr_port=EGRESS_PORT_INVALID,
                                         ing_mir=1,
                                         ing_sid=sid)

                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

                self.sids.append(sid)
                logger.info("Using session %d for port %d", sid, port)
                sys.stdout.flush()

                # Verify mirror session config using entry get
                logger.info("Verifying entry get for session %d for port %d", sid, port)
                resp = mirror_cfg_table.entry_get(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    {"from_hw": True},
                    mirror_cfg_table.make_data([gc.DataTuple('$direction'),
                                                gc.DataTuple('$ucast_egress_port'),
                                                gc.DataTuple('$ucast_egress_port_valid')],
                                               '$normal')
                )
                data_dict = next(resp)[0].to_dict()
                assert data_dict["$direction"] == "INGRESS"
                logger.info("Verified $direction in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port"] == port
                logger.info("Verified $ucast_egress_port in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port_valid"] == True
                logger.info("Verified $ucast_egress_port_valid in entry get for session %d for port %d", sid, port)

            # Verify mirror session config using wildcard entry get
            logger.info("Verifying entry get using Wildcard entry_get(target)")
            resps = mirror_cfg_table.entry_get(target)
            for (resp_sid, sid) in zip(resps, sorted(self.sids)):
                assert resp_sid[1].to_dict()["$sid"]["value"] == sid
                assert resp_sid[0].to_dict()["$direction"] == "INGRESS"
                assert resp_sid[0].to_dict()["$ucast_egress_port_valid"] == True

            pkt = simple_eth_packet(pktlen=79)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Modify the session parameters and verify using entry get
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=False),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

                logger.info("Verifying entry get after modify for session %d for port %d", sid, port)
                resp = mirror_cfg_table.entry_get(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    {"from_hw": True},
                    mirror_cfg_table.make_data([gc.DataTuple('$direction'),
                                                gc.DataTuple('$ucast_egress_port'),
                                                gc.DataTuple('$ucast_egress_port_valid')],
                                               '$normal')
                )
                data_dict = next(resp)[0].to_dict()
                assert data_dict["$direction"] == "EGRESS"
                logger.info("Verified $direction in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port"] == port
                logger.info("Verified $ucast_egress_port in entry get for session %d for port %d", sid, port)
                assert data_dict["$ucast_egress_port_valid"] == False
                logger.info("Verified $ucast_egress_port_valid in entry get for session %d for port %d", sid, port)

                # Reprogram the session back to original state
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=80)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=81)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Disable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=82)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions and check again
            for port, sid in zip(swports, self.sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=83)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

            # Delete the sessions, add them again in a disabled state.
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

            pkt = simple_eth_packet(pktlen=89)
            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_add(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=False)],
                                                '$normal')]
                )
                self.sids.append(sid)

            for port in swports:
                send_packet(self, port, pkt)
            verify_no_other_packets(self)

            # Enable the sessions in all pipes and check again
            for port, sid in zip(swports, sids):
                mirror_cfg_table.entry_mod(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                    [mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                                 gc.DataTuple('$ucast_egress_port', port),
                                                 gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                 gc.DataTuple('$session_enable', bool_val=True)],
                                                '$normal')]
                )

            pkt = simple_eth_packet(pktlen=90)
            for port in swports:
                send_packet(self, port, pkt)
            for port in swports:
                verify_packet(self, pkt, port)
            verify_no_other_packets(self)

        finally:
            # Cleanup all the config
            # Delete the mirror forwarding table entries
            for port in swports:
                self.mirror_fwd_table.entry_del(
                    target,
                    [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])])
            # Delete all mirror sessions
            while len(self.sids):
                sid = self.sids.pop(0)
                mirror_cfg_table.entry_del(
                    target,
                    [mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

class TestIngEgrMirrorHA(BfRuntimeTest):
    """
    Check that we can ingress/egress mirror a packet out the port it came in on.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def pktVerify(self, port):
        pkt = simple_eth_packet(pktlen=79)
        send_packet(self, port, pkt)
        verify_packet(self, pkt, port)
        verify_no_other_packets(self)

    def configMirrorState(self, port, sid):
        logger.info("fw tbl program")
        if port%2 != 0:
            program_mirror_fwd_table(self,
                                     self.target,
                                     port,
                                     egr_port=EGRESS_PORT_INVALID,
                                     ing_mir=1,
                                     ing_sid=sid)
        else:
            program_mirror_fwd_table(self,
                                     self.target,
                                     port,
                                     egr_port=EGRESS_PORT_INVALID,
                                     egr_mir=1,
                                     egr_sid=sid)

        logger.info("cfg tbl program")
        if port%2 != 0:
            self.mirror_cfg_table.entry_add(
                self.target,
                [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                [self.mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                             gc.DataTuple('$ucast_egress_port', port),
                                             gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                             gc.DataTuple('$session_enable', bool_val=True)],
                                            '$normal')]
            )
        else:
            self.mirror_cfg_table.entry_add(
                self.target,
                [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
                [self.mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="EGRESS"),
                                             gc.DataTuple('$ucast_egress_port', port),
                                             gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                             gc.DataTuple('$session_enable', bool_val=True)],
                                            '$normal')]
            )

    def cleanupState(self, port, sid):
        logger.info("Cleaning up for port %d, sid %d ", port, sid)
        # Cleanup all the config
        self.mirror_fwd_table.entry_del(
            self.target,
            [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])])
        # Delete all mirror sessions
        self.mirror_cfg_table.entry_del(
            self.target,
            [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

    def runTest(self):
        # tofino2 mirroring HA not supported yet
        if test_param_get("arch") == "tofino2":
            return
        logger.info("=== Testing Ingress/Egress Mirroring HA ===")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        self.mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent to a dummy port that drops them.
            sid_start=5
            logger.info("Adding entries")
            sid=sid_start
            for port in sorted(swports):
                logger.info("Testing for session %d and port %d", sid, port)
                self.configMirrorState(port, sid)
                sid=sid+1

            logger.info("Send packet to verify")
            self.pktVerify(port)

            logger.info("Start warm init hitless")
            p4_name_to_put = p4_name_to_pick = p4_name = "tna_mirror"
            profile_name_to_put = "pipe"
            profile_name_to_pick = "pipe"
            config_list = [\
                gc.ForwardingConfig(p4_name_to_put,
                    create_path_bf_rt(base_pick_path, p4_name_to_pick),
                    [gc.ProfileInfo(profile_name_to_put,
                        create_path_context(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                        create_path_tofino(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                        [0,1,2,3])]
                )
            ]

            action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN
            init_mode = bfruntime_pb2.SetForwardingPipelineConfigRequest.HITLESS
            success = self.interface.send_set_forwarding_pipeline_config_request(\
                    action,
                    base_put_path,
                    config_list,
                    dev_init_mode=init_mode,
                    timeout = 30)
            if not success:
                raise RuntimeError("Failed to get response for setfwd")

            logger.info("Replay config")
            sid=sid_start
            for port in sorted(swports):
                self.configMirrorState(port, sid)
                sid=sid+1

            logger.info("warm init hitless end")
            action = bfruntime_pb2.SetForwardingPipelineConfigRequest.WARM_INIT_END
            success = self.interface.send_set_forwarding_pipeline_config_request(\
                    action,
                    base_put_path,
                    timeout = 30)
            if not success:
                raise RuntimeError("Failed to get response for setfwd")

            logger.info("Send pkt again to verify")
            self.pktVerify(port)

        finally:
            sid=sid_start
            for port in sorted(swports):
                self.cleanupState(port, sid)
                sid=sid+1

class TestNegativeMirrorHA(BfRuntimeTest):
    """
    Test negative cases during mirror HA: Invalid config replay, no config replay
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_mirror"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def pktVerify(self, port):
        pkt = simple_eth_packet(pktlen=79)
        send_packet(self, port, pkt)
        verify_packet(self, pkt, port)
        verify_no_other_packets(self)

    def configMirrorState(self, port1, port2, sid):
        logger.info("fw tbl program")
        program_mirror_fwd_table(self,
                                 self.target,
                                 port1,
                                 egr_port=EGRESS_PORT_INVALID,
                                 ing_mir=1,
                                 ing_sid=sid)

        logger.info("cfg tbl program")
        self.mirror_cfg_table.entry_add(
            self.target,
            [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])],
            [self.mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="INGRESS"),
                                         gc.DataTuple('$ucast_egress_port', port2),
                                         gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                         gc.DataTuple('$session_enable', bool_val=True)],
                                        '$normal')]
        )

    def cleanupState(self, port1, port2, sid):
        logger.info("Cleaning up for port %d sid %d ", port1, sid)
        # Cleanup all the config
        self.mirror_fwd_table.entry_del(
            self.target,
            [self.mirror_fwd_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port1)])])
        # Delete all mirror sessions
        self.mirror_cfg_table.entry_del(
            self.target,
            [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])])

    def runTest(self):
        # tofino2 mirroring HA not supported yet
        if test_param_get("arch") == "tofino2":
            return
        logger.info("=== Testing Negative Mirroring HA ===")

        bfrt_info = self.interface.bfrt_info_get("tna_mirror")

        self.mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")
        self.mirror_fwd_table = bfrt_info.table_get("mirror_fwd")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        num_pipes = int(test_param_get('num_pipes'))

        try:
            # Setup a session to mirror the packets out the ingress ports.
            # The original packets will be sent to a dummy port that drops them.
            sid_start=130
            logger.info("Adding entries")
            sid=sid_start
            sid_inc=3
            self.sid_offset=50
            self.port_offset=4
            for port in sorted(swports):
                logger.info("Testing for session %d and port %d", sid, port)
                self.configMirrorState(port, port, sid)
                sid=sid+sid_inc

            logger.info("Send packet to verify")
            self.pktVerify(port)

            logger.info("Start warm init hitless")
            p4_name_to_put = p4_name_to_pick = p4_name = "tna_mirror"
            profile_name_to_put = "pipe"
            profile_name_to_pick = "pipe"
            config_list = [\
                gc.ForwardingConfig(p4_name_to_put,
                    create_path_bf_rt(base_pick_path, p4_name_to_pick),
                    [gc.ProfileInfo(profile_name_to_put,
                        create_path_context(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                        create_path_tofino(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                        [0,1,2,3])]
                )
            ]

            action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN
            init_mode = bfruntime_pb2.SetForwardingPipelineConfigRequest.HITLESS
            success = self.interface.send_set_forwarding_pipeline_config_request(\
                    action,
                    base_put_path,
                    config_list,
                    dev_init_mode=init_mode,
                    timeout = 30)
            if not success:
                raise RuntimeError("Failed to get response for setfwd")

            logger.info("Replay config")
            sid=sid_start
            cfg_invalid_sid=0
            cfg_invalid_port=0
            count=0
            for port in sorted(swports):
                #skip first mirror config
                if count == 0:
                    count=count+1
                    sid=sid+sid_inc
                    continue
                sid_new=sid
                port_new=port
                # Use a new session id
                if count == 1:
                    sid_new=sid+self.sid_offset
                # Change the port
                if count == 2:
                    port_new=port+self.port_offset
                count=count+1
                self.configMirrorState(port, port_new, sid_new)
                sid=sid+sid_inc

            logger.info("warm init hitless end")
            action = bfruntime_pb2.SetForwardingPipelineConfigRequest.WARM_INIT_END
            success = self.interface.send_set_forwarding_pipeline_config_request(\
                    action,
                    base_put_path,
                    timeout = 30)
            if not success:
                raise RuntimeError("Failed to get response for setfwd")

            logger.info("Send pkt again to verify")
            self.pktVerify(port)

        finally:
            sid=sid_start
            count=0
            for port in sorted(swports):
                #skip first config
                if count == 0:
                    count=count+1
                    sid=sid+sid_inc
                    continue
                sid_new=sid
                port_new=port
                if count == 1:
                    sid_new=sid+self.sid_offset
                if count == 2:
                    port_new=port+self.port_offset
                count=count+1
                self.cleanupState(port, port_new, sid_new)
                sid=sid+sid_inc

