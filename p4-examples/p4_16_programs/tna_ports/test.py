# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

import ptf
from ptf import config
import ptf.testutils as testutils
from ptf.testutils import *
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as client
from ptf_port import *

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(20))


def ValueCheck(field, resp, expect_value):
    data_dict = next(resp)[0].to_dict()
    value = data_dict[field]
    if (value != expect_value):
        logger.info("Error: data %d, expect %d", value, expect_value)
        assert (0)


def PortStatusCbTest(self, port):
    logger.info("Test Port Up/Down Status Change Callback function")
    seed = random.randint(1, 65535)
    logger.info("Seed used for Port Status Change Callback Test is %d", seed)
    random.seed(seed)
    target = client.Target(device_id=0, pipe_id=0xffff)
    logger.info("Add a port")
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_25G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                    client.DataTuple('$PORT_ENABLE', bool_val=True)])])
    logger.info("Wait for Port Up")
    time.sleep(5)
    logger.info("Enable port status change notification")
    self.port_table.attribute_port_status_change_set(target, enable=True)
    logger.info("Take port down")
    self.port_table.entry_mod(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port)])],
        [self.port_table.make_data([client.DataTuple('$PORT_ENABLE', bool_val=False)])])
    logger.info("Wait for Port Down")
    time.sleep(5)
    logger.info("Get port status change notification")
    port_status_chg = self.interface.portstatus_notification_get()
    assert (port_status_chg.port_up == False)
    logger.info("Disable port status change notification")
    self.port_table.attribute_port_status_change_set(target, enable=False)
    logger.info("Delete the port")
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port)])])


def PortHdlInfoTest(self, port, conn_id, port_name):
    seed = random.randint(1, 65535)
    logger.info("Seed used for Port hdl info get Test is %d", seed)
    random.seed(seed)
    target = client.Target(device_id=0, pipe_id=0xffff)
    logger.info("add port")
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_10G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                    client.DataTuple('$PORT_ENABLE', bool_val=True)])])
    logger.info("dev_port get")
    resp = self.port_hdl_info_table.entry_get(
        target,
        [self.port_hdl_info_table.make_key([client.KeyTuple('$CONN_ID', conn_id),
                                            client.KeyTuple('$CHNL_ID', 0)])],
        {"from_hw": False})
    ValueCheck('$DEV_PORT', resp, port)
    resp = self.port_fp_idx_info_table.entry_get(
        target,
        [self.port_fp_idx_info_table.make_key([client.KeyTuple('$FP_IDX', port)])],
        {"from_hw": False})
    ValueCheck('$DEV_PORT', resp, 144)
    resp = self.port_str_info_table.entry_get(
        target,
        [self.port_str_info_table.make_key([client.KeyTuple('$PORT_NAME', port_name)])],
        {"from_hw": False})
    ValueCheck('$DEV_PORT', resp, port)
    logger.info("delete port")
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port)])])


def send_and_verify_packet(self, ingress_port, egress_port, pkt, exp_pkt):
    logger.info("Sending packet on port %d", ingress_port)
    testutils.send_packet(self, ingress_port, pkt)
    logger.info("Expecting packet on port %d", egress_port)
    testutils.verify_packets(self, exp_pkt, [egress_port])


def StatCheck(data_dict, field, expect_value):
    value = data_dict[field]
    if (value != expect_value):
        logger.info("Error: data %d, expect %d", value, expect_value)
        assert (0)


def PortStatTest(self, port1, port2):
    logger.info("Test Port Stats read and clear, mixed with p4 table entry operations")
    seed = random.randint(1, 65535)
    logger.info("Seed used for Port stat table Operation Test is %d", seed)
    random.seed(seed)
    target = client.Target(device_id=0, pipe_id=0xffff)
    ig_port = port1
    eg_port = port2
    dmac = '22:22:22:22:22:22'
    dkey = '22:22:22:22:22:23'
    dmask = 'ff:ff:ff:ff:ff:f0'
    pkt = testutils.simple_tcp_packet(eth_dst=dmac)
    exp_pkt = pkt

    logger.info("set dyn key mask")
    dyn_key = self.forward_table.make_key([client.KeyTuple("hdr.ethernet.dst_addr", dmask)])
    self.forward_table.attribute_dyn_key_mask_set(target, dyn_key)
    logger.info("Add forward table entry")
    self.forward_table.entry_add(
        target,
        [self.forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])],
        [self.forward_table.make_data([client.DataTuple('port', eg_port)],
                                      'SwitchIngress.hit')]
    )
    logger.info("Add two ports")
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_25G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                    client.DataTuple('$PORT_ENABLE', bool_val=True)])])
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', eg_port)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_25G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                    client.DataTuple('$PORT_ENABLE', bool_val=True)])])
    logger.info("Wait for port up")
    time.sleep(5)
    send_and_verify_packet(self, ig_port, eg_port, pkt, exp_pkt)
    logger.info("Set poll interval")
    self.port_stat_table.attribute_port_stat_poll_intvl_set(target, intvl=int(600))
    logger.info("port stats read from hw")
    get_data_list = None
    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])],
        {"from_hw": True},
        get_data_list)
    logger.info("Check pkt stats")
    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 1)
    StatCheck(data_dict, '$FramesReceivedAll', 1)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 0)
    StatCheck(data_dict, '$FramesTransmittedAll', 0)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)

    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', eg_port)])],
        {"from_hw": True},
        get_data_list)
    logger.info("Check pkt stats")
    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 0)
    StatCheck(data_dict, '$FramesReceivedAll', 0)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 1)
    StatCheck(data_dict, '$FramesTransmittedAll', 1)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)

    logger.info("Sleep for a second waiting for stats got sync to sw")
    time.sleep(1)
    logger.info("port stats read from sw")
    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])],
        {"from_hw": False},
        get_data_list)
    logger.info("Check pkt stats")
    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 1)
    StatCheck(data_dict, '$FramesReceivedAll', 1)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 0)
    StatCheck(data_dict, '$FramesTransmittedAll', 0)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)
    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', eg_port)])],
        {"from_hw": False},
        get_data_list)
    logger.info("Check pkt stats")
    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 0)
    StatCheck(data_dict, '$FramesReceivedAll', 0)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 1)
    StatCheck(data_dict, '$FramesTransmittedAll', 1)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)
    logger.info("clear stats")
    self.port_stat_table.entry_mod(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])],
        [self.port_stat_table.make_data([client.DataTuple('$FramesReceivedOK', 0),
                                         client.DataTuple('$FramesReceivedAll', 0),
                                         client.DataTuple('$FramesReceivedwithFCSError', 0),
                                         client.DataTuple('$FramesTransmittedOK', 0),
                                         client.DataTuple('$FramesTransmittedAll', 0),
                                         client.DataTuple('$FramesTransmittedwithError', 0)])])

    logger.info("port stats read from hw again")
    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])],
        {"from_hw": False},
        get_data_list)
    logger.info("Check pkt stats")
    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 0)
    StatCheck(data_dict, '$FramesReceivedAll', 0)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 0)
    StatCheck(data_dict, '$FramesTransmittedAll', 0)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)

    # Instead of clearing the stats of eg_port, just clear the entire table
    # which should in turn clear the stats for eg_port as well
    self.port_stat_table.entry_del(target, key_list=None)

    resp = self.port_stat_table.entry_get(
        target,
        [self.port_stat_table.make_key([client.KeyTuple('$DEV_PORT', eg_port)])],
        {"from_hw": False},
        get_data_list)
    logger.info("Check pkt stats")

    data_dict = next(resp)[0].to_dict()
    StatCheck(data_dict, '$FramesReceivedOK', 0)
    StatCheck(data_dict, '$FramesReceivedAll', 0)
    StatCheck(data_dict, '$FramesReceivedwithFCSError', 0)
    StatCheck(data_dict, '$FramesTransmittedOK', 0)
    StatCheck(data_dict, '$FramesTransmittedAll', 0)
    StatCheck(data_dict, '$FramesTransmittedwithError', 0)

    logger.info("Delete the entry from the forward table")
    self.forward_table.entry_del(
        target,
        [self.forward_table.make_key([client.KeyTuple('hdr.ethernet.dst_addr', dmac)])])

    # Reset the enry scope of the table to ALL_PIPES
    logger.info("Reset attributes")
    target = client.Target(device_id=0, pipe_id=0xffff)
    dmask = 'ff:ff:ff:ff:ff:ff'
    dyn_key = self.forward_table.make_key([client.KeyTuple("hdr.ethernet.dst_addr", dmask)])
    self.forward_table.attribute_dyn_key_mask_set(target, dyn_key)

    logger.info("delete ports")
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', ig_port)])])
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', eg_port)])])


def PortCfgTest(self, port1, port2):
    logger.info("Test Port cfg table add read and delete operations")
    target = client.Target(device_id=0, pipe_id=0xffff)

    logger.info("PortCfgTest: Adding entry for port %d", port1)
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_100G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE")])])

    logger.info("PortCfgTest: Adding entry for port %d", port2)
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port2)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_100G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                    client.DataTuple('$N_LANES', 4)])])

    logger.info("PortCfgTest: Modifying entry for port %d", port1)
    self.port_table.entry_mod(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])],
        [self.port_table.make_data([client.DataTuple('$PORT_ENABLE', bool_val=True),
         client.DataTuple('$AUTO_NEGOTIATION', str_val="PM_AN_FORCE_ENABLE"),
         client.DataTuple('$TX_MTU', 1500),
         client.DataTuple('$RX_MTU', 1500),
         client.DataTuple('$TX_PFC_EN_MAP', 1),
         client.DataTuple('$RX_PFC_EN_MAP', 1),
         client.DataTuple('$TX_PAUSE_FRAME_EN', bool_val=False),
         client.DataTuple('$RX_PAUSE_FRAME_EN', bool_val=False),
         client.DataTuple('$CUT_THROUGH_EN', bool_val=False),
         client.DataTuple('$PORT_DIR', str_val="PM_PORT_DIR_DEFAULT")])])

    logger.info("PortCfgTest: Modifying entry for port %d", port2)
    self.port_table.entry_mod(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port2)])],
        [self.port_table.make_data([client.DataTuple('$PORT_ENABLE', bool_val=True),
                                    client.DataTuple('$LOOPBACK_MODE', str_val="BF_LPBK_MAC_NEAR")])])

    logger.info("PortCfgTest: Reading entry for port %d", port1)
    resp = self.port_table.entry_get(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])])

    logger.info("PortCfgTest: Validating entry read for port %d", port1)
    for data, key in resp:
        data = data.to_dict()
        key = key.to_dict()
        assert(key['$DEV_PORT']['value'] == port1)
        assert(data['$SPEED'] == 'BF_SPEED_100G')
        assert(data['$FEC'] == 'BF_FEC_TYP_NONE')
        assert(data['$PORT_ENABLE'] == True)
        assert(data['$AUTO_NEGOTIATION'] == 'PM_AN_FORCE_ENABLE')
        assert(data['$TX_MTU'] == 1500)
        assert(data['$RX_MTU'] == 1500)
        assert(data['$TX_PFC_EN_MAP'] == 1)
        assert(data['$RX_PFC_EN_MAP'] == 1)
        assert(data['$TX_PAUSE_FRAME_EN'] == False)
        assert(data['$RX_PAUSE_FRAME_EN'] == False)
        assert(data['$CUT_THROUGH_EN'] == False)
        assert(data['$PORT_DIR'] == 'PM_PORT_DIR_DEFAULT')

    logger.info("PortCfgTest: Reading entry for port %d", port2)
    resp = self.port_table.entry_get(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port2)])])

    logger.info("PortCfgTest: Validating entry read for port %d", port2)
    for data, key in resp:
        data = data.to_dict()
        key = key.to_dict()
        assert(key['$DEV_PORT']['value'] == port2)
        assert(data['$SPEED'] == 'BF_SPEED_100G')
        assert(data['$FEC'] == 'BF_FEC_TYP_NONE')
        assert(data['$N_LANES'] == 4)
        assert(data['$PORT_ENABLE'] == True)
        assert(data['$LOOPBACK_MODE'] == 'BF_LPBK_MAC_NEAR')

    logger.info("PortCfgTest: Wild card read")
    resp = self.port_table.entry_get(target, None)

    logger.info("PortCfgTest: Validating wild card read")
    for data, key in resp:
        data = data.to_dict()
        key = key.to_dict()
        port = key['$DEV_PORT']['value']
        if port == port1:
            assert(key['$DEV_PORT']['value'] == port1)
            assert(data['$SPEED'] == 'BF_SPEED_100G')
            assert(data['$FEC'] == 'BF_FEC_TYP_NONE')
            assert(data['$PORT_ENABLE'] == True)
            assert(data['$AUTO_NEGOTIATION'] == 'PM_AN_FORCE_ENABLE')
            assert(data['$TX_MTU'] == 1500)
            assert(data['$RX_MTU'] == 1500)
            assert(data['$TX_PFC_EN_MAP'] == 1)
            assert(data['$RX_PFC_EN_MAP'] == 1)
            assert(data['$TX_PAUSE_FRAME_EN'] == False)
            assert(data['$RX_PAUSE_FRAME_EN'] == False)
            assert(data['$CUT_THROUGH_EN'] == False)
            assert(data['$PORT_DIR'] == 'PM_PORT_DIR_DEFAULT')
        elif port == port2:
            assert(key['$DEV_PORT']['value'] == port2)
            assert(data['$SPEED'] == 'BF_SPEED_100G')
            assert(data['$FEC'] == 'BF_FEC_TYP_NONE')
            assert(data['$N_LANES'] == 4)
            assert(data['$PORT_ENABLE'] == True)
            assert(data['$LOOPBACK_MODE'] == 'BF_LPBK_MAC_NEAR')

    logger.info("PortCfgTest: Delete entries for ports %d and %d", port1, port2)
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])])
    self.port_table.entry_del(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port2)])])


def CfgPortTableClearTest(self, port1):
    target = client.Target(device_id=0, pipe_id=0xffff)
    self.port_table.entry_del(
        target,
        key_list=None)
    self.port_table.entry_add(
        target,
        [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])],
        [self.port_table.make_data([client.DataTuple('$SPEED', str_val="BF_SPEED_100G"),
                                    client.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE")])])
    self.port_table.entry_del(
        target,
        key_list=None)
    try:
        get_data_list = self.port_table.make_data([client.DataTuple("$SPEED")])
        resp = self.port_table.entry_get(
            target,
            [self.port_table.make_key([client.KeyTuple('$DEV_PORT', port1)])],
            {"from_hw": False},
            get_data_list)
        data_dict = next(resp)[0].to_dict()
        # since we have deleted all the ports, the above API call should have
        # failed. Assert if not
        logger.error("Unable to clear port cfg table")
        assert(0)
    except:
        logger.info("Cleared port cfg table successfully")


class PortsTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_ports"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        if testutils.test_param_get('target') != "hw":
            return
        seed = random.randint(1, 65535)
        logger.info("Seed used for Ports Test is %d", seed)
        random.seed(seed)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get()
        # Initializing all tables
        self.port_table = bfrt_info.table_get("$PORT")
        self.port_hdl_info_table = bfrt_info.table_get("$PORT_HDL_INFO")
        self.port_fp_idx_info_table = bfrt_info.table_get("$PORT_FP_IDX_INFO")
        self.port_str_info_table = bfrt_info.table_get("$PORT_STR_INFO")

        # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()
        CfgPortTableClearTest(self, 32)
        PortCfgTest(self, 32, 36)
        # model 4/0, hw 19/0
        PortHdlInfoTest(self, 8, 18, "18/0")


class PortsWithProgramTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_ports"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        if testutils.test_param_get('target') != "hw":
            return
        seed = random.randint(1, 65535)
        logger.info("Seed used for Ports with p4program Test is %d", seed)
        random.seed(seed)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get("tna_ports")
        # Initializing all tables
        self.forward_table = bfrt_info.table_get("SwitchIngress.forward")
        self.forward_table.info.key_field_annotation_add("hdr.ethernet.dst_addr", "mac")
        self.port_table = bfrt_info.table_get("$PORT")
        self.port_stat_table = bfrt_info.table_get("$PORT_STAT")
        self.port_hdl_info_table = bfrt_info.table_get("$PORT_HDL_INFO")
        self.port_fp_idx_info_table = bfrt_info.table_get("$PORT_FP_IDX_INFO")
        self.port_str_info_table = bfrt_info.table_get("$PORT_STR_INFO")
        # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()
        PortCfgTest(self, 16, 20)
        PortStatTest(self, 12, 8)
        # model 3/0, hw 18/0
        PortHdlInfoTest(self, 8, 18, "18/0")
        PortStatusCbTest(self, 12)
