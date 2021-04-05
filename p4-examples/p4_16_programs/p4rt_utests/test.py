# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


from p4.v1 import p4runtime_pb2
from p4runtime_base_tests import P4RuntimeTest, autocleanup
from p4runtime_base_tests import ipv4_to_binary, stringify

from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client


def get_one_meter_config():
    meter_config = p4runtime_pb2.MeterConfig()
    meter_config.cir = 10 ** 5
    meter_config.cburst = 10 ** 4
    meter_config.pir = 10 ** 6
    meter_config.pburst = 10 ** 4
    return meter_config


class P4RuntimeUTest(P4RuntimeTest):
    """@brief Base test class for all p4rt_utests PTF tests.
    """

    def add_to_t_mtr_0(self, ig_port, eg_port, meter_config=None):
        req, _ = self.send_request_add_entry_to_action(
            "t_mtr_0",
            [self.Exact("sm.ingress_port", stringify(ig_port, 2))],
            "t_mtr_0_send", [("port", stringify(eg_port, 2))],
            meter_config=meter_config)
        return req.updates[0].entity

    def add_to_t_mtr_1(self, ig_port, a_name, eg_port, **kwargs):
        params = [("port", stringify(eg_port, 2))]
        for k, v in list(kwargs.items()):
            params += [(k, v)]
        req, _ = self.send_request_add_entry_to_action(
            "t_mtr_1",
            [self.Exact("sm.ingress_port", stringify(ig_port, 2))],
            a_name, params)
        return req.updates[0].entity

    def add_member_to_action_sel(self, mbr_id, eg_port):
        req, _ = self.send_request_add_member(
            "action_sel", mbr_id, "send", [("port", stringify(eg_port, 2))])
        return req.updates[0].entity

    def add_group_to_action_sel(self, grp_id, mbrs):
        req, _ = self.send_request_add_group("action_sel", grp_id, mbr_ids=mbrs)
        return req.updates[0].entity


class ReadMatchEntry(P4RuntimeUTest):
    """@brief Tests reading single entries (by setting the match key in the
    request) and all entries (using a wildcard request)."""

    @autocleanup
    def runTest(self):
        ig_port = self.swports(1)
        eg_port = ig_port
        entity = self.add_to_t_mtr_0(ig_port, eg_port)

        # read single entry
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        wildcard_entity = p4runtime_pb2.Entity()
        wildcard_entity.table_entry.table_id = entity.table_entry.table_id
        # wildcard read -> one entry
        rentity = self.read_one(wildcard_entity)
        self.assertProtoEqual(rentity, entity)

        # add second entry
        ig_port = self.swports(2)
        eg_port = ig_port
        entity2 = self.add_to_t_mtr_0(ig_port, eg_port)

        # wildcard read -> 2 entries
        rit = self.read(wildcard_entity)
        try:
            rentity1 = next(rit)
            rentity2 = next(rit)
        except StopIteration:
            self.fail(
                "Insufficient number of entries returned by wildcard read")

        # we make no assumptions regarding the order in which entries are
        # returned
        self.assertIn(entity, [rentity1, rentity2])
        self.assertIn(entity2, [rentity1, rentity2])


class ReadMatchEntryAndTestBfruntimeFixedInParallel(P4RuntimeUTest):
    """@brief Tests reading an entry from the P4 program over p4runtime.
    Also tests adding and reading from a non-P4 table over bfruntime in
    parallel"""
    class BriInterface(BfRuntimeTest):
        def __init__(self, p4_name):
            BfRuntimeTest.__init__(self)
            self.client_id = 0
            self.p4_name = p4_name
            BfRuntimeTest.setUp(self, self.client_id, self.p4_name)
            self._setup_tables()

        def _setup_tables(self):
            bfrt_info = self.interface.bfrt_info_get()
            # Get all table objects
            self.mgid_table = bfrt_info.table_get("$pre.mgid")
            self.node_table = bfrt_info.table_get("$pre.node")

        def runTest(self):
            pass

    @autocleanup
    def runTest(self):
        ig_port = self.swports(1)
        eg_port = ig_port

        meter_config = get_one_meter_config()

        entity = self.add_to_t_mtr_0(ig_port, eg_port, meter_config)

        # read table entry over p4runtime
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        # Set up Bfruntime interface
        bri = self.BriInterface("$SHARED")
        target = client.Target(device_id=0, pipe_id=0xffff)
        # Add an entry to the node table
        bri.node_table.entry_add(
            target,
            [bri.node_table.make_key([
                client.KeyTuple('$MULTICAST_NODE_ID', 1)])],
            [bri.node_table.make_data([
                client.DataTuple('$MULTICAST_RID', 2),
                client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=[10]),
                client.DataTuple('$DEV_PORT', int_arr_val=[20])])]
        )
        # Add an entry to the MGID table
        key = bri.mgid_table.make_key([client.KeyTuple('$MGID', 1)])
        data = bri.mgid_table.make_data([
            client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[1]),
            client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[True]),
            client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[2])
        ])
        bri.mgid_table.entry_add(target, [key], [data])
        # Read the entry back and ensure that it was the same
        for data_ret, key_ret in bri.mgid_table.entry_get(target, [key]):
            data_ret_dict = data_ret.to_dict()
            data_dict = data.to_dict()

            assert data_ret_dict["$MULTICAST_NODE_ID"] == data_dict["$MULTICAST_NODE_ID"]
            assert data_ret_dict["$MULTICAST_NODE_L1_XID_VALID"] == data_dict["$MULTICAST_NODE_L1_XID_VALID"]
            assert data_ret_dict["$MULTICAST_NODE_L1_XID"] == data_dict["$MULTICAST_NODE_L1_XID"]
        # Delete the entries
        bri.mgid_table.entry_del(target, [key])
        bri.node_table.entry_del(target,
                                 [bri.node_table.make_key([
                                     client.KeyTuple('$MULTICAST_NODE_ID', 1)])]
                                 )
        bri.tearDown()


class ReadMatchEntryWithDirectMeter(P4RuntimeUTest):
    """@brief Tests reading a direct meter config using the 2 different methods
    supported by P4Runtime: 1) by reading the match table entry, and 2) by using
    the DirectMeterEntry entity."""

    @autocleanup
    def runTest(self):
        ig_port = self.swports(1)
        eg_port = ig_port

        meter_config = get_one_meter_config()

        entity = self.add_to_t_mtr_0(ig_port, eg_port, meter_config)

        # Reading the meter spec from pipe_mgr match table manager returns the
        # user-programmed spec (we are not reading from hw), but reading the
        # meter spec from pipe_mgr meter table manager reads the spec from
        # shadow memories and returns "approximate" rates / burst sizes as
        # represented in the hw. This is something we should try to uniformize
        # later at the PI / P4Runtime level.

        # read table entry
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        # also read direct meter directly
        meter_entry = self.read_direct_meter(entity.table_entry)
        self.assertMeterConfigAlmostEqual(meter_entry.config, meter_config)


class ReadDefaultEntry(P4RuntimeUTest):
    """@brief This test reads the default entry for table t_mtr_0, which is set
    in the P4 program. We tell P4Runtime not to request the meter config when
    reading the entry (ReadDefaultEntryWithDirectMeter is a similar test, but
    does request the meter config when reading the default entry)."""

    @autocleanup
    def runTest(self):
        default_entry = self.read_table_entry("t_mtr_0", None, with_meter=False)
        expected_entry = self.make_entry_to_action(
            "t_mtr_0", None, "t_mtr_0_default", [], meter_config=None)
        self.assertProtoEqual(default_entry, expected_entry)


class ReadDefaultEntryWithDirectMeter(P4RuntimeUTest):
    """@brief This test first reads the default entry (including the meter
    config) for table t_mtr_0 as aset in the P4 program. We then set the meter
    config using the DirectMeterEntry entity and check that it was programmed
    correctly by reading the default entry again."""

    @autocleanup
    def runTest(self):
        meter_config = get_one_meter_config()

        # if we set with_meter to True here, it will not return a meter config
        # (meter_config field will be unset in the response). This is expected
        # pipe_mgr behavior: we didn't provide any meter spec yet so when
        # reading the entry "from software", nothing will be returned. This is
        # somewhat lucky because this is the behavior described by the P4Runtime
        # spec (leave meter_config unset if the meter spec is default).
        default_entry = self.read_table_entry("t_mtr_0", None, with_meter=False)
        expected_entry = self.make_entry_to_action(
            "t_mtr_0", None, "t_mtr_0_default", [], meter_config=None)
        self.assertProtoEqual(default_entry, expected_entry)
        self.send_request_write_direct_meter(expected_entry, meter_config)
        default_entry = self.read_table_entry("t_mtr_0", None, with_meter=True)
        expected_entry = self.make_entry_to_action(
            "t_mtr_0", None, "t_mtr_0_default", [], meter_config=meter_config)
        self.assertProtoEqual(default_entry, expected_entry)


class ReadEntryWithIndirectMeter(P4RuntimeUTest):
    """@brief Tests reading a match-table entry to which an indirect meter entry
    is attached. We test 3 possible cases (using different P4 actions) to ensure
    that the PI implementation is throughly tested."""

    @autocleanup
    def runTest(self):
        # same action parameter for port and index
        ig_port = self.swports(1)
        eg_port = ig_port
        entity = self.add_to_t_mtr_1(ig_port, "t_mtr_1_send", eg_port)
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        # different action parameters for port and index
        ig_port = self.swports(2)
        eg_port = ig_port
        meter_index = 10
        entity = self.add_to_t_mtr_1(
            ig_port, "t_mtr_1_send_with_index",
            eg_port, index=stringify(meter_index, 4))
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        # constant index
        ig_port = self.swports(3)
        eg_port = ig_port
        entity = self.add_to_t_mtr_1(
            ig_port, "t_mtr_1_send_constant_index", eg_port)
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)


class ReadActionProfileMember(P4RuntimeUTest):
    """@brief Tests reading a single action profile member (by setting the
    member id in the request) and all members (using a wildcard request)."""

    @autocleanup
    def runTest(self):
        ig_port = self.swports(1)
        eg_port = ig_port
        mbr_id = ig_port
        entity = self.add_member_to_action_sel(mbr_id, eg_port)
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        wildcard_entity = p4runtime_pb2.Entity()
        act_prof_id = entity.action_profile_member.action_profile_id
        wildcard_entity.action_profile_member.action_profile_id = act_prof_id
        # wildcard read -> one member
        rentity = self.read_one(wildcard_entity)
        self.assertProtoEqual(rentity, entity)

        # add second member
        ig_port = self.swports(2)
        eg_port = ig_port
        mbr_id = ig_port
        entity2 = self.add_member_to_action_sel(mbr_id, eg_port)

        # wildcard read -> 2 members
        rit = self.read(wildcard_entity)
        try:
            rentity1 = next(rit)
            rentity2 = next(rit)
        except StopIteration:
            self.fail(
                "Insufficient number of members returned by wildcard read")

        # we make no assumptions regarding the order in which members are
        # returned
        self.assertIn(entity, [rentity1, rentity2])
        self.assertIn(entity2, [rentity1, rentity2])


class ReadActionProfileGroup(P4RuntimeUTest):
    """@brief Tests reading a single action profile group (by setting the group
    id in the request) and all groups (using a wildcard request)."""

    @autocleanup
    def runTest(self):
        mbrs = []

        def add_member(idx):
            ig_port = self.swports(idx)
            eg_port = ig_port
            mbr_id = ig_port
            self.add_member_to_action_sel(mbr_id, eg_port)
            mbrs.append(mbr_id)

        for i in range(8):
            add_member(i)

        grp_id = 1000
        entity = self.add_group_to_action_sel(grp_id, mbrs=mbrs[1:4])
        rentity = self.read_one(entity)
        self.assertProtoEqual(rentity, entity)

        wildcard_entity = p4runtime_pb2.Entity()
        act_prof_id = entity.action_profile_group.action_profile_id
        wildcard_entity.action_profile_group.action_profile_id = act_prof_id
        # wildcard read -> one group
        rentity = self.read_one(wildcard_entity)
        self.assertProtoEqual(rentity, entity)

        # add second group
        ig_port = self.swports(2)
        grp_id = 2000
        entity2 = self.add_group_to_action_sel(grp_id, mbrs=mbrs[2:7])

        # wildcard read -> 2 groups
        rit = self.read(wildcard_entity)
        try:
            rentity1 = next(rit)
            rentity2 = next(rit)
        except StopIteration:
            self.fail(
                "Insufficient number of groups returned by wildcard read")

        # we make no assumptions regarding the order in which groups are
        # returned
        self.assertIn(entity, [rentity1, rentity2])
        self.assertIn(entity2, [rentity1, rentity2])


# This is copied from the @autocleanup code. It makes it easier to debug issues
# with default entry reset.
class ResetDefaultEntry(P4RuntimeUTest):
    """@brief Resets the default table entry for all the match tables (for which
    it is a valid operation to reset the default entry) in the P4 program."""

    def runTest(self):
        for (obj_type, name), obj in list(self.p4info_obj_map.items()):
            if obj_type != "tables":
                continue
            # skip tables with a const default entry and indirect tables
            if obj.const_default_action_id != 0 or obj.implementation_id != 0:
                continue
            self.send_request_reset_default_entry(name)
