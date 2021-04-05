# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as client

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

num_pipes = int(testutils.test_param_get('num_pipes'))

f16 = random.randint(0, 0xffff)
f8 = random.randint(0, 0xff)


class EntryOpTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used for PVS Entry Operation Test is %d", seed)
        random.seed(seed)
        logger.info("Test PVS entry add read and delete operations")
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()
        vs_table = bfrt_info.table_get("ParserI.vs")

        logger.info("Set all scopes to ALL")
        target = client.Target(device_id=0, pipe_id=0xffff, direction=0xff, prsr_id=0xff)
        vs_table.attribute_entry_scope_set(target,
                                           config_gress_scope=True, predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                           config_pipe_scope=True, predefined_pipe_scope=True,
                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL, pipe_scope_args=0xff,
                                           config_prsr_scope=True, predefined_prsr_scope_val=bfruntime_pb2.Mode.ALL,
                                           prsr_scope_args=0xff)

        # Get the table attribute and verify it is correct
        resp = vs_table.attribute_get(target, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL

        # Get the table usage, it should be empty.
        usage_resp = next(vs_table.usage_get(target))
        self.assertEqual(usage_resp, 0)

        f16 = random.randint(0, 2)
        f8 = random.randint(0, 1)
        logger.info("Insert an entry")
        test_key = vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])
        vs_table.entry_add(
            target,
            [test_key])

        # Get the table usage, it should have one entry now.
        usage_resp = next(vs_table.usage_get(target))
        self.assertEqual(usage_resp, 1)

        resp = vs_table.entry_get(
            target,
            [test_key],
            {"from_hw": True})

        # Check for errors
        for _, key in resp:
            key_dict = key.to_dict()
            assert key == test_key

        logger.info("Entry get success")
        logger.info("Delete the entry")
        vs_table.entry_del(
            target,
            [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])])

        # Add four entries and get them using get-first and get-next
        added = list()
        expected = list()
        for i in [1, 2, 3, 4]:
            f16 = i
            f8 = i + 10
            k = vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                   client.KeyTuple('f8', f8, 0xff)])
            expected.append(k)
            added.append(k)
            vs_table.entry_add(target, [k])

            # Check the usage as we add entries.
            usage_resp = next(vs_table.usage_get(target))
            self.assertEqual(usage_resp, i)

        resp = vs_table.entry_get(target, None, {"from_hw": False})
        for data, key in resp:
            "==============="
            logger.info(key.to_dict())
            logger.info(data.to_dict())
            assert key in expected
            expected.remove(key)

        vs_table.entry_del(target, None)
        logger.info("Check if entries were deleted in clear call")
        resp = vs_table.entry_get(target, added, {"from_hw": False})
        try:
            # If entries are not there exception will be raised on following line
            for data, key in resp:
                print("Stub print")
        except Exception:
            print("Entry deleted - PASS")
        else:
            raise AssertionError("Entries not deleted")


class AttributeTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id)

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used for PVS Entry Operation Test is %d", seed)
        random.seed(seed)
        logger.info("Test PVS attributes operations")
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()
        vs_table = bfrt_info.table_get("ParserI.vs")
        target = client.Target(device_id=0, pipe_id=0xffff, direction=0xff, prsr_id=0xff)
        target_ingress = client.Target(device_id=0, pipe_id=0xffff, direction=0x00, prsr_id=0xff)
        logger.info("Test pipe scope set")
        logger.info("Set all scope to single")
        # The below passes with both target and target_ingress
        vs_table.attribute_entry_scope_set(target,
                                           config_gress_scope=True,
                                           predefined_gress_scope_val=bfruntime_pb2.Mode.SINGLE,
                                           config_pipe_scope=True, predefined_pipe_scope=True,
                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.SINGLE, pipe_scope_args=0,
                                           config_prsr_scope=True,
                                           predefined_prsr_scope_val=bfruntime_pb2.Mode.SINGLE, prsr_scope_args=0)
        # Get the table attribute and verify it is correct. This will only work with target_ingress now
        resp = vs_table.attribute_get(target_ingress, "EntryScope")
        for d in resp:
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.SINGLE
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.SINGLE
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.SINGLE
            assert d["prsr_scope"]["args"] == 0

        logger.info("Try to insert pvs table to all_gress, expect fail")
        target = client.Target(device_id=0, pipe_id=0, direction=0xff, prsr_id=0)
        f16 = random.randint(0, 2)
        f8 = random.randint(0, 1)
        try:
            vs_table.entry_add(
                target,
                [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                    client.KeyTuple('f8', f8, 0xff)])])
            logger.info("Attribute set fail")
            assert (0)
        except:
            logger.info("Attribute set success")
        logger.info("Try to insert pvs table to all_pipe, expect fail")
        target = client.Target(device_id=0, pipe_id=0xffff, direction=0, prsr_id=0)
        try:
            vs_table.entry_add(
                target,
                [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                    client.KeyTuple('f8', f8, 0xff)])])
            logger.info("Attribute set fail")
            assert (0)
        except:
            logger.info("Attribute set success")
        logger.info("Try to insert pvs table to all_parser, expect fail")
        target = client.Target(device_id=0, pipe_id=1, direction=0, prsr_id=0xff)
        try:
            vs_table.entry_add(
                target,
                [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                    client.KeyTuple('f8', f8, 0xff)])])
            logger.info("Attribute set fail")
            assert (0)
        except:
            logger.info("Attribute set success")
        logger.info("Set all scope to all")
        target = client.Target(device_id=0, pipe_id=0xffff, direction=0xff, prsr_id=0xff)
        vs_table.attribute_entry_scope_set(target,
                                           config_gress_scope=True, predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                           config_pipe_scope=True, predefined_pipe_scope=True,
                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL, pipe_scope_args=0xff,
                                           config_prsr_scope=True, predefined_prsr_scope_val=bfruntime_pb2.Mode.ALL,
                                           prsr_scope_args=0xff)
        # Get the table attribute and verify it is correct.
        resp = vs_table.attribute_get(target, "EntryScope")
        for d in resp:
            print(d)
            assert d["gress_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["pipe_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["prsr_scope"]["predef"] == bfruntime_pb2.Mode.ALL
            assert d["prsr_scope"]["args"] == 0xff

        target = client.Target(device_id=0, pipe_id=0xffff, direction=0xff, prsr_id=0)
        if num_pipes == 4:
            logger.info("Set pipe scope to user_defined, pipe0,2, pipe1,3")
            vs_table.attribute_entry_scope_set(target,
                                               config_gress_scope=True,
                                               predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                               config_pipe_scope=True, predefined_pipe_scope=False,
                                               user_defined_pipe_scope_val=0x0a05, pipe_scope_args=0xff,
                                               config_prsr_scope=True,
                                               predefined_prsr_scope_val=bfruntime_pb2.Mode.SINGLE,
                                               prsr_scope_args=0xff)
        if num_pipes == 3:
            logger.info("Set pipe scope to user_defined, pipe0,2, pipe1")
            vs_table.attribute_entry_scope_set(target,
                                               config_gress_scope=True,
                                               predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                               config_pipe_scope=True, predefined_pipe_scope=False,
                                               user_defined_pipe_scope_val=0x0205, pipe_scope_args=0xff,
                                               config_prsr_scope=True,
                                               predefined_prsr_scope_val=bfruntime_pb2.Mode.SINGLE,
                                               prsr_scope_args=0xff)
        if num_pipes == 2:
            logger.info("Set pipe scope to user_defined, pipe0, pipe1")
            vs_table.attribute_entry_scope_set(target,
                                               config_gress_scope=True,
                                               predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                               config_pipe_scope=True, predefined_pipe_scope=False,
                                               user_defined_pipe_scope_val=0x0201, pipe_scope_args=0xff,
                                               config_prsr_scope=True,
                                               predefined_prsr_scope_val=bfruntime_pb2.Mode.SINGLE,
                                               prsr_scope_args=0xff)

        logger.info("Try to insert pvs table to pipe 0, expect success")
        target = client.Target(device_id=0, pipe_id=0, direction=0xff, prsr_id=0)
        f16 = random.randint(0, 2)
        f8 = random.randint(0, 1)
        vs_table.entry_add(
            target,
            [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])])
        vs_table.entry_del(
            target,
            [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])])

        logger.info("Try to insert pvs table to pipe 1, expect success")
        f16 = random.randint(0, 2)
        f8 = random.randint(0, 1)
        target = client.Target(device_id=0, pipe_id=1, direction=0xff, prsr_id=0)
        vs_table.entry_add(
            target,
            [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])])
        vs_table.entry_del(
            target,
            [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                client.KeyTuple('f8', f8, 0xff)])])

        if num_pipes > 2:
            logger.info("Try to insert pvs table to pipe 2(group pipe0,2), expect fail")
            target = client.Target(device_id=0, pipe_id=2, direction=0xff, prsr_id=0)
            try:
                vs_table.entry_add(
                    target,
                    [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                        client.KeyTuple('f8', f8, 0xff)])])
                logger.info("Attribute set fail")
                assert (0)
            except:
                logger.info("Attribute set success")

        if num_pipes > 3:
            logger.info("Try to insert pvs table to pipe 3(group pipe1,3), expect fail")
            target = client.Target(device_id=0, pipe_id=3, direction=0xff, prsr_id=0)
            try:
                vs_table.entry_add(
                    target,
                    [vs_table.make_key([client.KeyTuple('f16', f16, 0xffff),
                                        client.KeyTuple('f8', f8, 0xff)])])
                logger.info("Attribute set fail")
                assert (0)
            except:
                logger.info("Attribute set success")

        target = client.Target(device_id=0, pipe_id=0xffff, direction=0xff, prsr_id=0xff)
        vs_table.attribute_entry_scope_set(target,
                                           config_gress_scope=True, predefined_gress_scope_val=bfruntime_pb2.Mode.ALL,
                                           config_pipe_scope=True, predefined_pipe_scope=True,
                                           predefined_pipe_scope_val=bfruntime_pb2.Mode.ALL, pipe_scope_args=0xff,
                                           config_prsr_scope=True, predefined_prsr_scope_val=bfruntime_pb2.Mode.ALL,
                                           prsr_scope_args=0xff)
