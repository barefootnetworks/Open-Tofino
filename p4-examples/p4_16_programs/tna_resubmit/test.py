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
p4_program_name = "tna_resubmit"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
swports.sort()


class NoResubmitTest(BfRuntimeTest):
    """@brief Verify if the program forwards packets correctly without resubmit.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_program_name)

    def runTest(self):
        target = gc.Target(device_id=dev_id, pipe_id=0xffff)
        port_meta = self.bfrt_info.table_get("$PORT_METADATA")
        pass_one = self.bfrt_info.table_get("pass_one")
        port = random.choice(swports)
        port_meta_values = [random.getrandbits(32), random.getrandbits(32)]

        try:
            # Write values into the Port Metadata table which will be assigned
            # to the packet.
            k = port_meta.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])
            d = port_meta.make_data([gc.DataTuple('f1', port_meta_values[0]),
                             gc.DataTuple('f2', port_meta_values[1])])
            port_meta.entry_add(target, [k], [d])

            # Install an entry to match those metadata values in the "pass_one"
            # table with an action that will NOT perform a resubmit.
            k = pass_one.make_key([gc.KeyTuple("port", port),
                            gc.KeyTuple("f1", port_meta_values[0]),
                            gc.KeyTuple("f2", port_meta_values[1])])
            d = pass_one.make_data([], "SwitchIngress.no_resub")
            pass_one.entry_add(target, [k], [d])

            # Since the packet will not perform a resubmit operation the header
            # modifications on the first (and only) pass of the packet through
            # the ingress pipe will be kept, so expect the MAC addresses to be
            # modified.
            pkt_in = testutils.simple_ip_packet()
            pkt_out = testutils.simple_ip_packet(eth_dst='00:00:00:00:00:01',
                                                 eth_src='00:00:00:00:00:00')
            testutils.send_packet(self, port, pkt_in)
            testutils.verify_packet(self, pkt_out, port)

        finally:
            port_meta.entry_del(target)
            pass_one.entry_del(target)




class ResubmitTest(BfRuntimeTest):
    """@brief Resubmit a packet with a resubmit digest.
    """

    def setUp(self):
        client_id = 0
        BfRuntimeTest.setUp(self, client_id, p4_program_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_program_name)

    def getCntr(self, index):
        t = self.bfrt_info.table_get('cntr')
        resp = t.entry_get(gc.Target(device_id=dev_id, pipe_id=0xffff),
                           [t.make_key([gc.KeyTuple('$COUNTER_INDEX', index)])],
                           {"from_hw":True},
                           None)
        data_dict = next(resp)[0].to_dict()
        return data_dict['$COUNTER_SPEC_PKTS']

    def clrCntrs(self):
        t = self.bfrt_info.table_get('cntr')
        for i in [0,1,2,3]:
            t.entry_add(gc.Target(device_id=dev_id, pipe_id=0xffff),
                        [t.make_key([gc.KeyTuple('$COUNTER_INDEX', i)])],
                        [t.make_data([gc.DataTuple('$COUNTER_SPEC_PKTS', 0)])])

    def runTest(self):
        target = gc.Target(device_id=dev_id, pipe_id=0xffff)
        port_meta = self.bfrt_info.table_get("$PORT_METADATA")
        pass_one = self.bfrt_info.table_get("pass_one")
        pass_two_a = self.bfrt_info.table_get("pass_two_type_a")
        pass_two_b = self.bfrt_info.table_get("pass_two_type_b")
        pass_two_c = self.bfrt_info.table_get("pass_two_type_c")
        port = random.choice(swports)
        port_meta_values = [random.getrandbits(32), random.getrandbits(32)]
        a_f1 = random.getrandbits(8)
        a_f2 = random.getrandbits(16)
        a_f3 = random.getrandbits(32)
        a_md = random.getrandbits(64)
        b_f1 = random.getrandbits(8)
        b_md = random.getrandbits(64)
        c_f1 = random.getrandbits(16)
        c_f2 = random.getrandbits(16)
        c_f3 = random.getrandbits(16)
        c_md = random.getrandbits(64)
        pkt = testutils.simple_ip_packet()
        tof1 = testutils.test_param_get('arch') == 'tofino'

        try:
            # Add a port metadata entry to assign the metadata used in the table
            # key for the pass_one table.
            k = port_meta.make_key([gc.KeyTuple('ig_intr_md.ingress_port', port)])
            d = port_meta.make_data([gc.DataTuple('f1', port_meta_values[0]),
                             gc.DataTuple('f2', port_meta_values[1])])
            port_meta.entry_add(target, [k], [d])

            # Add an entry to each of the pass two tables which validates the
            # metadata passed through the resubmit path.
            key_fields = [gc.KeyTuple('md.a.f1', a_f1),
                          gc.KeyTuple('md.a.f2', a_f2),
                          gc.KeyTuple('md.a.f3', a_f3)]
            if not tof1:
                key_fields.append(gc.KeyTuple('md.a.additional', a_md))
            d = pass_two_a.make_data([], 'SwitchIngress.okay_a')
            pass_two_a.entry_add(target, [pass_two_a.make_key(key_fields)], [d])

            key_fields = [gc.KeyTuple('md.b.f1', b_f1)]
            if not tof1:
                key_fields.append(gc.KeyTuple('md.b.additional', b_md))
            d = pass_two_b.make_data([], 'SwitchIngress.okay_b')
            pass_two_b.entry_add(target, [pass_two_b.make_key(key_fields)], [d])

            key_fields = [gc.KeyTuple('md.c.f1', c_f1),
                          gc.KeyTuple('md.c.f2', c_f2),
                          gc.KeyTuple('md.c.f3', c_f3)]
            if not tof1:
                key_fields.append(gc.KeyTuple('md.c.additional', c_md))
            d = pass_two_c.make_data([], 'SwitchIngress.okay_c')
            pass_two_c.entry_add(target, [pass_two_c.make_key(key_fields)], [d])

            # Clear the counters to ensure we are starting in a known state
            # since other tests may have sent traffic which incremented them.
            self.clrCntrs()

            # Add an entry to the pass_one table to cause the packet to resubmit
            # with the first resubmit type.
            k = pass_one.make_key([gc.KeyTuple("port", port),
                            gc.KeyTuple("f1", port_meta_values[0]),
                            gc.KeyTuple("f2", port_meta_values[1])])
            d = pass_one.make_data([gc.DataTuple('f1', a_f1),
                                    gc.DataTuple('f2', a_f2),
                                    gc.DataTuple('f3', a_f3),
                                    gc.DataTuple('more_data', a_md)],
                                   "SwitchIngress.resub_a")
            pass_one.entry_add(target, [k], [d])

            # Verify it works as expected. There should be one count at index 0
            # from the first pass and one count at index 1 for a resubmit type A.
            testutils.send_packet(self, port, pkt)
            testutils.verify_packet(self, pkt, port)
            c1 = self.getCntr(0)
            c2 = self.getCntr(1)
            self.assertEqual(c1, 1)
            self.assertEqual(c2, 1)

            # Now modify the entry to use the next resubmit type and verify it
            # also works.
            d = pass_one.make_data([gc.DataTuple('f1', b_f1),
                                    gc.DataTuple('more_data', b_md)],
                                   "SwitchIngress.resub_b")
            pass_one.entry_mod(target, [k], [d])
            testutils.send_packet(self, port, pkt)
            testutils.verify_packet(self, pkt, port)
            c1 = self.getCntr(0)
            c2 = self.getCntr(2)
            self.assertEqual(c1, 2)
            self.assertEqual(c2, 1)

            # Modify it again to use the third resubmit type and verify.
            d = pass_one.make_data([gc.DataTuple('f1', c_f1),
                                    gc.DataTuple('f2', c_f2),
                                    gc.DataTuple('f3', c_f3),
                                    gc.DataTuple('more_data', c_md)],
                                   "SwitchIngress.resub_c")
            pass_one.entry_mod(target, [k], [d])
            testutils.send_packet(self, port, pkt)
            testutils.verify_packet(self, pkt, port)
            c1 = self.getCntr(0)
            c2 = self.getCntr(3)
            self.assertEqual(c1, 3)
            self.assertEqual(c2, 1)

        finally:
            port_meta.entry_del(target)
            pass_one.entry_del(target)
            pass_two_a.entry_del(target)
            pass_two_b.entry_del(target)
            pass_two_c.entry_del(target)
