# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
from ptf.thriftutils import *
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())


def make_port(pipe, local_port):
    """ Given a pipe and a port within that pipe construct the full port number. """
    return (pipe << 7) | local_port

def port_to_local_port(port):
    """ Given a port return its ID within a pipe. """
    local_port = port & 0x7F
    assert (local_port < 72)
    return local_port

def port_to_pipe(port):
    """ Given a port return the pipe it belongs to. """
    local_port = port_to_local_port(port)
    pipe = (port >> 7) & 0x3
    assert (port == make_port(pipe, local_port))
    return pipe


num_pipes = int(testutils.test_param_get('num_pipes'))
pipes = list(range(num_pipes))

swports = []
swports_by_pipe = {p:list() for p in pipes}
for device, port, ifname in config["interfaces"]:
    swports.append(port)
swports.sort()
for port in swports:
    pipe = port_to_pipe(port)
    swports_by_pipe[pipe].append(port)


# Tofino-1 uses pipes 0 and 2 as the external pipes while 1 and 3 are
# the internal pipes.
# Tofino-2 uses pipes 0 and 1 as the external pipes while 2 and 3 are
# the internal pipes.
arch = testutils.test_param_get('arch')
if arch == "tofino":
    external_pipes = [0,2]
    internal_pipes = [1,3]
elif arch == "tofino2":
    external_pipes = [0,1]
    internal_pipes = [2,3]
else:
    assert (arch == "tofino" or arch == "tofino2")


def get_internal_port_from_external(ext_port):
    pipe_local_port = port_to_local_port(ext_port)
    int_pipe = internal_pipes[external_pipes.index(port_to_pipe(ext_port))]

    if arch == "tofino":
        # For Tofino-1 we are currently using a 1-to-1 mapping from external
        # port to internal port so just replace the pipe-id.
        return make_port(int_pipe, pipe_local_port)
    elif arch == "tofino2":
        # For Tofino-2 we are currently using internal ports in 400g mode so up
        # to eight external ports (if maximum break out is configured) can map
        # to the same internal port.
        return make_port(int_pipe, pipe_local_port & 0x1F8)
    else:
        assert (arch == "tofino" or arch == "tofino2")


def get_port_from_pipes(pipes):
    ports = list()
    for pipe in pipes:
        ports = ports + swports_by_pipe[pipe]
    return random.choice(ports)


def verify_cntr_inc(test, target, dip, ttl, tag, num_pkts):
    logger.info("Verifying counter got incremented on external pipe egress")
    resp = test.a_forward_e.entry_get(target,
                                      [test.a_forward_e.make_key(
                                          [gc.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.255.255'),
                                           gc.KeyTuple('hdr.ipv4.ttl', ttl, 255),
                                           gc.KeyTuple('hdr.custom_metadata.custom_tag', tag, 0xFFFF),
                                           gc.KeyTuple('$MATCH_PRIORITY', 0)])],
                                      {"from_hw": True},
                                      test.a_forward_e.make_data(
                                          [gc.DataTuple("$COUNTER_SPEC_BYTES"),
                                           gc.DataTuple("$COUNTER_SPEC_PKTS")],
                                          'SwitchEgress_a.hit',
                                          get=True))

    # parse resp to get the counter
    data_dict = next(resp)[0].to_dict()
    recv_pkts = data_dict["$COUNTER_SPEC_PKTS"]
    recv_bytes = data_dict["$COUNTER_SPEC_BYTES"]

    if (num_pkts != recv_pkts):
        logger.error("Error! packets sent = %s received count = %s", str(num_pkts), str(recv_pkts))
        assert 0

    # Default packet size is 100 bytes and model adds 4 bytes of CRC
    # Add 2 bytes for the custom metadata header
    pkt_size = 100 + 4 + 2
    num_bytes = num_pkts * pkt_size

    if (num_bytes != recv_bytes):
        logger.error("Error! bytes sent = %s received count = %s", str(num_bytes), str(recv_bytes))
        assert 0


def get_all_tables(test):
    # Some of these tables can be retrieved using a lesser qualified name like storm_control
    # since it is not present in any other control block of the P4 program.  Other tables
    # such as forward or the port-metadata table need more specific names to uniquely identify
    # exactly which table is being requested.
    test.a_port_md = test.bfrt_info.table_get("pipeline_profile_a.SwitchIngressParser_a.$PORT_METADATA")
    test.a_pinning = test.bfrt_info.table_get("SwitchIngress_a.pinning")
    test.a_storm_control = test.bfrt_info.table_get("storm_control")
    test.a_stats = test.bfrt_info.table_get("stats")
    test.a_forward_i = test.bfrt_info.table_get("SwitchIngress_a.forward")
    test.a_encap = test.bfrt_info.table_get("encap_custom_metadata_hdr")
    test.a_forward_e = test.bfrt_info.table_get("SwitchEgress_a.forward")

    test.b_port_md = test.bfrt_info.table_get("pipeline_profile_b.SwitchIngressParser_b.$PORT_METADATA")
    test.b_forward_e = test.bfrt_info.table_get("SwitchEgress_b.forward")
    test.b_forward_i = test.bfrt_info.table_get("SwitchIngress_b.forward")
    test.b_pinning = test.bfrt_info.table_get("SwitchIngress_b.pinning")

    # Add annotations to a few fields to specify their type.
    test.a_forward_i.info.key_field_annotation_add('hdr.ethernet.dst_addr', "mac")
    test.a_forward_e.info.key_field_annotation_add('hdr.ipv4.dst_addr', "ipv4")
    test.b_forward_e.info.key_field_annotation_add('hdr.ipv4.dst_addr', "ipv4")
    test.b_forward_i.info.key_field_annotation_add('hdr.ipv4.dst_addr', "ipv4")


def program_entries(test, target, ig_port, int_port, eg_port, port_meta_a, port_meta_b, tag, dmac, dip, ttl):
    a_f1, a_f2 = port_meta_a
    b_f1 = port_meta_b

    # Use a fixed meter index and use the default meter configuration which
    # will give a green packet (i.e. color == 0).
    meter_idx = 1
    color = 0

    logger.info("Programming table entries")

    logger.info(" Programming table entries on ingress ext-pipe")
    logger.info("    Table: Port Metadata")
    test.a_port_md.entry_add(
        target,
        [test.a_port_md.make_key(
            [gc.KeyTuple('ig_intr_md.ingress_port', ig_port)])],
        [test.a_port_md.make_data(
            [gc.DataTuple('f1', a_f1),
             gc.DataTuple('f2', a_f2)])])

    logger.info("    Table: storm_control")
    test.a_storm_control.entry_add(
        target,
        [test.a_storm_control.make_key(
            [gc.KeyTuple('ig_intr_md.ingress_port', ig_port)])],
        [test.a_storm_control.make_data(
            [gc.DataTuple('index', meter_idx)],
            'SwitchIngress_a.set_color')])

    logger.info("    Table: stats")
    test.a_stats.entry_add(
        target,
        [test.a_stats.make_key(
            [gc.KeyTuple('qos_md.color', color),
             gc.KeyTuple('ig_intr_md.ingress_port', ig_port)])],
        [test.a_stats.make_data([], "SwitchIngress_a.count")])

    logger.info("    Table: forward")
    test.a_forward_i.entry_add(
        target,
        [test.a_forward_i.make_key(
            [gc.KeyTuple('hdr.ethernet.dst_addr', dmac),
             gc.KeyTuple('hdr.ipv4.ttl', ttl)])],
        [test.a_forward_i.make_data([], 'SwitchIngress_a.hit')])
    # The action will decrement the TTL so let's also decrement it here so it is
    # ready to use as the key in the next table.
    ttl = ttl - 1

    logger.info("    Table: encap_custom_metadata_hdr")
    test.a_encap.entry_add(
        target,
        [test.a_encap.make_key(
            [gc.KeyTuple('ig_md.port_md.f1', a_f1),
             gc.KeyTuple('ig_md.port_md.f2', a_f2)])],
        [test.a_encap.make_data(
            [gc.DataTuple('tag', tag)],
            'SwitchIngress_a.encap_custom_metadata')])

    logger.info("    Table: pinning")
    test.a_pinning.entry_add(
        target,
        [test.a_pinning.make_key(
            [gc.KeyTuple('ig_intr_md.ingress_port', ig_port)])],
        [test.a_pinning.make_data(
            [gc.DataTuple('port', int_port)],
            'SwitchIngress_a.modify_eg_port')])

    logger.info(" Programming table entries on egress int-pipe")
    logger.info("    Table: forward")
    test.b_forward_e.entry_add(
        target,
        [test.b_forward_e.make_key(
            [gc.KeyTuple('hdr.ipv4.dst_addr', dip, prefix_len=31),
             gc.KeyTuple('hdr.ipv4.ttl', ttl),
             gc.KeyTuple('hdr.custom_metadata.custom_tag', tag)])],
        [test.b_forward_e.make_data([], "SwitchEgress_b.hit")])
    # The action will decrement the TTL and increment the tag so let's do the
    # same here so the variables are ready to use in the next table.
    ttl = ttl - 1
    tag = tag + 1

    logger.info(" Programming table entries on ingress int-pipe")
    logger.info("    Table: Port Metadata")
    test.b_port_md.entry_add(
        target,
        [test.b_port_md.make_key(
            [gc.KeyTuple('ig_intr_md.ingress_port', int_port)])],
        [test.b_port_md.make_data(
            [gc.DataTuple('f1', b_f1)])])

    logger.info("    Table: forward")
    test.b_forward_i.entry_add(
        target,
        [test.b_forward_i.make_key(
            [gc.KeyTuple('hdr.ipv4.dst_addr', dip),
             gc.KeyTuple('hdr.ipv4.ttl', ttl),
             gc.KeyTuple('hdr.custom_metadata.custom_tag', tag),
             gc.KeyTuple('ig_md.port_md.f1', b_f1)])],
        [test.b_forward_i.make_data([], "SwitchIngress_b.hit")])
    # The action will decrement the TTL and increment the tag so let's do the
    # same here so the variables are ready to use in the next table.
    ttl = ttl - 1
    tag = tag + 1

    logger.info("    Table: pinning")
    test.b_pinning.entry_add(
        target,
        [test.b_pinning.make_key([gc.KeyTuple('ig_intr_md.ingress_port', int_port)])],
        [test.b_pinning.make_data([gc.DataTuple('port', eg_port)],
                                  'SwitchIngress_b.modify_eg_port')])

    logger.info(" Programming table entries on egress ext-pipe")
    logger.info("    Table: forward")
    test.a_forward_e.entry_add(
        target,
        [test.a_forward_e.make_key(
            [gc.KeyTuple('hdr.ipv4.dst_addr', dip, '255.255.255.255'),
             gc.KeyTuple('hdr.ipv4.ttl', ttl, 0xFF),
             gc.KeyTuple('hdr.custom_metadata.custom_tag', tag, 0xFFFF),
             gc.KeyTuple('$MATCH_PRIORITY', 0)])],
        [test.a_forward_e.make_data(
            [gc.DataTuple('$COUNTER_SPEC_BYTES', 0),
             gc.DataTuple('$COUNTER_SPEC_PKTS', 0)],
            'SwitchEgress_a.hit')])


def delete_entries(test, target):
    logger.info("Deleting table entries")

    logger.info(" Deleting table entries on external pipe ingress")
    logger.info("    Table: Port Metadata")
    test.a_port_md.entry_del(target, [])
    logger.info("    Table: storm_control")
    test.a_storm_control.entry_del(target, [])
    logger.info("    Table: stats")
    test.a_stats.entry_del(target, [])
    logger.info("    Table: forward")
    test.a_forward_i.entry_del(target, [])
    logger.info("    Table: encap_custom_metadata_hdr")
    test.a_encap.entry_del(target, [])
    logger.info("    Table: pinning")
    test.a_pinning.entry_del(target, [])

    logger.info(" Deleting table entries on internal pipe egress")
    logger.info("    Table: forward")
    test.b_forward_e.entry_del(target, [])

    logger.info(" Deleting table entries on internal pipe ingress")
    logger.info("    Table: Port Metadata")
    test.b_port_md.entry_del(target, [])
    logger.info("    Table: forward")
    test.b_forward_i.entry_del(target, [])
    logger.info("    Table: pinning")
    test.b_pinning.entry_del(target, [])

    logger.info(" Deleting table entries on external pipe egress")
    logger.info("    Table: forward")
    test.a_forward_e.entry_del(target, [])


# Symmetric table test. Program tables in both pipeline profiles symmetrically.
# Send packet on pipe 0 ingress and expect it to go to pipe 1 and then finally
# egress on pipe 0 egress.
# Pipe0 ingrss -> Pipe 1 Egress -> Pipe 1 Ingress -> Pipe 0 Egress
class Sym32Q(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_32q_2pipe"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_name)
        get_all_tables(self)

    def runTest(self):
        logger.info("")

        ig_port = get_port_from_pipes(external_pipes)
        eg_port = get_port_from_pipes(external_pipes)

        int_port = get_internal_port_from_external(ig_port)
        logger.info("Expected forwarding path:")
        logger.info(" 1. Ingress processing in external pipe %d, ingress port %d", port_to_pipe(ig_port), ig_port)
        logger.info(" 2. Egress processing in internal pipe %d, internal port %d", port_to_pipe(int_port), int_port)
        logger.info(" 3. Loopback on internal port %d", int_port)
        logger.info(" 4. Ingress processing in internal pipe %d, internal port %d", port_to_pipe(int_port), int_port)
        logger.info(" 5. Egress processing in external pipe %d, egress port %d", port_to_pipe(eg_port), eg_port)

        port_meta_a = (0x12345678, 0xabcd)
        port_meta_b = 0x81
        dmac = "00:11:22:33:44:55"
        dip = '5.6.7.8'
        ttl = 64
        tag = 100

        # Use the default "All Pipes" in the target.  This will result in table
        # operations (add/mod/del/etc.) to be applied to all pipes the table is
        # present in.  So, an add to a table in profile A will update the table
        # in all pipes profile A is applied to.
        target = gc.Target(device_id=0)

        try:
            # Add entries and send one packet, it should forward and come back.
            program_entries(self, target, ig_port, int_port, eg_port, port_meta_a, port_meta_b, tag, dmac, dip, ttl)

            logger.info("Sending packet on port %d", ig_port)
            pkt = testutils.simple_tcp_packet(eth_dst=dmac,
                                              ip_dst=dip,
                                              ip_ttl=ttl)
            testutils.send_packet(self, ig_port, pkt)

            pkt.ttl = pkt.ttl - 4
            exp_pkt = pkt
            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packets(self, exp_pkt, [eg_port])

            verify_cntr_inc(self, target, dip, ttl-3, tag+2, 1)

            # Delete the entries and send another packet, it should be dropped.
            delete_entries(self, target)
            logger.info("")
            logger.info("Sending another packet on port %d", ig_port)
            pkt = testutils.simple_tcp_packet(eth_dst=dmac,
                                              ip_dst=dip,
                                              ip_ttl=ttl)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)

        finally:
            # Call the entry cleanup function again incase of an error.
            delete_entries(self, target)


class PortMetadataTest(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "tna_32q_2pipe"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.bfrt_info = self.interface.bfrt_info_get(p4_name)
        get_all_tables(self)

    def runTest(self):
        target = gc.Target(device_id=0)

        # Get one external port and one internal port.
        ext_port = get_port_from_pipes(external_pipes)
        int_port = get_internal_port_from_external(ext_port)

        try:
            # Add entry to the external profile's Port-Metadata table.
            self.a_port_md.entry_add(
                target,
                [self.a_port_md.make_key(
                    [gc.KeyTuple('ig_intr_md.ingress_port', ext_port)])],
                [self.a_port_md.make_data(
                    [gc.DataTuple('f1', 0x12345678),
                     gc.DataTuple('f2', 0xfedc)])])

            # Add entry to the internal profile's Port-Metadata table.
            self.b_port_md.entry_add(
                target,
                [self.b_port_md.make_key(
                    [gc.KeyTuple('ig_intr_md.ingress_port', int_port)])],
                [self.b_port_md.make_data(
                    [gc.DataTuple('f1', 0xab)])])

            # Get the entry back from the external pipe.
            resp = self.a_port_md.entry_get(target)
            resp_cnt = 0
            for d,k in resp:
                resp_cnt += 1
                kd = k.to_dict()
                port_dict = kd.pop('ig_intr_md.ingress_port')
                value = port_dict.pop('value')
                self.assertEqual(value, ext_port)
                self.assertEqual(0, len(port_dict))
                self.assertEqual(0, len(kd))

                dd = d.to_dict()
                self.assertEqual(0x12345678, dd.pop('f1'))
                self.assertEqual(0xfedc, dd.pop('f2'))
                self.assertFalse(dd.pop('is_default_entry'))
                self.assertEqual(None, dd.pop('action_name'))
                self.assertEqual(0, len(dd))
            self.assertEqual(1, resp_cnt)

            # Read the entry back from the internal pipe.
            resp = self.b_port_md.entry_get(target)
            resp_cnt = 0
            for d,k in resp:
                resp_cnt += 1
                kd = k.to_dict()
                port_dict = kd.pop('ig_intr_md.ingress_port')
                value = port_dict.pop('value')
                self.assertEqual(value, int_port)
                self.assertEqual(0, len(port_dict))
                self.assertEqual(0, len(kd))

                dd = d.to_dict()
                self.assertEqual(0xab, dd.pop('f1'))
                self.assertFalse(dd.pop('is_default_entry'))
                self.assertEqual(None, dd.pop('action_name'))
                self.assertEqual(0, len(dd))
            self.assertEqual(1, resp_cnt)

        finally:
            self.a_port_md.entry_del(target, [])
            self.b_port_md.entry_del(target, [])
