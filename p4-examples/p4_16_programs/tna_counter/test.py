# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

##### Required for Thrift #####
import pd_base_tests

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


def _forward_table_add(table, target, smac, smac_mask, priority, port, c_bytes, c_pkts):
    table.entry_add(
        target,
        [table.make_key(
            [gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
             gc.KeyTuple('$MATCH_PRIORITY', priority)])],
        [table.make_data(
            [gc.DataTuple('port', port),
             gc.DataTuple('$COUNTER_SPEC_BYTES', c_pkts),
             gc.DataTuple('$COUNTER_SPEC_PKTS', c_bytes)],
            'SwitchIngress.hit')])


class DirectCounterTest(BfRuntimeTest):
    """@brief Simple test of the direct counter
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        target = gc.Target(device_id=0, pipe_id=0xffff)

        _forward_table_add(forward_table, target, smac, smac_mask, 0, eg_port, 0, 0)

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 2
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            testutils.verify_packets(self, exp_pkt, [eg_port])

        logger.info("Expecting packet on port %d", eg_port)

        resp = forward_table.entry_get(target,
                                       [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                                                gc.KeyTuple('$MATCH_PRIORITY', 0)])],
                                       {"from_hw": True},
                                       forward_table.make_data(
                                           [gc.DataTuple("$COUNTER_SPEC_BYTES"),
                                            gc.DataTuple("$COUNTER_SPEC_PKTS")],
                                           'SwitchIngress.hit', get=True)
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
            [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                     gc.KeyTuple('$MATCH_PRIORITY', 0)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class DirectCounterExmAddNoCounterFieldsTest(BfRuntimeTest):
    """@brief This test adds entries into exact match action tables with direct 
    attached counters but without initializing the counter spec values. The 
    expected behavior of BRI is to add the entries with default initialized
    counter spec values.

    The default value for counter spec is 0
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_table = bfrt_info.table_get("SwitchIngress.forward_exact")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        logger.info("Testing Exm Table")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        logger.info("Insert table entry")
        forward_table.entry_add(
            target,
            [forward_table.make_key(
                [gc.KeyTuple('hdr.ethernet.src_addr', smac)])],
            [forward_table.make_data(
                [gc.DataTuple('port', eg_port)],
                'SwitchIngress.hit_forward_exact')])

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 2
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)
        testutils.verify_no_other_packets(self)

        resp = forward_table.entry_get(target,
                                       [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])],
                                       {"from_hw": True},
                                       forward_table.make_data(
                                           [gc.DataTuple("$COUNTER_SPEC_BYTES"),
                                            gc.DataTuple("$COUNTER_SPEC_PKTS")],
                                           'SwitchIngress.hit_forward_exact', get=True)
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

        logger.info("Delete table entry")
        forward_table.entry_del(
            target,
            [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class DirectCounterTernaryAddNoCounterFieldsTest(BfRuntimeTest):
    """@brief This test adds entries into ternary match action tables with direct 
    attached counters but without initializing the counter spec values. The 
    expected behavior of BRI is to add the entries with default initialized
    counter spec values.

    The default value for counter spec is 0
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        logger.info("Testing Ternary Table")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        priority = 1
        logger.info("Insert table entry")
        forward_table.entry_add(
            target,
            [forward_table.make_key(
                [gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                 gc.KeyTuple('$MATCH_PRIORITY', priority)])],
            [forward_table.make_data(
                [gc.DataTuple('port', eg_port)],
                'SwitchIngress.hit')])

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 2
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)
        testutils.verify_no_other_packets(self)

        resp = forward_table.entry_get(target,
                                       [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                                                gc.KeyTuple('$MATCH_PRIORITY', priority)])],
                                       {"from_hw": True},
                                       forward_table.make_data(
                                           [gc.DataTuple("$COUNTER_SPEC_BYTES"),
                                            gc.DataTuple("$COUNTER_SPEC_PKTS")],
                                           'SwitchIngress.hit', get=True)
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

        logger.info("Delete table entry")
        forward_table.entry_del(
            target,
            [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                     gc.KeyTuple('$MATCH_PRIORITY', priority)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class DirectCounterIteratorTest(BfRuntimeTest):
    """@brief This test does the following:
    1. Adds 512 counter table entries with random data
    2. Gets all the entries from the counter table
    3. Verifies the read values match what has been programmed
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)
        ig_port = swports[1]
        eg_port = swports[2]
        smac = 'aa:aa:aa:aa:aa:aa'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'
        smac_set = set()
        pkt_init = random.randint(1, 255) 
        bytes_init = random.randint(1, 255)
        num_entries = 100

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        for i in range(num_entries):
            while (smac in smac_set):
                smac = "%02x:%02x:%02x:%02x:%02x:%02x" % tuple(random.randint(1, 255) for x in range(6))
            smac_set.add(smac)
            _forward_table_add(forward_table, target, smac, smac_mask, 0, eg_port, pkt_init, bytes_init)

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 2
        num_bytes = num_pkts * pkt_size
        logger.info("Sending packets on port %d", ig_port)
        for smac in smac_set:
            for j in range(0, num_pkts):
                pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
                testutils.send_packet(self, ig_port, pkt)
                testutils.verify_packet(self, pkt, eg_port)

        resp = forward_table.entry_get(target, None,
                                       {"from_hw": True})

        try:
            logger.info("Reading back all the entries")
            recv_entries = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                recv_smac = key_dict["hdr.ethernet.src_addr"]['value']
                recv_smac_mask = key_dict["hdr.ethernet.src_addr"]['mask']
                if recv_smac not in smac_set:
                    logger.error("Error! Unexpected smac recevied %s", recv_smac)
                    assert (0)
                if (recv_smac_mask != smac_mask):
                    logger.error("Error! Exp smac_mask = %s received smac+mask = %s", smac_mask, recv_smac_mask)
                    assert (0)
                recv_entries += 1

                recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
                recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

                if ((num_pkts + pkt_init) != recv_pkts):
                    logger.error("Error! packets sent = %s received count = %s", str(num_pkts), str(recv_pkts))
                    assert 0

                if ((num_bytes + bytes_init) != recv_bytes):
                    logger.error("Error! bytes sent = %s received count = %s", str(num_bytes), str(recv_bytes))
                    assert 0

            if len(smac_set) != recv_entries:
                logger.error("Error! %d entries were not read back", len(smac_set) - recv_entries)
                assert (0)
            logger.info("All Entries read successfully")
        finally:
            logger.info("Deleting all entries")
            for smac in smac_set:
                forward_table.entry_del(
                    target,
                    [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                             gc.KeyTuple('$MATCH_PRIORITY', 0)])]
                )


class IndirectCounterTest(BfRuntimeTest):
    """@brief Simple test of the indirect counter
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_dst_table = bfrt_info.table_get("SwitchIngress.forward_dst")
        counter_table = bfrt_info.table_get("SwitchIngress.indirect_counter")
        forward_dst_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        target = gc.Target(device_id=0, pipe_id=0xffff)
        # insert entry in MAT and the indirect counter table both
        forward_dst_table.entry_add(
            target,
            [forward_dst_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
            [forward_dst_table.make_data(
                [gc.DataTuple('port', eg_port)],
                'SwitchIngress.hit_dst')])

        counter_table.entry_add(
            target,
            [counter_table.make_key([gc.KeyTuple('$COUNTER_INDEX', eg_port)])],
            [counter_table.make_data([gc.DataTuple('$COUNTER_SPEC_BYTES', 0),
                                      gc.DataTuple('$COUNTER_SPEC_PKTS', 0)])])

        # Default packet size is 100 bytes and model adds 4 bytes of CRC
        pkt_size = 100 + 4
        num_pkts = 6
        num_bytes = num_pkts * pkt_size

        logger.info("Sending packet on port %d", ig_port)
        for i in range(0, num_pkts):
            testutils.send_packet(self, ig_port, pkt)
            testutils.verify_packets(self, exp_pkt, [eg_port])

        logger.info("Expecting packet on port %d", eg_port)

        resp = counter_table.entry_get(target,
                                       [counter_table.make_key([gc.KeyTuple('$COUNTER_INDEX', eg_port)])],
                                       {"from_hw": True},
                                       None)

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
            [forward_dst_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

        logger.info("Clearing table")
        counter_table.entry_del(target)

        num_pkts = 0
        num_bytes = 0
        logger.info("Expecting 0 packets on port %d", eg_port)
        resp = counter_table.entry_get(target,
                                       [counter_table.make_key([gc.KeyTuple('$COUNTER_INDEX', eg_port)])],
                                       {"from_hw": True},
                                       None)

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


class DirectCounterModifyTest(BfRuntimeTest):
    """@brief Test the modification of a single entry of a direct counter table.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        smac = '11:33:55:77:99:00'
        smac_mask = 'ff:ff:ff:ff:ff:ff'
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, eth_src=smac)
        exp_pkt = pkt

        target = gc.Target(device_id=0, pipe_id=0xffff)
        forward_table.entry_add(
            target,
            [forward_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                     gc.KeyTuple('$MATCH_PRIORITY', 0)])],
            [forward_table.make_data(
                [gc.DataTuple('port', eg_port),
                 gc.DataTuple('$COUNTER_SPEC_BYTES', 0),
                 gc.DataTuple('$COUNTER_SPEC_PKTS', 0)],
                'SwitchIngress.hit')]
        )

        logger.info("Expecting packet on port %d", eg_port)

        counter_value_bytes = 4000
        counter_value_packets = 20

        forward_table.entry_mod(
            target,
            [forward_table.make_key(
                [gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                 gc.KeyTuple('$MATCH_PRIORITY', 0)])],
            [forward_table.make_data(
                [gc.DataTuple('$COUNTER_SPEC_BYTES', counter_value_bytes),
                 gc.DataTuple('$COUNTER_SPEC_PKTS', counter_value_packets)],
                'SwitchIngress.hit')]
        )

        resp = forward_table.entry_get(target,
                                       [forward_table.make_key(
                                           [gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                                            gc.KeyTuple('$MATCH_PRIORITY', 0)])],
                                       {"from_hw": False},
                                       forward_table.make_data(
                                           [gc.DataTuple("$COUNTER_SPEC_BYTES"),
                                            gc.DataTuple("$COUNTER_SPEC_PKTS")],
                                           'SwitchIngress.hit', get=True)
                                       )

        # parse resp to get the counter
        data_dict = next(resp)[0].to_dict()
        recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
        recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

        if (counter_value_packets != recv_pkts):
            logger.error("Error! packets sent = %s received count = %s", str(counter_value_packets), str(recv_pkts))
            assert 0

        if (counter_value_bytes != recv_bytes):
            logger.error("Error! bytes sent = %s received count = %s", str(counter_value_bytes), str(recv_bytes))
            assert 0

        forward_table.entry_del(
            target,
            [forward_table.make_key(
                [gc.KeyTuple('hdr.ethernet.src_addr', smac, smac_mask),
                 gc.KeyTuple('$MATCH_PRIORITY', 0)])])


class IndirectCounterIteratorTest(BfRuntimeTest):
    """@brief This test does the following:
    1. Adds 512 counter table entries with random data
    2. Gets all the entries from the counter table
    3. Verifies the read values match what has been programmed
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_counter"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        counter_table = bfrt_info.table_get("SwitchIngress.indirect_counter")
        target = gc.Target(device_id=0, pipe_id=0xffff)

        num_entries = 512
        cntr_packets = [random.randint(1, 1000) for x in range(num_entries)]
        cntr_bytes = [cntr_packets[x] * 64 for x in range(num_entries)]

        cnt_keys = []
        logger.info("Inserting %d entries to the counter table", num_entries)
        # insert entry in MAT and the indirect counter table both
        for x in range(num_entries):
            cnt_key = counter_table.make_key([gc.KeyTuple('$COUNTER_INDEX', x)])
            cnt_keys.append(cnt_key)
            counter_table.entry_add(
                target,
                [cnt_key],
                [counter_table.make_data(
                    [gc.DataTuple('$COUNTER_SPEC_BYTES', cntr_bytes[x]),
                     gc.DataTuple('$COUNTER_SPEC_PKTS', cntr_packets[x])]
                )]
            )

        logger.info("Getting all %d entries of the counter table", num_entries)

        resp = counter_table.entry_get(target,
                                       key_list=cnt_keys,
                                       flags={"from_hw": False})

        logger.info("Verifying all %d entries of the counter table", num_entries)

        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
            recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

            if (cntr_packets[i] != recv_pkts):
                logger.error("Error! packets sent = %s received count = %s", str(cntr_packets[i]), str(recv_pkts))
                assert 0

            if (cntr_bytes[i] != recv_bytes):
                logger.error("Error! bytes sent = %s received count = %s", str(cntr_bytes[x]), str(recv_bytes))
                assert 0

            assert key_dict["$COUNTER_INDEX"]['value'] == i
            logger.info("Counter id %d matched", i)
            i += 1
        # Clear the counter table and read them back to verify that they have been cleared
        counter_table.entry_del(target)
        resp = counter_table.entry_get(target,
                                       key_list=cnt_keys,
                                       flags={"from_hw": False})
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            assert data_dict["$COUNTER_SPEC_PKTS"] == data_dict["$COUNTER_SPEC_BYTES"] == 0


class IndirectCounterLatencyTest(BfRuntimeTest):
    """@brief Verify the latency of the Thrift API.
    """

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
        p4_name = "tna_counter"
        self.fixedObject = self.FixedInterface([p4_name])
        # Initialize the fixed API thrift interface through setUp
        self.fixedObject.setUp()
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def readEntryHelper(self, target, eg_port, num_read, from_hw, init_pkts_val, init_bytes_val):
        from timeit import default_timer as timer

        avg_diff_time = 0
        for i in range(0, num_read):
            local_start_time = timer()
            resp = self.counter_table.entry_get(target,
                                                [self.counter_table.make_key(
                                                    [gc.KeyTuple('$COUNTER_INDEX', eg_port)])],
                                                {"from_hw": from_hw})
            data_dict = next(resp)[0].to_dict()
            recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
            recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

            local_end_time = timer()
            local_diff_time = (local_end_time - local_start_time) * 1000  # convert to milliseconds
            logger.info("Time to read hw counter is %s", str(local_diff_time))
            avg_diff_time = avg_diff_time + local_diff_time

            if (init_pkts_val != recv_pkts):
                logger.error("Error! inited pkts val = %s received val = %s", str(init_pkts_val), str(recv_pkts))
                assert 0

            if (init_bytes_val != recv_bytes):
                logger.error("Error! inited bytes val = %s received val = %s", str(init_bytes_val), str(recv_bytes))
                assert 0
        avg_diff_time = avg_diff_time / num_read
        return avg_diff_time

    def runTest(self):
        if testutils.test_param_get('target') != "hw":
            logger.info("This test is meant to be run only on hardware. Thus simply returning")
            return

        eg_port = 50
        init_bytes_val = 123456
        init_pkts_val = 654321
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_counter")

        self.counter_table = bfrt_info.table_get("SwitchIngress.indirect_counter")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # insert entry in the indirect counter table
        self.counter_table.entry_add(
            target,
            [self.counter_table.make_key([gc.KeyTuple('$COUNTER_INDEX', eg_port)])],
            [self.counter_table.make_data(
                [gc.DataTuple('$COUNTER_SPEC_BYTES', init_bytes_val),
                 gc.DataTuple('$COUNTER_SPEC_PKTS', init_pkts_val)]
            )]
        )

        num_read = 10
        logger.info("Reading Indirect Counter Table Entry for %d times", num_read)
        self.readEntryHelper(target, eg_port, num_read, True, init_pkts_val, init_bytes_val)

        # Change the lrt dr timeout to 10 milliseconds
        self.fixedObject.devport_mgr.devport_mgr_lrt_dr_timeout_set(target.device_id_, 10)
        hw_timeout = self.fixedObject.devport_mgr.devport_mgr_lrt_dr_timeout_get(target.device_id_)
        if hw_timeout != 10:
            logger.info("Read timeout value %d doesn't match up with the expected %d", hw_timeout, 10)
            assert (0)
        time_req = self.readEntryHelper(target, eg_port, 1, True, init_pkts_val, init_bytes_val)
        logger.info("Reading Indirect Counter Table Entry for %d times", num_read)
        time_req = self.readEntryHelper(target, eg_port, num_read, True, init_pkts_val, init_bytes_val)
        assert (time_req <= 15)

        # Change the lrt dr timeout back to the default value of 50 milliseconds
        self.fixedObject.devport_mgr.devport_mgr_lrt_dr_timeout_set(target.device_id_, 50)
        hw_timeout = self.fixedObject.devport_mgr.devport_mgr_lrt_dr_timeout_get(target.device_id_)
        if hw_timeout != 50:
            logger.info("Read timeout value %d doesn't match up with the expected %d", hw_timeout, 50)
            assert (0)
        time_req = self.readEntryHelper(target, eg_port, 1, True, init_pkts_val, init_bytes_val)
        logger.info("Reading Indirect Counter Table Entry for %d times", num_read)
        time_req = self.readEntryHelper(target, eg_port, num_read, True, init_pkts_val, init_bytes_val)
        assert (time_req > 15)
        assert (time_req <= 55)
