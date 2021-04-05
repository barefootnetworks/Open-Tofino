# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client

import base64

g_is_tofino = testutils.test_param_get("arch") == "tofino"
g_is_tofino2 = testutils.test_param_get("arch") == "tofino2"
assert g_is_tofino or g_is_tofino2

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())


def meterParamConvert(param):
    '''
    Converts a hex decoded meter parameter byte stream to an integer
    '''
    a = base64.b16encode(param)
    return int(a, 16)


def equalwithTolerance(param1, param2):
    '''
    Tests whether param2 is within a tolerance of +-5% of param1
    '''
    ratio = float(param1) / float(param2)
    if ratio > 0.95 and ratio < 1.05:
        return True
    logger.info("param1: %d param2: %d", param1, param2)
    logger.info("ratio : %f", ratio)
    return False


def meterGetTestHelper(self, table, key_fields, action_name):
    num_entries = len(key_fields)
    target = client.Target(device_id=0, pipe_id=0xffff)

    cir_kbps = [random.randint(1, 200) * 1000 for x in range(num_entries)]
    pir_kbps = [cir_kbps[x] + random.randint(1, 20) * 1000 for x in range(num_entries)]

    cbs = [cir_kbps[x] * 4 for x in range(num_entries)]
    pbs = [cbs[x] * 2 for x in range(num_entries)]

    logger.info("Inserting %d entries to the meter table", num_entries)
    data_list = []
    for x in range(num_entries):
        data_list.append(table.make_data(
            [client.DataTuple('$METER_SPEC_CIR_KBPS', cir_kbps[x]),
             client.DataTuple('$METER_SPEC_PIR_KBPS', pir_kbps[x]),
             client.DataTuple('$METER_SPEC_CBS_KBITS', cbs[x]),
             client.DataTuple('$METER_SPEC_PBS_KBITS', pbs[x])],
            action_name))
    table.entry_add(target, key_fields, data_list)

    resp = table.entry_get(
        target,
        key_fields,
        {"from_hw": False})

    x = 0
    for data, key in resp:
        data_dict = data.to_dict()
        assert equalwithTolerance(data_dict["$METER_SPEC_CIR_KBPS"], cir_kbps[x]) == True
        assert equalwithTolerance(data_dict["$METER_SPEC_PIR_KBPS"], pir_kbps[x]) == True
        assert equalwithTolerance(data_dict["$METER_SPEC_CBS_KBITS"], cbs[x]) == True
        assert equalwithTolerance(data_dict["$METER_SPEC_PBS_KBITS"], pbs[x]) == True
        x += 1


class IndirectMeterGetTest(BfRuntimeTest):
    """@brief This test sets meter indices in an indirect match table
    and gets the indices back as verification
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % seed)
        random.seed(seed)

        num_entries = random.randint(1, 100)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        meter_table = bfrt_info.table_get("SwitchIngress.meter")

        key_field_list = []
        for x in range(num_entries):
            key_field_list.append(meter_table.make_key([client.KeyTuple('$METER_INDEX', x + 1)]))

        meterGetTestHelper(self, meter_table, key_field_list, None)


class DirectMeterDefaultSpecTest(BfRuntimeTest):
    """@brief This test adds an entry to a match table with a direct meter and
    uses a default meter configuration, i.e. the meter configuration is not
    specified, and reads the entry back to validate it is a maximum rate
    configuration.
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)

        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        table = bfrt_info.table_get("SwitchIngress.direct_meter_color")
        table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        entry_key = table.make_key([client.KeyTuple('hdr.ethernet.src_addr', '00:11:22:33:44:55')])
        entry_data = table.make_data([], "SwitchIngress.set_color_direct")
        table.entry_add(target, [entry_key], [entry_data])

        for data, key in table.entry_get(target, [entry_key], {"from_hw": False}):
            data_dict = data.to_dict()
            if g_is_tofino and testutils.test_param_get("target") is not "hw":
                self.assertEqual(data_dict["$METER_SPEC_CIR_KBPS"], 5195848221)
                self.assertEqual(data_dict["$METER_SPEC_PIR_KBPS"], 5195848221)
            elif g_is_tofino2:
                self.assertEqual(data_dict["$METER_SPEC_CIR_KBPS"], 4087999890)
                self.assertEqual(data_dict["$METER_SPEC_PIR_KBPS"], 4087999890)
            self.assertEqual(data_dict["$METER_SPEC_CBS_KBITS"], 4380866642)
            self.assertEqual(data_dict["$METER_SPEC_PBS_KBITS"], 4380866642)

        table.entry_del(target, [entry_key])


class DirectMeterTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % (seed))
        random.seed(seed)

        num_entries = random.randint(1, 100)
        key_dict = {}
        mac_addrs = []

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")

        self.direct_match_table = bfrt_info.table_get("SwitchIngress.direct_meter_color")
        self.direct_match_table.info.key_field_annotation_add("hdr.ethernet.src_addr", "mac")

        for i in range(num_entries):
            mac_addr = "%02x:%02x:%02x:%02x:%02x:%02x" % (
                random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
                random.randint(0, 255), random.randint(0, 255))
            while mac_addr in key_dict:
                mac_addr = "%02x:%02x:%02x:%02x:%02x:%02x" % (
                    random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255))
            mac_addrs.append(mac_addr)
            key_dict[mac_addr] = True

        key_field_list = []
        for x in range(num_entries):
            key_field_list.append(self.direct_match_table.make_key(
                [client.KeyTuple('hdr.ethernet.src_addr', mac_addrs[x])]))

        meterGetTestHelper(self, self.direct_match_table,
                           key_field_list, "SwitchIngress.set_color_direct")
        logger.info("Deleting %d entries from the meter table", num_entries)

        self.direct_match_table.entry_del(target, key_field_list)


class IndirectMeterTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % (seed))
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        meter_table = bfrt_info.table_get("SwitchIngress.meter_color")
        meter_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

        num_entries = random.randint(1, 100)
        meter_indices = [x + 1 for x in range(num_entries)]
        random.shuffle(meter_indices)
        key_dict = {}
        mac_addrs = []

        for i in range(num_entries):
            mac_addr = "%02x:%02x:%02x:%02x:%02x:%02x" % (
                random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
                random.randint(0, 255), random.randint(0, 255))
            while mac_addr in key_dict:
                mac_addr = "%02x:%02x:%02x:%02x:%02x:%02x" % (
                    random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255))
            mac_addrs.append(mac_addr)
            key_dict[mac_addr] = True

        logger.info("Inserting %d entries to the Match table pointing to random meter indices", num_entries)
        for x in range(num_entries):
            meter_table.entry_add(
                target,
                [meter_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', mac_addrs[x])])],
                [meter_table.make_data(
                    [client.DataTuple('meter_idx', meter_indices[x])],
                    'SwitchIngress.set_color')]
            )

        logger.info("Reading %d entries from the Match table to verify meter indices", num_entries)
        for x in range(num_entries):
            resp = meter_table.entry_get(
                target,
                [meter_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', mac_addrs[x])])],
                {"from_hw": False})
            data_dict = next(resp)[0].to_dict()
            assert data_dict["meter_idx"] == meter_indices[x]

        logger.info("All entries matched")

        logger.info("Deleting %d match entries", num_entries)
        for x in range(num_entries):
            meter_table.entry_del(
                target,
                [meter_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', mac_addrs[x])])])


def lpfGetTestHelper(self, num_entries, table, key_fields, action_name):
    target = client.Target(device_id=0, pipe_id=0xffff)

    gain_time = [float(random.randint(1, 200) * 1000) for x in range(num_entries)]
    # decay_time = [random.randint(50, 100)*1000 for x in range(num_entries)]
    decay_time = gain_time
    out_scale = [random.randint(1, 10) for x in range(num_entries)]

    logger.info("Inserting %d entries to the lpf table", num_entries)
    data_list = []
    for x in range(num_entries):
        data_list.append(table.make_data(
            [client.DataTuple('$LPF_SPEC_TYPE', str_val="RATE"),
             client.DataTuple('$LPF_SPEC_GAIN_TIME_CONSTANT_NS', float_val=gain_time[x]),
             client.DataTuple('$LPF_SPEC_DECAY_TIME_CONSTANT_NS', float_val=decay_time[x]),
             client.DataTuple('$LPF_SPEC_OUT_SCALE_DOWN_FACTOR', out_scale[x])],
            action_name
        ))
    table.entry_add(target, key_fields, data_list)

    for x in range(num_entries):
        resp = table.entry_get(
            target,
            [key_fields[x]],
            {"from_hw": False})

        data_dict = next(resp)[0].to_dict()
        assert (data_dict["$LPF_SPEC_TYPE"] == "RATE")
        assert (data_dict["$LPF_SPEC_GAIN_TIME_CONSTANT_NS"] <= gain_time[x])
        assert (data_dict["$LPF_SPEC_DECAY_TIME_CONSTANT_NS"] <= decay_time[x])
        assert (data_dict["$LPF_SPEC_OUT_SCALE_DOWN_FACTOR"] == out_scale[x])


class DirectLPFTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        num_entries = random.randint(1, 100)
        lpf_indices = [x + 1 for x in range(num_entries)]
        random.shuffle(lpf_indices)
        key_dict = {}
        ip_addrs = []

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        lpf_match_table = bfrt_info.table_get("SwitchIngress.direct_lpf_match_tbl")
        lpf_match_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

        for i in range(num_entries):
            ip_addr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            while ip_addr in key_dict:
                ip_addr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ip_addrs.append(ip_addr)
            key_dict[ip_addr] = True

        key_field_list = []
        for x in range(num_entries):
            key_field_list.append(lpf_match_table.make_key(
                [client.KeyTuple('hdr.ipv4.dst_addr', ip_addrs[x])]))

        lpfGetTestHelper(self, num_entries, lpf_match_table,
                         key_field_list, "SwitchIngress.set_rate_direct")
        logger.info("Deleting %d entries from table SwitchIngress.direct_lpf_match_tbl", num_entries, )
        for x in range(num_entries):
            lpf_match_table.entry_del(
                target,
                [key_field_list[x]])


class IndirectLPFTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        num_entries = random.randint(1, 100)
        lpf_indices = [x + 1 for x in range(num_entries)]
        random.shuffle(lpf_indices)
        key_dict = {}
        ip_addrs = []

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        lpf_match_table = bfrt_info.table_get("SwitchIngress.lpf_match_tbl")
        lpf_match_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        lpf_table = bfrt_info.table_get("SwitchIngress.simple_lpf")

        for i in range(num_entries):
            ip_addr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            while ip_addr in key_dict:
                ip_addr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ip_addrs.append(ip_addr)
            key_dict[ip_addr] = True

        logger.info("Inserting %d entries to the match table with random LPF indices", num_entries)
        for x in range(num_entries):
            lpf_match_table.entry_add(
                target,
                [lpf_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                [lpf_match_table.make_data(
                    [client.DataTuple('lpf_idx', lpf_indices[x])],
                    'SwitchIngress.set_rate')]
            )

        for x in range(num_entries):
            resp = lpf_match_table.entry_get(
                target,
                [lpf_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                {"from_hw": False})

            data_dict = next(resp)[0].to_dict()
            assert data_dict["lpf_idx"] == lpf_indices[x]

        key_field_list = []
        for x in range(num_entries):
            key_field_list.append(lpf_table.make_key(
                [client.KeyTuple('$LPF_INDEX', lpf_indices[x])]))

        lpfGetTestHelper(self, num_entries, lpf_table,
                         key_field_list, None)

        for x in range(num_entries):
            lpf_match_table.entry_del(
                target,
                [lpf_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])])


class IndirectWREDTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        num_entries = random.randint(1, 100)
        wred_indices = [x + 1 for x in range(num_entries)]
        random.shuffle(wred_indices)
        key_dict = {}
        ip_addrs = []

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        wred_match_table = bfrt_info.table_get("SwitchIngress.wred_match_tbl")
        wred_match_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")

        for i in range(num_entries):
            ip_addr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            while ip_addr in key_dict:
                ip_addr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ip_addrs.append(ip_addr)
            key_dict[ip_addr] = True

        logger.info("Inserting %d entries to the match table with random WRED indices", num_entries)
        for x in range(num_entries):
            wred_match_table.entry_add(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                [wred_match_table.make_data(
                    [client.DataTuple('wred_idx', wred_indices[x])],
                    'SwitchIngress.mark_wred')]
            )

        for x in range(num_entries):
            resp = wred_match_table.entry_get(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                {"from_hw": False})

            data_dict = next(resp)[0].to_dict()

            assert data_dict["wred_idx"] == wred_indices[x]

        for x in range(num_entries):
            wred_match_table.entry_del(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])])


class DirectWREDTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        num_entries = random.randint(1, 500)
        wred_indices = [x + 1 for x in range(num_entries)]
        random.shuffle(wred_indices)
        key_dict = {}
        ip_addrs = []

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        wred_match_table = bfrt_info.table_get("SwitchIngress.wred_direct_match_tbl")
        wred_match_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")

        time_constant = [round(random.uniform(1, 1000), 2) for x in range(num_entries)]
        min_thresh_cells = [random.randint(20, 500) for x in range(num_entries)]
        max_thresh_cells = [min_thresh_cells[x] + random.randint(50, 200) for x in range(num_entries)]
        max_probability = [round(random.uniform(0.1, 1), 2) for x in range(num_entries)]

        for i in range(num_entries):
            ip_addr = "%d.%d.%d.%d" % (
                random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            while ip_addr in key_dict:
                ip_addr = "%d.%d.%d.%d" % (
                    random.randint(1, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            ip_addrs.append(ip_addr)
            key_dict[ip_addr] = True

        logger.info("Inserting %d entries to the match table with random WRED SPEC", num_entries)
        for x in range(num_entries):
            wred_match_table.entry_add(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                [wred_match_table.make_data(
                    [client.DataTuple('$WRED_SPEC_TIME_CONSTANT_NS', float_val=time_constant[x]),
                     client.DataTuple('$WRED_SPEC_MIN_THRESH_CELLS', min_thresh_cells[x]),
                     client.DataTuple('$WRED_SPEC_MAX_THRESH_CELLS', max_thresh_cells[x]),
                     client.DataTuple('$WRED_SPEC_MAX_PROBABILITY', float_val=max_probability[x])],
                    'SwitchIngress.mark_direct_wred')]
            )

        logger.info("Reading %d entries of the match table verify WRED SPEC", num_entries)
        for x in range(num_entries):
            resp = wred_match_table.entry_get(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])],
                {"from_hw": False})
            data_dict = next(resp)[0].to_dict()

            assert ((data_dict["$WRED_SPEC_TIME_CONSTANT_NS"] - time_constant[x]) <= time_constant[x] * 0.02)

            assert ((data_dict["$WRED_SPEC_MIN_THRESH_CELLS"] - min_thresh_cells[x]) <= min_thresh_cells[x] * 0.02)

            assert ((data_dict["$WRED_SPEC_MAX_THRESH_CELLS"] - max_thresh_cells[x]) <= max_thresh_cells[x] * 0.02)
            # The max drop probability is given a higher error tolerance. The percentage should be within +-10 of expected
            assert (abs((data_dict["$WRED_SPEC_MAX_PROBABILITY"] * 100 - max_probability[x] * 100)) <= 10)

        for x in range(num_entries):
            wred_match_table.entry_del(
                target,
                [wred_match_table.make_key(
                    [client.KeyTuple('hdr.ipv4.src_addr', ip_addrs[x])])])


class IndirectMeterIteratorTest(BfRuntimeTest):
    '''@brief
    This test does the following
    1. Adds 500 meter table entries with random data
    2. Gets all the entries from the WRED table
    3. Verifies the read values match what has been programmed
    '''

    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def getMeterData(self, num_entries):
        meter_data = {}
        meter_data['cir'] = [1000 * random.randint(1, 1000) for i in range(num_entries)]
        meter_data['pir'] = [meter_data['cir'][i] * random.randint(1, 5) for i in range(num_entries)]
        meter_data['cbs'] = [1000 * random.randint(1, 100) for i in range(num_entries)]
        meter_data['pbs'] = [meter_data['cbs'][i] * random.randint(1, 5) for i in range(num_entries)]
        return meter_data

    def runTest(self):
        target = client.Target(device_id=0, pipe_id=0xffff)
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % (seed))
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        meter_table = bfrt_info.table_get("SwitchIngress.meter")

        num_entries = 500
        meter_data = self.getMeterData(num_entries)

        logger.info("Adding %d entries to the Meter table", num_entries)
        for x in range(num_entries):
            meter_table.entry_add(
                target,
                [meter_table.make_key(
                    [client.KeyTuple('$METER_INDEX', x)])],
                [meter_table.make_data(
                    [client.DataTuple('$METER_SPEC_CIR_KBPS', meter_data['cir'][x]),
                     client.DataTuple('$METER_SPEC_PIR_KBPS', meter_data['pir'][x]),
                     client.DataTuple('$METER_SPEC_CBS_KBITS', meter_data['cbs'][x]),
                     client.DataTuple('$METER_SPEC_PBS_KBITS', meter_data['pbs'][x])])])

        logger.info("Reading %d entries from the Meter table to verify", num_entries)

        resp = meter_table.entry_get(target, None, {"from_hw": False})

        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            recv_cir = data_dict["$METER_SPEC_CIR_KBPS"]
            recv_pir = data_dict["$METER_SPEC_PIR_KBPS"]
            recv_cbs = data_dict["$METER_SPEC_CBS_KBITS"]
            recv_pbs = data_dict["$METER_SPEC_PBS_KBITS"]

            # Read back meter values are not always the same. It should be within a 2% error rate

            if abs(recv_cir - meter_data['cir'][i]) > meter_data['cir'][i] * 0.02:
                assert 0
            if abs(recv_pir - meter_data['pir'][i]) > meter_data['pir'][i] * 0.02:
                assert 0
            if abs(recv_cbs - meter_data['cbs'][i]) > meter_data['cbs'][i] * 0.02:
                assert 0
            if abs(recv_pbs - meter_data['pbs'][i]) > meter_data['pbs'][i] * 0.02:
                assert 0

            assert key_dict["$METER_INDEX"]['value'] == i
            i += 1
        logger.info("Deleting entries in the meter table")
        meter_table.entry_del(target)
        logger.info("Verifying that all entries were reset in the meter table")
        # Now get all the entries and check whether they were
        # reset to the defaults or not
        # Rate default values,i.e, for CIR and PIR depends upon clock speed
        # and hence checks are only enabled for model. Also, the clock speed
        # is different for tofino and tofino2 being 1.27 and 1.0 Ghz respectively
        # We come up with these values with reverse calculation from the
        # mantissa and exponent restrictions.
        resp = meter_table.entry_get(target, None, {"from_hw": False})
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            if testutils.test_param_get("target") is not "hw":
                if g_is_tofino:
                    self.assertEqual(data_dict["$METER_SPEC_CIR_KBPS"], 5195848221)
                    self.assertEqual(data_dict["$METER_SPEC_PIR_KBPS"], 5195848221)
                elif g_is_tofino2:
                    self.assertEqual(data_dict["$METER_SPEC_CIR_KBPS"], 4087999890)
                    self.assertEqual(data_dict["$METER_SPEC_PIR_KBPS"], 4087999890)
            self.assertEqual(data_dict["$METER_SPEC_CBS_KBITS"], 4380866642)
            self.assertEqual(data_dict["$METER_SPEC_PBS_KBITS"], 4380866642)

        logger.info("All entries matched")


class IndirectLPFIteratorTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        This test does the following
        1. Adds 500 LPF table entries with random data
        2. Gets all the entries from the LPF table
        3. Verifies the read values match what has been programmed
        '''
        target = client.Target(device_id=0, pipe_id=0xffff)
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % (seed))
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        lpf_match_table = bfrt_info.table_get("SwitchIngress.lpf_match_tbl")
        lpf_match_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
        lpf_table = bfrt_info.table_get("SwitchIngress.simple_lpf")
        num_entries = 500

        lpf_types = [random.choice(["RATE", "SAMPLE"]) for x in range(num_entries)]

        gain_time = [float(round(random.uniform(1, 1000), 2)) for x in range(num_entries)]
        decay_time = gain_time
        out_scale = [random.randint(1, 31) for x in range(num_entries)]
        logger.info("Adding %d entries to the LPF table", num_entries)
        for x in range(num_entries):
            lpf_table.entry_add(
                target,
                [lpf_table.make_key([client.KeyTuple('$LPF_INDEX', x)])],
                [lpf_table.make_data(
                    [client.DataTuple('$LPF_SPEC_TYPE', str_val=lpf_types[x]),
                     client.DataTuple('$LPF_SPEC_GAIN_TIME_CONSTANT_NS', float_val=gain_time[x]),
                     client.DataTuple('$LPF_SPEC_DECAY_TIME_CONSTANT_NS', float_val=decay_time[x]),
                     client.DataTuple('$LPF_SPEC_OUT_SCALE_DOWN_FACTOR', out_scale[x])])]
            )

        logger.info("Reading %d entries from the LPF table to verify", num_entries)

        resp = lpf_table.entry_get(target, None, {"from_hw": False})

        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            # Read back lpf values are not always the same. It should be within a 2% error rate
            assert (data_dict["$LPF_SPEC_TYPE"] == lpf_types[i])
            assert ((data_dict["$LPF_SPEC_GAIN_TIME_CONSTANT_NS"] - gain_time[i]) <= gain_time[i] * 0.02)
            assert ((data_dict["$LPF_SPEC_DECAY_TIME_CONSTANT_NS"] - decay_time[i]) <= decay_time[i] * 0.02)
            assert (data_dict["$LPF_SPEC_OUT_SCALE_DOWN_FACTOR"] == out_scale[i])
            assert key_dict["$LPF_INDEX"]['value'] == i
            i += 1

        logger.info("Deleting all entries in the lpf table")
        lpf_table.entry_del(target)
        logger.info("Verifying that all entries were reset in the lpf table")
        # Now get all the entries and check whether they were
        # reset to the defaults or not
        resp = lpf_table.entry_get(target, None, {"from_hw": False})
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            # Read back lpf values are not always the same. It should be minimum
            # Default spec type is SAMPLE
            assert (data_dict["$LPF_SPEC_TYPE"] == "SAMPLE")
            assert ((data_dict["$LPF_SPEC_GAIN_TIME_CONSTANT_NS"]) <= 1)
            assert ((data_dict["$LPF_SPEC_DECAY_TIME_CONSTANT_NS"]) <= 1)
            assert (data_dict["$LPF_SPEC_OUT_SCALE_DOWN_FACTOR"] == 0)
        logger.info("All entries matched")


class IndirectWREDIteratorTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_meter_lpf_wred"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        '''
        This test does the following
        1. Adds 500 WRED table entries with random data
        2. Gets all the entries from the WRED table
        3. Verifies the read values match what has been programmed
        '''
        target = client.Target(device_id=0, pipe_id=0xffff)
        seed = random.randint(1, 65535)
        logger.info("Seed used %d" % seed)
        random.seed(seed)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_meter_lpf_wred")
        wred_table = bfrt_info.table_get("SwitchIngress.simple_wred")

        num_entries = 500

        time_constant = [round(random.uniform(1, 1000), 2) for x in range(num_entries)]
        min_thresh_cells = [random.randint(20, 500) for x in range(num_entries)]
        max_thresh_cells = [min_thresh_cells[x] + random.randint(50, 200) for x in range(num_entries)]
        max_probability = [round(random.uniform(0.1, 1), 2) for x in range(num_entries)]

        logger.info("Adding %d entries to the WRED table", num_entries)
        for x in range(num_entries):
            wred_table.entry_add(
                target,
                [wred_table.make_key(
                    [client.KeyTuple('$WRED_INDEX', x)])],
                [wred_table.make_data(
                    [client.DataTuple('$WRED_SPEC_TIME_CONSTANT_NS', float_val=time_constant[x]),
                     client.DataTuple('$WRED_SPEC_MIN_THRESH_CELLS', min_thresh_cells[x]),
                     client.DataTuple('$WRED_SPEC_MAX_THRESH_CELLS', max_thresh_cells[x]),
                     client.DataTuple('$WRED_SPEC_MAX_PROBABILITY', float_val=max_probability[x])])]
            )

        logger.info("Reading %d entries from the WRED table to verify", num_entries)

        resp = wred_table.entry_get(target, None, {"from_hw": False})

        i = 0
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            # Read back lpf values are not always the same. It should be within a 2% error rate
            assert ((data_dict["$WRED_SPEC_TIME_CONSTANT_NS"] - time_constant[i]) <= time_constant[i] * 0.02)

            assert ((data_dict["$WRED_SPEC_MIN_THRESH_CELLS"] - min_thresh_cells[i]) <= min_thresh_cells[i] * 0.02)

            assert ((data_dict["$WRED_SPEC_MAX_THRESH_CELLS"] - max_thresh_cells[i]) <= max_thresh_cells[i] * 0.02)
            # The max drop probability is given a higher error tolerance. The percentage should be within +-10 of expected
            assert (abs((data_dict["$WRED_SPEC_MAX_PROBABILITY"] * 100 - max_probability[i] * 100)) <= 10)
            assert key_dict["$WRED_INDEX"]['value'] == i
            #logger.info("WRED entry id %d matched", i)
            i += 1
        logger.info("Clearing all entries in the wred table")
        wred_table.entry_del(target)
        logger.info("Verifying that all entries were reset in the wred table")
        # Now get all the entries and check whether they were
        # reset to the defaults or not
        resp = wred_table.entry_get(target, None, {"from_hw": False})
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            assert (data_dict["$WRED_SPEC_TIME_CONSTANT_NS"] <= 1)
            assert (hex(data_dict["$WRED_SPEC_MIN_THRESH_CELLS"]) == "0xff000000")
            assert (hex(data_dict["$WRED_SPEC_MAX_THRESH_CELLS"]) == "0xff000000")
            # The max drop probability is given a higher error tolerance. The percentage should be within +-10 of expected
            assert (abs(data_dict["$WRED_SPEC_MAX_PROBABILITY"]) == 0)

        logger.info("All entries matched")
