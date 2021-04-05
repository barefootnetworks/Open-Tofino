# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

dev_id = 0
p4_program_name = "tna_action_selector"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(11))


class ActionProfileSelectorGetTest(BfRuntimeTest):
    """@brief Populate the selector table and the action profile, read
    the entries and verify that they are equal.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = swports[2:]

        target = gc.Target(device_id=dev_id, pipe_id=0xffff)

        # Add the new member to the selection table.
        pkt = testutils.simple_tcp_packet()
        exp_pkt = pkt
        max_grp_size = 200

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        sel_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector")
        action_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector_ap")

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)
        num_entries = random.randint(5, 15)

        # Construct input for selector table
        # This list contains dictionaries for each entry
        # dict(grp_id -> dict(act_member -> mem_status) )
        mem_dict_dict = {}
        for j in range(num_entries):
            member_list_size = 5
            # temp list from the eg_ports tuple and shuffling it
            temp_eg_ports = list(eg_ports)
            random.shuffle(temp_eg_ports)
            # making members and member_statuses
            members = [x for x in temp_eg_ports[:member_list_size]]
            member_status = [True] * member_list_size
            # Making 2 of them as false
            member_status[0] = member_status[1] = False
            random.shuffle(member_status)
            mem_dict = {members[i]: member_status[i]
                        for i in range(0, len(members))}
            mem_dict_dict[j + 1] = mem_dict

        for port in eg_ports:
            # Create a new member for each port with the port number as the id.
            action_table.entry_add(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', port)])],
                [action_table.make_data([gc.DataTuple('port', port)],
                                        'SwitchIngress.hit')])

        # Add the new member to the selection table.
        # Adding all members at the same time along with the max group size
        for grp_id, mem_dict in mem_dict_dict.items():
            sel_table.entry_add(
                target,
                [sel_table.make_key([
                    gc.KeyTuple('$SELECTOR_GROUP_ID', grp_id)])],
                [sel_table.make_data([
                    gc.DataTuple('$MAX_GROUP_SIZE', max_grp_size),
                    gc.DataTuple('$ACTION_MEMBER_ID',
                                 int_arr_val=list(mem_dict.keys())),
                    gc.DataTuple('$ACTION_MEMBER_STATUS',
                                 bool_arr_val=list(mem_dict.values()))])])

        # Get test on selector table
        try:
            for grp_id, mem_dict in mem_dict_dict.items():
                logger.info("Checking Get for grp_id %d and member_status_list %s",
                            grp_id, str(mem_dict))
                resp = sel_table.entry_get(
                    target,
                    [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                     grp_id)])],
                    {"from_hw": False})
                data_dict = next(resp)[0].to_dict()

                # Create a dict out of the response recvd
                mem_dict_recv = {
                    data_dict["$ACTION_MEMBER_ID"][i]:
                        data_dict["$ACTION_MEMBER_STATUS"][i]
                    for i in range(0, len(data_dict["$ACTION_MEMBER_ID"]))}
                # Both the dictionaries should be equal
                assert mem_dict == mem_dict_recv
        finally:
            # Clearing Selector Table
            logger.info("Clearing selector table")
            sel_table.entry_del(target)

            assert next(sel_table.usage_get(target)) == 0, \
                "usage = %s expected = 0" % (sel_table.usage_get)
            for data, key in sel_table.entry_get(target):
                assert 0, "Not expecting any entries here"

            # Clearing Action profile members
            logger.info("Clearing action table")
            action_table.entry_del(target)

            assert next(action_table.usage_get(target)) == 0, \
                "usage = %s expected = 0" % (action_table.usage_get)
            for data, key in action_table.entry_get(target):
                assert 0, "Not expecting any entries here"


class ActionProfileSelectorTest(BfRuntimeTest):
    """@brief Basic test: populate the forward table, the selector table, and
    the action profile. Send a packet and verify it is received at one of the
    ports selected by the selector table.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        ig_port = swports[1]
        eg_ports = swports[2:6]
        group_id = 1

        target = gc.Target(device_id=dev_id, pipe_id=0xffff)

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        # Add the new member to the selection table.
        pkt = testutils.simple_tcp_packet()
        exp_pkt = pkt
        max_grp_size = 200

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector_ap")
        sel_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector")

        members = []
        member_status = [False] * len(eg_ports)
        exp_ports = []
        # Disable 2 of the ports
        disable = random.sample(list(range(len(eg_ports))), 2)
        for i, port in enumerate(eg_ports):
            # Create a new member for each port with the port number as the id.
            action_table.entry_add(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID',
                                                    port)])],
                [action_table.make_data([gc.DataTuple('port', port)],
                                        'SwitchIngress.hit')])

            members.append(port)
            if i not in disable:
                member_status[i] = True
                exp_ports.append(port)

        # Create a dictionary with the member_status and members
        mem_dict = {members[i]: member_status[i] for i in range(0, len(members))}

        # Add the new member to the selection table.
        # Adding all members at the same time along with the max group size
        sel_table.entry_add(
            target,
            [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID', group_id)])],
            [sel_table.make_data([gc.DataTuple('$MAX_GROUP_SIZE', max_grp_size),
                                  gc.DataTuple('$ACTION_MEMBER_ID', int_arr_val=members),
                                  gc.DataTuple('$ACTION_MEMBER_STATUS',
                                               bool_arr_val=member_status)])])

        forward_table.entry_add(
            target,
            [forward_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port',
                                                 ig_port)])],
            [forward_table.make_data([gc.DataTuple('$SELECTOR_GROUP_ID',
                                                   group_id)])])

        try:
            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on one of enabled ports %s", exp_ports)
            testutils.verify_any_packet_any_port(self, [exp_pkt], exp_ports)
        finally:
            forward_table.entry_del(
                target,
                [forward_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port',
                                                     ig_port)])])

            # Delete Selector Table entry
            logger.info("Deleting Selector group id %d", group_id)
            sel_table.entry_del(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID', group_id)])])

            # Delete Action profile members
            for port in eg_ports:
                logger.info("Deleting Action profile member id %d", port)
                action_table.entry_del(
                    target,
                    [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID',
                                                        port)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)


class ActionProfileSelectorIteratorTest(BfRuntimeTest):
    """@brief Populate the action profile and the selector table, delete random 
    entries from them and verify the remaining entries are the ones that we 
    expect to be still there.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        '''
        This test does the following:
        1. Adds certain number of action profile members
        2. Adds certain number of groups with random action profile members
           chosen, to the selector table (upto a max grp size)
        3. Deletes a random number of random groups.
        4. Does a get all from the selector table
        5. Verifies that the read data matches for each of the remaining groups
        '''
        target = gc.Target(device_id=dev_id, pipe_id=0xffff)

        # Add the new member to the selection table.
        max_grp_size = 7

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector_ap")
        sel_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector")

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

        num_act_prof_entries = 1024
        num_sel_grps = 100

        egress_ports = [random.randint(1, 5) for x in range(num_act_prof_entries)]
        action_mbr_ids = [x for x in range(num_act_prof_entries)]
        sel_grp_ids = [x for x in range(num_sel_grps)]
        status = [True, False]
        num_mbrs_in_grps = [random.randint(1, 7) for x in range(num_sel_grps)]
        mbrs_in_grps = [(random.sample(action_mbr_ids, num_mbrs_in_grps[x]),
                         [status[random.randint(0, 1)]
                          for y in range(num_mbrs_in_grps[x])])
                        for x in range(num_sel_grps)]

        # Construct input for selector table
        # This list contains dictionaries for each entry
        # dict(grp_id -> dict(act_member -> mem_status) )
        mem_dict_dict = {}
        for j in range(num_sel_grps):
            members, member_status = mbrs_in_grps[j]
            mem_dict = {members[i]: member_status[i]
                        for i in range(0, len(members))}
            mem_dict_dict[sel_grp_ids[j]] = mem_dict

        logger.info("Inserting %d entries to action profile table",
                    num_act_prof_entries)
        for j in range(num_act_prof_entries):
            # Create a new member for each port with the port number as the id.
            action_table.entry_add(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID',
                                                    action_mbr_ids[j])])],
                [action_table.make_data([gc.DataTuple('port', egress_ports[j])],
                                        'SwitchIngress.hit')])

        logger.info("DONE Inserting %d entries to action profile table",
                    num_act_prof_entries)

        # Add the new member to the selection table.
        # Adding all members at the same time along with the max group size
        logger.info("Inserting %d groups to selector table", num_sel_grps)
        for grp_id, mem_dict in mem_dict_dict.items():
            sel_table.entry_add(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                 grp_id)])],
                [sel_table.make_data([gc.DataTuple('$MAX_GROUP_SIZE',
                                                   max_grp_size),
                                      gc.DataTuple('$ACTION_MEMBER_ID',
                                                   int_arr_val=list(mem_dict.keys())),
                                      gc.DataTuple('$ACTION_MEMBER_STATUS',
                                                   bool_arr_val=list(mem_dict.values()))])])

        logger.info("DONE Inserting %d groups to selector table", num_sel_grps)

        # Get test on selector table
        # remove random number of groups from the existing groups
        grps_removed = random.sample(sel_grp_ids,
                                     random.randint(1, 0.25 * num_sel_grps))

        logger.info("Deleting %d groups from selector table", len(grps_removed))
        for grp_id in grps_removed:
            logger.info("Deleting Selector group id %d", grp_id)
            sel_table.entry_del(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID', grp_id)])])

        logger.info("DONE Deleting %d groups from selector table",
                    len(grps_removed))

        remaining_grps = [x for x in sel_grp_ids if x not in grps_removed]
        sel_grp_ids = remaining_grps[:]

        logger.info("Getting all %d remaining groups from the selector table",
                    len(remaining_grps))
        # This call will cause a BRI error on switchd, which can be ignored
        resp = sel_table.entry_get(
            target,
            None,
            {"from_hw": False})

        logger.info("DONE Getting all %d remaining groups from the selector table",
                    len(remaining_grps))

        logger.info("Verifying all %d remaining groups from the selector table",
                    len(remaining_grps))
        for data, key in resp:
            data_dict = data.to_dict()
            key_dict = key.to_dict()
            recv_grp_id = key_dict["$SELECTOR_GROUP_ID"]['value']
            assert recv_grp_id in remaining_grps
            remaining_grps.remove(recv_grp_id)

            mem_dict_recv = {
                data_dict["$ACTION_MEMBER_ID"][i]:
                    data_dict["$ACTION_MEMBER_STATUS"][i]
                for i in range(0, len(data_dict["$ACTION_MEMBER_ID"]))}
            # Both the dictionaries should be equal
            assert mem_dict_dict[recv_grp_id] == mem_dict_recv
            logger.info("Selector group id %d matched", recv_grp_id)

        assert len(remaining_grps) == 0
        logger.info("All %d remaining groups matched", len(remaining_grps))

        logger.info("Deleting %d remaining groups", len(remaining_grps))

        # Delete Selector Table entry
        for grp_id in sel_grp_ids:
            logger.info("Deleting Selector group id %d", grp_id)
            sel_table.entry_del(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                 grp_id)])])

        logger.info("DONE Deleting %d remaining groups", len(remaining_grps))

        logger.info("Deleting %d action profile entries", num_act_prof_entries)
        # Delete Action profile members
        for j in range(num_act_prof_entries):
            logger.info("Deleting Action profile member id %d",
                        action_mbr_ids[j])
            action_table.entry_del(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID',
                                                    action_mbr_ids[j])])])

        logger.info("DONE Deleting %d action profile entries",
                    num_act_prof_entries)


class ActionProfileSelectorTestMany(BfRuntimeTest):
    """@brief Fill the selector table, randomly shuffle its content and send
    100 randomized IPv4 packets to be processed by the table. Verify the the
    resulting behavior is as expected.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("Seed used %d", seed)

    def modify_grp(self, sel_table, members, member_status):
        target = gc.Target(device_id=dev_id, pipe_id=0xffff)

        # Modify group
        sel_table.entry_mod(
            target,
            [sel_table.make_key(
                [gc.KeyTuple('$SELECTOR_GROUP_ID', self.group_id)])],
            [sel_table.make_data(
                [gc.DataTuple('$ACTION_MEMBER_ID', int_arr_val=members),
                 gc.DataTuple('$ACTION_MEMBER_STATUS',
                              bool_arr_val=member_status)])
             ])

        # Verify
        member_dict = {members[i]: member_status[i]
                       for i in range(len(members))}
        get_resp = sel_table.entry_get(
            target,
            [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                             self.group_id)])],
            {"from_hw": False})
        data_dict = next(get_resp)[0].to_dict()
        member_dict_recv = {
            data_dict["$ACTION_MEMBER_ID"][i]:
                data_dict["$ACTION_MEMBER_STATUS"][i]
            for i in range(len(data_dict["$ACTION_MEMBER_ID"]))}
        assert member_dict == member_dict_recv

    def runTest(self):
        self.group_id = 1
        ig_port = swports[1]
        eg_ports = swports[2:-1]
        num_ports = len(eg_ports)
        target = gc.Target(device_id=dev_id, pipe_id=0xffff)

        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_action_selector")

        forward_table = bfrt_info.table_get("SwitchIngress.forward")
        action_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector_ap")
        sel_table = bfrt_info.table_get(
            "SwitchIngress.example_action_selector")
        set_dest_table = bfrt_info.table_get("SwitchIngress.set_dest")

        act_prof_size = 2048
        max_grp_size = 200
        # Prepare action profile table by filling with members
        logger.info("Filling action profile table")
        for i in range(1, act_prof_size + 1):
            action_table.entry_add(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', i)])],
                [action_table.make_data([gc.DataTuple('idx', i)],
                                        'SwitchIngress.set_md')])

            set_dest_table.entry_add(
                target,
                [set_dest_table.make_key([gc.KeyTuple('ig_md.md', i)])],
                [set_dest_table.make_data([gc.DataTuple('port',
                                                        eg_ports[i % num_ports])],
                                          'SwitchIngress.hit')])

        # Perform some preliminary member set operations. They should all succeed
        # since there are no match entries pointing to the group.

        # Fill up the selector group
        logger.info("Churning selector group without match ref")
        members = random.sample(range(1, act_prof_size + 1), max_grp_size)
        member_status = [True] * max_grp_size
        sel_table.entry_add(
            target,
            [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID', self.group_id)])],
            [sel_table.make_data([gc.DataTuple('$MAX_GROUP_SIZE', max_grp_size),
                                  gc.DataTuple('$ACTION_MEMBER_ID', int_arr_val=members),
                                  gc.DataTuple('$ACTION_MEMBER_STATUS', bool_arr_val=member_status)])])

        # Disable all members
        member_status = [False] * max_grp_size
        self.modify_grp(sel_table, members, member_status)

        # Perform a few churns
        for _ in range(10):
            num_mbrs = random.randint(1, max_grp_size)
            members = random.sample(range(1, act_prof_size + 1), num_mbrs)
            member_status = [random.choice([False, True]) for _ in range(num_mbrs)]
            self.modify_grp(sel_table, members, member_status)

        # Clear the group
        members = []
        member_status = []
        self.modify_grp(sel_table, members, member_status)

        logger.info("Try to add a match entry to refer to empty group. " +
                    "This should fail, i.e. we expect an gRPC error to be displayed:")
        try:
            forward_table.entry_add(
                target,
                [forward_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port',
                                                     ig_port)])],
                [forward_table.make_data([gc.DataTuple('$SELECTOR_GROUP_ID',
                                                       self.group_id)])])
        except:
            pass

        usage = next(forward_table.usage_get(target))
        assert usage == 0
        logger.info("    Test successful")

        # Add a member so the match entry can be attached
        members = [1]
        member_status = [True]
        self.modify_grp(sel_table, members, member_status)

        logger.info("Adding match entry")
        forward_table.entry_add(
            target,
            [forward_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port',
                                                 ig_port)])],
            [forward_table.make_data([gc.DataTuple('$SELECTOR_GROUP_ID',
                                                   self.group_id)])])

        # Verify
        pkt = testutils.simple_tcp_packet()
        testutils.send_packet(self, ig_port, pkt)
        testutils.verify_any_packet_any_port(self, [pkt], [eg_ports[1]])

        # Perform some membership churn. Packets should come out only on
        # active members.
        for i in range(10):
            logger.info("Preparing group for churn iter %d", i)
            num_mbrs = random.randint(1, max_grp_size)
            members = random.sample(range(1, act_prof_size + 1), num_mbrs)
            member_status = [random.choice([False, True])
                             for _ in range(num_mbrs)]
            # If all members are disabled by chance, ensure the API will fail,
            # then enable the first and move on.
            if True not in member_status:
                api_failed = False
                try:
                    sel_table.entry_mod(
                        target,
                        [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                         self.group_id)])],
                        [sel_table.make_data([gc.DataTuple('$ACTION_MEMBER_ID',
                                                           int_arr_val=members),
                                              gc.DataTuple('$ACTION_MEMBER_STATUS',
                                                           bool_arr_val=member_status)])])
                except:
                    api_failed = True
                    pass
                assert api_failed
                member_status[0] = True

            self.modify_grp(sel_table, members, member_status)

            logger.info("Sending some packets for churn iter %d", i)
            active_ports = [eg_ports[members[idx] % num_ports]
                            for idx, status in enumerate(member_status) if status]

            for _ in range(100):
                eth_src = '%x:%x:%x:%x:%x:%x' % (
                    random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255)
                )
                eth_dst = '%x:%x:%x:%x:%x:%x' % (
                    random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255),
                    random.randint(0, 255), random.randint(0, 255)
                )
                ip_src = '%d.%d.%d.%d' % (random.randint(0, 255),
                                          random.randint(0, 255),
                                          random.randint(0, 255),
                                          random.randint(0, 255))
                ip_dst = '%d.%d.%d.%d' % (random.randint(0, 255),
                                          random.randint(0, 255),
                                          random.randint(0, 255),
                                          random.randint(0, 255))
                pkt = testutils.simple_tcp_packet(eth_src=eth_src,
                                                  eth_dst=eth_dst,
                                                  ip_src=ip_src,
                                                  ip_dst=ip_dst)
                testutils.send_packet(self, ig_port, pkt)
                testutils.verify_any_packet_any_port(self, [pkt], active_ports)

        # Try to empty or delete the group. Should fail since there is a match
        # reference
        logger.info("Try to empty or delete a group with active references. " +
                    "This should fail, i.e. we expect an gRPC error to be displayed:")
        try:
            members = []
            member_status = []
            sel_table.entry_mod(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                 self.group_id)])],
                [sel_table.make_data([gc.DataTuple('$ACTION_MEMBER_ID',
                                                   int_arr_val=members),
                                      gc.DataTuple('$ACTION_MEMBER_STATUS',
                                                   bool_arr_val=member_status)])])
        except:
            pass

        get_resp = sel_table.entry_get(
            target,
            [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                             self.group_id)])],
            {"from_hw": False})
        data_dict = next(get_resp)[0].to_dict()
        assert len(data_dict["$ACTION_MEMBER_ID"]) > 0

        try:
            sel_table.entry_del(
                target,
                [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                                 self.group_id)])])
        except:
            pass
        usage = next(sel_table.usage_get(target))
        assert usage == 1

        # Delete match entry
        logger.info("Deleting match entry")
        forward_table.entry_del(
            target,
            [forward_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port',
                                                 ig_port)])])

        # Delete Selector Table entry
        logger.info("Deleting Selector group id %d", self.group_id)
        sel_table.entry_del(
            target,
            [sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID',
                                             self.group_id)])])

        # Delete Action profile members
        logger.info("Deleting action profile members")
        for i in range(1, act_prof_size + 1):
            action_table.entry_del(
                target,
                [action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', i)])])
            set_dest_table.entry_del(
                target,
                [set_dest_table.make_key([gc.KeyTuple('ig_md.md', i)])])

        logger.info("Sending packet on port %d", ig_port)
        testutils.send_packet(self, ig_port, pkt)

        logger.info("Packet is expected to get dropped.")
        testutils.verify_no_other_packets(self)
