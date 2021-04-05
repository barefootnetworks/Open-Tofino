# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import time

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client

g_arch        = testutils.test_param_get("arch").lower()
g_is_tofino   = ( g_arch == "tofino" )
g_is_tofino2  = ( g_arch == "tofino2" )

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

class SnapshotTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_snapshot"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]
        dmac = '11:22:33:44:55:66'
        sip = "1.2.3.4"
        dip = "5.6.7.8"
        start_stage = 0
        end_stage = 2
        snap_enable = True
        ethernet_valid = 1

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_snapshot")
        snapshot_ig_liveness_table = bfrt_info.table_get("snapshot.ingress_liveness")
        snapshot_cfg_table = bfrt_info.table_get("snapshot.cfg")
        snapshot_ig_trig_table = bfrt_info.table_get("snapshot.ingress_trigger")
        snapshot_ig_data_table = bfrt_info.table_get("snapshot.ingress_data")
        snapshot_phv_table = bfrt_info.table_get("snapshot.phv")
        forward_table = bfrt_info.table_get("forward")
        forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        snapshot_ig_trig_table.info.data_field_annotation_add("trig.hdr.ethernet.dst_addr", None, "mac")
        snapshot_ig_trig_table.info.data_field_annotation_add("trig.hdr.ethernet.dst_addr_mask", None, "mac")
        snapshot_ig_data_table.info.data_field_annotation_add("hdr.ethernet.dst_addr", None, "mac")
        snapshot_ig_trig_table.info.data_field_annotation_add("trig.hdr.ipv4.src_addr", None, "ipv4")
        snapshot_ig_trig_table.info.data_field_annotation_add("trig.hdr.ipv4.src_addr_mask", None, "ipv4")
        snapshot_ig_data_table.info.data_field_annotation_add("hdr.ipv4.src_addr", None, "ipv4")

        pkt = testutils.simple_tcp_packet(eth_dst=dmac, ip_dst=dip, ip_src=sip)
        exp_pkt = pkt

        # Snapshot can be programmed on all stages, but data must be fetched from
        # specific pipe
        target = client.Target(device_id=0, pipe_id=0xffff, direction=0)
        target_0 = client.Target(device_id=0, pipe_id=0, direction=0)

        logger.info("Check for snapshot trigger field scope")
        scope_resp = snapshot_ig_liveness_table.entry_get(
                        target_0,
                        [snapshot_ig_liveness_table.make_key(
                        [client.KeyTuple('field_name', 'hdr.ipv4.src_addr')])])

        logger.info("Parsing snapshot field scope response")
        field_stage_validated = 0
        # Go over the response data
        for data,key in scope_resp:
            data_dict = data.to_dict()
            print(data_dict)
            valid_stages = data_dict["valid_stages"]
            logger.info("Valid Stages : %s", str(valid_stages))
            # Make sure start and end stage exists in the returned stage list
            for stage_val in valid_stages:
                if stage_val == start_stage or stage_val == end_stage:
                    field_stage_validated += 1
        # Field should be present in both start and end stage
        assert field_stage_validated == 2
        logger.info("-- Snapshot field scope validated --")

        logger.info("Inserting fwding entry")
        forward_table.entry_add(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
            [forward_table.make_data([client.DataTuple('port', eg_port)],
            'SwitchIngress.hit')]
        )

        logger.info("Setting up the snapshot config table")
        # ipv4 src addr field is a mocha phv on tofino2, skip it
        snapshot_cfg_key = snapshot_cfg_table.make_key([
             client.KeyTuple('start_stage', start_stage),
             client.KeyTuple('end_stage', end_stage)])

        snapshot_cfg_data = snapshot_cfg_table.make_data([
             client.DataTuple('thread', str_val="INGRESS")])

        snapshot_cfg_table.entry_add(
            target,
            [snapshot_cfg_key],
            [snapshot_cfg_data])

        logger.info("Setting up the snapshot trigger table")
        # Trigger table key stage must match first stage of config table entry
        snapshot_trig_key = snapshot_ig_trig_table.make_key([
             client.KeyTuple('stage', start_stage)])

        if g_is_tofino2:
            snapshot_trig_data = snapshot_ig_trig_table.make_data([
                client.DataTuple('enable', bool_val=True),
                client.DataTuple('trig.hdr.ethernet.$valid', 1),
                client.DataTuple('trig.hdr.ethernet.$valid_mask', 1),
                client.DataTuple('trig.hdr.ethernet.dst_addr', dmac),
                client.DataTuple('trig.hdr.ethernet.dst_addr_mask', dmac)])
        else:
            snapshot_trig_data = snapshot_ig_trig_table.make_data([
                client.DataTuple('enable', bool_val=True),
                client.DataTuple('trig.hdr.ethernet.$valid', 1),
                client.DataTuple('trig.hdr.ethernet.$valid_mask', 1),
                client.DataTuple('trig.hdr.ethernet.dst_addr', dmac),
                client.DataTuple('trig.hdr.ethernet.dst_addr_mask', dmac),
                client.DataTuple('trig.hdr.ipv4.src_addr', sip),
                client.DataTuple('trig.hdr.ipv4.src_addr_mask', sip)])

        snapshot_ig_trig_table.entry_add(
             target,
             [snapshot_trig_key],
             [snapshot_trig_data])

        time.sleep(2)
        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Expecting packet on port %d", eg_port)
        testutils.verify_packets(self, exp_pkt, [eg_port])
        time.sleep(1)

        # Check snapshot capture state on config and trigger table
        logger.info("Verify snapshot status in snapshot config table")
        resp = snapshot_cfg_table.entry_get(
                target,
                [snapshot_cfg_key])
        for data, key in resp:
            # Get Snapshot enable state
            assert key == snapshot_cfg_key
            data_dict = data.to_dict()
            # Check ingress capture lists, expected pipe is 0
            pipe_num = 0
            assert pipe_num in data_dict['ingress_capture']

        logger.info("Verify snapshot status in snapshot trigger table")
        resp = snapshot_ig_trig_table.entry_get(
                target,
                [snapshot_trig_key])
        for data, key in resp:
            # Get Snapshot enable state
            assert key == snapshot_trig_key
            data_dict = data.to_dict()
            # Capture status on pipe 0 shoud be 'FULL'
            pipe_num = 0
            assert 'FULL' == data_dict['trigger_state'][pipe_num]

        logger.info("Getting the captured snapshot data")
        # Data table must be fetched per stage on specific pipe
        for stage in range(end_stage + 1):
            snapshot_data_key = snapshot_ig_data_table.make_key([
                client.KeyTuple('stage', stage)])

            resp = snapshot_ig_data_table.entry_get(
                        target_0,
                        [snapshot_data_key])

            logger.info("Parsing snapshot data on stage {}".format(stage))
            # Iterate over the response data
            for data, key in resp:
                assert key == snapshot_data_key

                data_dict = data.to_dict()

                logger.info("Field-info")
                # Go over the field-info container items
                assert dmac == data_dict["hdr.ethernet.dst_addr"]
                logger.info("  Verified captured destination mac in stage %d", stage)
                assert sip == data_dict["hdr.ipv4.src_addr"]
                logger.info("  Verified captured ipv4 src addr in stage %d", stage)
                assert 1 == data_dict["hdr.ipv4.$valid"]
                logger.info("  Verified captured ipv4_valid bit in stage %d", stage)

                # Control info container list
                logger.info("\n\nControl-info:")
                # Table info is again a list of containers hence taking 0th index
                tbl_name = data_dict["table_info"][0]["table_name"]
                logger.info("Table found : %s", tbl_name)
                table_hit = data_dict["table_info"][0]["table_hit"]
                logger.info("%s was %s", tbl_name, "hit" if table_hit else "miss")
                # Only forward table should be hit
                assert table_hit if tbl_name == "SwitchIngress_forward" else not table_hit

                # Verify that local stage trigger is only true for start stage
                local_stage_trigger = data_dict["local_stage_trigger"]
                assert local_stage_trigger if stage == start_stage else not local_stage_trigger

                # Verify that previous stage trigger is true for all except start stage
                prev_stage_trigger = data_dict["prev_stage_trigger"]
                assert not prev_stage_trigger if stage == start_stage else prev_stage_trigger

                # Verify next table name
                next_table_name = data_dict["next_table_name"]
                if stage == 0 and g_is_tofino:
                    expected_next_table_name = "tbl_tna_snapshot144"
                elif stage == 1 and g_is_tofino:
                    expected_next_table_name = "SwitchIngress_ipRoute"
                else:
                    expected_next_table_name = "NO_NEXT_TABLE"
                print(next_table_name)
                assert (next_table_name == expected_next_table_name)

            logger.info("-- Snapshot capture field values validated --")

        # Get and validate Snapshot PHV table data
        logger.info("\nGetting the captured raw PHV data")
        # We should get snapshot data for stages 0, 1, 2
        stages = [0, 1, 2]
        resp = snapshot_phv_table.entry_get(
                                    target_0,
                                    [])

        # Iterate over the response data
        # Data depends on how compiler allocates fields to PHV containers.
        # Just validate that data is present.
        for data, key in resp:
            key_dict = key.to_dict()
            data_dict = data.to_dict()
            stages.remove(key_dict['stage']['value'])
            assert len(data_dict) > 3, "Received entry with no valid PHV containers"
        assert len(stages) == 0, "Not received all programmed stages"


        # It is possible to request only specific PHV containers
        stages = [0, 1, 2]
        resp = snapshot_phv_table.entry_get(
                                    target_0,
                                    [],
                                    {"from_hw":True},
                                    snapshot_phv_table.make_data(
                                        [client.DataTuple('phv0', 0),
                                         client.DataTuple('phv1', 0)]))

        # In this case data dict should contain 2 PHVs,
        # is_default_entry and action_name fields.
        for data, key in resp:
            key_dict = key.to_dict()
            data_dict = data.to_dict()
            stages.remove(key_dict['stage']['value'])
            assert len(data_dict) < 5, "Received more data than requested"
            data_exp = ['is_default_entry', 'action_name', 'phv0', 'phv1']
            for data_key in data_dict.keys():
                data_exp.remove(data_key)
            assert len(data_exp) == 0, "Not received all expected data"
        assert len(stages) == 0, "Not received all programmed stages"

        logger.info("-- Snapshot raw PHV values validated --")

        logger.info("\nDeleting table entry")
        forward_table.entry_del(
            target,
            [forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])
        logger.info("Deleting the snapshot")
        # Snapshot triggers must be cleared before snapshot instance can be deleted
        snapshot_ig_trig_table.entry_del(
            target,
            [])
        snapshot_cfg_table.entry_del(
            target,
            [])

        logger.info("Check if entry was deleted in clear call")
        resp = snapshot_cfg_table.entry_get(target_0, [snapshot_cfg_key])
        try:
            # If entry is not there exception will be raised on following line
            for data, key in resp:
                print("Stub print")
        except Exception:
            print("Entry deleted - PASS")
        else:
            raise AssertionError("Entry not deleted")
