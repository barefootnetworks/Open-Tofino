# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
from ptf.thriftutils import *
from ptf.testutils import *
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


class DynHashingTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_dyn_hashing"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_dyn_hashing")
        hash_config_table = bfrt_info.table_get("IngressP.hash_1.$CONFIGURE")

        logger.info("=============== Testing Dyn Hashing entry operation===============")
        target = client.Target(device_id=0, pipe_id=0xffff)
        logger.info("Modify entry")
        hash_config_table.entry_add(
            target,
            None,
            [hash_config_table.make_data([client.DataTuple('hdr.ipv4.proto.$PRIORITY', 0),
                                          client.DataTuple('hdr.ipv4.sip.$PRIORITY', 2),
                                          client.DataTuple('hdr.ipv4.dip.$PRIORITY', 1),
                                          client.DataTuple('hdr.tcp.sPort.$PRIORITY', 3),
                                          client.DataTuple('hdr.tcp.dPort.$PRIORITY', 4)])])
        logger.info("Read entry")
        resp = hash_config_table.entry_get(target, None, {"from_hw": False})
        data_dict = next(resp)[0].to_dict()
        logger.info("data_dict = %s", str(data_dict))
        assert (data_dict['hdr.ipv4.proto.$PRIORITY'] == 0)
        assert (data_dict['hdr.ipv4.sip.$PRIORITY'] == 2)
        assert (data_dict['hdr.ipv4.dip.$PRIORITY'] == 1)
        assert (data_dict['hdr.tcp.sPort.$PRIORITY'] == 3)
        assert (data_dict['hdr.tcp.dPort.$PRIORITY'] == 4)

        logger.info("set dyn hashing attribute")
        hash_config_table.attribute_dyn_hashing_set(target,
                                                    alg_hdl=587202560,
                                                    seed=0x12345)
        resp = hash_config_table.attribute_get(target, "DynamicHashing")
        for d in resp:
            assert d["alg"] == 587202560
            assert d["seed"] == 0x12345

class DynHashingAlgorithmChangeTest(BfRuntimeTest):
    """@brief
    This test does the following checks
    1. Change algorithm of a hash object to a predefined CRC Algorithm
    2. Change algorithm of a hash object to a predefined non-CRC algorithm like
    IDENTITY/RANDOM/XOR
    3. Change algorithm of a hash object to a user defined CRC algorithm
    4. Reset Algorithm and seed to the default values
    """
    def setUp(self):
        client_id = 0
        p4_name = "tna_dyn_hashing"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def verify_get(self, table, data_expected):
        resp = table.default_entry_get(self.target)
        for data, key in resp:
            assert data == data_expected, "Expected %s Received %s" %(str(data_expected), str(data))

    def runTest(self):
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_dyn_hashing")
        hash_algo_table = bfrt_info.table_get("hash_1.algorithm")

        logger.info("=============== Testing Dyn Hashing Algorithm predefined ===============")
        target = client.Target(device_id=0, pipe_id=0xffff)
        self.target = target
        logger.info("Changing to CRC_16")
        data_in = hash_algo_table.make_data([client.DataTuple('seed', 3045),
                                        client.DataTuple('msb', bool_val=random.choice([True, False])),
                                        client.DataTuple('extend', bool_val=random.choice([True, False])),
                                        client.DataTuple('algorithm_name', str_val="CRC_16")],
                                          "pre_defined")
        hash_algo_table.default_entry_set(target, data_in)
        self.verify_get(hash_algo_table, data_in)

        logger.info("Changing to predefined IDENTITY")
        data_in = hash_algo_table.make_data([client.DataTuple('seed', 1234),
                                        client.DataTuple('msb', bool_val=random.choice([True, False])),
                                        client.DataTuple('extend', bool_val=random.choice([True, False])),
                                        client.DataTuple('algorithm_name', str_val="IDENTITY")],
                                          "pre_defined")
        hash_algo_table.default_entry_set(target, data_in)
        self.verify_get(hash_algo_table, data_in)

        logger.info("Changing to predefined XOR")
        data_in = hash_algo_table.make_data([client.DataTuple('seed', 6789),
                                        client.DataTuple('msb', bool_val=random.choice([True, False])),
                                        client.DataTuple('extend', bool_val=random.choice([True, False])),
                                        client.DataTuple('algorithm_name', str_val="XOR")],
                                          "pre_defined")
        hash_algo_table.default_entry_set(target, data_in)
        self.verify_get(hash_algo_table, data_in)

        logger.info("Changing to user defined CRC")
        data_in = hash_algo_table.make_data([client.DataTuple('seed', 987),
                                        client.DataTuple('msb', bool_val=random.choice([True, False])),
                                        client.DataTuple('extend', bool_val=random.choice([True, False])),
                                        client.DataTuple('polynomial', 0x7f),
                                        client.DataTuple('init', 0x1),
                                        client.DataTuple('final_xor', 0x11),
                                        client.DataTuple('reverse', bool_val=random.choice([True, False])),
                                        client.DataTuple('hash_bit_width', 64)],
                                          "user_defined")
        hash_algo_table.default_entry_set(target, data_in)
        self.verify_get(hash_algo_table, data_in)

        # Modify just the seed
        logger.info("Modifying only seed")
        hash_algo_table.default_entry_set(target,
                hash_algo_table.make_data([client.DataTuple('seed', 345)],
                                          "user_defined"))
        data_in["seed"] = client.DataTuple('seed', 345)
        self.verify_get(hash_algo_table, data_in)

        # Clear the table and verify if it is the same as that defined in P4
        logger.info("Clearing algorithm table to default params")
        hash_algo_table.default_entry_reset(target)
        data_in = hash_algo_table.make_data([client.DataTuple('seed', 0x31e34),
                                        client.DataTuple('msb', bool_val=False),
                                        client.DataTuple('extend', bool_val=False),
                                        client.DataTuple('algorithm_name', str_val="RANDOM")],
                                          "pre_defined")
        self.verify_get(hash_algo_table, data_in)


