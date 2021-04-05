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


def make_phase0_data(field1, field2, field3, field4):
    """@brief Pack all fields into one pahse0_data. For tofino 2, it is
    left shifted 64 more because the field is a 128 bit value
    """
    phase0data = (field1 << 48) | (field2 << 24) | (field3 << 8) | field4
    if testutils.test_param_get("arch") == "tofino":
        pass
    elif testutils.test_param_get("arch") == "tofino2":
        phase0data = phase0data << 64
    return phase0data


class PortMetadataTest(BfRuntimeTest):
    """@brief This test does the following
    1. Insert table entries in port_metadata table and exact match table
    2. Sends packets to verify all the entries.
    3. Modifies the entries and ensures packet hits again.
    4. Delete the entries and verifies packet misses for all entries
    5. Insert entries and verify packet hits again.
    6. Cleanup
    """

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
        p4_name = "tna_port_metadata"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_name)
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
                field1 = random.randint(256, 0xffff)  # 16 bit
                field2 = random.randint(1, 0xffffff)  # 24 bits
                field3 = random.randint(1, 0xffff)  # 16 bits
                field4 = random.randint(1, 0xff)  # 8 bits

                phase0data = make_phase0_data(field1, field2, field3, field4)

                if self.phase0_data_map[igr_port] != phase0data:
                    self.phase0_data_map[igr_port] = phase0data
                    break

            self.port_metadata_table.entry_add(
                self.target,
                [self.port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                [self.port_metadata_table.make_data([client.DataTuple('$DEFAULT_FIELD', phase0data)],
                                                    None)])

            # Add the entry for the igr port in the exact match table
            self.port_md_exm_match_table.entry_add(
                self.target,
                [self.port_md_exm_match_table.make_key(
                    [client.KeyTuple('ig_md.port_md.field1', field1),
                     client.KeyTuple('ig_md.port_md.field2', field2),
                     client.KeyTuple('ig_md.port_md.field3', field3),
                     client.KeyTuple('ig_md.port_md.field4', field4)])],
                [self.port_md_exm_match_table.make_data(
                    [client.DataTuple('port', egr_port)],
                    'SwitchIngress.hit')]
            )

    def modifyEntries(self):
        # To make sure that the phase0 table modify has taken effect delete
        # the old entries from the exact match table
        self.port_md_exm_match_table.entry_del(self.target)
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
                field1 = random.randint(256, 0xffff)  # 16 bit
                field2 = random.randint(1, 0xffffff)  # 24 bits
                field3 = random.randint(1, 0xffff)  # 16 bits
                field4 = random.randint(1, 0xff)  # 8 bits

                phase0data = make_phase0_data(field1, field2, field3, field4)
                if self.phase0_data_map[igr_port] != phase0data:
                    old_phase0data = self.phase0_data_map[igr_port]
                    self.phase0_data_map[igr_port] = phase0data
                    break

            self.port_metadata_table.entry_mod(
                self.target,
                [self.port_metadata_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                [self.port_metadata_table.make_data([client.DataTuple('$DEFAULT_FIELD', phase0data)])])

            # Add the new entry for the igr port in the exact match table
            self.port_md_exm_match_table.entry_add(
                self.target,
                [self.port_md_exm_match_table.make_key(
                    [client.KeyTuple('ig_md.port_md.field1', field1),
                     client.KeyTuple('ig_md.port_md.field2', field2),
                     client.KeyTuple('ig_md.port_md.field3', field3),
                     client.KeyTuple('ig_md.port_md.field4', field4)])],
                [self.port_md_exm_match_table.make_data(
                    [client.DataTuple('port', egr_port)],
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
            testutils.verify_packets(self, exp_pkt, [egr_port])

            logger.info("Packet received on port %d as expected", egr_port)

    def verifyPacketMisses(self):
        pkt = testutils.simple_tcp_packet()
        exp_pkt = pkt
        for key, value in list(self.igr_to_egr_port_map.items()):
            igr_port = key
            egr_port = value
            try:
                logger.info("Sending packet on port %d", igr_port)
                testutils.send_packet(self, igr_port, pkt)

                logger.info("Expecting packet on port %d", egr_port)
                testutils.verify_packets(self, exp_pkt, [egr_port])

                assert(0)
            except:
                logger.info("Packet not received on port %d which is expected", egr_port)

    def deleteAllEntries(self):
        self.port_metadata_table.entry_del(self.target)
        self.port_md_exm_match_table.entry_del(self.target)

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
            self.port_metadata_table.entry_del(self.target)

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to miss the phase0 table and thus get dropped", self.num_entries)
            self.verifyPacketMisses()

            # Cleanup Entries From the exact match table
            self.port_md_exm_match_table.entry_del(self.target)

            # Again Insert entries in the phase0 and the exact match table
            logger.info("")
            logger.info("Inserting %d entries again in the phase0 table", self.num_entries)
            self.insertEntries()

            # Verify packets
            logger.info("")
            logger.info("Verifying %d packets to hit the phase0 table", self.num_entries)
            self.verifyPacketHits()

            logger.info("")
            logger.info("PortMetadata test passed!!! Now cleanup all the entries")

        finally:
            # Final cleanup
            logger.info("")
            logger.info("Cleaning up all the entries")
            self.deleteAllEntries()


class PortMetadataGetTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_port_metadata"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_name)
        self.port_metadata_table = self.bfrt_info.table_get("SwitchIngressParser.$PORT_METADATA")
        self.port_md_exm_match_table = self.bfrt_info.table_get("SwitchIngress.port_md_exm_match")

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

    def runTest(self):
        # Get bfrt_info and set it as part of the test

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Insert 100 entries
        num_entries = 100
        phase0_igr_ports = {}
        try:
            logger.info("Inserting %d entries in the phase0 table", num_entries)
            for i in range(num_entries):
                igr_port = 0
                phase0data = 0
                while True:
                    igr_pipe = random.randint(0, 3)
                    igr_local_port = random.randint(0, 63)
                    igr_port = (igr_pipe << 7) | (igr_local_port)

                    if igr_port not in phase0_igr_ports:
                        field1 = random.randint(1, 0xffff)  # 16 bit
                        field2 = random.randint(1, 0xffffff)  # 24 bits
                        field3 = random.randint(1, 0xffff)  # 16 bits
                        field4 = random.randint(1, 0xff)  # 8 bits

                        phase0data = make_phase0_data(field1, field2, field3, field4)

                        phase0_igr_ports[igr_port] = phase0data
                        break

                self.port_metadata_table.entry_add(
                    target,
                    [self.port_metadata_table.make_key(
                        [client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                    [self.port_metadata_table.make_data(
                        [client.DataTuple('$DEFAULT_FIELD', phase0data)])])

            # Read back the entries
            logger.info("Reading back %d entries", num_entries)
            for key, value in list(phase0_igr_ports.items()):
                igr_port = key
                phase0data = value
                resp = self.port_metadata_table.entry_get(
                    target,
                    [self.port_metadata_table.make_key(
                        [client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                    {"from_hw": True})
                fields = next(resp)[0].to_dict()
                logger.info("Verifying %d entry for igr port %d", i, igr_port)
                recv_data = fields["$DEFAULT_FIELD"]
                if recv_data != phase0data:
                    logger.info("Exp data : %s : Rcv data : %s", phase0data, recv_data)
                    assert(0)
        finally:
            logger.info("Cleaning up entries")
            self.port_metadata_table.entry_del(target)


class PortMetadataIteratorGetTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_port_metadata"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_name)
        self.port_metadata_table = self.bfrt_info.table_get("SwitchIngressParser.$PORT_METADATA")
        self.port_md_exm_match_table = self.bfrt_info.table_get("SwitchIngress.port_md_exm_match")

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

    def runTest(self):

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Insert 100 entries
        num_entries = 100
        phase0_igr_ports = {}
        phase0_igr_ports_list = [0 for x in range(num_entries)]
        try:
            logger.info("Inserting %d entries in the phase0 table", num_entries)
            for i in range(num_entries):
                igr_port = 0
                phase0data = 0
                while True:
                    igr_pipe = random.randint(0, 3)
                    igr_local_port = random.randint(0, 63)
                    igr_port = (igr_pipe << 7) | (igr_local_port)

                    if igr_port not in phase0_igr_ports:
                        field1 = random.randint(1, 0xffff)  # 16 bit
                        field2 = random.randint(1, 0xffffff)  # 24 bits
                        field3 = random.randint(1, 0xffff)  # 16 bits
                        field4 = random.randint(1, 0xff)  # 8 bits

                        phase0data = make_phase0_data(field1, field2, field3, field4)

                        phase0_igr_ports[igr_port] = phase0data
                        phase0_igr_ports_list[i] = igr_port
                        break

                self.port_metadata_table.entry_add(
                    target,
                    [self.port_metadata_table.make_key(
                        [client.KeyTuple('ig_intr_md.ingress_port', igr_port)])],
                    [self.port_metadata_table.make_data(
                        [client.DataTuple('$DEFAULT_FIELD', phase0data)])])

            # Read back all the entries
            # when reading back all the entries for phase0 tables, the pipe mgr
            # is going to return us the entries sequentially for the igr ports
            # This is because for phase0 tables, the entry handles are equal
            # to the igr_port+1. Thus even if we added the entry for port 2 before
            # addding the entry for port 1, the first entry handle returned by
            # pipe mgr is going to be 2 instead of 3. Thus we need to sort the
            # igr port list before we verify
            phase0_igr_ports_list.sort()

            logger.info("Ingress port are")
            print(phase0_igr_ports_list)

            logger.info("Reading back all %d entries", num_entries)

            resp = self.port_metadata_table.entry_get(target,
                                                      None,
                                                      {"from_hw": False})

            logger.info("Verifying all %d entries", num_entries)
            key_dict = {}
            i = 0
            for data, key in resp:
                data_dict = data.to_dict()
                recv_data = data_dict["$DEFAULT_FIELD"]

                igr_port = phase0_igr_ports_list[i]
                prog_data = phase0_igr_ports[igr_port]

                logger.info("Verifying %d entry for igr port %d", i, igr_port)
                if recv_data != prog_data:
                    logger.info("Exp data : %s : Rcv data : %s", prog_data, recv_data)
                    assert(0)
                i += 1
            # Verifying clean all
            self.port_metadata_table.entry_del(target)
            resp = self.port_metadata_table.entry_get(target,
                                                      None,
                                                      {"from_hw": False})

            logger.info("Verifying if all %d entries have been deleted", num_entries)
            for data, key in resp:
                assert 0, "Expecting no entries to be present in the table"
            logger.info("Everything has been cleaned")

        except:
            logger.info("PortMetadata Iterator Get Test Failed")
            assert(0)

        finally:
            logger.info("Cleaning up entries")
            self.port_metadata_table.entry_del(target)
