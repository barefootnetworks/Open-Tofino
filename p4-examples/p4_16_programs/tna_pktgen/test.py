# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import subprocess
import time

import ptf
from ptf import config
from ptf.thriftutils import *
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc
import grpc

g_is_tofino = testutils.test_param_get("arch") == "tofino"
g_is_tofino2 = testutils.test_param_get("arch") == "tofino2"
assert g_is_tofino or g_is_tofino2

g_num_pipes = int(testutils.test_param_get("num_pipes"))
g_timer_app_id = 1
g_port_down_app_id = 2

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())


def port_to_pipe(port):
    local_port = port & 0x7F
    pipe = port >> 7
    return pipe


def make_port(pipe, local_port):
    return (pipe << 7) | local_port


swports = []
for device, port, ifname in config["interfaces"]:
    pipe = port_to_pipe(port)
    if pipe < g_num_pipes:
        swports.append(port)
        swports.sort()

swports_0 = []
swports_1 = []
swports_2 = []
swports_3 = []
# the following method categorizes the ports in ports.json file as belonging to either of the pipes (0, 1, 2, 3)
for port in swports:
    pipe = port_to_pipe(port)
    if pipe == 0:
        swports_0.append(port)
    elif pipe == 1:
        swports_1.append(port)
    elif pipe == 2:
        swports_2.append(port)
    elif pipe == 3:
        swports_3.append(port)


def take_port_down(port_num):
    logger.info("Take port %d down", port_num)
    device = 0
    pm = config['port_map']
    veth = pm[(device, port_num)]
    veth_idx = veth.strip('veth')
    veth_num = int(veth_idx)
    veth_pair = "veth%d" % (veth_num - 1)
    print()
    port_num, veth, veth_pair
    subprocess.call(['port_ifdown', str(veth_pair)])


def bring_port_up(port_num):
    logger.info("Bring port %d up", port_num)
    device = 0
    pm = config['port_map']
    veth = pm[(device, port_num)]
    veth_idx = veth.strip('veth')
    veth_num = int(veth_idx)
    veth_pair = "veth%d" % (veth_num - 1)
    subprocess.call(['port_ifup', str(veth_pair)])


def verify_multiple_packets(test, port, pkts=[], pkt_lens=[], device_number=0, tmo=None, slack=0):
    # tmo: time out; slack: nonezero-allow getting slack number packets less than expected.
    rx_pkt_status = [False] * len(pkts)
    if tmo is None:
        tmo = ptf.ptfutils.default_negative_timeout
    rx_pkts = 0
    while rx_pkts < len(pkts):
        (rcv_device, rcv_port, rcv_pkt, pkt_time) = testutils.dp_poll(
            test,
            device_number=device_number,
            port_number=port,
            timeout=tmo)
        if not rcv_pkt:
            if slack:
                test.assertTrue((slack > (len(pkts) - rx_pkts)),
                                "Timeout:Port[%d]:Got:[%d]:Allowed slack[%d]:Left[%d]\n" % (
                                port, rx_pkts, slack, len(pkts) - rx_pkts))
                return
            else:
                logger.info("No more packets but still expecting", len(pkts) - rx_pkts)
                for i, a_pkt in enumerate(pkts):
                    # print rx_pkt_status[i] #can be used for future debug when test case cannot pass.
                    # print format_packet(a_pkt) #can be used for future debug when test case cannot pass.
                    if not rx_pkt_status[i]:
                        logger.error("%s", str(a_pkt))
                test.assertTrue(False, "Timeout:Port:[%d]:Got[%d]:Left[%d]\n" % (port, rx_pkts, len(pkts) - rx_pkts))
                return
        rx_pkts = rx_pkts + 1
        found = False
        for i, a_pkt in enumerate(pkts):
            if str(a_pkt) == str(rcv_pkt[:pkt_lens[i]]) and not rx_pkt_status[i]:
                rx_pkt_status[i] = True
                found = True
                break
        if not found:
            test.assertTrue(False, "RxPort:[%u]:Pkt#[%u]:Pkt:%s:Unmatched\n" % (
                port, rx_pkts, ":".join("{:02x}".format(ord(c)) for c in rcv_pkt[:pkt_lens[0]])))


def CfgTimerTable(self, target, i_port, pipe_id, batch_id, packet_id, o_port):
    logger.info("configure forwarding table")
    self.i_t_table.entry_add(
        target,
        [self.i_t_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', i_port),
                                  gc.KeyTuple('hdr.timer.pipe_id', pipe_id),
                                  gc.KeyTuple('hdr.timer.app_id', g_timer_app_id),
                                  gc.KeyTuple('hdr.timer.batch_id', batch_id),
                                  gc.KeyTuple('hdr.timer.packet_id', packet_id)])],
        [self.i_t_table.make_data([gc.DataTuple('port', o_port)],
                                  'SwitchIngress.match')]
    )


def CleanupTimerTable(self, target=None):
    if not target:
        target = gc.Target(device_id=0, pipe_id=0xFFFF)
    resp = self.i_t_table.entry_get(target, [], {"from_hw": False})
    for _, key in resp:
        if key:
            self.i_t_table.entry_del(target, [key])


def CfgPortDownTable(self, target, i_port, pipe_id, port_num, packet_id, o_port):
    logger.info("IgPort %d Pipe %d App %d PortNum %d PktId %d Fwds-to %d", i_port, pipe_id, g_port_down_app_id,
                port_num, packet_id, o_port)
    self.i_p_table.entry_add(
        target,
        [self.i_p_table.make_key([gc.KeyTuple('ig_intr_md.ingress_port', i_port),
                                  gc.KeyTuple('hdr.port_down.pipe_id', pipe_id),
                                  gc.KeyTuple('hdr.port_down.app_id', g_port_down_app_id),
                                  gc.KeyTuple('hdr.port_down.port_num', port_num),
                                  gc.KeyTuple('hdr.port_down.packet_id', packet_id)])],
        [self.i_p_table.make_data([gc.DataTuple('port', o_port)],
                                  'SwitchIngress.match')]
    )


def CleanupPortDownTable(self, target=None):
    if not target:
        target = gc.Target(device_id=0, pipe_id=0xFFFF)
    resp = self.i_p_table.entry_get(target, [], {"from_hw": False})
    for _, key in resp:
        if key:
            self.i_p_table.entry_del(target, [key])


def ValueCheck(self, field, data_dict, expect_value):
    value = data_dict[field]
    if (value != expect_value):
        logger.info("Error: data %d, expect %d", value, expect_value)
        # assert(0)


def pgen_timer_hdr_to_dmac(pipe_id, app_id, batch_id, packet_id):
    """
    Given the fields of a 6-byte packet-gen header return an Ethernet MAC address
    which encodes the same values.
    """
    if g_is_tofino:
        pipe_shift = 3
    else:
        pipe_shift = 4
    return '%02x:00:%02x:%02x:%02x:%02x' % ((pipe_id << pipe_shift) | app_id,
                                            batch_id >> 8,
                                            batch_id & 0xFF,
                                            packet_id >> 8,
                                            packet_id & 0xFF)


def pgen_port_down_hdr_to_dmac(pipe_id, app_id, down_port, packet_id):
    """
    Given the fields of a 6-byte packet-gen header return an Ethernet MAC address
    which encodes the same values.
    """
    if g_is_tofino:
        pipe_shift = 3
    else:
        pipe_shift = 4
    return '%02x:00:%02x:%02x:%02x:%02x' % ((pipe_id << pipe_shift) | app_id,
                                            down_port >> 8,
                                            down_port & 0xFF,
                                            packet_id >> 8,
                                            packet_id & 0xFF)


def pgen_port(pipe_id):
    """
    Given a pipe return a port in that pipe which is usable for packet
    generation.  Note that Tofino allows ports 68-71 in each pipe to be used for
    packet generation while Tofino2 allows ports 0-7.  This example will use
    either port 68 or port 6 in a pipe depending on chip type.
    """
    if g_is_tofino:
        pipe_local_port = 68
    if g_is_tofino2:
        pipe_local_port = 6
    return make_port(pipe_id, pipe_local_port)


class TimerPktgenTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_pktgen"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        logger.info("=============== Testing Packet Generator trigger by Timer ===============")
        bfrt_info = self.interface.bfrt_info_get("tna_pktgen")

        pktgen_app_cfg_table = bfrt_info.table_get("$PKTGEN_APPLICATION_CFG")
        pktgen_pkt_buffer_table = bfrt_info.table_get("$PKTGEN_PKT_BUFFER")
        pktgen_port_cfg_table = bfrt_info.table_get("$PKTGEN_PORT_CFG")
        self.i_t_table = bfrt_info.table_get("SwitchIngress.t")

        # timer pktgen app_id = 1 one shot 0
        target = gc.Target(device_id=0, pipe_id=0xffff)
        app_id = g_timer_app_id
        pktlen = 100
        pgen_pipe_id = 0
        src_port = pgen_port(pgen_pipe_id)
        p_count = 2  # packets per batch
        b_count = 4  # batch number
        buff_offset = 144  # generated packets' payload will be taken from the offset in buffer
        out_port = swports[0]
        # build expected generated packets
        p = testutils.simple_eth_packet(pktlen=pktlen)
        pkt_lst = []
        pkt_len = [pktlen] * p_count * b_count
        for batch in range(b_count):
            for pkt_num in range(p_count):
                dmac = pgen_timer_hdr_to_dmac(pgen_pipe_id, g_timer_app_id, batch, pkt_num)
                p_exp = testutils.simple_eth_packet(pktlen=pktlen, eth_dst=dmac)
                pkt_lst.append(p_exp)

        try:
            for batch in range(b_count):
                for pkt_num in range(p_count):
                    CfgTimerTable(self, target, src_port, 0, batch, pkt_num, out_port)

            # Enable packet generation on the port
            logger.info("enable pktgen port")
            pktgen_port_cfg_table.entry_add(
                target,
                [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', src_port)])],
                [pktgen_port_cfg_table.make_data([gc.DataTuple('pktgen_enable', bool_val=True)])])

            # Read back the enable status after configuring
            resp = pktgen_port_cfg_table.entry_get(
                target,
                [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', src_port)])],
                {"from_hw": False},
                pktgen_port_cfg_table.make_data([gc.DataTuple("pktgen_enable")], get=True))
            data_dict = next(resp)[0].to_dict()
            self.assertTrue(data_dict["pktgen_enable"])

            # Configure the packet generation timer application
            logger.info("configure pktgen application")
            if g_is_tofino:
                data = pktgen_app_cfg_table.make_data([gc.DataTuple('timer_nanosec', 100),
                                                       gc.DataTuple('app_enable', bool_val=False),
                                                       gc.DataTuple('pkt_len', (pktlen - 6)),
                                                       gc.DataTuple('pkt_buffer_offset', buff_offset),
                                                       gc.DataTuple('pipe_local_source_port', src_port),
                                                       gc.DataTuple('increment_source_port', bool_val=False),
                                                       gc.DataTuple('batch_count_cfg', b_count - 1),
                                                       gc.DataTuple('packets_per_batch_cfg', p_count - 1),
                                                       gc.DataTuple('ibg', 1),
                                                       gc.DataTuple('ibg_jitter', 0),
                                                       gc.DataTuple('ipg', 1000),
                                                       gc.DataTuple('ipg_jitter', 500),
                                                       gc.DataTuple('batch_counter', 0),
                                                       gc.DataTuple('pkt_counter', 0),
                                                       gc.DataTuple('trigger_counter', 0)],
                                                      '$PKTGEN_TRIGGER_TIMER_ONE_SHOT')
            if g_is_tofino2:
                data = pktgen_app_cfg_table.make_data([gc.DataTuple('timer_nanosec', 100),
                                                       gc.DataTuple('app_enable', bool_val=False),
                                                       gc.DataTuple('pkt_len', (pktlen - 6)),
                                                       gc.DataTuple('pkt_buffer_offset', buff_offset),
                                                       gc.DataTuple('pipe_local_source_port', src_port),
                                                       gc.DataTuple('increment_source_port', bool_val=False),
                                                       gc.DataTuple('batch_count_cfg', b_count - 1),
                                                       gc.DataTuple('packets_per_batch_cfg', p_count - 1),
                                                       gc.DataTuple('ibg', 1),
                                                       gc.DataTuple('ibg_jitter', 0),
                                                       gc.DataTuple('ipg', 1000),
                                                       gc.DataTuple('ipg_jitter', 500),
                                                       gc.DataTuple('batch_counter', 0),
                                                       gc.DataTuple('pkt_counter', 0),
                                                       gc.DataTuple('trigger_counter', 0),
                                                       gc.DataTuple('assigned_chnl_id', pgen_port(0))],
                                                      '$PKTGEN_TRIGGER_TIMER_ONE_SHOT')
            pktgen_app_cfg_table.entry_add(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_timer_app_id)])],
                [data])

            # Read the app configuration back
            resp = pktgen_app_cfg_table.entry_get(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_timer_app_id)])],
                {"from_hw": False},
                pktgen_app_cfg_table.make_data([gc.DataTuple('timer_nanosec'),
                                                gc.DataTuple('app_enable'),
                                                gc.DataTuple('pkt_len'),
                                                gc.DataTuple('pkt_buffer_offset'),
                                                gc.DataTuple('pipe_local_source_port'),
                                                gc.DataTuple('increment_source_port'),
                                                gc.DataTuple('batch_count_cfg'),
                                                gc.DataTuple('packets_per_batch_cfg'),
                                                gc.DataTuple('ibg'),
                                                gc.DataTuple('ibg_jitter'),
                                                gc.DataTuple('ipg'),
                                                gc.DataTuple('ipg_jitter')],
                                               '$PKTGEN_TRIGGER_TIMER_ONE_SHOT', get=True))
            data_dict = next(resp)[0].to_dict()
            print()
            data_dict
            ValueCheck(self, 'timer_nanosec', data_dict, 100)
            assert data_dict["app_enable"] == False  # app_enable field_id=12
            ValueCheck(self, 'pkt_len', data_dict, (pktlen - 6))
            ValueCheck(self, 'pkt_buffer_offset', data_dict, buff_offset)
            ValueCheck(self, 'pipe_local_source_port', data_dict, src_port)
            assert data_dict["increment_source_port"] == False  # increment_source_port field_id=16
            ValueCheck(self, 'batch_count_cfg', data_dict, b_count - 1)
            ValueCheck(self, 'packets_per_batch_cfg', data_dict, p_count - 1)
            ValueCheck(self, 'ibg', data_dict, 1)
            ValueCheck(self, 'ibg_jitter', data_dict, 0)
            ValueCheck(self, 'ipg', data_dict, 1000)
            ValueCheck(self, 'ipg_jitter', data_dict, 500)

            logger.info("configure packet buffer")
            pktgen_pkt_buffer_table.entry_add(
                target,
                [pktgen_pkt_buffer_table.make_key([gc.KeyTuple('pkt_buffer_offset', buff_offset),
                                                   gc.KeyTuple('pkt_buffer_size', (pktlen - 6))])],
                [pktgen_pkt_buffer_table.make_data([gc.DataTuple('buffer', str(p)[6:])])])  # p[6:]))])
            resp = pktgen_pkt_buffer_table.entry_get(
                target,
                [pktgen_pkt_buffer_table.make_key([gc.KeyTuple('pkt_buffer_offset', buff_offset),
                                                   gc.KeyTuple('pkt_buffer_size', (pktlen - 6))])],
                {"from_hw": False})
            data_dict = next(resp)[0].to_dict()
            print()
            data_dict

            logger.info("enable pktgen")
            pktgen_app_cfg_table.entry_mod(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_timer_app_id)])],
                [pktgen_app_cfg_table.make_data([gc.DataTuple('app_enable', bool_val=True)],
                                                '$PKTGEN_TRIGGER_TIMER_ONE_SHOT')]
            )

            # Use the per-app counters to wait for all packets to be generated.
            for _ in range(b_count * p_count):

                # verify pktgen related counters
                resp = pktgen_app_cfg_table.entry_get(
                    target,
                    [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_timer_app_id)])],
                    {"from_hw": True},
                    pktgen_app_cfg_table.make_data([gc.DataTuple('batch_counter'),
                                                    gc.DataTuple('pkt_counter'),
                                                    gc.DataTuple('trigger_counter')],
                                                   '$PKTGEN_TRIGGER_TIMER_ONE_SHOT', get=True)
                )
                data_dict = next(resp)[0].to_dict()
                tri_value = data_dict["trigger_counter"]
                if tri_value != 1:
                    logger.info("Triggered %d of 1 times", tri_value)
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                batch_value = data_dict["batch_counter"]
                if batch_value != b_count:
                    logger.info("Generated %d of %d batches", batch_value, b_count)
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                pkt_value = data_dict["pkt_counter"]
                if pkt_value != b_count * p_count:
                    logger.info("Generated %d of %d packets", pkt_value, b_count * p_count)
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                break

            # Verify generated packets
            verify_multiple_packets(self, out_port, pkt_lst, pkt_len, tmo=5)

            # Disable the application.
            logger.info("disable pktgen")
            pktgen_app_cfg_table.entry_mod(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_timer_app_id)])],
                [pktgen_app_cfg_table.make_data([gc.DataTuple('app_enable', bool_val=False)],
                                                '$PKTGEN_TRIGGER_TIMER_ONE_SHOT')])
            # Disable packet generation on the port
            pktgen_port_cfg_table.entry_mod(
                target,
                [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', src_port)])],
                [pktgen_port_cfg_table.make_data([gc.DataTuple('pktgen_enable', bool_val=False)])])
            CleanupTimerTable(self, target)
        except gc.BfruntimeRpcException as e:
            raise e
        finally:
            pass


class PortDownPktgenTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_pktgen"
        BfRuntimeTest.setUp(self, client_id, p4_name)

    def runTest(self):
        logger.info("=============== Testing Packet Generator trigger by port down ===============")
        bfrt_info = self.interface.bfrt_info_get("tna_pktgen")

        pktgen_app_cfg_table = bfrt_info.table_get("$PKTGEN_APPLICATION_CFG")
        pktgen_pkt_buffer_table = bfrt_info.table_get("$PKTGEN_PKT_BUFFER")
        pktgen_port_cfg_table = bfrt_info.table_get("$PKTGEN_PORT_CFG")
        self.i_p_table = bfrt_info.table_get("SwitchIngress.p")

        target = gc.Target(device_id=0, pipe_id=0xffff)
        pktlen = 100
        p_count = 2  # packets per batch
        # The batch count should be 1 for port down events since it encodes the
        # port number that went down.
        b_count = 1
        buff_offset = 0  # generated packets' payload starts from the offset in buffer

        # Find a pipe that has more than a single port.
        if len(swports_0) > 1:
            ports_to_flap = swports_0[1:]
            outport = swports_0[0]
            pipe = 0
        elif len(swports_1) > 1:
            ports_to_flap = swports_1[1:]
            outport = swports_1[0]
            pipe = 1
        elif len(swports_2) > 1:
            ports_to_flap = swports_2[1:]
            outport = swports_2[0]
            pipe = 2
        elif len(swports_3) > 1:
            ports_to_flap = swports_3[1:]
            outport = swports_3[0]
            pipe = 3
        else:
            self.assertTrue(False, "Need at least two ports in a pipe")

        src_port = pgen_port(pipe)

        # Build the expected packets
        p = testutils.simple_eth_packet(pktlen=pktlen)
        pkt_lst = []
        pkt_len_list = list()
        for pkt_id in range(p_count):
            for port in ports_to_flap:
                dmac = pgen_port_down_hdr_to_dmac(pipe, g_port_down_app_id, port, pkt_id)
                p_exp = testutils.simple_eth_packet(pktlen=pktlen, eth_dst=dmac)
                pkt_lst.append(p_exp)
                pkt_len_list.append(pktlen)

        try:
            # Add entries to the verify table.
            for port in ports_to_flap:
                for pkt_num in range(p_count):
                    CfgPortDownTable(self, target, src_port, pipe, port, pkt_num, outport)

            # Enable packet generation on the port
            logger.info("enable pktgen port")
            pktgen_port_cfg_table.entry_add(
                target,
                [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', src_port)])],
                [pktgen_port_cfg_table.make_data([gc.DataTuple('pktgen_enable', bool_val=True)])])

            logger.info("configure pktgen application")
            port_mask_sel = 0
            if g_is_tofino:
                data = pktgen_app_cfg_table.make_data([gc.DataTuple('app_enable', bool_val=False),
                                                       gc.DataTuple('pkt_len', (pktlen - 6)),
                                                       gc.DataTuple('pkt_buffer_offset', buff_offset),
                                                       gc.DataTuple('pipe_local_source_port', src_port),
                                                       gc.DataTuple('increment_source_port', bool_val=False),
                                                       gc.DataTuple('batch_count_cfg', b_count - 1),
                                                       gc.DataTuple('packets_per_batch_cfg', p_count - 1),
                                                       gc.DataTuple('batch_counter', 0),
                                                       gc.DataTuple('pkt_counter', 0),
                                                       gc.DataTuple('trigger_counter', 0)],
                                                      '$PKTGEN_TRIGGER_PORT_DOWN')
            if g_is_tofino2:
                data = pktgen_app_cfg_table.make_data([gc.DataTuple('port_mask_sel', port_mask_sel),
                                                       gc.DataTuple('app_enable', bool_val=False),
                                                       gc.DataTuple('pkt_len', (pktlen - 6)),
                                                       gc.DataTuple('pkt_buffer_offset', buff_offset),
                                                       gc.DataTuple('pipe_local_source_port', src_port),
                                                       gc.DataTuple('increment_source_port', bool_val=False),
                                                       gc.DataTuple('batch_count_cfg', b_count - 1),
                                                       gc.DataTuple('packets_per_batch_cfg', p_count - 1),
                                                       gc.DataTuple('batch_counter', 0),
                                                       gc.DataTuple('pkt_counter', 0),
                                                       gc.DataTuple('trigger_counter', 0),
                                                       gc.DataTuple('assigned_chnl_id', src_port)],
                                                      '$PKTGEN_TRIGGER_PORT_DOWN')
            pktgen_app_cfg_table.entry_add(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_port_down_app_id)])],
                [data]
            )

            logger.info("configure packet buffer")
            pktgen_pkt_buffer_table.entry_add(
                target,
                [pktgen_pkt_buffer_table.make_key([gc.KeyTuple('pkt_buffer_offset', buff_offset),
                                                   gc.KeyTuple('pkt_buffer_size', (pktlen - 6))])],
                [pktgen_pkt_buffer_table.make_data([gc.DataTuple('buffer', str(p)[6:])])])

            port_mask = 0
            if g_is_tofino2:
                pktgen_port_mask_table = bfrt_info.table_get("$PKTGEN_PORT_MASK")
                for port in ports_to_flap:
                    port_mask |= 1 << (port & 0x3f)
                # set the port mask
                logger.info("Set port down mask")
                pktgen_port_mask_table.entry_add(
                    target,
                    [pktgen_port_mask_table.make_key([gc.KeyTuple('port_mask_sel', port_mask_sel)])],
                    [pktgen_port_mask_table.make_data([gc.DataTuple('mask', port_mask)])])
                resp = pktgen_port_mask_table.entry_get(
                    target,
                    [pktgen_port_mask_table.make_key([gc.KeyTuple('port_mask_sel', port_mask_sel)])],
                    {"from_hw": False})
                data_dict = next(resp)[0].to_dict()
                if data_dict['mask'] != port_mask:
                    logger.error("Port Down Mask not set correctly")

            logger.info("enable pktgen")
            pktgen_app_cfg_table.entry_mod(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_port_down_app_id)])],
                [pktgen_app_cfg_table.make_data([gc.DataTuple('app_enable', bool_val=True)],
                                                '$PKTGEN_TRIGGER_PORT_DOWN')]
            )

            logger.info("Clear port down")
            for port in ports_to_flap:
                pktgen_port_cfg_table.entry_mod(
                    target,
                    [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', port)])],
                    [pktgen_port_cfg_table.make_data([gc.DataTuple('clear_port_down_enable', bool_val=True)])])
                resp = pktgen_port_cfg_table.entry_get(
                    target,
                    [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', port)])],
                    {"from_hw": False})
                data_dict = next(resp)[0].to_dict()
                if data_dict["clear_port_down_enable"] != True:
                    logger.info("port down event clear status is false, port %d", port)

            # Take port down to trigger pktgen
            for port in ports_to_flap:
                take_port_down(port)
            time.sleep(1)
            for port in ports_to_flap:
                bring_port_up(port)

            # Use the per-app counters to wait for all packets to be generated.
            for _ in range(len(ports_to_flap) * p_count):
                # verify pktgen related counters
                resp = pktgen_app_cfg_table.entry_get(
                    target,
                    [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_port_down_app_id)])],
                    {"from_hw": True},
                    pktgen_app_cfg_table.make_data([gc.DataTuple('batch_counter'),
                                                    gc.DataTuple('pkt_counter'),
                                                    gc.DataTuple('trigger_counter')],
                                                   '$PKTGEN_TRIGGER_PORT_DOWN', get=True)
                )
                data_dict = next(resp)[0].to_dict()
                tri_value = data_dict["trigger_counter"]
                if tri_value != len(ports_to_flap):
                    logger.info("Triggered %d of %d times", tri_value, len(ports_to_flap))
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                batch_value = data_dict["batch_counter"]
                if batch_value != len(ports_to_flap) * b_count:
                    logger.info("Generated %d of %d batches", batch_value, len(ports_to_flap) * b_count)
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                pkt_value = data_dict["pkt_counter"]
                if pkt_value != len(ports_to_flap) * b_count * p_count:
                    logger.info("Generated %d of %d packets", pkt_value, len(ports_to_flap) * b_count * p_count)
                    # Wait for packets to be generated
                    time.sleep(2)
                    continue
                break

            # verify received pkts
            verify_multiple_packets(self, outport, pkt_lst, pkt_len_list, tmo=5)

            # disable tables
            logger.info("disable pktgen")
            pktgen_app_cfg_table.entry_mod(
                target,
                [pktgen_app_cfg_table.make_key([gc.KeyTuple('app_id', g_port_down_app_id)])],
                [pktgen_app_cfg_table.make_data([gc.DataTuple('app_enable', bool_val=False)],
                                                '$PKTGEN_TRIGGER_PORT_DOWN')]
            )

            logger.info("disable port for pktgen")
            pktgen_port_cfg_table.entry_mod(
                target,
                [pktgen_port_cfg_table.make_key([gc.KeyTuple('dev_port', src_port)])],
                [pktgen_port_cfg_table.make_data([gc.DataTuple('pktgen_enable', bool_val=False)])])

            CleanupPortDownTable(self, target)
        finally:
            pass
