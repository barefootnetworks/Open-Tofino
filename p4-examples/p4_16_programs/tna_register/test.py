# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import random

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

num_pipes = int(testutils.test_param_get('num_pipes'))
pipes = list(range(num_pipes))

swports = []
for device, port, ifname in config["interfaces"]:
    pipe = port >> 7
    if pipe in pipes:
        swports.append(port)
swports.sort()


def VerifyReadRegisters(self, register_name_lo, register_name_hi, register_value_lo, register_value_hi, data_dict):
    # since the table is symmetric and exists only in stage 0, we know that the response data is going to have
    # 8 data fields (2 (hi and lo) * 4 (num pipes) * 1 (num_stages)). bfrt_server returns all (4) register values
    # corresponding to one field id followed by all (4) register values corresponding to the other field id

    for i in range(num_pipes):
        value_lo = data_dict[register_name_lo][i]
        value_hi = data_dict[register_name_hi][i]
        assert value_lo == register_value_lo[i], "Register field lo didn't match with the read value." \
                                                 " Lo Expected Value (%s) : Read value (%s)" % (
                                                 str(register_value_lo[i]), str(value_lo))
        assert value_hi == register_value_hi[i], "Register field hi didn't match with the read value." \
                                                 " Hi Expected Value (%s) : Read value (%s)" % (
                                                 str(register_value_hi[i]), str(value_hi))


class RegisterTest(BfRuntimeTest):
    """@brief This test adds a value in an indirect register table and then
        reads the value back using the single-entry sync-from-hw functionality.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_register"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used for RegisterTest is %d", seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")

        register_idx = random.randint(0, 500)
        register_value_hi = random.randint(0, 1000)
        register_value_lo = random.randint(0, 1000)
        logger.info("Register value hi %s", str(register_value_hi))
        logger.info("Register value lo %s", str(register_value_lo))
        register_value_hi_arr = {}
        register_value_lo_arr = {}
        num_pipes = int(testutils.test_param_get('num_pipes'))

        for i in range(num_pipes):
            register_value_hi_arr[i] = register_value_hi
            register_value_lo_arr[i] = register_value_lo

        target = gc.Target(device_id=0, pipe_id=0xffff)
        register_table = bfrt_info.table_get("SwitchIngress.test_reg")
        register_table.entry_add(
            target,
            [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            [register_table.make_data(
                [gc.DataTuple('SwitchIngress.test_reg.first', register_value_lo),
                 gc.DataTuple('SwitchIngress.test_reg.second', register_value_hi)])])

        resp = register_table.entry_get(
            target,
            [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            {"from_hw": True})

        data, _ = next(resp)
        data_dict = data.to_dict()
        VerifyReadRegisters(self, "SwitchIngress.test_reg.first", "SwitchIngress.test_reg.second",
                            register_value_lo_arr, register_value_hi_arr, data_dict)


class IndirectRegisterSyncTest(BfRuntimeTest):
    """@brief This test adds a value in a register table and then checks the sync from_hw functionality.
        It also clears the table and checks whether the entries added were removed or not.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_register"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(0, 10000000)
        logger.info("Seed used for Indirect Register Sync Test is %d", seed)
        random.seed(seed)
        ig_port = swports[1]
        reg_index = random.randint(0, 1023)
        dmac = '00:11:22:33:44:55'

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")

        register_table = bfrt_info.table_get("SwitchIngress.test_reg")
        reg_match_table = bfrt_info.table_get("SwitchIngress.reg_match")
        reg_match_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        register_value_hi = random.randint(0, 12345678)
        register_value_lo = random.randint(0, 23456789)
        register_value_hi_arr = {}
        register_value_lo_arr = {}
        num_pipes = int(testutils.test_param_get('num_pipes'))
        for i in range(num_pipes):
            register_value_hi_arr[i] = register_value_hi
            register_value_lo_arr[i] = register_value_lo

        pkt = testutils.simple_tcp_packet(eth_dst=dmac)
        exp_pkt = pkt

        # Program the same register values in all the 4 pipes
        logger.info("Inserting entry in test_reg stful table with hi = %s lo = %s", str(register_value_hi),
                    str(register_value_lo))
        target = gc.Target(device_id=0, pipe_id=0xffff)
        try:
            # insert entry in MAT and the indirect register table both
            register_table.entry_add(
                target,
                [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', reg_index)])],
                [register_table.make_data(
                    [gc.DataTuple('SwitchIngress.test_reg.first', register_value_lo),
                     gc.DataTuple('SwitchIngress.test_reg.second', register_value_hi)])])
            reg_match_table.entry_add(
                target,
                [reg_match_table.make_key([gc.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                [reg_match_table.make_data(
                    [gc.DataTuple('idx', reg_index)],
                    'SwitchIngress.register_action')])

            # Apply table operations to sync the registers on the indirect register table
            logger.info("Syncing indirect stful registers")
            register_table.operations_execute(target, 'Sync')

            # Get from sw and check its value.
            logger.info("Reading back the indirect stful registers for the programmed match entry")
            resp = register_table.entry_get(
                target,
                [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', reg_index)])],
                {"from_hw": False})
            # The values read back should match the initialized values
            logger.info("Verifying read back indirect register values")
            data, _ = next(resp)
            data_dict = data.to_dict()
            VerifyReadRegisters(self, "SwitchIngress.test_reg.first", "SwitchIngress.test_reg.second",
                                register_value_lo_arr, register_value_hi_arr, data_dict)
            logger.info("Indirect Register values matched before seding packets")

            num_pkts = random.randint(1, 10)
            logger.info("Sending %d packets on port %d", num_pkts, ig_port)
            for i in range(num_pkts):
                testutils.send_packet(self, ig_port, pkt)
                testutils.verify_packet(self, pkt, ig_port)

            # Apply table operations to sync the registers on the indirect register table
            logger.info("Syncing indirect stful registers")
            register_table.operations_execute(target, 'Sync')

            # Get from sw and check its value. They should be the correct updated values now
            logger.info("Reading back the indirect stful registers for the programmed match entry")
            resp = register_table.entry_get(
                target,
                [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', reg_index)])],
                {"from_hw": False})
            # Since the packet is sent on a single pipe only the register entry
            # from that pipe will be updated.
            ingress_pipe = ig_port >> 7
            register_value_lo_arr[ingress_pipe] = register_value_lo + (1 * num_pkts)
            register_value_hi_arr[ingress_pipe] = register_value_hi + (100 * num_pkts)
            logger.info("Verifying read back indirect register values")
            data, _ = next(resp)
            data_dict = data.to_dict()
            VerifyReadRegisters(self, "SwitchIngress.test_reg.first", "SwitchIngress.test_reg.second",
                                register_value_lo_arr, register_value_hi_arr, data_dict)
            logger.info("Indirect Register values matched after seding packets")

        finally:
            logger.info("Deleting entries from the match action and the indirect stful tables")
            reg_match_table.entry_del(target)
            register_table.entry_del(target)
            logger.info("Reading back the indirect register table and ensuring it got deleted")
            resp = register_table.entry_get(
                target,
                [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', reg_index)])],
                {"from_hw": True})
            for data, key in resp:
                data_dict = data.to_dict()
                assert data_dict["SwitchIngress.test_reg.first"] == data_dict["SwitchIngress.test_reg.second"]\
                    == [0]*num_pipes


class DirectRegisterSyncTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_register"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        smac = '00:11:22:33:44:55'
        seed = random.randint(0, 10000000)
        logger.info("Seed used for Direct Register Sync Test is %d", seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")

        register_dir_table = bfrt_info.table_get("SwitchIngress.reg_match_dir")
        register_dir_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        register_value_hi = random.randint(0, 12345678)
        register_value_lo = random.randint(0, 23456789)
        register_value_hi_arr = {}
        register_value_lo_arr = {}

        num_pipes = int(testutils.test_param_get('num_pipes'))
        for i in range(num_pipes):
            register_value_hi_arr[i] = register_value_hi
            register_value_lo_arr[i] = register_value_lo

        pkt = testutils.simple_tcp_packet(eth_src=smac)
        exp_pkt = pkt

        # Program the same register values in all the 4 pipes
        logger.info("Inserting entry in reg_match_dir table and programming stful reg with hi = %s lo = %s",
                    str(register_value_hi), str(register_value_lo))
        target = gc.Target(device_id=0, pipe_id=0xffff)
        try:
            # Insert entry in Match table
            register_dir_table.entry_add(
                target,
                [register_dir_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])],
                [register_dir_table.make_data(
                    [gc.DataTuple('SwitchIngress.test_reg_dir.first', register_value_lo),
                     gc.DataTuple('SwitchIngress.test_reg_dir.second', register_value_hi)],
                    'SwitchIngress.register_action_dir')])

            # Apply table operations to sync the direct registers
            logger.info("Syncing stful registers")
            register_dir_table.operations_execute(target, 'SyncRegisters')

            # Get from sw and check its value
            logger.info("Reading back the stful registers for the programmed match entry")
            resp = register_dir_table.entry_get(
                target,
                [register_dir_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])],
                {"from_hw": False})
            # The values read back should match the initialized values
            logger.info("Verifying read back register values")
            data_dict = next(resp)[0].to_dict()
            VerifyReadRegisters(self, "SwitchIngress.test_reg_dir.first", "SwitchIngress.test_reg_dir.second",
                                register_value_lo_arr, register_value_hi_arr, data_dict)
            logger.info("Register values matched before sending packets")

            num_pkts = random.randint(1, 10)
            logger.info("Sending %d packets on port %d", num_pkts, ig_port)
            for i in range(num_pkts):
                testutils.send_packet(self, ig_port, pkt)
                testutils.verify_packet(self, pkt, ig_port)

            # Apply table operations to sync the direct registers
            logger.info("Syncing stful registers")
            register_dir_table.operations_execute(target, 'SyncRegisters')

            # Get from sw and check its value. They should be updated
            logger.info("Reading back the stful registers for the programmed match entry")
            resp = register_dir_table.entry_get(
                target,
                [register_dir_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])],
                {"from_hw": False})

            # Since the packet is sent on a single pipe only the register entry
            # from that pipe will be updated.
            ingress_pipe = ig_port >> 7
            register_value_lo_arr[ingress_pipe] = register_value_lo + (1 * num_pkts)
            register_value_hi_arr[ingress_pipe] = register_value_hi + (100 * num_pkts)
            logger.info("Verifying read back register values")
            data_dict = next(resp)[0].to_dict()
            VerifyReadRegisters(self, "SwitchIngress.test_reg_dir.first", "SwitchIngress.test_reg_dir.second",
                                register_value_lo_arr, register_value_hi_arr, data_dict)
            logger.info("Register values matched after seding packets")

        finally:
            logger.info("Deleting entry from the table")
            register_dir_table.entry_del(
                target,
                [register_dir_table.make_key([gc.KeyTuple('hdr.ethernet.src_addr', smac)])])


class RegisterAttributesTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_register"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def verifyRegisterValue(self, resp, register_value):
        # Since the table exists in all pipes and 1 stage per pipe, we expect
        # to get back a number of register values equal to the number of pipes
        # (one each per pipe and stage).
        num_pipes = int(testutils.test_param_get('num_pipes'))
        num_register_values = num_pipes

        data_dict = next(resp)[0].to_dict()

        for i in range(num_register_values):
            read_value = data_dict["SwitchIngress.bool_register_table.f1"][i]
            logger.info("Register Expected Value (%s) : Read value (%s)", str(register_value[i]), str(read_value))
            if read_value != register_value[i]:
                logger.error("Register field didn't match with the read value")
                assert (0)

    def insertEntry(self, target, register_idx, register_val):
        self.register_bool_table.entry_add(
            target,
            [self.register_bool_table.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            [self.register_bool_table.make_data(
                [gc.DataTuple('SwitchIngress.bool_register_table.f1', register_val)])])

    def getEntry(self, register_idx):
        # Please note that grpc server is always going to return all instances of the register
        # i.e. one per pipe and stage the table exists in. The asymmetric suport for indirect
        # register tables is limited only to the insertion of the entries. Thus even if we
        # made the indirect register table asymmetric, we need to pass in the device target
        # as consisting of all the pipes while reading the entry
        target = gc.Target(device_id=0, pipe_id=0xffff)
        resp = self.register_bool_table.entry_get(
            target,
            [self.register_bool_table.make_key(
                [gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            {"from_hw": True})
        return resp

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used for RegisterAttributesTest is %d", seed)
        random.seed(seed)
        register_idx = random.randint(0, 500)
        register_value = {}

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")
        self.register_bool_table = bfrt_info.table_get('SwitchIngress.bool_register_table')

        # Run this test only for 4 pipe devices
        if num_pipes < 4:
            logger.info("Skipping Entry scope test for a non 4 pipe device")
        else:
            # The default values of all the registers is 0 (see p4)
            target = gc.Target(device_id=0, pipe_id=0xffff)
            self.register_bool_table.attribute_entry_scope_set(
                target, predefined_pipe_scope=True,
                predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)
            target = gc.Target(device_id=0, pipe_id=0xffff)
            self.insertEntry(target, register_idx, 0)

            # Set pipes 0 and 1 in scope 1 and pipes 2 and 3 in scope 2
            logger.info("")
            logger.info("=============== Tesing User Defined Scope:"
                        "Scope 1 (pipe 0 and 1), Scope 2 (pipe 2 and 3) ===============")
            logger.info("Scope 1 (pipe 0 and 1), Scope 2 (pipe 2 and 3)")
            target = gc.Target(device_id=0, pipe_id=0xffff)
            self.register_bool_table.attribute_entry_scope_set(
                target, predefined_pipe_scope=False,
                predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL, user_defined_pipe_scope_val=0xc03)

            # Set the register in pipes 0 and 1 to 1
            logger.info("Set the registers in pipes 0 and 1 to 1")
            target = gc.Target(device_id=0, pipe_id=0x0)
            self.insertEntry(target, register_idx, 1)
            resp = self.getEntry(register_idx)
            # pipes 2 and 3 will have the default value of 0
            register_value = {0: 1, 1: 1, 2: 0, 3: 0}
            self.verifyRegisterValue(resp, register_value)

            # Now set the registers in pipes 2 and 3 to 1 as well
            logger.info("Now set the registers in pipes 2 and 3 to 1 as well")
            target = gc.Target(device_id=0, pipe_id=0x2)
            self.insertEntry(target, register_idx, 1)
            resp = self.getEntry(register_idx)
            # pipes 2 and 3 will also be equal to 1
            register_value = {0: 1, 1: 1, 2: 1, 3: 1}
            self.verifyRegisterValue(resp, register_value)

        # program 0 in all the pipes
        # Set the scope of the table to ALL_PIPES
        logger.info("")
        logger.info("=============== Testing All Pipe Scope ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        self.register_bool_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL)
        logger.info("Set registers in all pipes to 0")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        self.insertEntry(target, register_idx, 0)

        # Read back values should be all 0s
        resp = self.getEntry(register_idx)
        register_value = {0: 0, 1: 0, 2: 0, 3: 0}
        self.verifyRegisterValue(resp, register_value)

        # Set the scope of the table to SINGLE_PIPE
        logger.info("=============== Testing Single Pipe Scope ===============")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        self.register_bool_table.attribute_entry_scope_set(target, predefined_pipe_scope=True,
                                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE)
        # Program a register in pipe 1 as 1
        if 1 in pipes:
            logger.info("Set register in pipe 1 to 1")
            target = gc.Target(device_id=0, pipe_id=0x1)
            self.insertEntry(target, register_idx, 1)
            resp = self.getEntry(register_idx)
            register_value = {pipe:0 for pipe in pipes}
            register_value[1] = 1
            self.verifyRegisterValue(resp, register_value)

        if 3 in pipes:
            logger.info("Adding entry in pipe 3")
            # Program a register in pipe 3 as 1
            logger.info("Set register in pipe 3 to 1")
            target = gc.Target(device_id=0, pipe_id=0x3)
            self.insertEntry(target, register_idx, 1)
            resp = self.getEntry(register_idx)
            register_value = {pipe:0 for pipe in pipes}
            register_value[1] = 1
            register_value[3] = 1
            self.verifyRegisterValue(resp, register_value)


class IndirectRegisterIteratorTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_register"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        This test does the following
        1. Adds 1k register table entries with random data
        2. Does a register sync table operation
        3. Gets all the entries from the register table
        4. Verifies the read values match what has been programmed
        '''
        seed = random.randint(0, 65535)
        logger.info("Seed used for Indirect Register Sync Test is %d", seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")

        register_table = bfrt_info.table_get("SwitchIngress.test_reg")

        num_entries = 1024
        # Program the same register values in all the 4 pipes
        logger.info("Inserting %d entries into the register table", num_entries)
        target = gc.Target(device_id=0, pipe_id=0xffff)

        register_value_hi = [random.randint(1, 10000) for x in range(num_entries)]
        register_value_lo = [random.randint(1, 10000) for x in range(num_entries)]

        for x in range(num_entries):
            register_table.entry_add(
                target,
                [register_table.make_key([gc.KeyTuple('$REGISTER_INDEX', x)])],
                [register_table.make_data(
                    [gc.DataTuple('SwitchIngress.test_reg.first', register_value_lo[x]),
                     gc.DataTuple('SwitchIngress.test_reg.second', register_value_hi[x])])])
        # Apply table operations to sync the registers on the indirect register table
        logger.info("Syncing indirect stful registers")
        register_table.operations_execute(target, 'Sync')

        # Get from sw and check its value.
        logger.info("Reading back the register table entries")
        resp = register_table.entry_get(
            target,
            flags={"from_hw": False})

        # The values read back should match the initialized values
        logger.info("Verifying read back indirect register values")
        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            VerifyReadRegisters(self, "SwitchIngress.test_reg.first", "SwitchIngress.test_reg.second",
                                [register_value_lo[i]] * 4, [register_value_hi[i]] * 4, data_dict)
            assert key_dict["$REGISTER_INDEX"]['value'] == i
            i += 1
