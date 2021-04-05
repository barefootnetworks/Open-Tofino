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
import bfrt_grpc.client as gc
import grpc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class LpmMatchTest(BfRuntimeTest):
    """@brief Basic test for TCAM-based lpm matches.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_lpm_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = [swports[5], swports[3]]

        seed = random.randint(1, 65535)
        logger.info("Using seed %d", seed)
        random.seed(seed)
        num_entries = random.randint(5, 10)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_lpm_match")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        key_random_tuple = namedtuple('key_random', 'vrf dst_ip prefix_len')
        tuple_list = []
        unique_keys = {}
        i = 0
        ip_list = self.generate_random_ip_list(num_entries, seed)
        lpm_dict = {}
        while (i < num_entries):
            vrf = 0
            dst_ip = getattr(ip_list[i], "ip")
            p_len = getattr(ip_list[i], "prefix_len")
            tuple_list.append(key_random_tuple(vrf, dst_ip, p_len))
            logger.info("Adding %d %s %d", vrf, dst_ip, p_len)

            target = gc.Target(device_id=0, pipe_id=0xffff)
            key = forward_table.make_key([gc.KeyTuple('vrf', vrf),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip, prefix_len=p_len)])
            data = forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                         'SwitchIngress.hit')
            forward_table.entry_add(target, [key], [data])
            key.apply_mask()
            lpm_dict[key] = data
            i += 1

        # send pkt and verify sent
        for item in tuple_list:
            pkt = testutils.simple_tcp_packet(ip_dst=item[1])
            exp_pkt = pkt
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_ports[0])  # Change this --> eg_port[0]
            testutils.verify_packets(self, exp_pkt, [eg_ports[0]])

        # check get
        resp  = forward_table.entry_get(target)
        for data, key in resp:
            assert lpm_dict[key] == data
            lpm_dict.pop(key)
        assert len(lpm_dict) == 0

        # delete all entries
        for item in tuple_list:
            forward_table.entry_del(
                target,
                [forward_table.make_key([gc.KeyTuple('vrf', item[0]),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', item[1], prefix_len=item[2])])])

        # send pkt and verify dropped
        for item in tuple_list:
            pkt = testutils.simple_tcp_packet(ip_dst=item[1])
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class LpmMatchMultipleEntryOneErrorTest(BfRuntimeTest):
    """@brief Create random lpm table entries and read them back from the device.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_lpm_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = [swports[5], swports[3]]

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Using seed %d", seed)
        num_entries = random.randint(1, 20)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_lpm_match")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        key_random_tuple = namedtuple('key_random', 'vrf dst_ip prefix_len')
        tuple_list = []
        unique_keys = {}
        i = 0
        ip_list = self.generate_random_ip_list(num_entries, seed)
        while (i < num_entries):
            vrf = 0
            dst_ip = getattr(ip_list[i], "ip")
            p_len = getattr(ip_list[i], "prefix_len")
            tuple_list.append(key_random_tuple(vrf, dst_ip, p_len))
            logger.info("Adding %d %s %d", vrf, dst_ip, p_len)

            target = gc.Target(device_id=0, pipe_id=0xffff)
            forward_table.entry_add(
                target,
                [forward_table.make_key([gc.KeyTuple('vrf', vrf),
                                         gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip, prefix_len=p_len)])],
                [forward_table.make_data([gc.DataTuple('port', eg_ports[0])],
                                         'SwitchIngress.hit')])
            i += 1

        # check get
        key_list = []
        for item in tuple_list:
            key_list.append(forward_table.make_key([gc.KeyTuple('vrf', item[0]),
                                                    gc.KeyTuple('hdr.ipv4.dst_addr', item[1], prefix_len=item[2])]))
        # Just for checking, let's tableGet one invalid entry
        key_list.append(forward_table.make_key([gc.KeyTuple('vrf', item[0]),
                                                gc.KeyTuple('hdr.ipv4.dst_addr', "0.255.255.255", prefix_len=32)]))

        resp = forward_table.entry_get(target,
                                       key_list, {"from_hw": True})
        try:
            for data, key in resp:
                data_dict = data.to_dict()
                recv_port = data_dict["port"]
                if (recv_port != eg_ports[0]):
                    logger.error("Error! port sent = %s received port = %s", str(eg_ports[0]), str(recv_port))
                    assert 0
        except gc.BfruntimeRpcException as e:
            # The error list should have 1 entry
            error_list = e.sub_errors_get()
            logger.info("Expected error length = %d Received = %d", 1,
                        len(error_list))
            assert len(error_list) == 1
        finally:
            # delete all entries
            for item in tuple_list:
                forward_table.entry_del(
                    target,
                    [forward_table.make_key([gc.KeyTuple('vrf', item[0]),
                                             gc.KeyTuple('hdr.ipv4.dst_addr', item[1], prefix_len=item[2])])])


class AlpmMatchTest(BfRuntimeTest):
    """@brief Basic test for algorithmic-lpm-based lpm matches.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_lpm_match"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        seed = random.randint(1, 65535)
        logger.info("Seed used %d", seed)
        random.seed(seed)
        num_entries = random.randint(1, 30)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_lpm_match")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        alpm_forward_table = bfrt_info.table_get("SwitchIngress.alpm_forward")
        alpm_forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
        alpm_forward_table.info.data_field_annotation_add("srcMac", "SwitchIngress.route", "mac")
        alpm_forward_table.info.data_field_annotation_add("dstMac", "SwitchIngress.route", "mac")

        key_random_tuple = namedtuple('key_random', 'vrf dst_ip prefix_len')
        data_random_tuple = namedtuple('data_random', 'smac dmac eg_port')
        key_tuple_list = []
        data_tuple_list = []
        unique_keys = {}
        lpm_dict= {}

        logger.info("Installing %d ALPM entries" % (num_entries))
        ip_list = self.generate_random_ip_list(num_entries, seed)
        for i in range(0, num_entries):
            vrf = 0
            dst_ip = getattr(ip_list[i], "ip")
            p_len = getattr(ip_list[i], "prefix_len")

            srcMac = "%02x:%02x:%02x:%02x:%02x:%02x" % tuple([random.randint(0, 255) for x in range(6)])
            dstMac = "%02x:%02x:%02x:%02x:%02x:%02x" % tuple([random.randint(0, 255) for x in range(6)])
            eg_port = swports[random.randint(1, 4)]

            key_tuple_list.append(key_random_tuple(vrf, dst_ip, p_len))
            data_tuple_list.append(data_random_tuple(srcMac, dstMac, eg_port))

            target = gc.Target(device_id=0, pipe_id=0xffff)
            logger.info("Inserting table entry with IP address %s, prefix length %d" % (dst_ip, p_len))
            logger.info("With expected dstMac %s, srcMac %s on port %d" % (srcMac, dstMac, eg_port))
            key = alpm_forward_table.make_key([gc.KeyTuple('vrf', vrf),
                                              gc.KeyTuple('hdr.ipv4.dst_addr', dst_ip, prefix_len=p_len)])
            data = alpm_forward_table.make_data([gc.DataTuple('dst_port', eg_port),
                                               gc.DataTuple('srcMac', srcMac),
                                               gc.DataTuple('dstMac', dstMac)],
                                              'SwitchIngress.route')
            alpm_forward_table.entry_add(target, [key], [data])
            key.apply_mask()
            lpm_dict[key] = data

        # check get
        resp  = alpm_forward_table.entry_get(target)
        for data, key in resp:
            assert lpm_dict[key] == data
            lpm_dict.pop(key)
        assert len(lpm_dict) == 0

        test_tuple_list = list(zip(key_tuple_list, data_tuple_list))

        logger.info("Sending packets for the installed entries to verify")
        # send pkt and verify sent
        for key_item, data_item in test_tuple_list:
            pkt = testutils.simple_tcp_packet(ip_dst=key_item.dst_ip)
            exp_pkt = testutils.simple_tcp_packet(eth_dst=data_item.dmac,
                                                  eth_src=data_item.smac,
                                                  ip_dst=key_item.dst_ip)
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Verifying entry for IP address %s, prefix_length %d" % (key_item.dst_ip, key_item.prefix_len))
            logger.info("Expecting packet on port %d", data_item.eg_port)
            testutils.verify_packets(self, exp_pkt, [data_item.eg_port])

        logger.info("All expected packets received")
        logger.info("Deleting %d ALPM entries" % (num_entries))

        # Delete table entries
        for item in key_tuple_list:
            alpm_forward_table.entry_del(
                target,
                [alpm_forward_table.make_key([gc.KeyTuple('vrf', item.vrf),
                                              gc.KeyTuple('hdr.ipv4.dst_addr', item.dst_ip,
                                                          prefix_len=item.prefix_len)])])
