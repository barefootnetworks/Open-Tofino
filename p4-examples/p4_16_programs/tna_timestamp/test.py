# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import struct

from ptf import config
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

p4_program_name = "tna_timestamp"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


class TimestampTest(BfRuntimeTest):
    """@brief Demonstrate the six timestamps that are available per packet 
    throughout the packet processing pipeline by writing their values into the 
    payload of a UDP packet.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=0, pipe_id=0xffff)
        # Get bfrt_info and set it as part of the test
        bfrt_info = self.interface.bfrt_info_get(p4_program_name)

        # Set default output port
        table_output_port = bfrt_info.table_get("SwitchIngress.output_port")
        action_data = table_output_port.make_data(
            action_name="SwitchIngress.set_output_port",
            data_field_list_in=[gc.DataTuple(name="port_id", val=swports[1])]
        )
        table_output_port.default_entry_set(
            target=target,
            data=action_data)

        try:
            ipkt_payload = struct.pack("I", 0) * 10
            ipkt = testutils.simple_udp_packet(eth_dst='11:11:11:11:11:11',
                                               eth_src='22:22:22:22:22:22',
                                               ip_src='1.2.3.4',
                                               ip_dst='100.99.98.97',
                                               ip_id=101,
                                               ip_ttl=64,
                                               udp_sport=0x1234,
                                               udp_dport=0xabcd,
                                               with_udp_chksum=False,
                                               udp_payload=ipkt_payload)

            testutils.send_packet(self, swports[0], ipkt)

            (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                testutils.dp_poll(self, 0, swports[1], timeout=2)
            nrcv = ipkt.__class__(rcv_pkt)

            # Parse the payload and extract the timestamps
            # import pdb; pdb.set_trace()
            ts_ingress_mac, ts_ingress_global, \
                ts_enqueue, ts_dequeue_delta, \
                ts_egress_global, ts_egress_tx = \
                struct.unpack("!QQIIQQxxxxxxxxxxxxxxxxxx", nrcv.load)

            ns = 1000000000.0
            logger.info("Timestamps")
            logger.info("  raw values in ns:")
            logger.info("    ingress mac                   : {:>15}".format(ts_ingress_mac))
            logger.info("    ingress global                : {:>15}".format(ts_ingress_global))
            logger.info("    traffic manager enqueue       : {:>15}".format(ts_enqueue))
            logger.info("    traffic manager dequeue delta : {:>15}".format(ts_dequeue_delta))
            logger.info("    egress global                 : {:>15}".format(ts_egress_global))
            logger.info("    egress tx (no value in model) : {:>15}".format(ts_egress_tx))
            logger.info("  values in s:")
            logger.info("    ingress mac                   : {:>15.9f}".format(ts_ingress_mac / ns))
            logger.info("    ingress global                : {:>15.9f}".format(ts_ingress_global / ns))
            logger.info("    traffic manager enqueue       : {:>15.9f}".format(ts_enqueue / ns))
            logger.info("    traffic manager dequeue delta : {:>15.9f}".format(ts_dequeue_delta / ns))
            logger.info("    egress global                 : {:>15.9f}".format(ts_egress_global / ns))
            logger.info("    egress tx (no value in model) : {:>15.9f}".format(ts_egress_tx))
            logger.info("Please note that the timestamps are using the internal time " +
                        "of the model/chip. They are not synchronized with the global time. "
                        "Furthermore, the traffic manager timestamps in the model do not " +
                        "accurately reflect the packet processing. Correct values are shown " +
                        "by the hardware implementation.")
        finally:
            table_output_port.default_entry_reset(target)
