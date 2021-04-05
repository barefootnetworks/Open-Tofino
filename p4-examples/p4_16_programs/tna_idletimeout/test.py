# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import time
import datetime
import random

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as client

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

num_pipes = testutils.test_param_get('num_pipes')
if num_pipes:
    num_pipes = int(num_pipes)
else:
    num_pipes = 4


def port_to_pipe(port):
    local_port = port & 0x7F
    assert (local_port < 72)
    pipe = (port >> 7) & 0x3
    assert (port == ((pipe << 7) | local_port))
    return pipe


swports = []
for device, port, ifname in config["interfaces"]:
    if port_to_pipe(port) < num_pipes:
        swports.append(port)
swports.sort()

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


class IdleTimeoutTest(BfRuntimeTest):
    """@brief Simple test of the basic idle timeout configuration parameters and
    their usage.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used %d", seed)

        dmac = client.bytes_to_mac(client.to_bytes(random.randint(0, 2 ** 48 - 1), 6))
        smac = client.bytes_to_mac(client.to_bytes(random.randint(0, 2 ** 48 - 1), 6))
        prefix_len = random.randint(0, 48)
        dip = client.bytes_to_ipv4(client.to_bytes(random.randint(0, 2 ** 32 - 1), 4))
        dip_mask = ((0xffffffff) << (32 - random.randint(0, 32))) & (0xffffffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_table = bfrt_info.table_get("SwitchIngress.dmac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = client.Target(device_id=0, pipe_id=0xffff)

        dmac_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                             predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)
        # Set Idle Table attributes
        ttl_query_length = 1000
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)

        resp = dmac_table.attribute_get(target, "IdleTimeout")
        for d in resp:
            assert d["ttl_query_interval"] == ttl_query_length
            assert d["idle_table_mode"] == bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE
            assert d["enable"] == True

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, eth_src=smac)
        exp_pkt = pkt

        dmac_table.entry_add(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=prefix_len),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, dip_mask),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],  # priority req for ternary
            [dmac_table.make_data([client.DataTuple('port', eg_port),
                                   client.DataTuple('$ENTRY_TTL', 1000)],
                                  'SwitchIngress.hit')]
        )

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        time.sleep(2)

        # Check for timeout notification.
        idle_time = self.interface.idletime_notification_get()
        recv_key = bfrt_info.key_from_idletime_notification(idle_time)
        key_dict = recv_key.to_dict()
        recv_dmac = key_dict["hdr.ethernet.dst_addr"]["value"]
        recv_smac = key_dict["hdr.ethernet.src_addr"]["value"]
        recv_smac_prefix_len = key_dict["hdr.ethernet.src_addr"]["prefix_len"]
        recv_dip = key_dict["hdr.ipv4.dst_addr"]["value"]
        recv_dip_mask = key_dict["hdr.ipv4.dst_addr"]["mask"]

        if (dmac != recv_dmac):
            logger.error("Error! dmac = %s received dmac = %s", str(dmac), str(recv_dmac))
            assert 0

        mask = 0
        for i in range(48):
            if (i < prefix_len):
                mask = (mask << 1) | 1
            else:
                mask = mask << 1
        smac_mask = client.to_bytes(mask, 6)
        smac_hex = client.mac_to_bytes(smac)
        recv_smac_hex = client.mac_to_bytes(recv_smac)
        for i in range(6):
            smac_hex[i] = smac_hex[i] & smac_mask[i]
        if (smac_hex != recv_smac_hex):
            logger.error("Error! smac = %s received smac = %s", str(smac), str(recv_smac))
            assert 0

        dip_mask_hex = client.to_bytes(dip_mask, 4)
        dip_hex = client.ipv4_to_bytes(dip)
        recv_dip_hex = client.ipv4_to_bytes(recv_dip)
        for i in range(4):
            dip_hex[i] = dip_hex[i] & dip_mask_hex[i]
        if (dip_hex != recv_dip_hex):
            logger.error("Error! dip = %s received dip = %s", str(dip), str(recv_dip))
            assert 0

        logger.info("Deleting entry from table")
        dmac_table.entry_del(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=prefix_len),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, dip_mask),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])])  # priority req for ternary

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)

        logger.info("Disable idle timeout on the table")
        dmac_table.attribute_idle_time_set(target,
                                           False,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)
        default_interval = 5000
        resp = dmac_table.attribute_get(target, "IdleTimeout")
        for d in resp:
            assert d["ttl_query_interval"] == default_interval
            assert d["idle_table_mode"] == bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE
            assert d["enable"] == False


class IdleTimeoutGetTTLTest(BfRuntimeTest):
    """@brief Show how to set the TTL of an entry and verify it is descreasing
    as expected.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '12:34:45:67:89:ab'
        dmac = '11:22:33:44:55:77'
        dip = '10.11.12.1'
        ttl_set = 10000

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_table = bfrt_info.table_get("SwitchIngress.dmac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = client.Target(device_id=0, pipe_id=0xffff)
        # Set Idle Table attributes
        ttl_query_length = 1000
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)

        dmac_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                             predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, eth_src=smac)
        exp_pkt = pkt

        dmac_table.entry_add(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],  # priority req for ternary
            [dmac_table.make_data([client.DataTuple('port', eg_port),
                                   client.DataTuple('$ENTRY_TTL', ttl_set)],
                                  'SwitchIngress.hit')]
        )

        # sleep for sometime so we know that the ttl of the entry is definitely
        # less than 10000 when we query it later
        time.sleep(7)

        # check ttl get
        resp = dmac_table.entry_get(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),
                                  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            {"from_hw": True})

        data_dict = next(resp)[0].to_dict()
        recv_port = data_dict["port"]
        recv_ttl = data_dict["$ENTRY_TTL"]
        logger.info("Received TTL is %s", str(recv_ttl))
        if (recv_port != eg_port):
            logger.error("Error! port sent = %s received port = %s", str(eg_port), str(recv_port))
            assert 0
        if (recv_ttl >= ttl_set):
            logger.error("Error! ttl set = %s received ttl = %s",
                         str(ttl_set),
                         str(recv_ttl))
            assert 0

        logger.info("Deleting entry from table")
        dmac_table.entry_del(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),
                                  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])])  # priority req for ternary

        logger.info("Disable idle timeout on the table")
        dmac_table.attribute_idle_time_set(target, False,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)


class IdlePollTest(BfRuntimeTest):
    """@brief Demonstrate how to use the hit state attribute and verify it is
    working as expected.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def verifyHitStateValue(self, resp, hit_state):
        data_dict = next(resp)[0].to_dict()
        assert data_dict["$ENTRY_HIT_STATE"] == hit_state

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '12:34:45:67:89:ab'
        dmac = '11:22:33:44:55:77'
        dip = '10.11.12.1'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_table = bfrt_info.table_get("SwitchIngress.dmac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = client.Target(device_id=0, pipe_id=0xffff)
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)
        resp = dmac_table.attribute_get(target, "IdleTimeout")
        # Server sets attributes only on notify mode.
        for d in resp:
            assert d["ttl_query_interval"] == 0
            assert d["idle_table_mode"] == bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE
            assert d["enable"] == True

        dmac_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                             predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, eth_src=smac)
        exp_pkt = pkt

        dmac_table.entry_add(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],  # priority req for ternary
            [dmac_table.make_data([client.DataTuple('port', eg_port)],
                                  'SwitchIngress.hit')]
        )

        # Get hit state update from sw. It should be ENTRY_IDLE
        resp = dmac_table.entry_get(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip,
                                                  '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            # priority req for ternary
            {"from_hw": False})
        self.verifyHitStateValue(resp, "ENTRY_IDLE")

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])

        # Apply table operations to sync the hit status bits
        dmac_table.operations_execute(target, 'UpdateHitState')

        # Get hit state update from sw. It should be ENTRY_ACTIVE
        resp = dmac_table.entry_get(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip,
                                                  '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            # priority req for ternary
            {"from_hw": False})
        self.verifyHitStateValue(resp, "ENTRY_ACTIVE")

        # Apply table operations to sync the hit status bits
        dmac_table.operations_execute(target, 'UpdateHitState')

        # Get hit state update from sw again. It should be ENTRY_IDLE since we have read it already
        # and it's clear on read
        resp = dmac_table.entry_get(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip,
                                                  '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            # priority req for ternary
            {"from_hw": False})
        self.verifyHitStateValue(resp, "ENTRY_IDLE")

        # Set hit state to ACTIVE and verify it works for both SW and HW read.
        dmac_table.entry_mod(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            [dmac_table.make_data([client.DataTuple('$ENTRY_HIT_STATE', str_val="ENTRY_ACTIVE")])])

        for hw_read in (False, True):
            resp = dmac_table.entry_get(
                target,
                [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),
                                      client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                      client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),
                                      client.KeyTuple('$MATCH_PRIORITY', 1)])],
                # priority req for ternary
                {"from_hw": hw_read})
            self.verifyHitStateValue(resp, "ENTRY_ACTIVE")

        # Apply table operations to sync the hit status bits
        dmac_table.operations_execute(target, 'UpdateHitState')

        # Get hit state update from sw again. It should be ENTRY_IDLE since we have read it already
        # and it's clear on read
        resp = dmac_table.entry_get(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),
                                  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip,
                                                  '255.255.0.0'),  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])],
            # priority req for ternary
            {"from_hw": False})
        self.verifyHitStateValue(resp, "ENTRY_IDLE")

        logger.info("Deleting entry from the table")
        dmac_table.entry_del(
            target,
            [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                  client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=7),  # LPM
                                  client.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0'),
                                  # ternary
                                  client.KeyTuple('$MATCH_PRIORITY', 1)])])  # priority req for ternary

        logger.info("Disable idle timeout on the table")
        # Server sets attributes only on notify mode.
        dmac_table.attribute_idle_time_set(target, False,
                bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)
        resp = dmac_table.attribute_get(target, "IdleTimeout")
        for d in resp:
            # Server sets values only for notify mode.
            assert d["ttl_query_interval"] == 0
            assert d["idle_table_mode"] == bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE
            assert d["enable"] == False


class IdlePollIndirectTest(BfRuntimeTest):
    """@brief This test does the following
    1. Puts the idle table associated with the match indirect table in poll mode.
    2. Adds match entry.
    3. Send a packet to make the entry ACTIVE
    4. Read HIT STATE to verify that its ACTIVE
    5. Read HIT STATE again and verify that its IDLE
    6. Repeat the same set of steps with table in asymmetric mode with entries in each pipe
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def verifyHitStateValue(self, resp, hit_state):
        data_dict = next(resp)[0].to_dict()
        assert data_dict["$ENTRY_HIT_STATE"] == hit_state

    def runTest(self):
        ig_eg_port_list = {}
        for p in range(num_pipes):
            ig_eg_port_list[p] = list()
        if len(swports_0):
            ig_eg_port_list[0] = random.sample(swports_0, min(2, len(swports_0)))
        if len(swports_1):
            ig_eg_port_list[1] = random.sample(swports_1, min(2, len(swports_1)))
        if len(swports_2):
            ig_eg_port_list[2] = random.sample(swports_2, min(2, len(swports_2)))
        if len(swports_3):
            ig_eg_port_list[3] = random.sample(swports_3, min(2, len(swports_3)))

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_indirect_table = bfrt_info.table_get("SwitchIngress.dmac_indirect")
        dmac_indirect_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        action_profile_table = bfrt_info.table_get("SwitchIngress.action_profile")

        # First, test symmetric mode
        logger.info("Testing Poll Mode for Match indirect table in symmetric mode")
        target = client.Target(device_id=0, pipe_id=0xffff)

        dmac_indirect_table.attribute_idle_time_set(target, True,
                                                    bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)
        resp = dmac_indirect_table.attribute_get(target, "IdleTimeout")
        for d in resp:
            assert d["idle_table_mode"] == bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE
            assert d["enable"] == True

        dmac_indirect_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                      predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)

        logger.info("Adding match entry to indirect table in symmetric mode")
        sips = ["%d.%d.%d.%d" % tuple([random.randint(1, 255) for x in range(4)]) for x in range(num_pipes)]
        action_mbr_ids = random.sample(list(range(1, 30000)), num_pipes)

        ig_port = swports[0]
        eg_port = swports[1]

        action_profile_table.entry_add(
            target,
            [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[0])])],
            [action_profile_table.make_data([client.DataTuple('port', eg_port)],
                                            'SwitchIngress.hit')]
        )

        dmac_indirect_table.entry_add(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sips[0])])],
            [dmac_indirect_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', action_mbr_ids[0])])])

        pkt = testutils.simple_tcp_packet(ip_src=sips[0])
        exp_pkt = pkt

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packet(self, exp_pkt, eg_port)

        logger.info("Reading hit state of the entry and expecting it to be HIT")
        # Apply table operations to sync the hit status bits
        dmac_indirect_table.operations_execute(target, 'UpdateHitState')

        resp = dmac_indirect_table.entry_get(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr',
                                                           sips[0])])],
            {"from_hw": False})
        self.verifyHitStateValue(resp, "ENTRY_ACTIVE")

        # Now read the HIT STATE again, and we should expect IDLE
        # Apply table operations to sync the hit status bits
        dmac_indirect_table.operations_execute(target, 'UpdateHitState')

        resp = dmac_indirect_table.entry_get(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr',
                                                           sips[0])])],
            {"from_hw": True})
        self.verifyHitStateValue(resp, "ENTRY_IDLE")

        logger.info("Deleting entry from the Match table and action profile table")

        dmac_indirect_table.entry_del(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sips[0])])])

        action_profile_table.entry_del(
            target,
            [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[0])])])

        logger.info("Disable idle timeout on the table")
        dmac_indirect_table.attribute_idle_time_set(target, False,
                            bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)

        logger.info("Testing Poll mode for Match indirect table in asymmetric mode")

        dmac_indirect_table.attribute_idle_time_set(target, True,
                                                    bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)

        dmac_indirect_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                      predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE)

        logger.info("Installing match entries in all the pipes")
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            target = client.Target(device_id=0, pipe_id=i)
            eg_port = ig_eg_port_list[i][-1]
            sip = sips[i]

            action_profile_table.entry_add(
                target,
                [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[i])])],
                [action_profile_table.make_data([client.DataTuple('port', eg_port)],
                                                'SwitchIngress.hit')]
            )

            dmac_indirect_table.entry_add(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])],
                [dmac_indirect_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', action_mbr_ids[i])])])

        logger.info("Sending packets for each pipe")
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            ig_port = ig_eg_port_list[i][0]
            eg_port = ig_eg_port_list[i][-1]

            sip = sips[i]
            pkt = testutils.simple_tcp_packet(ip_src=sip)
            exp_pkt = pkt

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)

        # Apply table operations to sync the hit status bits
        target = client.Target(device_id=0, pipe_id=0xffff)
        dmac_indirect_table.operations_execute(target, 'UpdateHitState')

        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            target = client.Target(device_id=0, pipe_id=i)
            resp = dmac_indirect_table.entry_get(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr',
                                                               sips[i])])],
                {"from_hw": False})
            self.verifyHitStateValue(resp, "ENTRY_ACTIVE")

        dmac_indirect_table.operations_execute(target, 'UpdateHitState')

        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            target = client.Target(device_id=0, pipe_id=i)
            resp = dmac_indirect_table.entry_get(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr',
                                                               sips[i])])],
                {"from_hw": False})
            self.verifyHitStateValue(resp, "ENTRY_IDLE")

        logger.info("Deleting entries from table")
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            sip = sips[i]
            target = client.Target(device_id=0, pipe_id=i)

            dmac_indirect_table.entry_del(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])])

            action_profile_table.entry_del(
                target,
                [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[i])])])

        logger.info("Disable idle timeout on the table")
        dmac_indirect_table.attribute_idle_time_set(target, False,
                        bfruntime_pb2.IdleTable.IDLE_TABLE_POLL_MODE)


class IdleTableIndirectTableTest(BfRuntimeTest):
    """@brief Show how to set the TTL of an entry in an indirect table and verify
    it is descreasing as expected as well as removed after the timeout interval.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '12:34:45:67:89:ab'
        dmac = '11:22:33:44:55:77'
        dip = '10.11.12.1'
        sip = '1.2.3.4'
        ttl_set = 10000

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_indirect_table = bfrt_info.table_get("SwitchIngress.dmac_indirect")
        dmac_indirect_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        action_profile_table = bfrt_info.table_get("SwitchIngress.action_profile")

        target = client.Target(device_id=0, pipe_id=0xffff)
        # Set Idle Table attributes
        ttl_query_length = 1000

        dmac_indirect_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                      predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)

        dmac_indirect_table.attribute_idle_time_set(target,
                                                    True,
                                                    bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                                    ttl_query_length)

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, eth_src=smac)
        exp_pkt = pkt

        action_profile_table.entry_add(
            target,
            [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', 1)])],
            [action_profile_table.make_data([client.DataTuple('port', eg_port)],
                                            'SwitchIngress.hit')]
        )

        dmac_indirect_table.entry_add(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])],
            [dmac_indirect_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', 1),
                                            client.DataTuple('$ENTRY_TTL', ttl_set)])])

        # sleep for sometime so we know that the ttl of the entry is definitely less than 10000 when we query it later
        time.sleep(7)

        # check ttl get
        resp = dmac_indirect_table.entry_get(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr',
                                                           sip)])],
            {"from_hw": True},
            dmac_indirect_table.make_data([client.DataTuple("$ENTRY_TTL")]))

        data_dict = next(resp)[0].to_dict()
        print(data_dict)
        recv_ttl = data_dict["$ENTRY_TTL"]
        logger.info("Received TTL is %s", str(recv_ttl))
        if (recv_ttl >= ttl_set):
            logger.error("Error! ttl set = %s received ttl = %s",
                         str(ttl_set),
                         str(recv_ttl))
            assert 0

        # Now wait enough time for idle_timeout to expire
        logger.info("Waiting for idle timeout")
        time.sleep(7)

        idle_time = self.interface.idletime_notification_get()
        key_dict = bfrt_info.key_from_idletime_notification(idle_time).to_dict()
        recv_sip = key_dict["hdr.ipv4.src_addr"]["value"]
        assert sip == recv_sip, "sip = %s recv sip = %s" % (sip, recv_sip)

        logger.info("Deleting entry from table")
        dmac_indirect_table.entry_del(
            target,
            [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])])
        action_profile_table.entry_del(
            target,
            [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', 1)])])

        logger.info("Disable idle timeout on the table")
        dmac_indirect_table.attribute_idle_time_set(target, False,
                                                    bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)


class IdleTimeoutAsymmetricTest(BfRuntimeTest):
    """@brief This test adds entries to a match table with ENTRY_SCOPE as
    single-pipeline and verifies that the aging notification is received for
    entries from all the pipes.
    Here are the steps that this test follows:

    1. Set entry scope of the match table to single-pipeline
    2. Add match entry to each of the pipe-line
    3. Send packets to each of the match entry, so that the entry goes active
    4. Wait for idle time notification from each of the pipe
    5. Delete the match entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        ig_eg_port_list = {}
        for p in range(num_pipes):
            ig_eg_port_list[p] = list()
        if len(swports_0):
            ig_eg_port_list[0] = random.sample(swports_0, min(2, len(swports_0)))
        if len(swports_1):
            ig_eg_port_list[1] = random.sample(swports_1, min(2, len(swports_1)))
        if len(swports_2):
            ig_eg_port_list[2] = random.sample(swports_2, min(2, len(swports_2)))
        if len(swports_3):
            ig_eg_port_list[3] = random.sample(swports_3, min(2, len(swports_3)))

        dmacs = ['%s:%s:%s:%s:%s:%s' % tuple([hex(random.randint(1, 255))[2:].zfill(2) for x in range(6)]) for i in
                 range(num_pipes)]
        smacs = ['%s:%s:%s:%s:%s:%s' % tuple([hex(random.randint(1, 255))[2:].zfill(2) for x in range(6)]) for i in
                 range(num_pipes)]
        prefix_len = random.randint(1, 48)
        dips = ["%d.%d.%d.%d" % tuple([random.randint(1, 255) for x in range(4)]) for x in range(num_pipes)]
        dip_masks = [(((0xffffffff) << (32 - random.randint(0, 32))) & (0xffffffff)) for x in range(num_pipes)]

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_table = bfrt_info.table_get("SwitchIngress.dmac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")
        dmac_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = client.Target(device_id=0, pipe_id=0xffff)
        # Set Idle Table attributes
        ttl_query_length = 1000
        dmac_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                             predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE)

        dmac_table.attribute_idle_time_set(target, True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)

        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            target = client.Target(device_id=0, pipe_id=i)
            eg_port = ig_eg_port_list[i][-1]
            smac = smacs[i]
            dmac = dmacs[i]
            dip = dips[i]
            dip_mask = dip_masks[i]
            dmac_table.entry_add(
                target,
                [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                      client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=prefix_len),  # LPM
                                      client.KeyTuple('hdr.ipv4.dst_addr', dip, dip_mask),  # ternary
                                      client.KeyTuple('$MATCH_PRIORITY', 1)])],  # priority req for ternary
                [dmac_table.make_data([client.DataTuple('port', eg_port),
                                       client.DataTuple('$ENTRY_TTL', 1000)],
                                      'SwitchIngress.hit')]
            )

        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            ig_port = ig_eg_port_list[i][0]
            eg_port = ig_eg_port_list[i][-1]

            smac = smacs[i]
            dmac = dmacs[i]
            dip = dips[i]
            pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, eth_src=smac)
            exp_pkt = pkt

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)

        testutils.verify_no_other_packets(self, timeout=2)

        logger.info("Waiting for entries to age out")
        time.sleep(5)

        # find the field_ids of all the key_fields
        pipes_seen = set()
        # Check for timeout notification.
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            idle_time = self.interface.idletime_notification_get()
            recv_dmac = recv_smac = recv_dip = ""
            assert idle_time.target.device_id == 0
            pipe_id = idle_time.target.pipe_id
            assert pipe_id >= 0 and pipe_id < num_pipes

            smac = smacs[pipe_id]
            dmac = dmacs[pipe_id]
            dip = dips[pipe_id]
            dip_mask = dip_masks[pipe_id]

            logger.info("Received idle timeout notification from pipe %d" % pipe_id)
            assert pipe_id not in pipes_seen

            pipes_seen.add(pipe_id)
            recv_key = bfrt_info.key_from_idletime_notification(idle_time).to_dict()
            recv_dmac = recv_key["hdr.ethernet.dst_addr"]["value"]
            recv_smac = recv_key["hdr.ethernet.src_addr"]["value"]
            recv_dip = recv_key["hdr.ipv4.dst_addr"]["value"]

            if (dmac != recv_dmac):
                logger.error("Error! dmac = %s received dmac = %s", str(dmac), str(recv_dmac))
                assert 0

            mask = 0
            for i in range(48):
                if (i < prefix_len):
                    mask = (mask << 1) | 1
                else:
                    mask = mask << 1
            smac_mask = client.to_bytes(mask, 6)
            smac_hex = client.mac_to_bytes(smac)
            recv_smac_hex = client.mac_to_bytes(recv_smac)
            for i in range(6):
                smac_hex[i] = smac_hex[i] & smac_mask[i]
            if (smac_hex != recv_smac_hex):
                logger.error("Error! smac = %s received smac = %s", str(smac), str(recv_smac))
                assert 0

            dip_mask_hex = client.to_bytes(dip_mask, 4)
            dip_hex = client.ipv4_to_bytes(dip)
            recv_dip_hex = client.ipv4_to_bytes(recv_dip)
            for i in range(4):
                dip_hex[i] = dip_hex[i] & dip_mask_hex[i]
            if (dip_hex != recv_dip_hex):
                logger.error("Error! dip = %s received dip = %s", str(dip), str(recv_dip))
                assert 0

        logger.info("Deleting entries from table")
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            smac = smacs[i]
            dmac = dmacs[i]
            dip = dips[i]
            dip_mask = dip_masks[i]

            target = client.Target(device_id=0, pipe_id=i)

            dmac_table.entry_del(
                target,
                [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac),  # exact
                                      client.KeyTuple('hdr.ethernet.src_addr', smac, prefix_len=prefix_len),  # LPM
                                      client.KeyTuple('hdr.ipv4.dst_addr', dip, dip_mask),  # ternary
                                      client.KeyTuple('$MATCH_PRIORITY', 1)])])  # priority req for ternary

            ig_port = ig_eg_port_list[i][0]

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)

        logger.info("Disable idle timeout on the table")
        dmac_table.attribute_idle_time_set(target, False,
                bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)


class IdleTimeoutIndirectAsymmetricTest(BfRuntimeTest):
    """@brief This test adds entries to a match table with ENTRY_SCOPE as
    single-pipeline and verifies that the aging notification is received for
    entries from all the pipes.
    Here are the steps that this test follows:

    1. Set entry scope of the match table to single-pipeline
    2. Add match entry to each of the pipe-line
    3. Send packets to each of the match entry, so that the entry goes active
    4. Wait for idle time notification from each of the pipe
    5. Delete the match entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        ig_eg_port_list = {}
        for p in range(num_pipes):
            ig_eg_port_list[p] = list()
        if len(swports_0):
            ig_eg_port_list[0] = random.sample(swports_0, min(2, len(swports_0)))
        if len(swports_1):
            ig_eg_port_list[1] = random.sample(swports_1, min(2, len(swports_1)))
        if len(swports_2):
            ig_eg_port_list[2] = random.sample(swports_2, min(2, len(swports_2)))
        if len(swports_3):
            ig_eg_port_list[3] = random.sample(swports_3, min(2, len(swports_3)))

        sips = ["%d.%d.%d.%d" % tuple([random.randint(1, 255) for x in range(4)]) for x in range(num_pipes)]
        action_mbr_ids = random.sample(list(range(1, 30000)), num_pipes)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_indirect_table = bfrt_info.table_get("SwitchIngress.dmac_indirect")
        dmac_indirect_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        action_profile_table = bfrt_info.table_get("SwitchIngress.action_profile")

        target = client.Target(device_id=0, pipe_id=0xffff)
        # Set Idle Table attributes
        ttl_query_length = 1000
        dmac_indirect_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                      predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE)

        dmac_indirect_table.attribute_idle_time_set(target, True,
                                                    bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                                    ttl_query_length)

        for i in range(num_pipes):
            target = client.Target(device_id=0, pipe_id=i)
            eg_port = ig_eg_port_list[i][-1]
            sip = sips[i]

            action_profile_table.entry_add(
                target,
                [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[i])])],
                [action_profile_table.make_data([client.DataTuple('port', eg_port)],
                                                'SwitchIngress.hit')]
            )

            dmac_indirect_table.entry_add(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])],
                [dmac_indirect_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', action_mbr_ids[i]),
                                                client.DataTuple('$ENTRY_TTL', 1000)])])

        for i in range(num_pipes):
            ig_port = ig_eg_port_list[i][0]
            eg_port = ig_eg_port_list[i][-1]

            sip = sips[i]
            pkt = testutils.simple_tcp_packet(ip_src=sip)
            exp_pkt = pkt

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)

        testutils.verify_no_other_packets(self, timeout=2)

        logger.info("Waiting for entries to age out")
        time.sleep(5)

        # find the field_ids of all the key_fields
        pipes_seen = set()
        # Check for timeout notification.
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            idle_time = self.interface.idletime_notification_get()
            recv_sip = ""
            assert idle_time.target.device_id == 0
            pipe_id = idle_time.target.pipe_id
            assert pipe_id >= 0 and pipe_id < num_pipes

            sip = sips[pipe_id]

            logger.info("Received idle timeout notification from pipe %d" % pipe_id)
            assert pipe_id not in pipes_seen

            pipes_seen.add(pipe_id)
            key_dict = bfrt_info.key_from_idletime_notification(idle_time).to_dict()
            recv_sip = key_dict["hdr.ipv4.src_addr"]["value"]

            if (sip != recv_sip):
                logger.error("Error! sip = %s received sip = %s for pipe id %d", str(sip), str(recv_sip), i)
                assert 0

        logger.info("Deleting entries from table")
        for i in range(num_pipes):
            if len(ig_eg_port_list[i]) == 0:
                continue
            sip = sips[i]
            target = client.Target(device_id=0, pipe_id=i)

            dmac_indirect_table.entry_del(
                target,
                [dmac_indirect_table.make_key([client.KeyTuple('hdr.ipv4.src_addr', sip)])])

            action_profile_table.entry_del(
                target,
                [action_profile_table.make_key([client.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[i])])])

            ig_port = ig_eg_port_list[i][0]

            pkt = testutils.simple_tcp_packet(ip_src=sip)
            exp_pkt = pkt

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)

        logger.info("Disable idle timeout on the table")
        dmac_indirect_table.attribute_idle_time_set(target, False,
                bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)


class IdleTimeoutEnableDisable(BfRuntimeTest):
    """@brief Test table with added entries before table is enabled.
    1. Set table to notify mode.
    2. Add entries with different TTLs and verify aging is disabled.
    3. Enable table and verify TTL is decreasing as it should.
    4. Change ttl query length to much higher value and verify TTL doesn't
       get updated after similar wait time as before (change was applied).
    5. Disable table after one entry ages out.
    6. Enable table and verify that aged entry stayed aged and
       active entries continue with last seen TTL value.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_idletimeout"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        eg_port = swports[2]
        dmac = '11:22:33:44:55:'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_idletimeout")
        dmac_table = bfrt_info.table_get("SwitchIngress.dmac")
        dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        target = client.Target(device_id=0, pipe_id=0xffff)
        # Set Idle Table attributes
        ttl_query_length = 1000
        logger.info("Set idle timeout table mode to notify")
        dmac_table.attribute_idle_time_set(target,
                                           False,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)

        dmac_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                             predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)

        # x used as both last DMAC byte and ttl value
        ttl_list = [10000, 20000, 30000]
        logger.info("Add {} entries".format(len(ttl_list)))
        for x in (ttl_list):
            dmac_table.entry_add(
                target,
                [dmac_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr',
                                                      dmac + str(x/1000))])],
                [dmac_table.make_data([client.DataTuple('port', eg_port),
                                       client.DataTuple('$ENTRY_TTL', x)],
                                      'SwitchIngress.hit')]
            )

        # Sleep for 5000ms (5 times query length), then get the entries.
        # The entries current TTL should not have decreased because idletimeouts
        # are not enabled on the table.
        time.sleep(5 * ttl_query_length / 1000)

        # check ttl get on all entries
        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        for data, _ in resp:
            dd = data.to_dict()
            recv_port = dd["port"]
            recv_ttl = dd["$ENTRY_TTL"]
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_port != eg_port):
                logger.error("Error! port sent = %s received port = %s", str(eg_port), str(recv_port))
                assert 0
            if recv_ttl not in ttl_list:
                logger.error("Error! ttl set = %s received ttl = %s",
                             str(ttl_list)[1:-1],
                             str(recv_ttl))
                assert 0

        logger.info("Enable with ttl_query_length to {}".format(ttl_query_length))
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)
        logger.info(datetime.datetime.now())

        # Enable idle timeout and sleep for another 5000ms, then get the entries.
        # We expect the entries TTL to have decreased from their initial values
        # since timeouts are enabled now and the entries are aging.
        time.sleep(5 * ttl_query_length / 1000)

        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        exp_ttl = 5000
        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl > exp_ttl + ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl + ttl_query_length),
                             str(recv_ttl))
                assert 0
            if (recv_ttl < exp_ttl - ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl - ttl_query_length),
                             str(recv_ttl))
                assert 0
            exp_ttl += 10000
        logger.info(datetime.datetime.now())

        ttl_query_length = 10000
        logger.info("Change ttl_query_length to {}".format(ttl_query_length))
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)
        logger.info(datetime.datetime.now())

        # In this test ttl_query_length is much longer than before.
        # Get entries two times with sleep in between equal to half of
        # ttl_query_length. Expectation is that if new configuration was properly
        # applied, then ttl will not change.
        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        recv_ttl_list = []
        exp_ttl = 5000
        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            recv_ttl_list.append(recv_ttl)
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl > exp_ttl + ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl + ttl_query_length),
                             str(recv_ttl))
                assert 0
            if (recv_ttl < exp_ttl - ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl - ttl_query_length),
                             str(recv_ttl))
                assert 0
            exp_ttl += 10000
        logger.info(datetime.datetime.now())

        time.sleep((ttl_query_length / 2) / 1000)

        resp = dmac_table.entry_get(target, None, {"from_hw": True})
        exp_ttl = 5000
        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl > exp_ttl + ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl + ttl_query_length),
                             str(recv_ttl))
                assert 0
            if (recv_ttl < exp_ttl - ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl - ttl_query_length),
                             str(recv_ttl))
                assert 0
            exp_ttl += 10000
        logger.info(datetime.datetime.now())

        ttl_query_length = 1000
        logger.info("Change ttl_query_length to {}".format(ttl_query_length))
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)
        logger.info(datetime.datetime.now())

        # Changing query length back to 1000 will reset the sweep period.
        # Sleep for another 7 intervals to make sure entry ages out.
        time.sleep(7 * ttl_query_length / 1000)
        # Check if first entry aged out as expected.
        exp_dmac = dmac+"10"
        idle_time = self.interface.idletime_notification_get()
        recv_key = bfrt_info.key_from_idletime_notification(idle_time)
        key_dict = recv_key.to_dict()
        recv_dmac = key_dict["hdr.ethernet.dst_addr"]["value"]
        if (exp_dmac != recv_dmac):
            logger.error("Error! dmac = %s received dmac = %s", str(exp_dmac), str(recv_dmac))
            assert 0

        # Disable idle timeout and verify ttl values on all entries.
        logger.info("Disable idle table")
        dmac_table.attribute_idle_time_set(target,
                                           False,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)
        logger.info(datetime.datetime.now())

        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        num_aged_entries = 0
        recv_ttl_list = []
        # There should be 12 intervals total that passed
        exp_ttl = 20000 - 12 * ttl_query_length
        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            recv_ttl_list.append(recv_ttl)
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl == 0):
                num_aged_entries += 1
            elif (recv_ttl > exp_ttl + ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl + ttl_query_length),
                             str(recv_ttl))
                assert 0
            elif (recv_ttl < exp_ttl - ttl_query_length):
                logger.error("Error! ttl exp = %s received ttl = %s",
                             str(exp_ttl - ttl_query_length),
                             str(recv_ttl))
                assert 0
            if recv_ttl != 0 :
                exp_ttl += 10000

        assert num_aged_entries == 1
        logger.info(datetime.datetime.now())

        logger.info("Enable with ttl_query_length to {}".format(ttl_query_length))
        dmac_table.attribute_idle_time_set(target,
                                           True,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE,
                                           ttl_query_length)

        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        # Verify that after re-enabling table ttl values remain as before
        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl not in recv_ttl_list):
                logger.error("Error! received ttl = %s not as expected %s",
                             str(recv_ttl),
                             str(recv_ttl_list)[1:-1])
                assert 0
        logger.info(datetime.datetime.now())

        # Wait for highest entry TTL + query length
        time.sleep((max(recv_ttl_list)+ttl_query_length)/1000)

        # Check if remaining entries aged out as expected.
        for x in (["20","30"]):
            exp_dmac = dmac+x
            idle_time = self.interface.idletime_notification_get()
            recv_key = bfrt_info.key_from_idletime_notification(idle_time)
            key_dict = recv_key.to_dict()
            recv_dmac = key_dict["hdr.ethernet.dst_addr"]["value"]
            if (exp_dmac != recv_dmac):
                logger.error("Error! dmac = %s received dmac = %s", str(exp_dmac), str(recv_dmac))
                assert 0

        # All entries should have TTL = 0
        resp = dmac_table.entry_get(target, None, {"from_hw": True})

        for data, _ in resp:
            dd = data.to_dict()
            recv_ttl = dd["$ENTRY_TTL"]
            logger.info("Received TTL is %s", str(recv_ttl))
            if (recv_ttl != 0):
                logger.error("Error! received ttl = %s != 0",
                             str(recv_ttl))
                assert 0
        logger.info(datetime.datetime.now())

        logger.info("Deleting all entries from the table")
        dmac_table.entry_del(target, [])
        logger.info("Disable idle timeout on the table")
        dmac_table.attribute_idle_time_set(target, False,
                                           bfruntime_pb2.IdleTable.IDLE_TABLE_NOTIFY_MODE)
