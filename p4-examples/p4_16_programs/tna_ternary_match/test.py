# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import ipaddress
import random

from ptf import config
from collections import namedtuple
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import grpc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

# tuple for future refs
key_random_tuple = namedtuple('key_random', 'dst_ip mask priority')
key_random_tuple.__new__.__defaults__ = (None, None, None)

num_pipes = int(testutils.test_param_get('num_pipes'))


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


def delete_all(forward_table, num_entries, tuple_list, target):
    i = 0
    for item in tuple_list:
        usage = next(forward_table.usage_get(target))
        logger.info("While deleting current entries = %d expected = %d", usage, num_entries - i)
        forward_table.entry_del(
            target,
            [forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', getattr(tuple_list[i], "priority")),
                                     gc.KeyTuple('vrf', 0),
                                     gc.KeyTuple('hdr.ipv4.dst_addr', getattr(item, "dst_ip"),
                                                 getattr(item, "mask"))])])
        i += 1


def setup_random(seed_val=None):
    if seed_val is None:
        if testutils.test_param_get('test_seed') is not 'None':
            seed_val = int(testutils.test_param_get('test_seed'))
    if seed_val is None:
        seed_val = random.randint(1, 65535)
    logger.info("Seed is: %d", seed_val)
    random.seed(seed_val)
    return seed_val


class TernaryMatchTest(BfRuntimeTest):
    """@brief Basic ternary match test using a TCAM-based match table
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = [swports[2], swports[3]]
        dip = '10.10.0.1'

        pkt = testutils.simple_tcp_packet(ip_dst=dip)
        exp_pkt = pkt

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        try:
            forward_table.entry_add(
                target,
                [forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', 1),
                                         gc.KeyTuple('vrf', 0),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0')])],
                [forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                         'SwitchIngress.hit')])

            # check get
            resp = forward_table.entry_get(
                target,
                [forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', 1),
                                         gc.KeyTuple('vrf', 0),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0')])],
                {"from_hw": True})

            data_dict = next(resp)[0].to_dict()
            recv_port = data_dict["port"]
            if (recv_port != eg_ports[0]):
                logger.error("Error! port sent = %s received port = %s", str(eg_ports[0]), str(recv_port))
                assert 0

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_ports[0])
            testutils.verify_packet(self, exp_pkt, eg_ports[0])
            testutils.verify_no_other_packets(self, timeout=2)

        finally:
            forward_table.entry_del(
                target,
                [forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', 1),
                                         gc.KeyTuple('vrf', 0),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.0.0')])])

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class TernaryMultipleRollbackTest(BfRuntimeTest):
    """@brief This test writes multiple entries to a ternary match table with 
    ROLLBACK_ON_ERROR mode on. So we check that no entries go in if there was 
    an error
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        eg_ports = [swports[5], swports[3]]

        setup_random()

        num_entries = random.randint(10, 100)
        num_entries = 30
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        target = gc.Target(device_id=0, pipe_id=0xffff)

        key_list = []
        data_list = []

        try:
            i = 0
            tuple_list = []

            logger.info("Inserting %d entries", num_entries)
            while i < num_entries:
                dst_ip = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                p_len = random.randint(1, 32)
                prio = random.randint(1, 5000)
                mask = ((0xffffffff) << (32 - p_len)) & (0xffffffff)
                logger.info("Inserting entry_#=%d and dst_ip=%s", i + 1, dst_ip)

                # Check if the ip,mask is already present in the tuple list
                dup_list = [[item] for item in tuple_list if
                            (item.dst_ip == dst_ip and item.mask == mask and item.priority == prio)]
                if (len(dup_list) != 0):
                    continue

                tuple_list.append(key_random_tuple(dst_ip, mask, prio))
                key_list.append(forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', prio),
                                                        gc.KeyTuple('vrf', 0),
                                                        gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip, mask)]))

                data_list.append(forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                                         'SwitchIngress.hit'))

                # Add a duplicate entry after every 3 entries
                if (i % 3 == 0):
                    key_list.append(
                        forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', getattr(tuple_list[i], "priority")),
                                                gc.KeyTuple('vrf', 0),
                                                gc.KeyTuple('hdr.ipv4.dst_addr', getattr(tuple_list[i], "dst_ip"),
                                                            getattr(tuple_list[i], "mask"))]))

                    data_list.append(forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                                             'SwitchIngress.hit'))
                i += 1

            try:
                forward_table.entry_add(target, key_list, data_list, bfruntime_pb2.WriteRequest.ROLLBACK_ON_ERROR)
            except gc.BfruntimeRpcException as e:
                # The error list should only have one error since the write
                # request should have failed at the very first error
                error_list = e.sub_errors_get()
                logger.info("Expected error length = %d Received %d",
                            1, len(error_list))
                assert len(error_list) == 1

            # Get Table Usage
            usage = next(forward_table.usage_get(target))
            logger.info("Current entries = %d expected = %d", usage, 0)
            assert usage == 0
        finally:
            logger.info("The cleanup is expected to fail because the rollback removed all entries added before.")
            try:
                delete_all(forward_table, num_entries, tuple_list, target)
            except gc.BfruntimeRpcException as e:
                pass


class TernaryMultipleContinueOnErrorTest(BfRuntimeTest):
    """@brief This test writes multiple entries to a ternary match table with 
    the default CONTINUE_ON_ERROR mode on. Only the ones that are supposed to 
    fail should and the rest of them should go in
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        eg_ports = [swports[5], swports[3]]

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        setup_random()
        num_entries = random.randint(10, 100)

        target = gc.Target(device_id=0, pipe_id=0xffff)

        try:
            i = 0
            tuple_list = []
            key_list = []
            data_list = []
            logger.info("Inserting %d entries", num_entries)
            while i < num_entries:
                dst_ip = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                p_len = random.randint(1, 32)
                prio = random.randint(1, 5000)
                mask = ((0xffffffff) << (32 - p_len)) & (0xffffffff)
                logger.info("Inserting entry_#=%d and dst_ip=%s", i + 1, dst_ip)

                # Check if the ip,mask is already present in the tuple list
                dup_list = [[item] for item in tuple_list if
                            (item.dst_ip == dst_ip and item.mask == mask and item.priority == prio)]
                if (len(dup_list) != 0):
                    continue

                tuple_list.append(key_random_tuple(dst_ip, mask, prio))
                key_list.append(forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', prio),
                                                        gc.KeyTuple('vrf', 0),
                                                        gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip, mask)]))

                data_list.append(forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                                         'SwitchIngress.hit'))
                # Add a duplicate entry after every 3 entries
                if (i % 3 == 0):
                    key_list.append(
                        forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', getattr(tuple_list[i], "priority")),
                                                gc.KeyTuple('vrf', 0),
                                                gc.KeyTuple('hdr.ipv4.dst_addr', getattr(tuple_list[i], "dst_ip"),
                                                            getattr(tuple_list[i], "mask"))]))

                    data_list.append(forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                                             'SwitchIngress.hit'))
                i += 1

            try:
                forward_table.entry_add(target, key_list, data_list)
            except gc.BfruntimeRpcException as e:
                # The error list should have as many errors as there were duplicate entries
                # That means there are num_entries/3 errors
                error_list = e.sub_errors_get()
                import math
                logger.info("Expected error length = %d Received = %d",
                            math.ceil(float(num_entries) / 3), len(error_list))
                print(e)
                assert len(error_list) == math.ceil(float(num_entries) / 3)

            # Get Table Usage
            usage = next(forward_table.usage_get(target))
            logger.info("Current entries = %d expected = %d", usage, num_entries)
            assert usage == num_entries

        finally:
            delete_all(forward_table, num_entries, tuple_list, target)


class TernaryMatchMultipleEntryTest(BfRuntimeTest):
    """@brief Add a set of entries and verify the correct one is selected.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        eg_ports = [swports[5], swports[3]]

        setup_random()
        num_entries = random.randint(1, 100)

        target = gc.Target(device_id=0, pipe_id=0xffff)
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "bytes")

        try:
            i = 0
            tuple_list = []
            key_list = []
            data_list = []
            while i < num_entries:
                dst_ip = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                p_len = random.randint(1, 32)
                prio = random.randint(1, 5000)
                mask = ((0xffffffff) << (32 - p_len)) & (0xffffffff)
                logger.info("Inserting entry_#=%d and dst_ip=%s", i + 1, dst_ip)

                # Check if the ip,mask is already present in the tuple list
                dup_list = [[item] for item in tuple_list if
                            (item.dst_ip == dst_ip and item.mask == mask and item.priority == prio)]
                if (len(dup_list) != 0):
                    continue

                i += 1
                tuple_list.append(key_random_tuple(gc.ipv4_to_bytes(dst_ip), gc.to_bytes(mask, 4), prio))
                key_list.append(forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', prio),
                                                        gc.KeyTuple('vrf', 0),
                                                        gc.KeyTuple('hdr.ipv4.dst_addr', gc.ipv4_to_bytes(dst_ip),
                                                                    gc.to_bytes(mask, 4))]))

                data_list.append(forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                                         'SwitchIngress.hit'))
            forward_table.entry_add(target, key_list, data_list)

            # Get Table Usage
            usage = next(forward_table.usage_get(target))
            logger.info("Current entries = %d expected = %d",
                        usage, num_entries)
            assert usage == num_entries

            # check get all
            resp = forward_table.entry_get(
                target,
                None,
                {"from_hw": True})

            i = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                recv_port = data_dict["port"]
                if (recv_port != eg_ports[0]):
                    logger.error("Error! port sent = %s received port = %s", str(eg_ports[0]), str(recv_port))
                    assert 0
                ip_addr = getattr(tuple_list[i], "dst_ip")
                mask = getattr(tuple_list[i], "mask")
                for k in range(4):
                    ip_addr[k] = ip_addr[k] & mask[k]
                assert key_dict["vrf"]['value'] == 0
                assert key_dict["hdr.ipv4.dst_addr"]["value"] == ip_addr
                assert key_dict["hdr.ipv4.dst_addr"]["mask"] == mask
                assert key_dict["$MATCH_PRIORITY"]["value"] == getattr(tuple_list[i], "priority")
                i += 1

        finally:
            delete_all(forward_table, num_entries, tuple_list, target)


class AlgorithmicTernaryMatchTest(BfRuntimeTest):
    """@brief Basic example for algorithmic-TCAM-based match tables.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = [swports[5], swports[3]]

        seed = setup_random()
        num_entries = random.randint(5, 15)

        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")
        forward_atcam_table = bfrt_info.table_get("SwitchIngress.forward_atcam")
        set_partition_table = bfrt_info.table_get("SwitchIngress.set_partition")
        forward_atcam_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        tuple_list = []
        target = gc.Target(device_id=0, pipe_id=0xffff)
        logger.info("Testing for %d entries", num_entries)
        set_partition_table.entry_add(
            target,
            [set_partition_table.make_key([gc.KeyTuple('hdr.ipv4.protocol', 6)])],
            [set_partition_table.make_data([gc.DataTuple('p_index', 3)],
                                           'SwitchIngress.init_index')]
        )

        set_partition_table.entry_add(
            target,
            [set_partition_table.make_key([gc.KeyTuple('hdr.ipv4.protocol', 17)])],
            [set_partition_table.make_data([gc.DataTuple('p_index', 1)],
                                           'SwitchIngress.init_index')]
        )

        dst_ip_list = self.generate_random_ip_list(num_entries, seed)
        atcam_dict = {}
        for i in range(num_entries):
            # insert entry in both set_partition and forward_atcam
            # insert entry for TCP and UDP both in set_partition
            key = forward_atcam_table.make_key([gc.KeyTuple('ig_md.partition.partition_index', 3),
                                               gc.KeyTuple('$MATCH_PRIORITY', 1),
                                               gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip_list[i].ip,
                                                           dst_ip_list[i].mask)])
            data = forward_atcam_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                               'SwitchIngress.hit')
            forward_atcam_table.entry_add(target, [key], [data])
            key.apply_mask()
            atcam_dict[key] = data

            key = forward_atcam_table.make_key([gc.KeyTuple('ig_md.partition.partition_index', 1),
                                               gc.KeyTuple('$MATCH_PRIORITY', 1),
                                               gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip_list[i].ip,
                                                           dst_ip_list[i].mask)])
            data = forward_atcam_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                               'SwitchIngress.hit')
            forward_atcam_table.entry_add(target, [key], [data])
            key.apply_mask()
            atcam_dict[key] = data

        # Check get
        resp = forward_atcam_table.entry_get(target)
        for data, key in resp:
            assert atcam_dict[key] == data, "Received key = %s, received data = %s" %(str(key), str(data))
            atcam_dict.pop(key)
        assert len(atcam_dict) == 0

        # send pkt and verify sent
        for item in dst_ip_list:
            # TCP
            pkt = testutils.simple_tcp_packet(ip_dst=item.ip)
            exp_pkt = pkt
            logger.info("Sending TCP packet to %s on port %d", item.ip, ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting TCP packet on port %d", eg_ports[0])
            testutils.verify_packet(self, exp_pkt, eg_ports[0])
            # UDP
            pkt = testutils.simple_udp_packet(ip_dst=item.ip)
            exp_pkt = pkt
            logger.info("Sending UDP packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting UDP packet on port %d", eg_ports[0])
            testutils.verify_packet(self, exp_pkt, eg_ports[0])

        testutils.verify_no_other_packets(self, timeout=2)
        # Delete both the partition table entries
        set_partition_table.entry_del(
            target,
            [set_partition_table.make_key([gc.KeyTuple('hdr.ipv4.protocol', 6)])])

        set_partition_table.entry_del(
            target,
            [set_partition_table.make_key([gc.KeyTuple('hdr.ipv4.protocol', 17)])])

        # Delete all entries
        logger.info("Deleting %d entries", num_entries)

        for item in dst_ip_list:
            forward_atcam_table.entry_del(
                target,
                [forward_atcam_table.make_key([gc.KeyTuple('ig_md.partition.partition_index', 1),
                                               gc.KeyTuple('$MATCH_PRIORITY', 1),
                                               gc.KeyTuple('hdr.ipv4.dst_addr', item.ip, item.mask)])])
            forward_atcam_table.entry_del(
                target,
                [forward_atcam_table.make_key([gc.KeyTuple('ig_md.partition.partition_index', 3),
                                               gc.KeyTuple('$MATCH_PRIORITY', 1),
                                               gc.KeyTuple('hdr.ipv4.dst_addr', item.ip, item.mask)])])

        # send pkt and verify dropped
        for item in dst_ip_list:
            pkt = testutils.simple_tcp_packet(ip_dst=item.ip)
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class TCAMMatchModifyTest(BfRuntimeTest):
    """@brief This test does the following
    1. Adds 100 match entries
    2. Sends packets to 100 match entries and verifies
    3. Modifies 100 match entries
    4. Sends packets to 100 modified entries and verifies
    5. Modifies just the direct register resource
    6. Reads back the direct register resource and verifies.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        setup_random()

        num_entries = 100
        ig_ports = [random.choice(swports) for x in range(num_entries)]
        all_ports = swports_0 + swports_1 + swports_2 + swports_3
        eg_ports = [random.choice(all_ports) for x in range(num_entries)]

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")
        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        tcam_table = bfrt_info.table_get("SwitchIngress.tcam_table")
        self.tcam_table = tcam_table
        tcam_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
        tcam_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        tcam_table.info.data_field_annotation_add("srcMac", "SwitchIngress.change_smac", "mac")
        tcam_table.info.data_field_annotation_add("dstMac", "SwitchIngress.change_dmac", "mac")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        ipdst_dict = {}
        ipsrc_dict = {}
        ipDstAddrs = []
        ipSrcAddrs = []
        ipDstAddrsMask = []
        ipSrcAddrsMask = []
        priorities = [x for x in range(num_entries)]
        random.shuffle(priorities)
        action_choices = ['SwitchIngress.change_smac', 'SwitchIngress.change_dmac']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        srcMacAddrs = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]
        dstMacAddrs = ["%02x:%02x:%02x:%02x:%02x:%02x" % (
            random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255), random.randint(0, 255)) for x in range(num_entries)]

        register_value_hi = [random.randint(1, 10000) for x in range(num_entries)]
        register_value_lo = [random.randint(1, 10000) for x in range(num_entries)]

        for x in range(num_entries):
            ipDstAddr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ipSrcAddr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ipDstAddrMask = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ipSrcAddrMask = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

            masked_ipdst = int(ipaddress.ip_address(ipDstAddr.encode("utf-8").decode())) & int(
                ipaddress.ip_address(ipDstAddrMask.encode("utf-8").decode()))
            masked_ipsrc = int(ipaddress.ip_address(ipSrcAddr.encode("utf-8").decode())) & int(
                ipaddress.ip_address(ipSrcAddrMask.encode("utf-8").decode()))

            while masked_ipdst in ipdst_dict:
                ipDstAddr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

            ipdst_dict[masked_ipdst] = True

            while masked_ipsrc in ipsrc_dict:
                ipSrcAddr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

            ipsrc_dict[masked_ipsrc] = True

            ipDstAddrs.append(ipDstAddr)
            ipSrcAddrs.append(ipSrcAddr)
            ipDstAddrsMask.append(ipDstAddrMask)
            ipSrcAddrsMask.append(ipSrcAddrMask)

        logger.info("Adding %d entries to SwitchIngress.tcam_table table", num_entries)

        for x in range(num_entries):
            if action[x] == 'SwitchIngress.change_smac':
                tcam_table.entry_add(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                           gc.DataTuple('srcMac', srcMacAddrs[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_smac')])

            elif action[x] == 'SwitchIngress.change_dmac':
                tcam_table.entry_add(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                           gc.DataTuple('dstMac', dstMacAddrs[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_dmac')])

        logger.info("DONE Adding %d entries to SwitchIngress.tcam_table table", num_entries)

        logger.info("Sending packets to all %d entries of SwitchIngress.tcam_table table", num_entries)
        for x in range(num_entries):
            pkt = testutils.simple_tcp_packet(ip_dst=ipDstAddrs[x],
                                              ip_src=ipSrcAddrs[x],
                                              with_tcp_chksum=False)
            if action[x] == 'SwitchIngress.change_smac':
                exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                      ip_dst=ipDstAddrs[x],
                                                      ip_src=ipSrcAddrs[x],
                                                      with_tcp_chksum=False)
            elif action[x] == 'SwitchIngress.change_dmac':
                exp_pkt = testutils.simple_tcp_packet(eth_dst=dstMacAddrs[x],
                                                      ip_dst=ipDstAddrs[x],
                                                      ip_src=ipSrcAddrs[x],
                                                      with_tcp_chksum=False)

            logger.info("Sending packet on port %d", ig_ports[x])
            testutils.send_packet(self, ig_ports[x], pkt)

            logger.info("Expecting packet on port %d", eg_ports[x])
            testutils.verify_packet(self, exp_pkt, eg_ports[x])

        testutils.verify_no_other_packets(self, timeout=2)
        logger.info("DONE Sending packets to all %d entries of SwitchIngress.tcam_table table", num_entries)

        # Shuffle around the action data and the register values for a modify
        random.shuffle(srcMacAddrs)
        random.shuffle(dstMacAddrs)
        random.shuffle(action)

        random.shuffle(register_value_hi)
        random.shuffle(register_value_lo)

        logger.info("Modifying %d entries of SwitchIngress.tcam_table table", num_entries)

        for x in range(num_entries):
            if action[x] == 'SwitchIngress.change_smac':
                tcam_table.entry_mod(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                           gc.DataTuple('srcMac', srcMacAddrs[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_smac')])

            elif action[x] == 'SwitchIngress.change_dmac':
                tcam_table.entry_mod(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                           gc.DataTuple('dstMac', dstMacAddrs[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_dmac')])

        logger.info("DONE Modifying %d entries of SwitchIngress.tcam_table table", num_entries)

        logger.info("Sending packets to all %d modified entries of SwitchIngress.tcam_table table", num_entries)

        for x in range(num_entries):
            pkt = testutils.simple_tcp_packet(ip_dst=ipDstAddrs[x],
                                              ip_src=ipSrcAddrs[x],
                                              with_tcp_chksum=False)
            if action[x] == 'SwitchIngress.change_smac':
                exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                      ip_dst=ipDstAddrs[x],
                                                      ip_src=ipSrcAddrs[x],
                                                      with_tcp_chksum=False)
            elif action[x] == 'SwitchIngress.change_dmac':
                exp_pkt = testutils.simple_tcp_packet(eth_dst=dstMacAddrs[x],
                                                      ip_dst=ipDstAddrs[x],
                                                      ip_src=ipSrcAddrs[x],
                                                      with_tcp_chksum=False)

            logger.info("Sending packet on port %d", ig_ports[x])
            testutils.send_packet(self, ig_ports[x], pkt)

            logger.info("Expecting packet on port %d", eg_ports[x])
            testutils.verify_packet(self, exp_pkt, eg_ports[x])

        testutils.verify_no_other_packets(self, timeout=2)

        logger.info("DONE Sending packets to all %d modified entries of SwitchIngress.tcam_table table",
                    num_entries)

        logger.info("Modifying direct register for %d entries of SwitchIngress.tcam_table table", num_entries)

        for x in range(num_entries):
            if action[x] == 'SwitchIngress.change_smac':
                tcam_table.entry_mod(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_smac')])

            elif action[x] == 'SwitchIngress.change_dmac':
                tcam_table.entry_mod(
                    target,
                    [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                          gc.KeyTuple('hdr.ipv4.dst_addr',
                                                      ipDstAddrs[x],
                                                      ipDstAddrsMask[x]),
                                          gc.KeyTuple('hdr.ipv4.src_addr',
                                                      ipSrcAddrs[x],
                                                      ipSrcAddrsMask[x])])],
                    [tcam_table.make_data([gc.DataTuple('SwitchIngress.direct_reg.first', register_value_hi[x]),
                                           gc.DataTuple('SwitchIngress.direct_reg.second', register_value_lo[x])],
                                          'SwitchIngress.change_dmac')])

        logger.info("DONE Modifying direct register for %d entries of SwitchIngress.tcam_table table", num_entries)

        logger.info("Reading direct register from hardware for %d entries of SwitchIngress.tcam_table table", num_entries)

        for x in range(num_entries):
            resp = tcam_table.entry_get(
                target,
                [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                      gc.KeyTuple('hdr.ipv4.dst_addr',
                                                  ipDstAddrs[x],
                                                  ipDstAddrsMask[x]),
                                      gc.KeyTuple('hdr.ipv4.src_addr',
                                                  ipSrcAddrs[x],
                                                  ipSrcAddrsMask[x])])],
                {"from_hw": True},
                tcam_table.make_data([gc.DataTuple("SwitchIngress.direct_reg.first"),
                                      gc.DataTuple("SwitchIngress.direct_reg.second")],
                                     None, get=True))

            fields = next(resp)[0].to_dict()
            assert [register_value_hi[x]] * num_pipes == fields["SwitchIngress.direct_reg.first"]
            assert [register_value_lo[x]] * num_pipes == fields["SwitchIngress.direct_reg.second"]

        logger.info("-ve tests for entry get")
        #negative wildcard read tests
        #1. get for an entry using wrong action should fail
        #2. wildcard read using an action should only get back some entries
        for x in range(num_entries):
            act_to_query = ""
            if action[x] == "SwitchIngress.change_smac":
                act_to_query = "SwitchIngress.change_dmac"
            else:
                act_to_query = "SwitchIngress.change_smac"

            resp = tcam_table.entry_get(
                target,
                [tcam_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                      gc.KeyTuple('hdr.ipv4.dst_addr',
                                                  ipDstAddrs[x],
                                                  ipDstAddrsMask[x]),
                                      gc.KeyTuple('hdr.ipv4.src_addr',
                                                  ipSrcAddrs[x],
                                                  ipSrcAddrsMask[x])])],
                {"from_hw": True},
                tcam_table.make_data([gc.DataTuple("SwitchIngress.direct_reg.first"),
                                      gc.DataTuple("SwitchIngress.direct_reg.second")],
                                     act_to_query, get=True))
            error_recvd = False
            try:
                fields = next(resp)[0].to_dict()
            except gc.BfruntimeRpcException as e:
                error_recvd = True
            assert error_recvd, "Expecting error as part of -ve test of filter get with action"

        # Select opposite of the action of the first entry so that entryGetFirst itself fails
        act_to_query = ""
        if action[0] == "SwitchIngress.change_smac":
            act_to_query = "SwitchIngress.change_dmac"
        else:
            act_to_query = "SwitchIngress.change_smac"
        num_data_for_act = action.count(act_to_query)
        resp = tcam_table.entry_get(target, None, {"from_hw": True}, 
                tcam_table.make_data([gc.DataTuple("SwitchIngress.direct_reg.first"),
                                      gc.DataTuple("SwitchIngress.direct_reg.second")],
                                     act_to_query, get=True))
        recv_count = 0
        for data, key in resp:
            recv_count += 1
        assert recv_count == num_data_for_act, "Expecting %d number of data objects, received %d" \
                % (recv_count, num_data_for_act)

        logger.info("Deleting %d entries of SwitchIngress.tcam_table table", num_entries)
        tcam_table.entry_del(target)
        logger.info("DONE Deleting %d entries of SwitchIngress.tcam_table table", num_entries)


class TCAMMatchIndirectModifyTest(BfRuntimeTest):
    """@brief This test does the following
    1. Adds 100 action profile entries
    2. Adds 100 match entries pointing ot action member IDs
    3. Sends packets to 100 match entries and verifies
    4. Modifies 100 action profile entries
    5. Modifies 100 match entries to point to different action member ids
    6. Sends packets to 100 modified entries and verifies
    7. Modifies just the direct LPF spec
    8. Reads back the direct LPF spec and verifies.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_ternary_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def macAddrtoint(self, macAddr):
        a = macAddr.split(":")
        a = "".join(a)
        return int(a, 16)

    def findHit(self, num_entries, x, MacAddrs, MacAddrsMask, priorities):
        pkt_mac = self.macAddrtoint(MacAddrs[x])
        hit_index = x
        for y in range(num_entries):
            if priorities[y] < priorities[hit_index]:
                mac = self.macAddrtoint(MacAddrs[y])
                mask = self.macAddrtoint(MacAddrsMask[y])
                if (pkt_mac & mask) == (mac & mask):
                    hit_index = y
        return hit_index

    def runTest(self):
        seed = setup_random()

        num_entries = 100
        ig_ports = [random.choice(swports) for x in range(num_entries)]
        all_ports = swports_0 + swports_1 + swports_2 + swports_3
        eg_ports = [random.choice(all_ports) for x in range(num_entries)]

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ternary_match")

        action_profile_table = bfrt_info.table_get("SwitchIngress.action_profile")
        tcam_direct_lpf_table = bfrt_info.table_get("SwitchIngress.tcam_direct_lpf")

        action_profile_table.info.data_field_annotation_add("srcAddr", "SwitchIngress.change_ipsrc", "ipv4")
        action_profile_table.info.data_field_annotation_add("dstAddr", "SwitchIngress.change_ipdst", "ipv4")
        tcam_direct_lpf_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        tcam_direct_lpf_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        srcMac_dict = {}
        dstMac_dict = {}
        srcMacAddrs = []
        dstMacAddrs = []
        srcMacAddrsMask = []
        dstMacAddrsMask = []
        priorities = [x for x in range(num_entries)]
        random.shuffle(priorities)

        action_choices = ['SwitchIngress.change_ipsrc', 'SwitchIngress.change_ipdst']
        action = [action_choices[random.randint(0, 1)] for x in range(num_entries)]

        action_mbr_ids = [x + 1 for x in range(num_entries)]

        ipDstAddrs = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]
        ipSrcAddrs = ["%d.%d.%d.%d" % (
            random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)) for x in
            range(num_entries)]

        lpf_types = [random.choice(["RATE", "SAMPLE"]) for x in range(num_entries)]

        gain_time = [round(random.uniform(1, 1000), 2) for x in range(num_entries)]
        decay_time = gain_time
        out_scale = [random.randint(1, 31) for x in range(num_entries)]

        srcMacAddrtuple = self.generate_random_mac_list(num_entries, seed)
        dstMacAddrtuple = self.generate_random_mac_list(num_entries, seed)

        srcMacAddrs = [getattr(each, "mac") for each in srcMacAddrtuple]
        srcMacAddrsMask = [getattr(each, "mask") for each in srcMacAddrtuple]

        dstMacAddrs = [getattr(each, "mac") for each in dstMacAddrtuple]
        dstMacAddrsMask = [getattr(each, "mask") for each in dstMacAddrtuple]

        random.shuffle(action_mbr_ids)

        logger.info("Adding %d entries to SwitchIngress.action_profile table", num_entries)

        try:
            for x in range(num_entries):
                if action[x] == 'SwitchIngress.change_ipsrc':
                    action_profile_table.entry_add(
                        target,
                        [action_profile_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[x])])],
                        [action_profile_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                                         gc.DataTuple('srcAddr', ipSrcAddrs[x])],
                                                        'SwitchIngress.change_ipsrc')])
                elif action[x] == 'SwitchIngress.change_ipdst':
                    action_profile_table.entry_add(
                        target,
                        [action_profile_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[x])])],
                        [action_profile_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                                         gc.DataTuple('dstAddr', ipDstAddrs[x])],
                                                        'SwitchIngress.change_ipdst')])

            logger.info("DONE Adding %d entries to SwitchIngress.action_profile table", num_entries)

            logger.info("Adding %d entries to SwitchIngress.tcam_direct_lpf table", num_entries)

            for x in range(num_entries):
                tcam_direct_lpf_table.entry_add(
                    target,
                    [tcam_direct_lpf_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                                     gc.KeyTuple('hdr.ethernet.dst_addr',
                                                                 dstMacAddrs[x],
                                                                 dstMacAddrsMask[x]),
                                                     gc.KeyTuple('hdr.ethernet.src_addr',
                                                                 srcMacAddrs[x],
                                                                 srcMacAddrsMask[x])])],
                    [tcam_direct_lpf_table.make_data([gc.DataTuple('$ACTION_MEMBER_ID', action_mbr_ids[x]),
                                                      gc.DataTuple('$LPF_SPEC_TYPE', str_val=lpf_types[x]),
                                                      gc.DataTuple('$LPF_SPEC_GAIN_TIME_CONSTANT_NS',
                                                                   float_val=gain_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_DECAY_TIME_CONSTANT_NS',
                                                                   float_val=decay_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_OUT_SCALE_DOWN_FACTOR', out_scale[x])])])

            logger.info("DONE Adding %d entries to SwitchIngress.tcam_direct_lpf table", num_entries)

            logger.info("Sending packets to all %d entries of SwitchIngress.tcam_table table", num_entries)
            for x in range(num_entries):
                pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                  eth_dst=dstMacAddrs[x],
                                                  with_tcp_chksum=False)
                y = self.findHit(num_entries, x, dstMacAddrs, dstMacAddrsMask, priorities)
                if action[y] == 'SwitchIngress.change_ipsrc':
                    exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                          eth_dst=dstMacAddrs[x],
                                                          ip_src=ipSrcAddrs[y],
                                                          with_tcp_chksum=False)
                elif action[y] == 'SwitchIngress.change_ipdst':
                    exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                          eth_dst=dstMacAddrs[x],
                                                          ip_dst=ipDstAddrs[y],
                                                          with_tcp_chksum=False)

                logger.info("Sending packet on port %d", ig_ports[x])
                testutils.send_packet(self, ig_ports[x], pkt)

                logger.info("Expecting packet on port %d", eg_ports[y])
                testutils.verify_packet(self, exp_pkt, eg_ports[y])

            testutils.verify_no_other_packets(self, timeout=2)

            logger.info("DONE Sending packets to all %d entries of SwitchIngress.tcam_table table", num_entries)

            # Shuffle around the action data values for a modify
            random.shuffle(ipSrcAddrs)
            random.shuffle(ipDstAddrs)
            random.shuffle(action)
            random.shuffle(action_mbr_ids)

            logger.info("Modifying %d entries of SwitchIngress.action_profile table", num_entries)
            for x in range(num_entries):
                if action[x] == 'SwitchIngress.change_ipsrc':
                    action_profile_table.entry_mod(
                        target,
                        [action_profile_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[x])])],
                        [action_profile_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                                         gc.DataTuple('srcAddr', ipSrcAddrs[x])],
                                                        'SwitchIngress.change_ipsrc')])

                elif action[x] == 'SwitchIngress.change_ipdst':
                    action_profile_table.entry_mod(
                        target,
                        [action_profile_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[x])])],
                        [action_profile_table.make_data([gc.DataTuple('dst_port', eg_ports[x]),
                                                         gc.DataTuple('dstAddr', ipDstAddrs[x])],
                                                        'SwitchIngress.change_ipdst')])
            logger.info("DONE Modifying %d entries of SwitchIngress.action_profile table", num_entries)

            logger.info("Modifying %d entries of SwitchIngress.tcam_direct_lpf table", num_entries)
            for x in range(num_entries):
                tcam_direct_lpf_table.entry_mod(
                    target,
                    [tcam_direct_lpf_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                                     gc.KeyTuple('hdr.ethernet.dst_addr',
                                                                 dstMacAddrs[x],
                                                                 dstMacAddrsMask[x]),
                                                     gc.KeyTuple('hdr.ethernet.src_addr',
                                                                 srcMacAddrs[x],
                                                                 srcMacAddrsMask[x])])],
                    [tcam_direct_lpf_table.make_data([gc.DataTuple('$ACTION_MEMBER_ID', action_mbr_ids[x]),
                                                      gc.DataTuple('$LPF_SPEC_TYPE', str_val=lpf_types[x]),
                                                      gc.DataTuple('$LPF_SPEC_GAIN_TIME_CONSTANT_NS',
                                                                   float_val=gain_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_DECAY_TIME_CONSTANT_NS',
                                                                   float_val=decay_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_OUT_SCALE_DOWN_FACTOR', out_scale[x])])])

            logger.info("Sending packets to all %d MODIFIED entries of SwitchIngress.tcam_table table", num_entries)

            for x in range(num_entries):
                pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                  eth_dst=dstMacAddrs[x],
                                                  with_tcp_chksum=False)
                y = self.findHit(num_entries, x, dstMacAddrs, dstMacAddrsMask, priorities)
                if action[y] == 'SwitchIngress.change_ipsrc':
                    exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                          eth_dst=dstMacAddrs[x],
                                                          ip_src=ipSrcAddrs[y],
                                                          with_tcp_chksum=False)
                elif action[y] == 'SwitchIngress.change_ipdst':
                    exp_pkt = testutils.simple_tcp_packet(eth_src=srcMacAddrs[x],
                                                          eth_dst=dstMacAddrs[x],
                                                          ip_dst=ipDstAddrs[y],
                                                          with_tcp_chksum=False)

                logger.info("Sending packet on port %d", ig_ports[x])
                testutils.send_packet(self, ig_ports[x], pkt)

                logger.info("Expecting packet on port %d", eg_ports[y])
                testutils.verify_packet(self, exp_pkt, eg_ports[y])

            testutils.verify_no_other_packets(self, timeout=2)

            logger.info("DONE Sending packets to all %d MODIFIED entries of SwitchIngress.tcam_table table",
                        num_entries)

            # Now shuffle around LPF data to do modify
            random.shuffle(lpf_types)
            random.shuffle(gain_time)
            decay_time = gain_time
            random.shuffle(out_scale)

            logger.info("Modifying direct LPF SPEC of %d entries of SwitchIngress.tcam_direct_lpf table", num_entries)

            for x in range(num_entries):
                tcam_direct_lpf_table.entry_mod(
                    target,
                    [tcam_direct_lpf_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                                     gc.KeyTuple('hdr.ethernet.dst_addr',
                                                                 dstMacAddrs[x],
                                                                 dstMacAddrsMask[x]),
                                                     gc.KeyTuple('hdr.ethernet.src_addr',
                                                                 srcMacAddrs[x],
                                                                 srcMacAddrsMask[x])])],
                    [tcam_direct_lpf_table.make_data([gc.DataTuple('$LPF_SPEC_TYPE', str_val=lpf_types[x]),
                                                      gc.DataTuple('$LPF_SPEC_GAIN_TIME_CONSTANT_NS',
                                                                   float_val=gain_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_DECAY_TIME_CONSTANT_NS',
                                                                   float_val=decay_time[x]),
                                                      gc.DataTuple('$LPF_SPEC_OUT_SCALE_DOWN_FACTOR', out_scale[x])])])

            logger.info("DONE Modifying direct LPF SPEC of %d entries of SwitchIngress.tcam_direct_lpf table",
                        num_entries)

            logger.info("Reading direct LPF spec from hardware for %d entries of SwitchIngress.tcam_direct_lpf table",
                        num_entries)

            for x in range(num_entries):
                resp = tcam_direct_lpf_table.entry_get(
                    target,
                    [tcam_direct_lpf_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                                     gc.KeyTuple('hdr.ethernet.dst_addr',
                                                                 dstMacAddrs[x],
                                                                 dstMacAddrsMask[x]),
                                                     gc.KeyTuple('hdr.ethernet.src_addr',
                                                                 srcMacAddrs[x],
                                                                 srcMacAddrsMask[x])])],
                    {"from_hw": True})

                fields = next(resp)[0].to_dict()
                assert fields["$LPF_SPEC_TYPE"] == lpf_types[x]
                assert abs(fields["$LPF_SPEC_GAIN_TIME_CONSTANT_NS"] - gain_time[x]) <= gain_time[x] * 0.02
                assert abs(fields["$LPF_SPEC_DECAY_TIME_CONSTANT_NS"] - decay_time[x]) <= decay_time[x] * 0.02
                assert fields["$LPF_SPEC_OUT_SCALE_DOWN_FACTOR"] == out_scale[x]

            logger.info("DONE Reading direct LPF spec from hardware for %d entries of SwitchIngress.tcam_table table",
                        num_entries)

        finally:
            logger.info("Deleting %d entries of SwitchIngress.tcam_direct_lpf table", num_entries)
            try:
                for x in range(num_entries):
                    tcam_direct_lpf_table.entry_del(
                        target,
                        [tcam_direct_lpf_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', priorities[x]),
                                                         gc.KeyTuple('hdr.ethernet.dst_addr',
                                                                     dstMacAddrs[x],
                                                                     dstMacAddrsMask[x]),
                                                         gc.KeyTuple('hdr.ethernet.src_addr',
                                                                     srcMacAddrs[x],
                                                                     srcMacAddrsMask[x])])])
            except gc.BfruntimeRpcException as e:
                raise e
            try:
                for x in range(num_entries):
                    action_profile_table.entry_del(
                        target,
                        [action_profile_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', action_mbr_ids[x])])])
            except gc.BfruntimeRpcException as e:
                raise e
