# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random
from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class ActionProfileTest(BfRuntimeTest):
    """@brief Simple test to add a single entry in the ActionProfile and verify
    that packets are processed as expected.
    """

    def setUp(self):
        ''' Setup the test. '''
        client_id = 0
        p4_name = "tna_action_profile"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '22:22:22:22:22:22'

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_action_profile")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_profile = bfrt_info.table_get("SwitchIngress.action_profile")

        ap_key = action_profile.make_key([client.KeyTuple('$ACTION_MEMBER_ID', 1)])
        action_profile.entry_add(
            target,
            [ap_key],
            [action_profile.make_data(
                [client.DataTuple('port', eg_port)], 'SwitchIngress.set_port')])

        fwd_key = forward_table.make_key([
            client.KeyTuple('ig_intr_md.ingress_port', ig_port),
            client.KeyTuple('vid', 0)])

        forward_table.entry_add(
            target,
            [fwd_key],
            [forward_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', 1)])])

        try:
            pkt = testutils.simple_tcp_packet(eth_dst=dmac)
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            exp_pkt = pkt
            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packets(self, exp_pkt, [eg_port])
        finally:
            forward_table.entry_del(target, [fwd_key])

            action_profile.entry_del(target, [ap_key])

        try:
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)
        finally:
            pass


class ActionProfileEntryGetTest(BfRuntimeTest):
    """@brief Simple test to read multiple ActionProfile entries from the target.
    """

    def setUp(self):
        ''' Setup the test. '''
        client_id = 0
        p4_name = "tna_action_profile"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        num_entries = 100
        seed = random.randint(1, 65535)
        logger.info("Seed used is %d", seed)
        random.seed(seed)

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_action_profile")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_profile = bfrt_info.table_get("SwitchIngress.action_profile")

        eg_ports = [random.randint(1, 5) for x in range(num_entries)]
        ig_ports = list(range(num_entries))
        vids = random.sample(range(4096), num_entries)
        member_ids = list(range(1, num_entries + 1))
        random.shuffle(member_ids)

        logger.info("Adding %d entries to the action profile and match action table.", num_entries)

        ap_keys = []
        fwd_keys = []
        for idx, member_id in enumerate(member_ids):
            ap_key = action_profile.make_key([client.KeyTuple('$ACTION_MEMBER_ID', member_id)])
            ap_keys.append(ap_key)
            action_profile.entry_add(
                target,
                [ap_key],
                [action_profile.make_data(
                    [client.DataTuple('port', eg_ports[idx])], 'SwitchIngress.set_port')])

            fwd_key = forward_table.make_key([
                client.KeyTuple('ig_intr_md.ingress_port', ig_ports[idx]),
                client.KeyTuple('vid', vids[idx])])
            fwd_keys.append(fwd_key)
            forward_table.entry_add(
                target,
                [fwd_key],
                [forward_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', member_id)])])

        try:
            logger.info("Reading %d entries from SwitchIngress.forward table.", num_entries)
            for idx, member_id in enumerate(member_ids):
                resp = forward_table.entry_get(
                    target,
                    [forward_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', ig_ports[idx]),
                        client.KeyTuple('vid', vids[idx])])],
                    {"from_hw": True})
                data_dict = next(resp)[0].to_dict()
                self.assertTrue(data_dict["$ACTION_MEMBER_ID"] == member_id)
        finally:
            # Removing table entries.
            forward_table.entry_del(target, fwd_keys)
            # Removing ActionProfile entries.
            action_profile.entry_del(target, ap_keys)


class ActionProfileEntryIteratorTest(BfRuntimeTest):
    """@brief Simple test to 
    1. read All ActionProfile entries from the target.
    2. delete All ActionProfile entries and MatchActionIndirect
    entries and verify if deleteAll works.
    """

    def setUp(self):
        ''' Setup the test. '''
        client_id = 0
        p4_name = "tna_action_profile"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        num_entries = 100
        seed = random.randint(1, 65535)
        logger.info("Seed used is %d", seed)
        random.seed(seed)

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_action_profile")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_profile = bfrt_info.table_get("SwitchIngress.action_profile")

        eg_ports = [random.randint(1, 5) for x in range(num_entries)]
        ig_ports = list(range(num_entries))
        vids = random.sample(range(4096), num_entries)
        member_ids = list(range(1, num_entries + 1))
        random.shuffle(member_ids)

        target = client.Target(device_id=0, pipe_id=0xffff)

        ap_keys = []
        fwd_keys = []
        logger.info("Adding %d entries to action profile and match table", num_entries)
        for idx, member_id in enumerate(member_ids):
            ap_key = action_profile.make_key([client.KeyTuple('$ACTION_MEMBER_ID', member_id)])
            ap_keys.append(ap_key)
            action_profile.entry_add(
                target,
                [ap_key],
                [action_profile.make_data(
                    [client.DataTuple('port', eg_ports[idx])], 'SwitchIngress.set_port')])

            fwd_key = forward_table.make_key([
                client.KeyTuple('ig_intr_md.ingress_port', ig_ports[idx]),
                client.KeyTuple('vid', vids[idx])])
            fwd_keys.append(fwd_key)
            forward_table.entry_add(
                target,
                [fwd_key],
                [forward_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', member_id)])])

        # Program the default entry
        default_member_id = member_ids[random.randint(0, num_entries - 1)]
        forward_table.default_entry_set(
            target,
            forward_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', default_member_id)]))

        try:
            # Get all the entries
            logger.info("Getting all the %d entries", num_entries)
            resp = forward_table.entry_get(target, None, {"from_hw": False})

            idx = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                assert data_dict["is_default_entry"] == False
                assert key_dict["ig_intr_md.ingress_port"]['value'] == ig_ports[idx]
                assert key_dict["vid"]['value'] == vids[idx]
                assert data_dict["$ACTION_MEMBER_ID"] == member_ids[idx]
                idx += 1

            # Get default entry as well
            resp = forward_table.default_entry_get(target)
            for data, _ in resp:
                data_dict = data.to_dict()
                assert data_dict["is_default_entry"] == True
                assert data_dict["$ACTION_MEMBER_ID"] == default_member_id

            logger.info("SUCCESS : Read entries matched")

        finally:
            # Reset the default entry
            forward_table.default_entry_reset(target)
            # Removing table entries.
            forward_table.entry_del(target)
            # Removing ActionProfile entries.
            action_profile.entry_del(target)

            logger.info("Getting all match entries after clear")
            resp = forward_table.entry_get(target, None, {"from_hw": False})
            for data, key in resp:
                assert 0, "No entries were expected in MatchAction table since they \
                        should have been cleared"

            logger.info("Getting all action entries after clear")
            resp = action_profile.entry_get(target, None, {"from_hw": False})
            for data, key in resp:
                assert 0, "No entries were expected in ActionProfile table since they \
                        should have been cleared"


class SharedActionProfileTest(BfRuntimeTest):
    """@brief Simple test to add entries in a shared ActionProfile and verifying
    that the packets are processed correctly by both tables.
    """

    def setUp(self):
        ''' Setup the test. '''
        client_id = 0
        p4_name = "tna_action_profile"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        num_entries = 50
        seed = random.randint(1, 65535)
        logger.info("Seed used is %d", seed)
        random.seed(seed)

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_action_profile")

        fib_table = bfrt_info.table_get("SwitchIngress.fib")
        fib_table.info.key_field_annotation_add("dst_addr", "ipv4")

        action_profile = bfrt_info.table_get("SwitchIngress.shared_action_profile")
        action_profile.info.data_field_annotation_add("dmac", "SwitchIngress.set_nexthop", "mac")
        action_profile.info.data_field_annotation_add("smac", "SwitchIngress.set_nexthop", "mac")

        member_ids = list(range(1, num_entries + 1))
        eg_ports = [random.choice(swports) for x in range(num_entries)]

        # These values are used as match keys, ensure their uniqueness
        def two_byte_format(format_str):
            return lambda value: format_str.format((value >> 8) & 0xff,
                                                   value & 0xff)

        ip_addrs = list(map(two_byte_format("10.0.{}.{}"),
                            random.sample(range(65536), num_entries)))

        src_mac_addrs = list(map(two_byte_format("11:11:11:11:{:02x}:{:02x}"),
                                 random.sample(range(65536), num_entries)))
        dst_mac_addrs = list(map(two_byte_format("22:22:22:22:{:02x}:{:02x}"),
                                 random.sample(range(65536), num_entries)))

        assert len(ip_addrs) == len(set(ip_addrs))
        assert len(src_mac_addrs) == len(set(src_mac_addrs))
        assert len(dst_mac_addrs) == len(set(dst_mac_addrs))

        actions = [random.choice(['SwitchIngress.set_nexthop', 'SwitchIngress.send_to_cpu']) for x in
                   range(num_entries)]

        logger.info("Adding up to %d entries to action profile table", num_entries)

        reserved_id = None
        ap_keys = []
        fib_keys = []
        for idx, member_id in enumerate(member_ids):
            if actions[idx] == 'SwitchIngress.set_nexthop':
                ap_key = action_profile.make_key([client.KeyTuple('$ACTION_MEMBER_ID', member_id)])
                ap_keys.append(ap_key)
                action_profile.entry_add(
                    target,
                    [ap_key],
                    [action_profile.make_data(
                        [client.DataTuple('port', eg_ports[idx]),
                         client.DataTuple('smac', src_mac_addrs[idx]),
                         client.DataTuple('dmac', dst_mac_addrs[idx])],
                        'SwitchIngress.set_nexthop')])
            elif actions[idx] == 'SwitchIngress.send_to_cpu':
                # Since 'send_to_cpu' has NO action data, we only need to add a single entry in
                # ActionProfile.
                if reserved_id is None:
                    reserved_id = member_ids[idx]
                    ap_key = action_profile.make_key([client.KeyTuple('$ACTION_MEMBER_ID', reserved_id)])
                    ap_keys.append(ap_key)
                    action_profile.entry_add(
                        target,
                        [ap_key],
                        [action_profile.make_data([], 'SwitchIngress.send_to_cpu')])
                else:
                    member_ids[idx] = reserved_id

            fib_key = fib_table.make_key([
                client.KeyTuple('dst_addr', ip_addrs[idx], prefix_len=32),
                client.KeyTuple('vrf', 0)])
            fib_keys.append(fib_key)
            fib_table.entry_add(
                target,
                [fib_key],
                [fib_table.make_data([client.DataTuple('$ACTION_MEMBER_ID', member_ids[idx])])])

        try:
            idx = random.choice(
                [x for x in range(num_entries) if actions[x] == 'SwitchIngress.set_nexthop'])
            pkt = testutils.simple_tcp_packet(ip_dst=ip_addrs[idx])
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            exp_pkt = testutils.simple_tcp_packet(
                eth_src=src_mac_addrs[idx], eth_dst=dst_mac_addrs[idx], ip_dst=ip_addrs[idx], ip_ttl=63)
            logger.info("Expecting packet on port %d", eg_ports[idx])
            testutils.verify_packets(self, exp_pkt, [eg_ports[idx]])
        finally:
            # Clearing table entries.
            logger.info("Clearing fib table")
            fib_table.entry_del(target)
            assert next(fib_table.usage_get(target)) == 0, \
                "usage = %s expected = 0" % (fib_table.usage_get)
            for data, key in fib_table.entry_get(target):
                assert 0, "Not expecting any entries here"

            # Clearing ActionProfile entries.
            logger.info("Clearing action_profile table")
            action_profile.entry_del(target)
            assert next(action_profile.usage_get(target)) == 0, \
                "usage = %s expected = 0" % (action_profile.usage_get)
            for data, key in action_profile.entry_get(target):
                assert 0, "Not expecting any entries here"
