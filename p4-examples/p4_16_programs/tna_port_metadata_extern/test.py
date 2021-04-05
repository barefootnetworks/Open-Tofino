# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
from ptf.thriftutils import *
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
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


class PortMetadataTest(BfRuntimeTest):
    def __init__(self):
        super(PortMetadataTest, self).__init__()
        self.num_entries = 5
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        self.igr_to_egr_port_map = {}
        igr_port_list = random.sample(swports, self.num_entries)
        egr_port_list = random.sample(swports, self.num_entries)

        for x in range(self.num_entries):
            self.igr_to_egr_port_map[igr_port_list[x]] = egr_port_list[x]

        self.target = client.Target(device_id=0, pipe_id=0xffff)

        self.phase0_data_map = {}
        # Initialize the phase0 data map
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            self.phase0_data_map[igr_port] = 0

    def setUp(self):
        client_id = 0
        p4_name = "tna_port_metadata_extern"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get("tna_port_metadata_extern")
        self.port_metadata_table = self.bfrt_info.table_get("SwitchIngressParser.$PORT_METADATA")
        self.port_md_exm_match_table = self.bfrt_info.table_get("SwitchIngress.port_md_exm_match")

    def insertEntries(self):
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value

            # For each igr port add a entry in the port_metadata (phase0) table
            # Form data to be programmed in the phase0 table for this ingress port
            phase0data = 0
            field1 = 0
            field2 = 0
            field3 = 0
            field4 = 0
            while True:
                field1 = random.randint(1, 0xffff)  # 16 bit
                field2 = random.randint(1, 0xffffff)  # 24 bits
                field3 = random.randint(1, 0xffff)  # 16 bits
                field4 = random.randint(1, 0xff)  # 8 bits

                phase0data = (field1 << 48) | (field2 << 24) | (field3 << 8) | field4

                if self.phase0_data_map[igr_port] != phase0data:
                    self.phase0_data_map[igr_port] = phase0data
                    break

            self.port_metadata_table.entry_add(
                self.target,
                [self.port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                [self.port_metadata_table.make_data([client.DataTuple('field1', field1),
                                                     client.DataTuple('field2', field2),
                                                     client.DataTuple('field3', field3),
                                                     client.DataTuple('field4', field4)])])

            # Add the entry for the igr port in the exact match table
            self.port_md_exm_match_table.entry_add(
                self.target,
                [self.port_md_exm_match_table.make_key([client.KeyTuple('ig_md.port_md.field1', field1),
                                                        client.KeyTuple('ig_md.port_md.field2', field2),
                                                        client.KeyTuple('ig_md.port_md.field3', field3),
                                                        client.KeyTuple('ig_md.port_md.field4', field4)])],
                [self.port_md_exm_match_table.make_data([client.DataTuple('port', egr_port)],
                                                        'SwitchIngress.hit',)]
            )
            logger.info("Added entry : ig_port: %d  -> eg_port: %d", igr_port, egr_port)

    def modifyEntries(self):
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value

            # For each igr port add a entry in the port_metadata (phase0) table
            # Form data to be programmed in the phase0 table for this ingress port
            phase0data = 0
            field1 = 0
            field2 = 0
            field3 = 0
            field4 = 0
            old_phase0data = 0
            while True:
                field1 = random.randint(1, 0xffff)  # 16 bit
                field2 = random.randint(1, 0xffffff)  # 24 bits
                field3 = random.randint(1, 0xffff)  # 16 bits
                field4 = random.randint(1, 0xff)  # 8 bits

                phase0data = (field1 << 48) | (field2 << 24) | (field3 << 8) | field4
                if self.phase0_data_map[igr_port] != phase0data:
                    old_phase0data = self.phase0_data_map[igr_port]
                    self.phase0_data_map[igr_port] = phase0data
                    break

            self.port_metadata_table.entry_mod(
                self.target,
                [self.port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                [self.port_metadata_table.make_data([client.DataTuple('field1', field1),
                                                     client.DataTuple('field2', field2),
                                                     client.DataTuple('field3', field3),
                                                     client.DataTuple('field4', field4)])])

            # To make sure that the phase0 table modify has taken effect delete the old entries from the exact match table
            self.port_md_exm_match_table.entry_del(
                self.target,
                [self.port_md_exm_match_table.make_key(
                    [client.KeyTuple('ig_md.port_md.field1', (old_phase0data >> 48) & 0xffff),
                     client.KeyTuple('ig_md.port_md.field2', (old_phase0data >> 24) & 0xffffff),
                     client.KeyTuple('ig_md.port_md.field3', (old_phase0data >> 8) & 0xffff),
                     client.KeyTuple('ig_md.port_md.field4', old_phase0data & 0xff)])])

            # Add the new entry for the igr port in the exact match table
            self.port_md_exm_match_table.entry_add(
                self.target,
                [self.port_md_exm_match_table.make_key([client.KeyTuple('ig_md.port_md.field1', field1),
                                                        client.KeyTuple('ig_md.port_md.field2', field2),
                                                        client.KeyTuple('ig_md.port_md.field3', field3),
                                                        client.KeyTuple('ig_md.port_md.field4', field4)])],
                [self.port_md_exm_match_table.make_data([client.DataTuple('port', egr_port)],
                                                        'SwitchIngress.hit')]
            )

    def verifyPacketHits(self):
        pkt = testutils.simple_tcp_packet()
        exp_pkt = pkt
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value

            logger.info("Sending packet on port %d", igr_port)
            testutils.send_packet(self, igr_port, pkt)

            logger.info("Expecting packet on port %d", egr_port)
            testutils.verify_packet(self, exp_pkt, egr_port)

            logger.info("Packet received on port %d as expected", egr_port)

    def verifyPacketMisses(self):
        pkt = testutils.simple_tcp_packet()
        exp_pkt = pkt
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value
            logger.info("Sending packet on port %d", igr_port)
            testutils.send_packet(self, igr_port, pkt)

            logger.info("Expecting packet to be dropped.")
        testutils.verify_no_other_packets(self, timeout=2)

    def deleteAllEntries(self):
        self.port_metadata_table.entry_del(self.target)
        self.port_md_exm_match_table.entry_del(self.target)

    def deletePortMetadataEntries(self):
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value

            self.port_metadata_table.entry_del(
                self.target,
                [self.port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])])

    def deleteExactMatchTableEntries(self):
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value

            old_phase0data = self.phase0_data_map[igr_port]

            self.port_md_exm_match_table.entry_del(
                self.target,
                [self.port_md_exm_match_table.make_key([client.KeyTuple('ig_md.port_md.field1', (old_phase0data >> 48) & 0xffff),
                                                        client.KeyTuple('ig_md.port_md.field2',
                                                                        (old_phase0data >> 24) & 0xffffff),
                                                        client.KeyTuple('ig_md.port_md.field3',
                                                                        (old_phase0data >> 8) & 0xffff),
                                                        client.KeyTuple('ig_md.port_md.field4', old_phase0data & 0xff)])])

    def runTest(self):

        try:
            # Insert entries in the phase0 and the exact match table
            logger.info("Inserting %d entries in the phase0 table", self.num_entries)
            self.insertEntries()

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to hit the phase0 table", self.num_entries)
            self.verifyPacketHits()

            # Modify entries in the phase0 table
            logger.info("")
            logger.info("Modifying %d entries in the phase0 table", self.num_entries)
            self.modifyEntries()

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to hit the phase0 table", self.num_entries)
            self.verifyPacketHits()

            # Delete entries from the phase0 table
            logger.info("")
            logger.info("Deleting %d entries from the phase0 table", self.num_entries)
            self.deletePortMetadataEntries()

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to miss the phase0 table and thus get dropped", self.num_entries)
            self.verifyPacketMisses()

            # Cleanup Entries From the exact match table
            self.deleteExactMatchTableEntries()

            # Again Insert entries in the phase0 and the exact match table
            logger.info("")
            logger.info("Inserting %d entries again in the phase0 table", self.num_entries)
            self.insertEntries()

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to hit the phase0 table", self.num_entries)
            self.verifyPacketHits()

            logger.info("")
            logger.info("PortMetadata test passed!!! Now calm down and cleanup all the entries")

        finally:
            # Final cleanup
            logger.info("")
            logger.info("Cleaning up all the entries")
            self.deleteAllEntries()


class PortMetadataGetTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_port_metadata_extern"
        BfRuntimeTest.setUp(self, client_id, p4_name)

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

    def runTest(self):
        # Get bfrt_info and set it as part of the test
        self.bfrt_info = self.interface.bfrt_info_get("tna_port_metadata_extern")
        port_metadata_table = self.bfrt_info.table_get("SwitchIngressParser.$PORT_METADATA")
        port_md_exm_match_table = self.bfrt_info.table_get("SwitchIngress.port_md_exm_match")

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Insert 100 entries
        phase0_igr_ports = {}
        try:
            logger.info("Inserting entries in the phase0 table")
            for i in range(num_pipes):
                for j in range(72):
                    igr_pipe = i
                    igr_local_port = j
                    igr_port = (igr_pipe << 7) | (igr_local_port)

                    field1 = random.randint(1, 0xffff)  # 16 bit
                    field2 = random.randint(1, 0xffffff)  # 24 bits
                    field3 = random.randint(1, 0xffff)  # 16 bits
                    field4 = random.randint(1, 0xff)  # 8 bits

                    phase0data = (field1 << 48) | (field2 << 24) | (field3 << 8) | field4

                    phase0_igr_ports[igr_port] = phase0data

                    port_metadata_table.entry_add(
                        target,
                        [port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                        [port_metadata_table.make_data([client.DataTuple('field1', field1),
                                                        client.DataTuple('field2', field2),
                                                        client.DataTuple('field3', field3),
                                                        client.DataTuple('field4', field4)])])

            # Read back the entries
            logger.info("Reading back entries")
            for key, value in list(phase0_igr_ports.items()):
                phase0data = value
                val1 = (phase0data >> 48) & 0xffff
                val2 = (phase0data >> 24) & 0xffffff
                val3 = (phase0data >> 8) & 0xffff
                val4 = (phase0data) & 0xff
                resp = port_metadata_table.entry_get(
                    target,
                    [port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', key)])],
                    {"from_hw": True})
                fields = next(resp)[0].to_dict()
                assert fields['field1'] == val1
                assert fields['field2'] == val2
                assert fields['field3'] == val3
                assert fields['field4'] == val4

        finally:
            logger.info("Cleaning up entries")
            for key, value in list(phase0_igr_ports.items()):
                port_metadata_table.entry_del(
                    target,
                    [port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', key)])])


class PortMetadataIteratorGetTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_port_metadata_extern"
        BfRuntimeTest.setUp(self, client_id, p4_name)

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

    def runTest(self):
        # Get bfrt_info and set it as part of the test
        self.bfrt_info = self.interface.bfrt_info_get("tna_port_metadata_extern")
        port_metadata_table = self.bfrt_info.table_get("SwitchIngressParser.$PORT_METADATA")
        port_md_exm_match_table = self.bfrt_info.table_get("SwitchIngress.port_md_exm_match")

        target = client.Target(device_id=0, pipe_id=0xffff)

        phase0_igr_ports = {}
        try:
            logger.info("Inserting entries in the phase0 table")
            for i in range(num_pipes):
                for j in range(72):
                    igr_pipe = i
                    igr_local_port = j
                    igr_port = (igr_pipe << 7) | (igr_local_port)

                    field1 = random.randint(1, 0xffff)  # 16 bit
                    field2 = random.randint(1, 0xffffff)  # 24 bits
                    field3 = random.randint(1, 0xffff)  # 16 bits
                    field4 = random.randint(1, 0xff)  # 8 bits

                    phase0data = (field1 << 48) | (field2 << 24) | (field3 << 8) | field4

                    phase0_igr_ports[igr_port] = phase0data

                    port_metadata_table.entry_add(
                        target,
                        [port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                        [port_metadata_table.make_data([client.DataTuple('field1', field1),
                                                        client.DataTuple('field2', field2),
                                                        client.DataTuple('field3', field3),
                                                        client.DataTuple('field4', field4)])])

            # Read back all the entries

            logger.info("Reading back all entries")

            resp = port_metadata_table.entry_get(
                target,
                None,
                {"from_hw": False})

            logger.info("Verifying all entries")
            i = 0
            for data, key in resp:
                key_dict = key.to_dict()
                data_dict = data.to_dict()
                prog_data = phase0_igr_ports[key_dict["ig_intr_md.ingress_port"]["value"]]

                val1 = (prog_data >> 48) & 0xffff
                val2 = (prog_data >> 24) & 0xffffff
                val3 = (prog_data >> 8) & 0xffff
                val4 = (prog_data) & 0xff

                assert data_dict['field1'] == val1
                assert data_dict['field2'] == val2
                assert data_dict['field3'] == val3
                assert data_dict['field4'] == val4
                i += 1
            assert i == num_pipes * 72

        finally:
            logger.info("Cleaning up entries")
            port_metadata_table.entry_del(target)
