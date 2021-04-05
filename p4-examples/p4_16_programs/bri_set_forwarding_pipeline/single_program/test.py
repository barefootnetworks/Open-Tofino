# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

from ptf import config
import ptf.testutils as testutils
from ptf.testutils import *
from bfruntime_client_base_tests import BfRuntimeTest, BaseTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import random

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

base_pick_path = testutils.test_param_get("base_pick_path")
binary_name = testutils.test_param_get("arch")
if binary_name is not "tofino2" and binary_name is not "tofino":
    assert 0, "%s is unknown arch" % (binary_name)

if not base_pick_path:
    base_pick_path = "install/share/" + binary_name + "pd/"

base_put_path = testutils.test_param_get("base_put_path")
if not base_put_path:
    base_put_path = "install/share/" + binary_name + "pd/forwarding"

logger.info("\nbase_put_path=%s \nbase_pick_path=%s", base_pick_path, base_put_path)

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


def create_path_bf_rt(base_path, p4_name_to_use):
    return base_path + "/" + p4_name_to_use + "/bf-rt.json"


def create_path_context(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/context.json"


def create_path_tofino(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/" + binary_name + ".bin"


def VerifyReadRegisters(self, register_name_lo, register_name_hi, resp, register_value_lo, register_value_hi,
                        data_dict):
    # since the table is symmetric and exists only in stage 0, we know that the response data is going to have
    # 8 data fields (2 (hi and lo) * 4 (num pipes) * 1 (num_stages)). bfrt_server returns all (4) register values
    # corresponding to one field id followed by all (4) register values corresponding to the other field id

    num_pipes = int(testutils.test_param_get('num_pipes'))
    for i in range(num_pipes):
        value_lo = data_dict[register_name_lo][i]
        value_hi = data_dict[register_name_hi][i]
        logger.info("Register Lo Expected Value (%s) : Read value (%s)", str(register_value_lo[i]), str(value_lo))
        logger.info("Register Hi Expected Value (%s) : Read value (%s)", str(register_value_hi[i]), str(value_hi))
        if data_dict[register_name_lo][i] != register_value_lo[i]:
            logger.info("Register field lo didn't match with the read value")
            assert (0)
        if data_dict[register_name_hi][i] != register_value_hi[i]:
            logger.info("Register field hi didn't match with the read value")
            assert (0)


class SingleProgramNoReplayTest(BfRuntimeTest):
    '''
    Case 1
    This test does the following
    setUp()
    1. Client Subscribes to server
    2. Send a VERIFY_AND_WARM_INIT_BEGIN_AND_END SetForwardingPipelineConfig msg
    3. Wait for WARM_INIT_FINISHED
    4. BIND to program
    runTest()
    1. run the test -> insert entries
    '''

    '''
    Taken from IndirectRegisterIteratorTest
    '''

    def setUp(self):
        client_id = 0

        p4_name_to_put = p4_name_to_pick = p4_name = "tna_register"
        profile_name_to_put = profile_name_to_pick = "pipe"

        # Setup and don't perform bind
        BfRuntimeTest.setUp(self, client_id, perform_bind=False)
        # Send a Verify_and_warm_init_begin_and_end
        logger.info("Sending Verify and warm_init_begin and warm_init_end for %s", p4_name_to_put)
        action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN_AND_END
        success = self.interface.send_set_forwarding_pipeline_config_request(
            action,
            base_put_path,
            [gc.ForwardingConfig(p4_name_to_put,
                                 create_path_bf_rt(base_pick_path, p4_name_to_pick),
                                 [gc.ProfileInfo(profile_name_to_put,
                                                 create_path_context(base_pick_path, p4_name_to_pick,
                                                                     profile_name_to_pick),
                                                 create_path_tofino(base_pick_path, p4_name_to_pick,
                                                                    profile_name_to_pick),
                                                 [0, 1, 2, 3])]
                                 )])
        if not success:
            raise RuntimeError("Failed to setFwd")

        self.p4_name = p4_name
        self.interface.bind_pipeline_config("tna_register")

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used for RegisterTest is %d", seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_register")
        test_reg_table = bfrt_info.table_get("SwitchIngress.test_reg")

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
        test_reg_table.entry_add(
            target,
            [test_reg_table.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            [test_reg_table.make_data(
                [gc.DataTuple('SwitchIngress.test_reg.first', register_value_lo),
                 gc.DataTuple('SwitchIngress.test_reg.second', register_value_hi)])])

        resp = test_reg_table.entry_get(
            target,
            [test_reg_table.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)])],
            {"from_hw": True})

        data_dict = next(resp)[0].to_dict()
        VerifyReadRegisters(self, "SwitchIngress.test_reg.first", "SwitchIngress.test_reg.second", resp,
                            register_value_lo_arr, register_value_hi_arr, data_dict)


class SingleProgramWithReplayTest(BfRuntimeTest):
    '''
    Case 2
    This test does the following
    setUp()
    1. Client Subscribes to server 
    2. Send a VERIFY_AND_WARM_INIT_BEGIN SetForwardingPipelineConfig msg
    3. Wait for WARM_INIT_STARTED
    runTest()
    4. BINDS to program with SetForwardingPipelineConfig msg
    5. As part of replay, adds 1k register table entries with random data
    6. Sends a WARM_INIT_END SetForwardingPipelineConfig msg
    7. Wait for WARM_INIT_FINISHED
    8. Does a register sync table operation
    9. Gets all the entries from the register table
    10. Verifies the read values match what has been programmed
    '''
    '''
    Taken from IndirectRegisterIteratorTest
    '''

    def setUp(self):
        client_id = 0
        # Setup and don't perform bind
        BfRuntimeTest.setUp(self, client_id, perform_bind=False)
        # Send a Verify_and_warm_init_begin_and_end
        self.p4_name_to_put = self.p4_name_to_pick = p4_name = "tna_register"
        self.profile_name_to_put = "pipe"
        self.profile_name_to_pick = "pipe"

        logger.info("Sending Verify and warm_init_begin and warm_init_end for %s", self.p4_name_to_put)
        action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN
        success = self.interface.send_set_forwarding_pipeline_config_request(
            action,
            base_put_path,
            [gc.ForwardingConfig(self.p4_name_to_put,
                                 create_path_bf_rt(base_pick_path, self.p4_name_to_pick),
                                 [gc.ProfileInfo(self.profile_name_to_put,
                                                 create_path_context(base_pick_path, self.p4_name_to_pick,
                                                                     self.profile_name_to_pick),
                                                 create_path_tofino(base_pick_path, self.p4_name_to_pick,
                                                                    self.profile_name_to_pick),
                                                 [0, 1, 2, 3])]
                                 )])
        if not success:
            raise RuntimeError("Failed to setFwd")

    def replay_entries(self):
        seed = random.randint(0, 65535)
        logger.info("Seed used for Indirect Register Sync Test is %d", seed)
        random.seed(seed)

        num_entries = 1024
        # Program the same register values in all the 4 pipes
        logger.info("Inserting %d entries into the register table", num_entries)
        target = gc.Target(device_id=0, pipe_id=0xffff)

        self.register_value_hi = [random.randint(1, 10000) for x in range(num_entries)]
        self.register_value_lo = [random.randint(1, 10000) for x in range(num_entries)]

        for x in range(num_entries):
            self.test_reg_table.entry_add(
                target,
                [self.test_reg_table.make_key([gc.KeyTuple('$REGISTER_INDEX', x)])],
                [self.test_reg_table.make_data(
                    [gc.DataTuple('SwitchIngress.test_reg.first', self.register_value_lo[x]),
                     gc.DataTuple('SwitchIngress.test_reg.second', self.register_value_hi[x])])])

    def cross_check_entries(self):
        # Apply table operations to sync the registers on the indirect register table
        logger.info("Syncing indirect stful registers")
        target = gc.Target(device_id=0, pipe_id=0xffff)
        self.test_reg_table.operations_execute(target, 'Sync')

        # Get from sw and check its value.
        logger.info("Reading back the register table entries")
        resp = self.test_reg_table.entry_get(
            target,
            None,
            {"from_hw": False})

        # The values read back should match the initialized values
        logger.info("Verifying read back indirect register values")
        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            VerifyReadRegisters(self,
                                "SwitchIngress.test_reg.first",
                                "SwitchIngress.test_reg.second",
                                resp,
                                [self.register_value_lo[i]] * 4,
                                [self.register_value_hi[i]] * 4,
                                data_dict)
            assert key_dict["$REGISTER_INDEX"]['value'] == i
            logger.info("Indirect Register values matched for idx %d", i)
            i += 1
        pass

    def runTest(self):
        self.p4_name = "tna_register"
        self.interface.bind_pipeline_config(self.p4_name)

        bfrt_info = self.interface.bfrt_info_get("tna_register")
        self.test_reg_table = bfrt_info.table_get("SwitchIngress.test_reg")
        # Insert some entries as replays
        self.replay_entries()

        # Send a warm_init_end
        logger.info("Sending warm_init_end")

        action = bfruntime_pb2.SetForwardingPipelineConfigRequest.WARM_INIT_END
        self.interface.send_set_forwarding_pipeline_config_request(action)

        self.cross_check_entries()


"""
Case 3: Case where switchd starts with a Program and client just wants to BIND.
For this case, please refer to any of the tna_* programs.
"""
