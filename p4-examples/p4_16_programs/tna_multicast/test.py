# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import time
import sys
import logging
import copy

import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
import ptf.dataplane as dataplane

from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as client
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import grpc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


# Most of the test cases were ported from P4-14 multicast_test
# New test cases are negative test cases specific to BRI
#       and separate PRE attributes set test
# TODO:
#    - Add verification and enable EntryGet test
#    - Use logger instead of direct print statements
#    - Add more negative test cases
#    - Add new test cases once PRE pending items get completed
#    - Port the remaining test cases from P4-14 as well (LongTree(s))

# TestUtil that configures LAG Table & Prune Table and manages port states
class TestUtil:
    def __init__(self):
        self.bfrt_test = None
        self.target = None
        self.lag_tbl = None
        self.yid_tbl = None
        self.sw_mask = [0 for x in range(288)]
        self.hw_mask = [0 for x in range(288)]
        self.pbt_on = False

    def setup(self, bfrt_test, target):
        self.bfrt_test = bfrt_test
        self.target = target
        self.lag_tbl = LagTable(self.bfrt_test, target)
        self.yid_tbl = YidTable(self.bfrt_test, target, 0)
        self.sw_mask = [0 for x in range(288)]
        self.hw_mask = [0 for x in range(288)]
        self.backup_ports = [0 for x in range(288)]

        for x in range(288):
            self.backup_ports[x] = BitIdxToPort(x)

    def cleanUp(self):
        self.lag_tbl.cleanUp()
        self.yid_tbl.cleanUp()
        self.sw_mask = [0 for x in range(288)]
        self.hw_mask = [0 for x in range(288)]
        for x in range(288):
            self.clr_backup_port(BitIdxToPort(x))
        self.pm_shdl = None
        self.mc_shdl = None
        self.lag_tbl = None
        self.yid_tbl = None
        self.test = None

    def get_lag_tbl(self):
        return self.lag_tbl

    def get_yid_tbl(self):
        return self.yid_tbl

    def get_sw_mask(self):
        return self.sw_mask

    def get_hw_mask(self):
        return self.hw_mask

    def sw_port_down(self, port):
        self.sw_mask[portToBitIdx(port)] = 1

    def sw_port_up(self, port):
        self.sw_mask[portToBitIdx(port)] = 0

    def enable_port_ff(self):
        pass

    def disable_port_ff(self):
        pass

    def enable_backup_ports(self):
        self.pbt_on = True

    def disable_backup_ports(self):
        self.pbt_on = False

    def set_backup_port(self, pport, bport):
        self.backup_ports[portToBitIdx(pport)] = bport

    def clr_backup_port(self, port):
        self.backup_ports[portToBitIdx(port)] = port

    def get_backup_port(self, port):
        if self.pbt_on:
            return self.backup_ports[portToBitIdx(port)]
        else:
            return port

    def clr_hw_port_down(self, port):
        self.hw_mask[portToBitIdx(port)] = 0

    def set_port_down(self, port):
        self.hw_mask[portToBitIdx(port)] = 1
        take_port_down(port)

    def set_port_up(self, port):
        bring_port_up(port)


t = TestUtil()


# Helper routines that get reused across all tests
def tofLocalPortToOfPort(port, dev_id):
    assert port < MAX_PORT_COUNT
    return (dev_id * MAX_PORT_COUNT) + port


def setup_random(seed_val=0):
    if 0 == seed_val:
        seed_val = int(time.time())
    print("Seed is:", seed_val)
    sys.stdout.flush()
    random.seed(seed_val)


def portToPipe(port):
    return port >> 7


def portToPipeLocalId(port):
    return port & 0x7F


def portToBitIdx(port):
    pipe = portToPipe(port)
    index = portToPipeLocalId(port)
    return 72 * pipe + index


def BitIdxToPort(index):
    pipe = index // 72
    local_port = index % 72
    return (pipe << 7) | local_port


def set_port_map(indicies):
    bit_map = [0] * ((288 + 7) // 8)
    for i in indicies:
        index = portToBitIdx(i)
        bit_map[index // 8] = (bit_map[index // 8] | (1 << (index % 8))) & 0xFF
    return bytearray(bit_map)


def set_lag_map(indicies):
    bit_map = [0] * ((256 + 7) // 8)
    for i in indicies:
        bit_map[i // 8] = (bit_map[i // 8] | (1 << (i % 8))) & 0xFF
    return bytearray(bit_map)


def verify_packet_list(test, port_ll, pkt_ll):
    more_to_rx = False
    for port_list in port_ll:
        if len(port_list) != 0:
            more_to_rx = True
    while more_to_rx:
        found_port = False
        found_pkt = False
        (rcv_device, rcv_port, rcv_pkt, pkt_time) = test.dataplane.poll(timeout=3.0)

        if rcv_port is None:
            print("Didn't receive packet!!!")
            print("Expected ports remaining:", port_ll)
            sys.stdout.flush()
            test.assertTrue(rcv_port is not None)

        # See if the received port+packet pair is in any of the lists passed in.
        for port_list, pkt_list in zip(port_ll, pkt_ll):
            if rcv_port in port_list:
                found_port = True
                for exp_pkt in pkt_list:
                    if dataplane.match_exp_pkt(exp_pkt, rcv_pkt):
                        pkt_list.remove(exp_pkt)
                        found_pkt = True
                        break
                if found_pkt:
                    port_list.remove(rcv_port)
                    break

        if found_port != True or found_pkt != True:
            print("Unexpected Rx: port", rcv_port)
            print(format_packet(rcv_pkt))
            print("Expected the following:")
            for port_list, pkt_list in zip(port_ll, pkt_ll):
                print("  Ports:", sorted(port_list))
                for pkt in pkt_list:
                    print("  Pkt:  ", format_packet(pkt))
            sys.stdout.flush()
            test.assertTrue(found_port == True, "Unexpected port %r" % rcv_port)
            test.assertTrue(found_pkt == True, "Unexpected pkt on port %r" % rcv_port)

        more_to_rx = False
        for port_list in port_ll:
            if len(port_list) != 0:
                more_to_rx = True

    (rcv_device, rcv_port, rcv_pkt, pkt_time) = test.dataplane.poll(timeout=0.1)
    if rcv_port != None:
        print("Extra Rx: port", rcv_port, "Packet", format_packet(rcv_pkt))
        sys.stdout.flush()
        test.assertTrue(rcv_pkt == None, "Receive extra packet")


def build_rx_packet_list(port_list, pkt):
    pkt_list = []
    tmp_port_list = sorted(port_list)
    last_pipe = -1
    for port in tmp_port_list:
        pkt_list.append(copy.deepcopy(pkt))

    return pkt_list


# L1 Multicast Node Class
class L1Node:
    # BRI
    l1_node_id = 1

    def __init__(self, bfrt_test, target, rid):
        self.bfrt_test = bfrt_test
        self.rid = rid
        self.target = target
        self.xid = None
        self.l2_hdl = None
        self.mgid = None
        self.mbr_ports = []
        self.mbr_lags = []
        self.node_id = L1Node.l1_node_id
        L1Node.l1_node_id = L1Node.l1_node_id + 1
        # Creates node id entry in node table
        self.bfrt_test.node_table.entry_add(
            self.target,
            [self.bfrt_test.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', self.node_id)])],
            [self.bfrt_test.node_table.make_data([client.DataTuple('$MULTICAST_RID', self.rid),
                                                  client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=self.mbr_lags),
                                                  client.DataTuple('$DEV_PORT', int_arr_val=self.mbr_ports)])])

    def __repr__(self):
        return "L1Node_" + str(hex(self.l1_hdl))

    def __str__(self):
        return str(hex(self.l1_hdl))

    def l1_hdl(self):
        return self.l1_hdl

    def get_rid(self):
        return self.rid

    def get_mbr_ports(self):
        return list(self.mbr_ports)

    def associate(self, mgid, xid):
        self.mgid = mgid
        self.xid = xid
        if xid is None:
            xid = 0
            use_xid = 0
        else:
            use_xid = 1
        self.bfrt_test.mgid_table.entry_mod_inc(
            self.target,
            [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', self.mgid)])],
            [self.bfrt_test.mgid_table.make_data([
                client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[self.node_id]),
                client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[use_xid]),
                client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[xid])])],
            bfruntime_pb2.TableModIncFlag.MOD_INC_ADD)
        # TODO: Add verification and enable entry get test
        '''
        resp = self.bfrt_test.get_table_entry(
            self.target,
            '$pre.mgid',
            [table.make_key([client.KeyTuple('$MGID', self.mgid)])],
            {"from_hw":False})
        data_dict = next(self.bfrt_test.parseEntryGetResponse(resp))
        '''

    def dissociate(self, test):
        # Not used
        pass

    def is_associated(self, test):
        # Not used
        pass

    def addMbrs(self, port_list, lag_list):
        if port_list is None and lag_list is None:
            return 0
        if port_list is not None:
            self.mbr_ports += port_list
            self.mbr_ports.sort()
        if lag_list is not None:
            for i in lag_list:
                assert i >= 0 and i <= 255
            self.mbr_lags += lag_list
            self.mbr_lags.sort()
        self.bfrt_test.node_table.entry_mod(
            self.target,
            [self.bfrt_test.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', self.node_id)])],
            [self.bfrt_test.node_table.make_data([client.DataTuple('$MULTICAST_RID', self.rid),
                                                  client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=self.mbr_lags),
                                                  client.DataTuple('$DEV_PORT', int_arr_val=self.mbr_ports)])])

    def getPorts(self, rid, yid, h2):
        global t
        # Start with the individual ports on the L1 and then apply pruning
        port_list = self.get_mbr_ports()
        if self.rid == rid or rid == t.get_yid_tbl().global_rid():
            t.get_yid_tbl().prune_ports(yid, port_list)
        # If any ports are down, replace them with their backup
        # Since the backup table is initialized such that each port backups up
        # itself we can blindly take the backup table contents if the port is
        # down.
        if port_list:
            for x in range(len(port_list)):
                pport = port_list[x]
                pport_idx = portToBitIdx(pport)
                if t.sw_mask[pport_idx] == 1 or t.hw_mask[pport_idx] == 1:
                    port_list[x] = t.get_backup_port(pport)
        # For each LAG on the L1, pick the correct member port
        for lag_id in self.mbr_lags:
            lag = t.get_lag_tbl().getLag(lag_id)
            port = lag.getMbrByHash(h2, rid, self.rid, yid)
            if port is not None:
                port_list.append(port)
        return port_list

    def cleanUp(self, test):
        if self.mgid is not None:
            self.bfrt_test.mgid_table.entry_mod_inc(
                self.target,
                [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', self.mgid)])],
                [self.bfrt_test.mgid_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[self.node_id]),
                                                      client.DataTuple('$MULTICAST_NODE_L1_XID_VALID',
                                                                       bool_arr_val=[0]),
                                                      client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])],
                bfruntime_pb2.TableModIncFlag.MOD_INC_DELETE)
            self.mgid = None
        self.bfrt_test.node_table.entry_del(
            self.target,
            [self.bfrt_test.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', self.node_id)])])
        self.node_id = 0


class LagGrp:
    def __init__(self, bfrt_test, target, lag_id):
        assert lag_id >= 0 and lag_id <= 255
        self.bfrt_test = bfrt_test
        self.target = target
        self.lag_id = lag_id
        self.left_cnt = 0
        self.right_cnt = 0
        self.mbrs = []

    def setRmtCnt(self, left, right):
        self.left_cnt = left
        self.right_cnt = right

    def addMbr(self, port_list):
        self.mbrs = list(set(self.mbrs + port_list))
        self.mbrs.sort()
        bit_map = set_port_map(self.mbrs)
        self.bfrt_test.lag_table.entry_add(
            self.target,
            [self.bfrt_test.lag_table.make_key([client.KeyTuple('$MULTICAST_LAG_ID', self.lag_id)])],
            [self.bfrt_test.lag_table.make_data([client.DataTuple('$DEV_PORT', int_arr_val=self.mbrs),
                                                 client.DataTuple('$MULTICAST_LAG_REMOTE_MSB_COUNT', val=self.left_cnt),
                                                 client.DataTuple('$MULTICAST_LAG_REMOTE_LSB_COUNT', val=self.right_cnt)])
            ])
        #Validate Get  GetFirst  GetNext
        logger.info("Verifying entry get for PRE LAG Table for lag id %d", self.lag_id)
        resp = self.bfrt_test.lag_table.entry_get(self.target,
                    [self.bfrt_test.lag_table.make_key([client.KeyTuple('$MULTICAST_LAG_ID', self.lag_id)])],
                    {"from_hw":True})
        data_dict = next(resp)[0].to_dict()
        assert sorted(data_dict["$DEV_PORT"]) == sorted(self.mbrs)
        #Validate Wildcard Read for PRE Lag Table
        logger.info("Verifying entry get using Wildcard entry_get(target)")
        resps = self.bfrt_test.lag_table.entry_get(self.target)
        for resp in resps:
            if resp[1].to_dict()['$MULTICAST_LAG_ID']['value'] == self.lag_id:
                assert sorted(resp[0].to_dict()['$DEV_PORT']) == sorted(self.mbrs)
                assert (resp[0].to_dict()['$MULTICAST_LAG_REMOTE_MSB_COUNT']) == (self.left_cnt)
                assert (resp[0].to_dict()['$MULTICAST_LAG_REMOTE_LSB_COUNT']) == (self.right_cnt)

    def rmvMbr(self, port_list):
        l = [x for x in self.mbrs if x not in port_list]
        self.mbrs = list(set(l))
        # Add and modify are same for LAG table. There is no delete.
        self.bfrt_test.lag_table.entry_add(
            self.target,
            [self.bfrt_test.lag_table.make_key([client.KeyTuple('$MULTICAST_LAG_ID', self.lag_id)])],
            [self.bfrt_test.lag_table.make_data([client.DataTuple('$DEV_PORT', int_arr_val=self.mbrs),
                                                 client.DataTuple('$MULTICAST_LAG_REMOTE_MSB_COUNT', val=self.left_cnt),
                                                 client.DataTuple('$MULTICAST_LAG_REMOTE_LSB_COUNT', val=self.right_cnt)])
            ])

    def getMbrByHash(self, h, pkt_rid, node_rid, yid):
        global t
        if len(self.mbrs) == 0:
            return None  # No members
        len_pack = (self.left_cnt + len(self.mbrs) + self.right_cnt) & 0x1FFF
        if len_pack:
            index_pack = h % len_pack
        else:
            index_pack = 0
        vec_pack = sorted(self.mbrs)

        vec_pack_mask = []
        for m in vec_pack:
            if t.get_sw_mask()[portToBitIdx(m)] == 1:
                continue
            if t.get_hw_mask()[portToBitIdx(m)] == 1:
                continue
            vec_pack_mask.append(m)
        len_pack_mask = len(vec_pack_mask)
        if len_pack_mask == 0:
            index_pack_mask = 0
        else:
            index_pack_mask = h % len_pack_mask

        if index_pack < self.right_cnt:  # Hashed to remote right member
            return None
        if index_pack >= (len(self.mbrs) + self.right_cnt):  # Hashed to remote left member
            return None

        if len_pack_mask == 0:  # No live ports
            port = vec_pack[index_pack - self.right_cnt]
        elif vec_pack[index_pack - self.right_cnt] in vec_pack_mask:
            port = vec_pack[index_pack - self.right_cnt]
        else:
            port = vec_pack_mask[index_pack_mask]

        # Apply pruning to the selected port.
        if pkt_rid == node_rid or pkt_rid == t.get_yid_tbl().global_rid():
            if t.get_yid_tbl().is_port_pruned(yid, port):
                return None
        if port in vec_pack_mask:
            # Port is up
            return port
        else:
            # Port is down, use the backup instead.
            return t.get_backup_port(port)

    def cleanUp(self):
        self.setRmtCnt(0, 0)
        self.rmvMbr(self.mbrs)


class LagTable:
    def __init__(self, bfrt_test, target):
        self.bfrt_test = bfrt_test
        self.target = target
        self.lags = []
        for i in range(255):
            lag = LagGrp(self.bfrt_test, self.target, i)
            self.lags.append(lag)

    def getLag(self, lag_id):
        assert lag_id >= 0 and lag_id <= 255
        return self.lags[lag_id]

    def cleanUp(self):
        for lag in self.lags:
            lag.cleanUp()


class EcmpGrp:
    # BRI specific - each ECMP would have a unique ECMP id
    l1_ecmp_id = 1

    def __init__(self, bfrt_test, target):
        self.bfrt_test = bfrt_test
        self.target = target
        self.mbrs = [None for _ in range(32)]
        self.mgids = []
        self.ecmp_id = EcmpGrp.l1_ecmp_id
        EcmpGrp.l1_ecmp_id = EcmpGrp.l1_ecmp_id + 1
        self.bfrt_test.ecmp_table.entry_add(
            self.target,
            [self.bfrt_test.ecmp_table.make_key([client.KeyTuple('$MULTICAST_ECMP_ID', self.ecmp_id)])])

    def addMbr(self, rid, port_list, lag_list):
        l1 = L1Node(self.bfrt_test, self.target, rid)
        l1.addMbrs(port_list, lag_list)
        self.bfrt_test.ecmp_table.entry_mod_inc(
            self.target,
            [self.bfrt_test.ecmp_table.make_key([client.KeyTuple('$MULTICAST_ECMP_ID', self.ecmp_id)])],
            [self.bfrt_test.ecmp_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[l1.node_id])])],
            bfruntime_pb2.TableModIncFlag.MOD_INC_ADD)
        # TODO: Add verification and enable entry get test
        '''
        resp = self.bfrt_test.get_table_entry(
            self.target,
            '$pre.ecmp',
            [table.make_key([client.KeyTuple('$MULTICAST_ECMP_ID', self.ecmp_id)])],
            {"from_hw":False})
        data_dict = next(self.bfrt_test.parseEntryGetResponse(resp))
        '''
        self.mbrs[self.mbrs.index(None)] = l1

    def rmvMbr(self, index):
        l1 = self.mbrs[index]
        self.bfrt_test.ecmp_table.entry_mod_inc(
            self.target,
            [self.bfrt_test.ecmp_table.make_key([client.KeyTuple('$MULTICAST_ECMP_ID', self.ecmp_id)])],
            [self.bfrt_test.ecmp_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[l1.node_id])])],
            bfruntime_pb2.TableModIncFlag.MOD_INC_DELETE)
        self.mbrs[index] = None
        l1.cleanUp(self.bfrt_test)

    def getMbrs(self):
        mbrs = []
        return mbrs

    def associate(self, mgid, xid):
        if xid is None:
            xid = 0
            use_xid = 0
        else:
            use_xid = 1
        self.bfrt_test.mgid_table.entry_mod_inc(
            self.target,
            [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', mgid)])],
            [self.bfrt_test.mgid_table.make_data([
                client.DataTuple('$MULTICAST_ECMP_ID', int_arr_val=[self.ecmp_id]),
                client.DataTuple('$MULTICAST_ECMP_L1_XID_VALID', bool_arr_val=[use_xid]),
                client.DataTuple('$MULTICAST_ECMP_L1_XID', int_arr_val=[xid])])],
            bfruntime_pb2.TableModIncFlag.MOD_INC_ADD)
        # TODO: Add verification and enable entry get test
        '''
        resp = self.bfrt_test.get_table_entry(
            self.target,
            '$pre.mgid',
            [table.make_key([client.KeyTuple('$MGID', mgid)])],
            {"from_hw":False})
        data_dict = next(self.bfrt_test.parseEntryGetResponse(resp))
        '''
        self.mgids.append(mgid)

    def dissociate(self, mgid):
        self.bfrt_test.mgid_table.entry_mod_inc(
            self.target,
            [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', mgid)])],
            [self.bfrt_test.mgid_table.make_data([
                client.DataTuple('$MULTICAST_ECMP_ID', int_arr_val=[self.ecmp_id]),
                client.DataTuple('$MULTICAST_ECMP_L1_XID_VALID', bool_arr_val=[0]),
                client.DataTuple('$MULTICAST_ECMP_L1_XID', int_arr_val=[0])])],
            bfruntime_pb2.TableModIncFlag.MOD_INC_DELETE)
        self.mgids.remove(mgid)

    def getMbrByHash(self, val):
        live_cnt = 32 - self.mbrs.count(None)
        idx1 = val % 32
        idx2 = idx1
        if live_cnt != 0:
            idx2 = val % live_cnt
        if self.mbrs[idx1] is not None:
            return self.mbrs[idx1]
        idx = 0
        for node in self.mbrs:
            if node is not None and idx == idx2:
                return node
            if node is not None:
                idx = idx + 1
        return None

    def getRid(self, h):
        node = self.getMbrByHash(h)
        if node is not None:
            return node.get_rid()
        return 0xDEAD

    def cleanUp(self):
        # Remove members
        for i in range(len(self.mbrs)):
            if self.mbrs[i] is not None:
                self.rmvMbr(i)
        # Dissociate mgids
        for mgid in self.mgids:
            self.bfrt_test.mgid_table.entry_mod_inc(
                self.target,
                [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', mgid)])],
                [self.bfrt_test.mgid_table.make_data([
                    client.DataTuple('$MULTICAST_ECMP_ID', int_arr_val=[self.ecmp_id]),
                    client.DataTuple('$MULTICAST_ECMP_L1_XID_VALID', bool_arr_val=[0]),
                    client.DataTuple('$MULTICAST_ECMP_L1_XID', int_arr_val=[0])])],
                bfruntime_pb2.TableModIncFlag.MOD_INC_DELETE)
        self.mgids = []
        # Clean up ECMP group
        self.bfrt_test.ecmp_table.entry_del(
            self.target,
            [self.bfrt_test.ecmp_table.make_key([client.KeyTuple('$MULTICAST_ECMP_ID', self.ecmp_id)])])
        self.ecmp_id = None


class YidTable:
    # Prune Table Class
    def __init__(self, bfrt_test, target, global_rid):
        self.bfrt_test = bfrt_test
        self.target = target
        self.prune_list = []
        for x in range(288):
            self.prune_list.append([])
        self.set_global_rid(global_rid)

    def set_global_rid(self, grid):
        self.grid = grid
        self.bfrt_test.mgid_table.attribute_pre_device_config_set(self.target, global_rid=grid)

    def global_rid(self):
        return self.grid

    def set_pruned_ports(self, yid, new_prune_list, skip_get=False):
        self.prune_list[yid] = list(new_prune_list)
        self.bfrt_test.prune_table.entry_add(
            self.target,
            [self.bfrt_test.prune_table.make_key([client.KeyTuple('$MULTICAST_L2_XID', yid)])],
            [self.bfrt_test.prune_table.make_data([client.DataTuple('$DEV_PORT', int_arr_val=self.prune_list[yid])])])
        if skip_get:
            return

        # Validate Get  GetFirst  GetNext
        logger.info("Verifying entry get for PRE Prune Table for l2_xid %d", yid)
        resp = self.bfrt_test.prune_table.entry_get(self.target,
                    [self.bfrt_test.prune_table.make_key([client.KeyTuple('$MULTICAST_L2_XID', yid)])],
                    {"from_hw":True})
        data_dict = next(resp)[0].to_dict()
        assert sorted(data_dict["$DEV_PORT"]) == sorted(self.prune_list[yid])
        #Validate Wildcard Read for PRE Lag Table
        logger.info("Verifying entry get using Wildcard entry_get(target)")
        resps = self.bfrt_test.prune_table.entry_get(self.target)
        seen = []
        for resp in resps:
            read_back_yid = resp[1].to_dict()['$MULTICAST_L2_XID']['value']
            seen.append(read_back_yid)
            if read_back_yid == yid:
                if sorted(resp[0].to_dict()['$DEV_PORT']) != sorted(self.prune_list[yid]):
                    logger.info("Read: %s", sorted(resp[0].to_dict()['$DEV_PORT']))
                    logger.info("Expt: %s", sorted(self.prune_list[yid]))
                assert sorted(resp[0].to_dict()['$DEV_PORT']) == sorted(self.prune_list[yid])
        assert sorted(seen) == list(range(self.bfrt_test.prune_table.info.size_get()))

    def get_pruned_ports(self, yid):
        return list(self.prune_list[yid])

    def prune_ports(self, yid, port_list):
        l = list(port_list)
        for p in l:
            if p in self.prune_list[yid]:
                port_list.remove(p)

    def is_port_pruned(self, yid, port):
        if port in self.prune_list[yid]:
            return True
        return False

    def cleanUp(self):
        for yid in range(288):
            self.set_pruned_ports(yid, [], skip_get=True)
        self.set_global_rid(0)


class MCTree:
    # Class for MGID table and cpmplete multicast engine config
    def __init__(self, bfrt_test, target, mgid):
        self.bfrt_test = bfrt_test
        self.target = target
        self.mgid = mgid
        self.bfrt_test.mgid_table.entry_add(
            self.target,
            [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', mgid)])])
        self.nodes = []
        self.ecmps = []

    def add_node(self, rid, xid, mbr_ports, mbr_lags):
        l1 = L1Node(self.bfrt_test, self.target, rid)
        l1.addMbrs(mbr_ports, mbr_lags)
        l1.associate(self.mgid, xid)
        self.nodes.append(l1)

    def rmv_last_node(self):
        l1 = self.nodes[-1]
        self.nodes = self.nodes[:-1]
        l1.cleanUp(self.bfrt_test)

    def add_ecmp(self, grp, xid):
        grp.associate(self.mgid, xid)
        grp_tup = (grp, xid)
        self.ecmps.append(grp_tup)

    def reprogram(self):
        for l1 in self.nodes:
            l1.dissociate(self.bfrt_test)
        for grp, xid in self.ecmps:
            grp.dissociate(self.mgid)
        for l1 in self.nodes:
            l1.associate(self.test, self.mgid_hdl, l1.xid)
        for grp, xid in self.ecmps:
            grp.associate(self.mgid_hdl, xid)

    def cleanUp(self):
        for l1 in self.nodes:
            l1.cleanUp(self.bfrt_test)
        for grp, _ in self.ecmps:
            grp.dissociate(self.mgid)
        self.bfrt_test.mgid_table.entry_del(
            self.target,
            [self.bfrt_test.mgid_table.make_key([client.KeyTuple('$MGID', self.mgid)])])
        self.nodes = []
        self.ecmps = []

    def get_ports(self, pkt_rid, pkt_xid, pkt_yid, pkt_hash1=0, pkt_hash2=0):
        port_data = []
        ecmp_data = []
        for l1 in self.nodes:
            if l1.xid is not None and pkt_xid == l1.xid:
                continue
            ports = l1.getPorts(pkt_rid, pkt_yid, pkt_hash2)
            port_data.append((l1.rid, ports))

        for grp, xid in self.ecmps:
            if xid is not None and pkt_xid == xid:
                continue
            l1 = grp.getMbrByHash(pkt_hash1)
            if l1 is not None:
                ports = l1.getPorts(pkt_rid, pkt_yid, pkt_hash2)
                ecmp_data.append((l1.rid, ports))
        return port_data + ecmp_data

    def print_tree(self):
        print("Dev:", self.dev, "MGID:", hex(self.mgid), "Num L1 Nodes:", len(self.nodes), "Num ECMPs:", len(self.ecmps))
        for l1 in self.nodes:
            if l1.xid is not None:
                print("  L1_Hdl:", l1.l1_hdl, "RID:", hex(l1.rid), "XID:", hex(
                    l1.xid), "Ports:", l1.mbr_ports, "LAGs:", l1.mbr_lags)
            else:
                print("  L1_Hdl:", l1.l1_hdl, "RID:", hex(
                    l1.rid), "XID:", l1.xid, "Ports:", l1.mbr_ports, "LAGs:", l1.mbr_lags)
        for grp, xid in self.ecmps:
            if xid is not None:
                print("  ECMP Hdl:", hex(grp), "XID:", hex(xid))
            else:
                print("  ECMP Hdl:", hex(grp))


# Use the first two and last two ports in each pipe by default.
swports = []
for device, port, ifname in config["interfaces"]:
    pipe = port >> 7
    if pipe in range(int(test_param_get('num_pipes'))):
        swports.append(port)
        swports.sort()

if swports == []:
    for p in range(int(test_param_get('num_pipes'))):
        for i in [0, 1, 62, 63]:
            port = (p << 7) | i
            swports.append(port)

print("Ports in use:", swports)
sys.stdout.flush()


def rmv_ports(test):
    if test_param_get('target') != "hw":
        return
    test.pal.pal_port_del_all(dev_id)


def add_ports(test):
    if test_param_get('target') != "hw":
        return
    # Remove any existing ports.
    rmv_ports(test)
    # Add all ports as 10g
    speed = pal_port_speed_t.BF_SPEED_10G
    fec = pal_fec_type_t.BF_FEC_TYP_NONE
    for port in swports:
        test.pal.pal_port_add(dev_id, port, speed, fec)
        test.pal.pal_port_an_set(dev_id, port, pal_autoneg_policy_t.BF_AN_FORCE_DISABLE)
    test.pal.pal_port_enable_all(dev_id)
    # Wait for the ports to come up
    print("Waiting for ports to come up...")
    time.sleep(1)
    attempt = 0
    while True:
        ports_all_up = True
        for p in swports:
            x = test.pal.pal_port_oper_status_get(0, p)
            if x == pal_oper_status_t.BF_PORT_DOWN:
                ports_all_up = False
                print("  port", p, "is down")
                time.sleep(5)
                break
        if ports_all_up:
            break
        else:
            attempt += 1
            assert attempt < 12


# Set to true or false depending on if the testbed supports flapping the ports.
# Harlyn model supports it, other test environments might not...
if test_param_get('target') != "hw":
    support_hw_port_flap = True
else:
    support_hw_port_flap = False


def setup_tables(self):
    bfrt_info = self.interface.bfrt_info_get()
    # Get all P4 table objects
    self.ing_port_table = bfrt_info.table_get("SwitchIngress.ing_port")
    self.ing_src_ifid_table = bfrt_info.table_get("SwitchIngress.ing_src_ifid")
    self.ing_dmac_table = bfrt_info.table_get("SwitchIngress.ing_dmac")
    self.ing_dmac_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")

    self.ing_ipv4_mcast_table = bfrt_info.table_get("SwitchIngress.ing_ipv4_mcast")
    self.ing_ipv4_mcast_table.info.key_field_annotation_add("hdr.ipv4.src_addr", "ipv4")
    self.ing_ipv4_mcast_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")

    # Get all PRE table objects
    self.mgid_table = bfrt_info.table_get("$pre.mgid")
    self.node_table = bfrt_info.table_get("$pre.node")
    self.ecmp_table = bfrt_info.table_get("$pre.ecmp")
    self.lag_table = bfrt_info.table_get("$pre.lag")
    self.prune_table = bfrt_info.table_get("$pre.prune")
    self.port_table = bfrt_info.table_get("$pre.port")

class TestBasic(BfRuntimeTest):
    # Sets up unicast and multicast forwarding tables
    # Tests basic unicast forwarding and
    # basic multicast forwarding (no pruning and no ECMPs/LAGs)
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - Programs unicast forwarding rule based on DMAC.
            If DMAC matches, packet will be forwarded to the next port
          - Programs multicast forwarding rule based on multicast DMAC.
            If multicast DMAC matches for an ingress port, separate MGID
            will be assigned and packets will be flooded based on the
            multicast programming for the MGID
          - For each MGID, creates a L1 node and adds random number of ports
            to the L1 node
          - Sends unicast packet to each port and verifies that it is
            correctly forwarded
          - Sends multicast packet to each port and verifies that the packet
            gets flooded based on the assigned MGID
        '''
        setup_random()

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Dict to hold interface id for each port
        port_to_ifid = {}
        # Dict to hold bridge id for each interface id
        ifid_to_brid = {}
        # Dict to hold multicast mgid for each bridge id
        brid_to_mgid = {}
        # Dict to hold multicast L1 id for each bridge id
        brid_to_l1 = {}
        # Set that has the list of interface ids being used
        ifids = set()
        # Set that has the list of bridge ids being used
        brids = set()

        for p in swports:
            port_to_ifid[p] = p
            ifids.add(p)

        # Randomly select bridge ids equal to the number of ports (ifids)
        # from the range between 0 and 64K
        brids.update(random.sample(range(0, 0x10000), len(ifids)))
        brid_list = list(brids)
        for ifid, brid in zip(ifids, brids):
            ifid_to_brid[ifid] = brid

        rid = 0
        yid = 0
        hash1 = 1
        hash2 = 1
        vlan_val = 0
        vlan_id = 0

        try:
            add_ports(self)

            # Add Ing Port table entries
            print("Adding SwitchIngress.ing_port table entries")
            for port_num in swports:
                ifid = port_to_ifid[port_num]
                self.ing_port_table.entry_add(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port_num),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])],
                    [self.ing_port_table.make_data([client.DataTuple('ifid', ifid)],
                                                   'SwitchIngress.set_ifid')]
                )

            # Add Ing Src IFID table entries
            print("Adding SwitchIngress.ing_src_ifid table entries")
            for port_num in swports:
                ifid = port_to_ifid[port_num]
                brid = ifid_to_brid[ifid]
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([
                        client.DataTuple('rid', rid),
                        client.DataTuple('yid', yid),
                        client.DataTuple('brid', brid),
                        client.DataTuple('hash1', hash1),
                        client.DataTuple('hash2', hash2)],
                        'SwitchIngress.set_src_ifid_md')]
                )

            # Add DMAC entries to switch
            print("Adding SwitchIngress.ing_dmac table entries")
            for i, port in enumerate(swports):
                ing_port = port
                dmac = "00:11:11:11:11:00"
                egr_port = swports[(i + 1) % len(swports)]
                ifid = port_to_ifid[ing_port]
                brid = ifid_to_brid[ifid]
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key([
                        client.KeyTuple('ig_md.brid', brid),
                        client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([client.DataTuple('port', egr_port)],
                                                   'SwitchIngress.l2_switch')]
                )

            # Add DMAC entries to flood
            print("Adding SwitchIngress.ing_src_ifid table entries")
            for brid in brids:
                dmac = "FF:FF:FF:FF:FF:FF"
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key(
                        [client.KeyTuple('ig_md.brid', brid),
                         client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([], 'SwitchIngress.flood')]
                )

            # Allocate an MGID per brid
            print("Adding MGID table entries")
            for brid in brids:
                self.mgid_table.entry_add(
                    target,
                    [self.mgid_table.make_key(
                        [client.KeyTuple('$MGID', (brid & 0xFFFF))])])
                brid_to_mgid[brid] = (brid & 0xFFFF)

            # Allocate a single L1 node for each brid
            print("Adding MC node table entries")
            l1_id = 1
            for brid in brids:
                rid = (~brid) & 0xFFFF  # rid is 16 bits and make sure it is different from brid
                self.node_table.entry_add(
                    target,
                    [self.node_table.make_key([
                        client.KeyTuple('$MULTICAST_NODE_ID', l1_id)])],
                    [self.node_table.make_data([
                        client.DataTuple('$MULTICAST_RID', rid),
                        client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=[]),
                        client.DataTuple('$DEV_PORT', int_arr_val=[])])]
                )
                # TODO: Add verification and enable entry get test
                '''
                resp = self.get_table_entry(
                    target,
                    '$pre.node',
                    [table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', l1_id)])],
                    {"from_hw":False})
                data_dict = next(self.parseEntryGetResponse(resp))
                '''
                brid_to_l1[brid] = l1_id
                l1_id = l1_id + 1

            # Add L2 nodes to the L1s
            ports_in_tree = {}
            # Add all ports to the first
            l2_node_ports = []
            l2_node_lags = []
            for p in swports:
                l2_node_ports.append(int(p))
            ports_in_tree[brid_list[0]] = sorted(l2_node_ports)
            self.node_table.entry_mod(
                target,
                [self.node_table.make_key([
                    client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid_list[0]])])],
                [self.node_table.make_data([
                    client.DataTuple('$MULTICAST_RID', ((~brid_list[0]) & 0xFFFF)),
                    client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=l2_node_lags),
                    client.DataTuple('$DEV_PORT', int_arr_val=l2_node_ports)])]
            )
            self.mgid_table.entry_mod(
                target,
                [self.mgid_table.make_key([
                    client.KeyTuple('$MGID', brid_to_mgid[brid_list[0]])])],
                [self.mgid_table.make_data([
                    client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[brid_to_l1[brid_list[0]]]),
                    client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                    client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])
            # Add no ports to the second
            l2_node_ports = []
            l2_node_lags = []
            ports_in_tree[brid_list[1]] = sorted(l2_node_ports)
            self.node_table.entry_mod(
                target,
                [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid_list[1]])])],
                [self.node_table.make_data([
                    client.DataTuple('$MULTICAST_RID', ((~brid_list[1]) & 0xFFFF)),
                    client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=l2_node_lags),
                    client.DataTuple('$DEV_PORT', int_arr_val=l2_node_ports)])])
            self.mgid_table.entry_mod(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', brid_to_mgid[brid_list[1]])])],
                [self.mgid_table.make_data([
                    client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[brid_to_l1[brid_list[1]]]),
                    client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                    client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])
            # Add a random number of ports to the rest
            for i in range(2, len(brid_list)):
                num_ports = random.randint(0, len(swports) - 1)
                l2_node_ports = random.sample(swports, num_ports)
                l2_node_lags = []
                ports_in_tree[brid_list[i]] = sorted(l2_node_ports)
                self.node_table.entry_mod(
                    target,
                    [self.node_table.make_key([
                        client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid_list[i]])])],
                    [self.node_table.make_data([
                        client.DataTuple('$MULTICAST_RID', ((~brid_list[i]) & 0xFFFF)),
                        client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=l2_node_lags),
                        client.DataTuple('$DEV_PORT', int_arr_val=l2_node_ports)])])
                # TODO: Add verification and enable entry get test
                '''
                resp = self.get_table_entry(
                    target,
                    '$pre.node',
                    [table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[ brid_list[i] ])])],
                    {"from_hw":False})
                data_dict = next(self.parseEntryGetResponse(resp))
                '''

                self.mgid_table.entry_mod(
                    target,
                    [self.mgid_table.make_key([client.KeyTuple('$MGID', brid_to_mgid[brid_list[i]])])],
                    [self.mgid_table.make_data([
                        client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[brid_to_l1[brid_list[i]]]),
                        client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                        client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])
                # TODO: Add verification and enable entry get test
                '''
                resp = self.get_table_entry(
                    target,
                    '$pre.mgid',
                    [table.make_key([client.KeyTuple('$MGID', brid_to_mgid[ brid_list[i] ])])],
                    {"from_hw":False})
                data_dict = next(self.parseEntryGetResponse(resp))
                '''

            for i in range(0, len(swports)):
                print("Sending unicast on port", swports[i],
                      "expecting receive on port", swports[(i + 1) % len(swports)])
                sys.stdout.flush()
                pkt = simple_tcp_packet(eth_dst='00:11:11:11:11:00',
                                        eth_src='00:22:22:22:22:22',
                                        ip_src='1.1.1.1',
                                        ip_dst='10.0.0.1',
                                        ip_tos=255,
                                        ip_ttl=64,
                                        ip_id=101)
                send_packet(self, swports[i], pkt)
                epkt = simple_tcp_packet(eth_dst='00:11:11:11:11:00',
                                         eth_src='00:22:22:22:22:22',
                                         ip_src='1.1.1.1',
                                         ip_dst='10.0.0.1',
                                         ip_tos=255,
                                         ip_ttl=64,
                                         ip_id=101)

                verify_packets(self, epkt, [swports[(i + 1) % len(swports)]])

            for ing_port in swports:
                ifid = port_to_ifid[ing_port]
                brid = ifid_to_brid[ifid]
                rx_port_list = ports_in_tree[brid]
                rx_pkt_list = build_rx_packet_list(rx_port_list, simple_tcp_packet(eth_dst='FF:FF:FF:FF:FF:FF',
                                                                                   eth_src='00:22:22:22:22:22',
                                                                                   ip_src='1.1.1.1',
                                                                                   ip_dst='10.0.0.1',
                                                                                   ip_tos=255,
                                                                                   ip_ttl=64,
                                                                                   ip_id=101))
                print("Sending multicast on port", ing_port, "expecting receive on ports", rx_port_list)
                sys.stdout.flush()
                pkt = simple_tcp_packet(eth_dst='FF:FF:FF:FF:FF:FF',
                                        eth_src='00:22:22:22:22:22',
                                        ip_src='1.1.1.1',
                                        ip_dst='10.0.0.1',
                                        ip_tos=255,
                                        ip_ttl=64,
                                        ip_id=101)
                send_packet(self, ing_port, pkt)

                verify_packet_list(self, [rx_port_list], [rx_pkt_list])

        finally:
            rmv_ports(self)

            for brid in brid_to_mgid:
                self.mgid_table.entry_del(
                    target,
                    [self.mgid_table.make_key([client.KeyTuple('$MGID', brid_to_mgid[brid])])])

            for brid in brid_to_l1:
                self.node_table.entry_del(
                    target,
                    [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid])])])

            for port_num in swports:
                vlan_val = 0
                vlan_id = 0
                ifid = port_to_ifid[port_num]
                self.ing_port_table.entry_del(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port_num),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])])

            for port_num in swports:
                ifid = port_to_ifid[port_num]
                rid = 0
                yid = 0
                brid = ifid_to_brid[ifid]
                hash1 = 1
                hash2 = 1
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])

            for i, port in enumerate(swports):
                ing_port = port
                egr_port = swports[(i + 1) % len(swports)]
                ifid = port_to_ifid[ing_port]
                brid = ifid_to_brid[ifid]
                dmac = "00:11:11:11:11:00"
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

            for brid in brids:
                dmac = "FF:FF:FF:FF:FF:FF"
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])


class TestYid(BfRuntimeTest):
    # YID Prune table tests
    # Tests three different cases
    #    - no pruning
    #    - prune map table pruning
    #    - global rid pruning
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - Programs multicast forwarding rule based on multicast DMAC.
            If multicast DMAC matches for an ingress port, separate MGID
            will be assigned and packet will be flooded based on the
            multicast programming for the MGID
          - Assigns a YID (L2 XID) for each packet based on the source port
          - For each MGID, creates a L1 node and adds random number of ports
            to the L1 node
          - Programs the YID table for each port to prune random ports
          - Sends multicast packet to each port and verifies that the packet
            gets flooded based on the assigned MGID and also gets pruned as
            expected based on the YID table programming
          - Deletes the L2 XID (YID) assignment entry for each port and then
            sends the multicast packet to each port and verifies that the
            packet gets flooded based on the assigned MGID and no pruning
            happens this time
          - Programs the global rid and enables pruning and then sends the
            multicast packet to each port and verifies that the packet gets
            flooded based on the assigned MGID and also gets pruned as
            expected
        '''
        setup_random()

        num_pipes = int(test_param_get('num_pipes'))
        print("Num pipes is", num_pipes)
        pipe_list = [x for x in range(num_pipes)]
        num_ports = len(pipe_list) * 72

        # Legal YID values
        yid_space = []
        for yid in range(0, 288):
            yid_space.append(yid)
        # Shuffle them so we can get a random set just by taking a few from
        # the front of the list.
        random.shuffle(yid_space)

        # Legal port values
        port_space = []
        for bit_number in range(0, num_ports):
            port = BitIdxToPort(bit_number)
            port_space.append(port)

        port_list = swports

        brid_list = [0x0001,
                     0x8000]

        # Legal interface ids
        ifid_list = []
        ifid = 0
        # Dict to hold interface id for each port
        port_to_ifid = {}
        for port in port_list:
            port_to_ifid[port] = ifid
            ifid_list.append(ifid)
            ifid = ifid + 1

        # Dict to hold multicast mgid for each bridge id
        brid_to_mgid = {}
        # Dict to hold multicast L1 id for each bridge id
        brid_to_l1 = {}
        yid_to_prune_list = {}
        ifid_to_yid = {}
        # Dict to hold bridge id for each interface id
        ifid_to_brid = {}

        target = client.Target(device_id=0, pipe_id=0xffff)

        vlan_val = 0
        vlan_id = 0
        rid = 0x4321
        hash1 = 0x1FFF
        hash2 = 0x1EEE
        dmac = "E4:83:A6:93:02:4B"

        try:
            add_ports(self)

            # Add Ing Port table entries
            print("Adding SwitchIngress.ing_port table entries")
            for port in port_list:
                ifid = port_to_ifid[port]
                self.ing_port_table.entry_add(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])],
                    [self.ing_port_table.make_data([client.DataTuple('ifid', ifid)],
                                                   'SwitchIngress.set_ifid')]
                )

            # Add Ing Src IFID table entries
            i = 0
            print("Adding SwitchIngress.ing_port table entries")
            for ifid in ifid_list:
                ifid_to_yid[ifid] = yid_space[i]
                ifid_to_brid[ifid] = brid_list[i % 2]
                i = i + 1
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([
                        client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([
                        client.DataTuple('rid', rid),
                        client.DataTuple('yid', ifid_to_yid[ifid]),
                        client.DataTuple('brid', ifid_to_brid[ifid]),
                        client.DataTuple('hash1', hash1),
                        client.DataTuple('hash2', hash2)],
                        'SwitchIngress.set_src_ifid_md',)]
                )

            # Add DMAC entries to flood
            print("Adding SwitchIngress.ing_dmac table entries")
            for brid in brid_list:
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key([
                        client.KeyTuple('ig_md.brid', brid),
                        client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([], 'SwitchIngress.flood')]
                )

            # Allocate an MGID per brid
            print("Adding MGID table entries")
            for brid in brid_list:
                self.mgid_table.entry_add(
                    target,
                    [self.mgid_table.make_key([client.KeyTuple('$MGID', (brid & 0xFFFF))])])
                brid_to_mgid[brid] = (brid & 0xFFFF)

            # Allocate a single L1 node for each brid
            print("Adding MC node table entries")
            l1_id = 1
            for brid in brid_list:
                port_map = set_port_map([])
                lag_map = set_lag_map([])
                self.node_table.entry_add(
                    target,
                    [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', l1_id)])],
                    [self.node_table.make_data([client.DataTuple('$MULTICAST_RID', rid),
                                                client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=[]),
                                                client.DataTuple('$DEV_PORT', int_arr_val=[])])])
                brid_to_l1[brid] = l1_id
                l1_id = l1_id + 1

            # Add L2 nodes to the L1s
            ports_in_tree = {}
            # Add all ports to the L1s
            for i in range(0, len(brid_list)):
                ports_in_tree[brid_list[i]] = port_list
                self.node_table.entry_mod(
                    target,
                    [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid_list[i]])])],
                    [self.node_table.make_data([client.DataTuple('$MULTICAST_RID', rid),
                                                client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=[]),
                                                client.DataTuple('$DEV_PORT', int_arr_val=port_list)])])
                self.mgid_table.entry_mod(
                    target,
                    [self.mgid_table.make_key([client.KeyTuple('$MGID', brid_to_mgid[brid_list[i]])])],
                    [self.mgid_table.make_data([
                        client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[brid_to_l1[brid_list[i]]]),
                        client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                        client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])

            # Program the YID table.
            # The YID used by the first ifid will prune all ports
            # The YID used by the second table will prune no ports
            for i in range(0, len(ifid_list)):
                yid = yid_space[i]
                pruned_ports = []
                if 0 == i:
                    port_cnt = len(port_space)
                    pruned_ports = port_space
                elif 1 == i:
                    port_cnt = 0
                    pruned_ports = []
                else:
                    port_cnt = random.randint(0, num_ports)
                    pruned_ports = random.sample(port_space, port_cnt)
                yid_to_prune_list[yid] = pruned_ports
                print("Adding MC prune table entries")
                self.prune_table.entry_add(
                    target,
                    [self.prune_table.make_key([client.KeyTuple('$MULTICAST_L2_XID', yid)])],
                    [self.prune_table.make_data([client.DataTuple('$DEV_PORT', int_arr_val=pruned_ports)])])
                # TODO: Add verification and enable entry get test
                '''
                resp = self.get_table_entry(
                    target,
                    '$pre.prune',
                    [table.make_key([client.KeyTuple('$MULTICAST_L2_XID', yid)])],
                    {"from_hw":True})
                data_dict = next(self.parseEntryGetResponse(resp))
                '''

            for port in port_list:
                ifid = port_to_ifid[port]
                yid = ifid_to_yid[ifid]
                brid = ifid_to_brid[ifid]
                prune_list = yid_to_prune_list[yid]
                mbr_port_list = ports_in_tree[brid]
                # Remove ports in prune_list from mbr_port_list
                rx_port_list = []
                for p in mbr_port_list:
                    if p not in prune_list:
                        rx_port_list.append(p)
                print("Sending multicast on port", port, "expecting receive on ports", rx_port_list)
                sys.stdout.flush()
                rx_pkt_list = build_rx_packet_list(rx_port_list, simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                                                                   eth_src='00:22:22:22:22:22',
                                                                                   ip_src='1.1.1.1',
                                                                                   ip_dst='10.0.0.1',
                                                                                   ip_ttl=64,
                                                                                   ip_id=0x4321))
                pkt = simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                        eth_src='00:22:22:22:22:22',
                                        ip_src='1.1.1.1',
                                        ip_dst='10.0.0.1',
                                        ip_id=0x4321,
                                        ip_ttl=64)
                send_packet(self, port, pkt)
                verify_packet_list(self, [rx_port_list], [rx_pkt_list])

            # Change ingress RID so that it no longer matches the L1 node RID.
            i = 0
            for ifid in ifid_list:
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])
            i = 0
            for ifid in ifid_list:
                ifid_to_yid[ifid] = yid_space[i]
                # Use alternate brids each time from the available 2 brids
                ifid_to_brid[ifid] = brid_list[i % 2]
                i = i + 1
                rid = 0x4322
                hash1 = 0x1FFF
                hash2 = 0x1EEE
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([
                        client.DataTuple('rid', rid),
                        client.DataTuple('yid', ifid_to_yid[ifid]),
                        client.DataTuple('brid', ifid_to_brid[ifid]),
                        client.DataTuple('hash1', hash1),
                        client.DataTuple('hash2', hash2)],
                        'SwitchIngress.set_src_ifid_md')]
                )

            # Send traffic again and nothing should be pruned
            for port in port_list:
                ifid = port_to_ifid[port]
                yid = ifid_to_yid[ifid]
                brid = ifid_to_brid[ifid]
                mbr_port_list = ports_in_tree[brid]
                rx_port_list = []
                for p in mbr_port_list:
                    rx_port_list.append(p)
                print("Sending multicast on port", port, "expecting receive on ports (NO PRUNING)", rx_port_list)
                sys.stdout.flush()
                pkt = simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                        eth_src='00:22:22:22:22:22',
                                        ip_src='1.1.1.1',
                                        ip_dst='10.0.0.1',
                                        ip_id=0x4321,
                                        ip_ttl=64)
                rx_pkt_list = build_rx_packet_list(rx_port_list, simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                                                                   eth_src='00:22:22:22:22:22',
                                                                                   ip_src='1.1.1.1',
                                                                                   ip_dst='10.0.0.1',
                                                                                   ip_id=0x4321,
                                                                                   ip_ttl=64))
                send_packet(self, port, pkt)
                verify_packet_list(self, [rx_port_list], [rx_pkt_list])

            global_rid = 0x4322
            self.mgid_table.attribute_pre_device_config_set(target, global_rid=global_rid)
            for port in port_list:
                ifid = port_to_ifid[port]
                yid = ifid_to_yid[ifid]
                brid = ifid_to_brid[ifid]
                prune_list = yid_to_prune_list[yid]
                mbr_port_list = ports_in_tree[brid]
                # Remove ports in prune_list from mbr_port_list
                rx_port_list = []
                for p in mbr_port_list:
                    if p not in prune_list:
                        rx_port_list.append(p)
                print("Sending multicast on port", port, "expecting receive on ports (GLOBAL PRUNE)", rx_port_list)
                sys.stdout.flush()
                pkt = simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                        eth_src='00:22:22:22:22:22',
                                        ip_src='1.1.1.1',
                                        ip_dst='10.0.0.1',
                                        ip_id=101,
                                        ip_ttl=64)
                rx_pkt_list = build_rx_packet_list(rx_port_list, simple_tcp_packet(eth_dst='E4:83:A6:93:02:4B',
                                                                                   eth_src='00:22:22:22:22:22',
                                                                                   ip_src='1.1.1.1',
                                                                                   ip_dst='10.0.0.1',
                                                                                   ip_id=101,
                                                                                   ip_ttl=64))
                send_packet(self, port, pkt)
                verify_packet_list(self, [rx_port_list], [rx_pkt_list])

        finally:
            print("Cleaning up")
            sys.stdout.flush()
            rmv_ports(self)
            global_rid = 0
            self.mgid_table.attribute_pre_device_config_set(target, global_rid=global_rid)
            for i in range(0, len(ifid_list)):
                yid = yid_space[i]
                # There is no delete API for prune table. Setting the
                # prune map to 0 is equivalent to deleting the entry
                self.prune_table.entry_add(
                    target,
                    [self.prune_table.make_key([client.KeyTuple('$MULTICAST_L2_XID', yid)])],
                    [self.prune_table.make_data([client.DataTuple('$DEV_PORT', int_arr_val=[])])])
            for brid in brid_to_mgid:
                self.mgid_table.entry_del(
                    target,
                    [self.mgid_table.make_key([client.KeyTuple('$MGID', brid_to_mgid[brid])])])

            for brid in brid_to_l1:
                self.node_table.entry_del(
                    target,
                    [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', brid_to_l1[brid])])])

            # Delete Ing Port table entries
            for port in port_list:
                vlan_val = 0
                vlan_id = 0
                self.ing_port_table.entry_del(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])])

            # Delete Ing Src IFID table entries
            for ifid in ifid_list:
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])

            # Delete DMAC entries to flood
            for brid in brid_list:
                dmac = "E4:83:A6:93:02:4B"
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])


class TestXid(BfRuntimeTest):
    # Tests L1 Nodes XID pruning
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - Programs multicast forwarding rule based on destination IP.
            Creates this rule for 8 different destination IPs
          - For each destination IP, sets two MGIDs and a L1 XID and
            also sets a YID (L2 XID)
          - Creates 16 MGID entries (2 entries for each destination IP)
          - Each MGID entry has 4 L1 nodes. Randomly 3 out of 4 L1 nodes
            will have L1 XID valid and programs a valid L1 XID
          - Programs YID (L2 XID) for each port with a random list of ports
          - Sends a packet from each port to each of the destination IPs
            and verifies that the packet gets flooded for both MGIDs assigned
            to the destination IP but it also gets pruned as expected based on
            L1 XID and YID
        '''
        global t
        setup_random()

        target = client.Target(device_id=0, pipe_id=0xffff)

        num_pipes = int(test_param_get('num_pipes'))
        print("Num pipes is", num_pipes)
        pipe_list = [x for x in range(num_pipes)]
        num_ports = len(pipe_list) * 72
        t.setup(self, target)

        # Legal YID values
        yid_space = []
        for yid in range(0, 288):
            yid_space.append(yid)
        # Shuffle them so we can get a random set just by taking a few from
        # the front of the list.
        random.shuffle(yid_space)

        # Legal port values
        port_space = []
        for bit_number in range(0, num_ports):
            port = BitIdxToPort(bit_number)
            port_space.append(port)

        # Ports this test uses.
        port_list = swports

        brid_list = [0x9347]

        ip_addr_str_list = ["224.100.0.0",
                            "224.100.0.1",
                            "224.100.0.2",
                            "224.100.0.3",
                            "224.100.0.4",
                            "224.100.0.5",
                            "224.100.0.6",
                            "224.100.0.7"]
        ip_addr_list = []
        for ip in ip_addr_str_list:
            ip_addr_list.append(ipv4Addr_to_i32(ip))
        # Dict to store the MGID1 for each IP
        ip_to_mgid1 = {}
        # Dict to store the MGID2 for each IP
        ip_to_mgid2 = {}
        # Dict to store the L1 exclusion id for each IP
        ip_to_xid = {}
        # Randomly select XIDs equal to the number of ip addresses
        # from the range between 0 and 64K
        xid_list = random.sample(list(range(0x10000)), len(ip_addr_list))
        # Randomly select MGIDs equal to twice the number of ip addresses
        # from the range between 0 and 64K
        mgid_sample_space = random.sample(list(range(1, 0x10000)), 2 * len(ip_addr_list))
        for ip in ip_addr_list:
            ip_to_mgid1[ip] = mgid_sample_space.pop(0)
            ip_to_mgid2[ip] = mgid_sample_space.pop(0)
            ip_to_xid[ip] = xid_list.pop(0)
            mgid_sample_space.append(ip_to_mgid1[ip])
            mgid_sample_space.append(ip_to_mgid2[ip])
            xid_list.append(ip_to_xid[ip])

        # Legal interface ids
        ifid_list = []
        ifid = 0
        # Dict to store interface id for each port
        port_to_ifid = {}
        for port in port_list:
            port_to_ifid[port] = ifid
            ifid_list.append(ifid)
            ifid = ifid + 1

        # Dict to store L2 YID for each interface id
        ifid_to_yid = {}
        # Dict to store bridge id for each interface id
        ifid_to_brid = {}

        mc_trees = {}

        vlan_val = 0
        vlan_id = 0
        rid = 0x4321
        hash1 = 0x1FFF
        hash2 = 0x0EEE
        dmac = "EE:EE:EE:EE:EE:EE"
        src_ip = 0
        src_ip_mask = 0
        dst_ip_mask = "255.255.255.255"

        try:
            add_ports(self)

            # Add Ing Port table entries
            print("Adding SwitchIngress.ing_port table entries")
            for port in port_list:
                ifid = port_to_ifid[port]
                self.ing_port_table.entry_add(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])],
                    [self.ing_port_table.make_data([client.DataTuple('ifid', ifid)],
                                                   'SwitchIngress.set_ifid')]
                )

            # Add Ing Src IFID table entries
            print("Adding SwitchIngress.ing_src_ifid table entries")
            i = 0
            for ifid in ifid_list:
                ifid_to_yid[ifid] = yid_space[i]
                ifid_to_brid[ifid] = brid_list[0]
                i = i + 1
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([
                        client.DataTuple('rid', rid),
                        client.DataTuple('yid', ifid_to_yid[ifid]),
                        client.DataTuple('brid', ifid_to_brid[ifid]),
                        client.DataTuple('hash1', hash1),
                        client.DataTuple('hash2', hash2)],
                        'SwitchIngress.set_src_ifid_md')]
                )

            # Add DMAC entries to route
            print("Adding SwitchIngress.ing_dmac table entries")
            for brid in brid_list:
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key([
                        client.KeyTuple('ig_md.brid', brid),
                        client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([client.DataTuple('vrf', brid)],
                                                   'SwitchIngress.route')]
                )

            # Add IP entries
            print("Adding SwitchIngress.ing_ipv4_mcast table entries")
            for ip in ip_addr_list:
                mgid1 = ip_to_mgid1[ip]
                mgid2 = ip_to_mgid2[ip]
                xid = ip_to_xid[ip]
                self.ing_ipv4_mcast_table.entry_add(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([
                        client.KeyTuple('ig_md.vrf', brid_list[0]),
                        client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                        client.KeyTuple('hdr.ipv4.dst_addr', ip_addr_str_list[ip_addr_list.index(ip)],
                                        dst_ip_mask),
                        client.KeyTuple('$MATCH_PRIORITY', 1)])],
                    [self.ing_ipv4_mcast_table.make_data([client.DataTuple('xid', xid),
                                                          client.DataTuple('mgid1', mgid1),
                                                          client.DataTuple('mgid2', mgid2)],
                                                         'SwitchIngress.mcast_route')]
                )

            # Build the multicast trees, four nodes for each MGID.
            for mgid in mgid_sample_space:
                mct = MCTree(self, target, mgid)
                for x in range(4):
                    xid = random.choice(xid_list)
                    if 1 == random.choice([1, 2, 3, 4]):
                        xid = None
                    mct.add_node(0x4320 + x, xid, port_list, [])
                mc_trees[mgid] = mct

            # Program the YID table.
            # The YID used by the first ifid will prune all ports
            # The YID used by the second table will prune no ports
            for i in range(0, len(ifid_list)):
                yid = yid_space[i]
                pruned_ports = []
                if 0 == i:
                    t.get_yid_tbl().set_pruned_ports(yid, port_list)
                elif 1 == i:
                    t.get_yid_tbl().set_pruned_ports(yid, [])
                else:
                    port_cnt = random.randint(0, len(port_list))
                    pruned_ports = random.sample(port_list, port_cnt)
                    t.get_yid_tbl().set_pruned_ports(yid, pruned_ports)

            test_ports = random.sample(port_list, len(port_list) // 2)
            for port in test_ports:
                test_addrs = random.sample(ip_addr_list, 2)
                for ip in test_addrs:
                    rid = 0x4321
                    tree1 = mc_trees[ip_to_mgid1[ip]]
                    tree2 = mc_trees[ip_to_mgid2[ip]]
                    ifid = port_to_ifid[port]
                    yid = ifid_to_yid[ifid]
                    xid = ip_to_xid[ip]
                    exp_data_list_1 = tree1.get_ports(rid, xid, yid)
                    exp_data_list_2 = tree2.get_ports(rid, xid, yid)
                    rx_port_list = []
                    rx_pkt_list = []
                    pkt = simple_tcp_packet(eth_dst='EE:EE:EE:EE:EE:EE',
                                            eth_src='AA:BB:CC:DD:EE:FF',
                                            ip_src='1.1.1.1',
                                            ip_dst=ip_addr_str_list[ip_addr_list.index(ip)],
                                            ip_ttl=64,
                                            ip_id=0xABCD)

                    for exp_data in exp_data_list_1 + exp_data_list_2:
                        exp_rid, exp_ports = exp_data
                        rx_port_list.append(exp_ports)
                        exp_pkt = simple_tcp_packet(eth_dst='EE:EE:EE:EE:EE:EE',
                                                    eth_src='AA:BB:CC:DD:EE:FF',
                                                    ip_src='1.1.1.1',
                                                    ip_dst=ip_addr_str_list[ip_addr_list.index(ip)],
                                                    ip_ttl=63,
                                                    ip_id=0xABCD)
                        rx_pkt_list.append(build_rx_packet_list(exp_ports, exp_pkt))
                        # print "RID:", hex(exp_rid), "Ports:", exp_ports
                    print("Sending", ip_addr_str_list[
                        ip_addr_list.index(ip)], "on port", port, "expecting receive on ports:", rx_port_list)
                    print(sys.stdout.flush())
                    send_packet(self, port, pkt)
                    verify_packet_list(self, rx_port_list, rx_pkt_list)

        finally:
            print("Cleaning up")
            sys.stdout.flush()
            rmv_ports(self)

            # Delete Ing Port table entries
            for port in port_list:
                vlan_val = 0
                vlan_id = 0
                ifid = port_to_ifid[port]
                self.ing_port_table.entry_del(
                    target,
                    [self.ing_port_table.make_key([
                        client.KeyTuple('ig_intr_md.ingress_port', port),
                        client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                        client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])])

            # Delete Ing Src IFID table entries
            i = 0
            for ifid in ifid_list:
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])

            # Delete DMAC entries
            for brid in brid_list:
                dmac = "EE:EE:EE:EE:EE:EE"
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

            # Delete IP entries
            for ip in ip_addr_list:
                src_ip = 0
                src_ip_mask = 0
                dst_ip_mask = "255.255.255.255"
                self.ing_ipv4_mcast_table.entry_del(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([client.KeyTuple('ig_md.vrf', brid_list[0]),
                                                         client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                                                         client.KeyTuple('hdr.ipv4.dst_addr', ip_addr_str_list[ip_addr_list.index(ip)],
                                                                         dst_ip_mask),
                                                         client.KeyTuple('$MATCH_PRIORITY', 1)])])

            for key in mc_trees:
                mc_trees[key].cleanUp()
            t.cleanUp()


class TestEcmp(BfRuntimeTest):
    # Tests many different cases for ECMP table
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - Programs multicast forwarding rule based on destination IP.
            Creates this rule for 4 different destination IPs
          - For each destination IP, sets two MGIDs and a L1 XID and
            also sets a YID (L2 XID)
          - Creates 8 ECMP groups with random numbers of members for each
            group between 0 and 32 members
          - Creates 8 MGID entries (2 entries for each destination IP)
          - Each MGID entry has all 8 ECMP groups and an optional L1
            nodes but each will have a random XID value and valid status.
          - Programs YID (L2 XID) for each port with a random list of ports
          - Since this test configures multiple MGIDs & ECMP groups and also
            few hundreds of L1 nodes (around 300), this also tests and verifies
            EntryGetFirst, EntryGetNext and UsageGet for MGID, ECMP and
            L1 node tables. This is done by reading all entries from each of
            these tables by not passing key from GRPC client for EntryGet
            and for which GRPC server does UsageGet, EntryGetFirst and then
            EntryGetNext for rest of the entries.
          - Sends a packet from each port to each of the destination IPs
            and verifies that the packet gets flooded for both MGIDs assigned
            to the destination IP and member within ECMP is chosen based on
            hash config and finally it also gets pruned as expected based on
            L1 XID and YID
        '''
        global t
        setup_random()

        target = client.Target(device_id=0, pipe_id=0xffff)

        num_pipes = int(test_param_get('num_pipes'))
        print("Num pipes is", num_pipes)
        pipe_list = [x for x in range(num_pipes)]
        num_ports = len(pipe_list) * 72
        t.setup(self, target)

        # Legal YID values
        yid_space = []
        for yid in range(0, 288):
            yid_space.append(yid)
        # Shuffle them so we can get a random set just by taking a few from
        # the front of the list.
        random.shuffle(yid_space)

        # Legal port values
        port_space = []
        for bit_number in range(0, num_ports):
            port = BitIdxToPort(bit_number)
            port_space.append(port)

        # Ports this test uses.
        port_list = swports

        brid_list = [0x9347]

        ip_addr_str_list = ["224.100.0.0",
                            "224.100.0.1",
                            "224.100.0.2",
                            "224.100.0.3"]
        ip_addr_list = []
        for ip in ip_addr_str_list:
            ip_addr_list.append(ipv4Addr_to_i32(ip))
        # Dict to store the MGID1 for each IP
        ip_to_mgid1 = {}
        # Dict to store the MGID2 for each IP
        ip_to_mgid2 = {}
        # Dict to store the L1 exclusion id for each IP
        ip_to_xid = {}
        # Randomly select XIDs equal to the number of ip addresses
        # from the range between 0 and 64K
        xid_list = random.sample(list(range(0x10000)), len(ip_addr_list))
        # Randomly select MGIDs equal to twice the number of ip addresses
        # from the range between 0 and 64K
        mgid_sample_space = random.sample(list(range(1, 0x10000)), 2 * len(ip_addr_list))
        # Sort the MGIDs to test and verify EntryGetFirst and EntryGetNext
        mgid_sample_space.sort()
        for ip in ip_addr_list:
            ip_to_mgid1[ip] = mgid_sample_space.pop(0)
            ip_to_mgid2[ip] = mgid_sample_space.pop(0)
            ip_to_xid[ip] = xid_list.pop(0)
            mgid_sample_space.append(ip_to_mgid1[ip])
            mgid_sample_space.append(ip_to_mgid2[ip])
            xid_list.append(ip_to_xid[ip])

        # Legal interface ids
        ifid_list = []
        ifid = 0
        # Dict to store interface id for each port
        port_to_ifid = {}
        for port in port_list:
            port_to_ifid[port] = ifid
            ifid_list.append(ifid)
            ifid = ifid + 1

        # Dict to store L2 YID for each interface id
        ifid_to_yid = {}
        # Dict to store bridge id for each interface id
        ifid_to_brid = {}

        # Dict to store MC tree for each MGID
        mc_trees = {}
        # List of ECMPs created
        ecmp_grps = []

        vlan_val = 0
        vlan_id = 0
        rid = 0x4321
        hash1 = 0x1FFF
        hash2 = 0x0EEE
        dmac = "EE:EE:EE:EE:EE:EE"
        src_ip = 0
        src_ip_mask = 0
        dst_ip_mask = "255.255.255.255"

        try:
            add_ports(self)

            # Add Ing Port table entries
            print("Adding SwitchIngress.ing_port table entries")
            for port in port_list:
                ifid = port_to_ifid[port]
                self.ing_port_table.entry_add(
                    target,
                    [self.ing_port_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', port),
                                                   client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                                                   client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])],
                    [self.ing_port_table.make_data([client.DataTuple('ifid', ifid)],
                                                   'SwitchIngress.set_ifid')]
                )

            # Add Ing Src IFID table entries
            print("Adding SwitchIngress.ing_src_ifid table entries")
            i = 0
            for ifid in ifid_list:
                ifid_to_yid[ifid] = yid_space[i]
                ifid_to_brid[ifid] = brid_list[0]
                i = i + 1
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([client.DataTuple('rid', rid),
                                                        client.DataTuple('yid', ifid_to_yid[ifid]),
                                                        client.DataTuple('brid', ifid_to_brid[ifid]),
                                                        client.DataTuple('hash1', hash1),
                                                        client.DataTuple('hash2', hash2)],
                                                       'SwitchIngress.set_src_ifid_md')]
                )

            # Add DMAC entries to route
            print("Adding SwitchIngress.ing_dmac table entries")
            for brid in brid_list:
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([client.DataTuple('vrf', brid)],
                                                   'SwitchIngress.route')]
                )

            # Add IP entries
            print("Adding SwitchIngress.ing_ipv4_mcast table entries")
            for ip in ip_addr_list:
                mgid1 = ip_to_mgid1[ip]
                mgid2 = 0
                xid = ip_to_xid[ip]
                self.ing_ipv4_mcast_table.entry_add(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([client.KeyTuple('ig_md.vrf', brid_list[0]),
                                                         client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                                                         client.KeyTuple('hdr.ipv4.dst_addr', ip_addr_str_list[ip_addr_list.index(ip)],
                                                                         dst_ip_mask),
                                                         client.KeyTuple('$MATCH_PRIORITY', 1)])],
                    [self.ing_ipv4_mcast_table.make_data([client.DataTuple('xid', xid),
                                                          client.DataTuple('mgid1', mgid1),
                                                          client.DataTuple('mgid2', mgid2)],
                                                         'SwitchIngress.mcast_route')]
                )

            # Setup some LAGs for the ECMP groups to use.
            lag_ids = random.sample(list(range(255)), 8)
            for i in lag_ids:
                count = random.randint(0, len(port_list))
                mbrs = random.sample(port_list, count)
                t.get_lag_tbl().getLag(i).addMbr(mbrs)
                # Set left and right counts 20% of the time
                for i in lag_ids:
                    if random.randint(1, 5) == 1:
                        left = random.randint(0, 8)
                        right = random.randint(0, 3)
                        lag = t.get_lag_tbl().getLag(i)
                        # lag.setRmtCnt(left, right)

            # Build the ECMP groups.  Each group should have 0 to 32 members.
            for g_idx in range(8):
                grp = EcmpGrp(self, target)
                # One group with no members
                if g_idx == 0:
                    pass
                # One group with 32 members
                elif g_idx == 1:
                    for rid in range(32):
                        ports = random.sample(port_list, random.randint(0, len(port_list)))
                        lags = random.sample(lag_ids, random.randint(0, len(lag_ids)))
                        node_rid = rid | (g_idx << 8)
                        grp.addMbr(node_rid, ports, lags)
                # Other groups with random members, do Add-Del to ensure there
                # are holes in the group
                else:
                    add_cnt = 32
                    del_cnt = random.randint(0, add_cnt)
                    for rid in range(add_cnt):
                        ports = random.sample(port_list, random.randint(0, len(port_list)))
                        lags = random.sample(lag_ids, random.randint(0, len(lag_ids)))
                        node_rid = rid | (g_idx << 8)
                        grp.addMbr(node_rid, ports, lags)
                    for del_idx in random.sample(list(range(add_cnt)), del_cnt):
                        grp.rmvMbr(del_idx)
                ecmp_grps.append(grp)

            # Build the multicast trees, four nodes for each MGID.
            for mgid in mgid_sample_space:
                mct = MCTree(self, target, mgid)
                for x in range(4):
                    # Optionally add a node
                    if 1 == random.choice([1, 2]):
                        xid = random.choice(xid_list)
                        if 1 == random.choice([1, 2, 3, 4]):
                            xid = None
                        mct.add_node(0x4320 + x, xid, port_list, [])

                # Add half the ECMP groups
                for g in ecmp_grps[:len(ecmp_grps) // 2]:
                    xid = random.choice(xid_list)
                    if 1 == random.choice([1, 2, 3, 4]):
                        xid = None
                    mct.add_ecmp(g, xid)

                    # Optionally add a node
                for x in range(4):
                    if 1 == random.choice([1, 2]):
                        xid = random.choice(xid_list)
                        if 1 == random.choice([1, 2, 3, 4]):
                            xid = None
                        mct.add_node(0x4320 + x, xid, port_list, [])

                # Add remaining half of the ECMP groups
                for g in ecmp_grps[len(ecmp_grps) // 2:]:
                    xid = random.choice(xid_list)
                    if 1 == random.choice([1, 2, 3, 4]):
                        xid = None
                    mct.add_ecmp(g, xid)

                for x in range(4):
                    # Optionally add a node
                    if 1 == random.choice([1, 2]):
                        xid = random.choice(xid_list)
                        if 1 == random.choice([1, 2, 3, 4]):
                            xid = None
                        mct.add_node(0x4320 + x, xid, port_list, [])

                mc_trees[mgid] = mct

            # Program the YID table.
            # The YID used by the first ifid will prune all ports
            # The YID used by the second table will prune no ports
            for i in range(0, len(ifid_list)):
                yid = yid_space[i]
                pruned_ports = []
                if 0 == i:
                    t.get_yid_tbl().set_pruned_ports(yid, port_list)
                elif 1 == i:
                    t.get_yid_tbl().set_pruned_ports(yid, [])
                else:
                    port_cnt = random.randint(0, len(port_list))
                    pruned_ports = random.sample(port_list, port_cnt)
                    t.get_yid_tbl().set_pruned_ports(yid, pruned_ports)

            # Since the test doesn't cache all L1 nodes created, build
            # the list of L1 nodes in order to test and verify EntryGetFirst
            # and EntryGetNext
            l1_nodes = []
            # First get all the ECMP groups and verify. Also, add the L1 nodes
            # in the ECMP groups to the L1 nodes list.
            resp = self.ecmp_table.entry_get(target, None, {"from_hw": True})
            i = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                ecmp_grp = ecmp_grps[i]
                assert key_dict['$MULTICAST_ECMP_ID']['value'] == ecmp_grp.ecmp_id
                num_nodes = 0
                for index in range(32):
                    if ecmp_grp.mbrs[index] != None:
                        num_nodes = num_nodes + 1
                assert len(data_dict['$MULTICAST_NODE_ID']) == num_nodes
                j = 0
                for l1 in ecmp_grp.mbrs:
                    if l1 is None:
                        continue
                    assert data_dict['$MULTICAST_NODE_ID'][j] == l1.node_id
                    l1_nodes.append(l1)
                    j = j + 1
                i = i + 1

            # Get all the MGIDs and verify. Also, add the L1 nodes
            # in the MGIDs to the L1 nodes list.
            resp = self.mgid_table.entry_get(target, None, {"from_hw": True})
            i = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                mgrp = mc_trees[mgid_sample_space[i]]
                assert key_dict['$MGID']['value'] == mgrp.mgid
                assert len(data_dict['$MULTICAST_NODE_ID']) == len(mgrp.nodes)
                assert len(data_dict['$MULTICAST_NODE_L1_XID_VALID']) == len(mgrp.nodes)
                assert len(data_dict['$MULTICAST_NODE_L1_XID']) == len(mgrp.nodes)
                j = 0
                for l1 in mgrp.nodes:
                    assert data_dict['$MULTICAST_NODE_ID'][j] == l1.node_id
                    l1_nodes.append(l1)
                    if l1.xid is not None:
                        assert data_dict['$MULTICAST_NODE_L1_XID'][j] == l1.xid
                        assert data_dict['$MULTICAST_NODE_L1_XID_VALID'][j] == True
                    else:
                        assert data_dict['$MULTICAST_NODE_L1_XID'][j] == 0
                        assert data_dict['$MULTICAST_NODE_L1_XID_VALID'][j] == False
                    j = j + 1

                assert len(data_dict['$MULTICAST_ECMP_ID']) == len(mgrp.ecmps)
                assert len(data_dict['$MULTICAST_ECMP_L1_XID_VALID']) == len(mgrp.ecmps)
                assert len(data_dict['$MULTICAST_ECMP_L1_XID']) == len(mgrp.ecmps)
                j = 0
                for grp, xid in mgrp.ecmps:
                    assert data_dict['$MULTICAST_ECMP_ID'][j] == grp.ecmp_id
                    if xid is not None:
                        assert data_dict['$MULTICAST_ECMP_L1_XID'][j] == xid
                        assert data_dict['$MULTICAST_ECMP_L1_XID_VALID'][j] == True
                    else:
                        assert data_dict['$MULTICAST_ECMP_L1_XID'][j] == 0
                        assert data_dict['$MULTICAST_ECMP_L1_XID_VALID'][j] == False
                    j = j + 1
                i = i + 1

            # Get all the L1 nodes and verify
            resp = self.node_table.entry_get(target, None, {"from_hw": True})
            i = 0
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                l1 = l1_nodes[i]
                assert key_dict['$MULTICAST_NODE_ID']['value'] == l1.node_id
                assert data_dict['$MULTICAST_RID'] == l1.rid
                assert data_dict['$DEV_PORT'] == l1.mbr_ports
                assert data_dict['$MULTICAST_LAG_ID'] == l1.mbr_lags
                i = i + 1

            test_ports = random.sample(port_list, len(port_list) // 2)
            for port in test_ports:
                for ip in ip_addr_list:
                    rid = 0x4321
                    tree1 = mc_trees[ip_to_mgid1[ip]]
                    ifid = port_to_ifid[port]
                    yid = ifid_to_yid[ifid]
                    xid = ip_to_xid[ip]
                    exp_data_list_1 = tree1.get_ports(rid, xid, yid, 0x1FFF, 0x0EEE)
                    rx_port_list = []
                    rx_pkt_list = []
                    pkt = simple_tcp_packet(eth_dst='EE:EE:EE:EE:EE:EE',
                                            eth_src='AA:BB:CC:DD:EE:FF',
                                            ip_src='1.1.1.1',
                                            ip_dst=ip_addr_str_list[ip_addr_list.index(ip)],
                                            ip_ttl=64,
                                            ip_id=0xABCD)

                    for exp_data in exp_data_list_1:
                        exp_rid, exp_ports = exp_data
                        rx_port_list.append(exp_ports)
                        exp_pkt = simple_tcp_packet(eth_dst='EE:EE:EE:EE:EE:EE',
                                                    eth_src='AA:BB:CC:DD:EE:FF',
                                                    ip_src='1.1.1.1',
                                                    ip_dst=ip_addr_str_list[ip_addr_list.index(ip)],
                                                    ip_ttl=63,
                                                    ip_id=0xABCD)
                        rx_pkt_list.append(build_rx_packet_list(exp_ports, exp_pkt))

                    print("Sending", ip_addr_str_list[
                        ip_addr_list.index(ip)], "on port", port, "expecting receive on ports:", rx_port_list)
                    print(sys.stdout.flush())
                    send_packet(self, port, pkt)
                    verify_packet_list(self, rx_port_list, rx_pkt_list)

        finally:
            print("Cleaning up")
            sys.stdout.flush()
            rmv_ports(self)

            # Delete Ing Port table entries
            for port in port_list:
                vlan_val = 0
                vlan_id = 0
                ifid = port_to_ifid[port]
                self.ing_port_table.entry_del(
                    target,
                    [self.ing_port_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', port),
                                                   client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                                                   client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])])

            # Delete Ing Src IFID table entries
            i = 0
            for ifid in ifid_list:
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])

            # Delete DMAC entries
            for brid in brid_list:
                dmac = "EE:EE:EE:EE:EE:EE"
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

            # Delete IP entries
            for ip in ip_addr_list:
                src_ip = 0
                src_ip_mask = 0
                dst_ip_mask = "255.255.255.255"
                self.ing_ipv4_mcast_table.entry_del(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([client.KeyTuple('ig_md.vrf', brid_list[0]),
                                                         client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                                                         client.KeyTuple('hdr.ipv4.dst_addr', ip_addr_str_list[ip_addr_list.index(ip)],
                                                                         dst_ip_mask),
                                                         client.KeyTuple('$MATCH_PRIORITY', 1)])])

            for key in mc_trees:
                mc_trees[key].cleanUp()
            for grp in ecmp_grps:
                grp.cleanUp()
            t.cleanUp()


class TestLag(BfRuntimeTest):
    # Tests many different cases for LAG table
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - Randomly selects 64 LAG ids from the available 256 LAG ids
          - Runs the following test for 8 iterations by picking up 8 LAG
            ids consecutively from the list of 64 LAG ids each time (so
            covers all 64 in the list)
                - Programs multicast forwarding rule based on destination IP
                  but to match any destination IP (match ALL)
                - Creates a MGID entry with one L1 node and adds all 8 LAG ids
                  to that node
                - Adds random list of port members to each LAG id
                - Programs YID table entry with random list of port members
                - Randomly selects a port from the available port list and
                  sends a packet to that ingress port and verifies that packet
                  grets flooded based on all LAG members and gets pruned based
                  on YID table programming
        '''
        global t
        setup_random()

        target = client.Target(device_id=0, pipe_id=0xffff)

        num_pipes = int(test_param_get('num_pipes'))
        print("Num pipes is", num_pipes)
        pipe_list = [x for x in range(num_pipes)]

        port_list = list(swports)

        hw_down_ports = random.sample(port_list, len(port_list) // 2)
        if not support_hw_port_flap:
            hw_down_ports = []
        print("HW down ports:", sorted(hw_down_ports))
        sw_down_ports = random.sample(port_list, len(port_list) // 2)
        print("SW down ports:", sorted(sw_down_ports))

        # BRI specific, no support yet for SW down ports
        sw_down_ports = []
        # BRI specific, no support yet for HW down ports
        hw_down_ports = []

        t.setup(self, target)
        t.disable_port_ff()

        port_hdl = None
        ifid_hdl = None
        dmac_hdl = None
        ipv4_hdl = None
        mct = None

        ing_port = random.choice(port_list)
        ifid = 3
        brid = 0x1234
        vrf = 0x4321
        mgid1 = 0
        mgid2 = random.randint(1, 0xFFFF)
        global_rid = 0
        irid = random.randint(1, 0xFFFF)
        xid = random.randint(0, 0xFFFF)
        yid = random.randint(0, 287)
        hash1 = random.randint(0, 0x1FFF)
        hash2 = random.randint(0, 0x1FFF)
        dmac = "00:11:22:33:44:55"
        vlan_val = 0
        vlan_id = 0
        src_ip = 0
        src_ip_mask = 0
        dst_ip = 0
        dst_ip_mask = 0

        try:
            add_ports(self)
            lags_per_pass = 8
            all_lags = list(range(255))
            test_lags = random.sample(all_lags, 64)
            while len(test_lags):
                if len(test_lags) > lags_per_pass:
                    lag_ids = test_lags[:lags_per_pass]
                    test_lags = test_lags[lags_per_pass:]
                else:
                    lag_ids = list(test_lags)
                    test_lags = []
                print("Checking LAG IDs:", lag_ids)
                sys.stdout.flush()

                # Add Ing Port table entries
                print("Adding SwitchIngress.ing_port table entries")
                self.ing_port_table.entry_add(
                    target,
                    [self.ing_port_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', ing_port),
                                                   client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                                                   client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])],
                    [self.ing_port_table.make_data([client.DataTuple('ifid', ifid)],
                                                   'SwitchIngress.set_ifid')]
                )

                # Add Ing Src IFID table entries
                print("Adding SwitchIngress.ing_src_ifid table entries")
                self.ing_src_ifid_table.entry_add(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])],
                    [self.ing_src_ifid_table.make_data([client.DataTuple('rid', irid),
                                                        client.DataTuple('yid', yid),
                                                        client.DataTuple('brid', brid),
                                                        client.DataTuple('hash1', hash1),
                                                        client.DataTuple('hash2', hash2)],
                                                       'SwitchIngress.set_src_ifid_md')]
                )

                # Add DMAC entries to route
                print("Adding SwitchIngress.ing_dmac table entries")
                self.ing_dmac_table.entry_add(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
                    [self.ing_dmac_table.make_data([client.DataTuple('vrf', vrf)],
                                                   'SwitchIngress.route')]
                )

                # Add IP entries
                print("Adding SwitchIngress.ing_ipv4_mcast table entries")
                self.ing_ipv4_mcast_table.entry_add(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([client.KeyTuple('ig_md.vrf', vrf),
                                                         client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                                                         client.KeyTuple('hdr.ipv4.dst_addr', dst_ip, dst_ip_mask),
                                                         client.KeyTuple('$MATCH_PRIORITY', 1)])],
                    [self.ing_ipv4_mcast_table.make_data([client.DataTuple('xid', xid),
                                                          client.DataTuple('mgid1', mgid1),
                                                          client.DataTuple('mgid2', mgid2)],
                                                         'SwitchIngress.mcast_route')]
                )

                # Set pruning mask
                if len(port_list) <= 4:
                    cnt = 1
                else:
                    cnt = len(port_list) // 4
                pdp = random.sample(port_list, cnt)
                t.get_yid_tbl().set_pruned_ports(yid, pdp)
                t.get_yid_tbl().set_global_rid(global_rid)

                # Build the multicast trees
                mct = MCTree(self, target, mgid2)
                erid_space = [global_rid, irid] + random.sample(list(range(0x10000)), 8)
                erid = erid_space[random.randint(0, len(erid_space) - 1)]
                mct.add_node(erid, None, [], lag_ids)

                # Add ports to the LAGs
                for i in lag_ids:
                    count = random.randint(0, len(port_list))
                    mbrs = random.sample(port_list, count)
                    t.get_lag_tbl().getLag(i).addMbr(mbrs)

                # Set left and right counts on 1 out of 4 lags
                for i in lag_ids:
                    if random.randint(1, 4) == 4:
                        left = random.randint(0, 0x1FFF)
                        right = random.randint(0, 0x1FFF)
                        lag = t.get_lag_tbl().getLag(i)
                        # lag.setRmtCnt(left, right)

                # Make sure all ports start as up.
                for p in port_list:
                    t.sw_port_up(p)
                    t.clr_hw_port_down(p)
                t.disable_port_ff()
                t.enable_port_ff()

                for step in range(1, 6):
                    print("  Step", step)
                    sys.stdout.flush()
                    if 1 == step:
                        pass

                    elif 2 == step:
                        # Mark some ports as SW down and send again
                        for p in sw_down_ports:
                            t.sw_port_down(p)

                    elif 3 == step:
                        # Mark all ports as SW up, mark some ports HW down.
                        for p in sw_down_ports:
                            t.sw_port_up(p)
                        for p in hw_down_ports:
                            t.set_port_down(p)
                        for p in hw_down_ports:
                            t.set_port_up(p)

                    elif 4 == step:
                        # Clear HW down state
                        for p in hw_down_ports:
                            t.clr_hw_port_down(p)

                    elif 5 == step:
                        # Mark some ports as SW down, mark some ports as HW down.
                        # Bring ports back up.
                        for p in sw_down_ports:
                            t.sw_port_down(p)
                        for p in hw_down_ports:
                            t.set_port_down(p)
                        for p in hw_down_ports:
                            t.set_port_up(p)

                    exp_data_list = mct.get_ports(irid, xid, yid, hash1, hash2)
                    rx_port_list = []
                    rx_rid_list = []
                    rx_pkt_list = []
                    pkt = simple_tcp_packet(eth_dst=dmac,
                                            eth_src='AA:BB:CC:DD:EE:FF',
                                            ip_src='1.1.1.1',
                                            ip_dst='10.9.8.7',
                                            ip_ttl=64,
                                            ip_id=0xABCD)

                    for exp_rid, exp_ports in exp_data_list:
                        if not exp_ports:
                            continue
                        rx_port_list.append(exp_ports)
                        rx_rid_list.append(exp_rid)
                        exp_pkt = simple_tcp_packet(eth_dst=dmac,
                                                    eth_src='AA:BB:CC:DD:EE:FF',
                                                    ip_src='1.1.1.1',
                                                    ip_dst='10.9.8.7',
                                                    ip_ttl=63,
                                                    ip_id=0xABCD)
                        rx_pkt_list.append(build_rx_packet_list(exp_ports, exp_pkt))

                    print("  Sending on port", ing_port, "expecting receive on ports:")
                    for x, y in zip(rx_rid_list, rx_port_list):
                        print("    RID", hex(x), "Ports", sorted(y))
                    print(sys.stdout.flush())
                    send_packet(self, ing_port, pkt)

                    verify_packet_list(self, rx_port_list, rx_pkt_list)

                # Clean up so we can add the next set of entries.
                t.get_yid_tbl().set_pruned_ports(yid, [])

                # Delete Ing Port table entries
                self.ing_port_table.entry_del(
                    target,
                    [self.ing_port_table.make_key([client.KeyTuple('ig_intr_md.ingress_port', ing_port),
                                                   client.KeyTuple('hdr.vlan_tag.$valid', vlan_val),
                                                   client.KeyTuple('hdr.vlan_tag.vid', vlan_id)])])

                # Delete Ing Src IFID table entries
                self.ing_src_ifid_table.entry_del(
                    target,
                    [self.ing_src_ifid_table.make_key([client.KeyTuple('ig_md.ifid', ifid)])])

                # Delete DMAC entries to route
                self.ing_dmac_table.entry_del(
                    target,
                    [self.ing_dmac_table.make_key([client.KeyTuple('ig_md.brid', brid),
                                                   client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

                # Delete IP entries
                self.ing_ipv4_mcast_table.entry_del(
                    target,
                    [self.ing_ipv4_mcast_table.make_key([client.KeyTuple('ig_md.vrf', vrf),
                                                         client.KeyTuple('hdr.ipv4.src_addr', src_ip, src_ip_mask),
                                                         client.KeyTuple('hdr.ipv4.dst_addr', dst_ip, dst_ip_mask),
                                                         client.KeyTuple('$MATCH_PRIORITY', 1)])]
                )

                if mct is not None:
                    mct.cleanUp()
                    mct = None

        finally:
            print("Cleaning up")
            sys.stdout.flush()
            rmv_ports(self)

            if mct is not None:
                mct.cleanUp()
                mct = None

            t.disable_port_ff()
            t.cleanUp()


class TestAttributes(BfRuntimeTest):
    # BRI specific PRE table attributes set and reset test
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
            - For PRE attributes, only SET is supported in MC mgr but not
              GET. So, there is no way to do SET and GET to verify.
              For some of the attributes (e.g. global rid), traffic tests
              cover the full functionality. As part of this test, we just do
              set for all supported attributes and reset to drivers default
              values to check if all return SUCCESS
        '''

        target = client.Target(device_id=0, pipe_id=0xffff)

        global_rid = 0x4322
        port_protection = True
        fast_failover = True
        max_nodes_before_yield_count = 2
        max_node_threshold_node_count = 100
        max_node_threshold_node_port_lag_count = 25

        try:
            # Set non default values
            print("Configuring global rid to " + str(global_rid))
            self.mgid_table.attribute_pre_device_config_set(target, global_rid=global_rid)
            print("Enabling port protection")
            self.mgid_table.attribute_pre_device_config_set(target, port_protection_enable=port_protection)
            print("Enabling fast failover")
            self.mgid_table.attribute_pre_device_config_set(target, fast_failover_enable=fast_failover)
            print("Configuring max nodes before yield to " + str(max_nodes_before_yield_count))
            self.mgid_table.attribute_pre_device_config_set(target,
                                                            max_nodes_before_yield=max_nodes_before_yield_count)
            print("Configuring max node threshold node count to " + str(max_node_threshold_node_count) +
                  " and port lag count to " + str(max_node_threshold_node_port_lag_count))
            self.mgid_table.attribute_pre_device_config_set(target,
                                                            max_node_threshold_node_count=max_node_threshold_node_count,
                                                            max_node_threshold_port_lag_count=max_node_threshold_node_port_lag_count)

        finally:
            # Reset to drivers default values
            global_rid = 0
            port_protection = False
            fast_failover = False
            max_nodes_before_yield_count = 16
            max_node_threshold_node_count = 4096
            max_node_threshold_node_port_lag_count = 133

            self.mgid_table.attribute_pre_device_config_set(target, global_rid=global_rid)
            self.mgid_table.attribute_pre_device_config_set(target, port_protection_enable=port_protection)
            self.mgid_table.attribute_pre_device_config_set(target, fast_failover_enable=fast_failover)
            self.mgid_table.attribute_pre_device_config_set(target,
                                                            max_nodes_before_yield=max_nodes_before_yield_count)
            self.mgid_table.attribute_pre_device_config_set(target,
                                                            max_node_threshold_node_count=max_node_threshold_node_count,
                                                            max_node_threshold_port_lag_count=max_node_threshold_node_port_lag_count)


class TestNegativeCases(BfRuntimeTest):
    # BRI specific negative test cases
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
            - Tests many different negative cases specific to BRI and
              verifies that the test case fails with the expected error
              code
        '''

        target = client.Target(device_id=0, pipe_id=0xffff)

        # Rid value exceeds 16bits
        try:
            global_rid = 0x10000
            self.mgid_table.attribute_pre_device_config_set(target, global_rid=global_rid)
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # MGID value exceeds 16bits
        try:
            self.mgid_table.entry_add(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', 0x10000)])],
                [self.mgid_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[1]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # MGID value exceeds 16bits
        try:
            self.mgid_table.entry_add(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', 0x10000)])],
                [self.mgid_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[1]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0])])])
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # Node L1 XID value exceeds 16bits
        try:
            self.mgid_table.entry_add(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', 0x22)])],
                [self.mgid_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[1]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0x10000])])])
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # Number of entries in data array are not same
        try:
            self.mgid_table.entry_add(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', 0x22)])],
                [self.mgid_table.make_data([client.DataTuple('$MULTICAST_NODE_ID', int_arr_val=[1]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID_VALID', bool_arr_val=[0]),
                                            client.DataTuple('$MULTICAST_NODE_L1_XID', int_arr_val=[0, 1])])])
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # ECMP L1 XID value exceeds 16bits
        try:
            self.mgid_table.entry_add(
                target,
                [self.mgid_table.make_key([client.KeyTuple('$MGID', 0x22)])],
                [self.mgid_table.make_data([client.DataTuple('$MULTICAST_ECMP_ID', int_arr_val=[1]),
                                            client.DataTuple('$MULTICAST_ECMP_L1_XID_VALID', bool_arr_val=[0]),
                                            client.DataTuple('$MULTICAST_ECMP_L1_XID', int_arr_val=[0x10000])])])
            assert (0)

        except client.BfruntimeReadWriteRpcException as e:
            if e.grpc_error.code() != grpc.StatusCode.UNKNOWN:
                raise e

        # RID value exceeds 16bits
        try:
            self.node_table.entry_add(
                target,
                [self.node_table.make_key([client.KeyTuple('$MULTICAST_NODE_ID', 1)])],
                [self.node_table.make_data([client.DataTuple('$MULTICAST_RID', 0x10000),
                                            client.DataTuple('$MULTICAST_LAG_ID', int_arr_val=[]),
                                            client.DataTuple('$DEV_PORT', int_arr_val=[])])])
            assert (0)

        except ValueError as e:
            pass


class TestPort(BfRuntimeTest):
    # Tests multicast port table entry add and modify
    def setUp(self):
        client_id = 0
        p4_name = "tna_multicast"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        setup_tables(self)

    def runTest(self):
        '''
        This test does the following:
          - For each port -
              - Sets a multicast backup port and clears the backup port config
              - Enables and disables multicast forwarding state
              - Clears fast failover state
              - Enables and disables CopyToCPU port
              - Sets all the above config in one entry_add call and clears
                all the config
        This test just verifies that the config API is execueted sucessfully
        or not but doesn't verify the actual functionality.
        '''
        setup_random()

        target = client.Target(device_id=0, pipe_id=0xffff)

        try:
            add_ports(self)

            for port_index in range(len(swports)):
                port = swports[port_index]
                next_port_index = (port_index + 1) % len(swports)
                next_port = swports[next_port_index]

                # Set next port as the multicast backup port for each port
                backup_port = next_port
                print("Setting port", backup_port, "as the backup port for", port)
                self.port_table.entry_add(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_BACKUP_PORT', backup_port)])]
                )

                # Clear the backup port config for each port
                print("Clearing backup port config for port", port)
                self.port_table.entry_mod(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_BACKUP_PORT', port)])]
                )

                # Set each port as CopyToCPU port
                print("Setting port", port, "as CopyToCPU port")
                self.port_table.entry_add(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$COPY_TO_CPU_PORT_ENABLE', bool_val=True)])]
                )

                # Clear the CopyToCPU port config
                print("Clearing port", port, "as CopyToCPU port")
                self.port_table.entry_mod(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$COPY_TO_CPU_PORT_ENABLE', bool_val=False)])]
                )

                # Set the muticast forward state
                print("Setting multicast forward state for port", port)
                self.port_table.entry_add(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_FORWARD_STATE', bool_val=True)])]
                )

                # Disable muticast forward state
                print("Clearing multicast forward state for port", port)
                self.port_table.entry_mod(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_FORWARD_STATE', bool_val=False)])]
                )

                # Clear the fast failover state
                print("Clearing multicast fast failover for port", port)
                self.port_table.entry_add(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_CLEAR_FAST_FAILOVER', bool_val=True)])]
                )

                # Set all multicast port config in one entry_add API
                print("Setting ALL multicast port config for port", port)
                self.port_table.entry_add(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_BACKUP_PORT', backup_port),
                        client.DataTuple('$COPY_TO_CPU_PORT_ENABLE', bool_val=True),
                        client.DataTuple('$MULTICAST_FORWARD_STATE', bool_val=True),
                        client.DataTuple('$MULTICAST_CLEAR_FAST_FAILOVER', bool_val=True)])]
                )

                # Clear all the multicast port config
                print("Clearing ALL multicast port config for port", port)
                self.port_table.entry_mod(
                    target,
                    [self.port_table.make_key([
                        client.KeyTuple('$DEV_PORT', port)])],
                    [self.port_table.make_data([
                        client.DataTuple('$MULTICAST_BACKUP_PORT', port),
                        client.DataTuple('$COPY_TO_CPU_PORT_ENABLE', bool_val=False),
                        client.DataTuple('$MULTICAST_FORWARD_STATE', bool_val=False),
                        client.DataTuple('$MULTICAST_CLEAR_FAST_FAILOVER', bool_val=True)])]
                )

        finally:
            pass
