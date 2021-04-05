# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging

import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as client
import base64
import itertools

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())


class MeterBytecountAdjustTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_bytecount_adjust"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()
        meter_table = bfrt_info.table_get("SwitchIngress.meter")
        direct_meter_color_table = bfrt_info.table_get("SwitchIngress.direct_meter_color")

        target = client.Target(device_id=0, pipe_id=0xffff)

        logger.info("set byte count adjust for meter table")
        meter_table.attribute_meter_bytecount_adjust_set(target, -12)
        resp = meter_table.attribute_get(target, "MeterByteCountAdjust")
        for d in resp:
            assert d["byte_count_adjust"] == -12

        logger.info("set byte count adjust for MatchAction_Direct meter table")
        direct_meter_color_table.attribute_meter_bytecount_adjust_set(target, 20)
        resp = direct_meter_color_table.attribute_get(target, "MeterByteCountAdjust")
        for d in resp:
            assert d["byte_count_adjust"] == 20
