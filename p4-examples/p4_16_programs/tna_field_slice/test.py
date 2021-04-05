# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import random

from ptf import config
import ptf.testutils as testutils
from ptf.thriftutils import *
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

logger.setLevel("DEBUG")

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()

if swports == []:
    swports = list(range(9))


# The following 2 functions extract the start and end bit from the name
# of the field slice respectively. A p4 key field slice has the following
# format - <field_name>[<end_bit>:<start_bit>]
# Thus these functions split the name at ':' and then extract the corresp
# onding bits. However, for the field slices which have aliases (which can
# be any random names) we purposely have named them to have the following
# format - <field_name>_<end_bit>_<start_bit>_
# Thus these functions split the name on the basis of '_' if the name is an
# alias.
def getStartBitFromName(addr):
    """ @brief This returns the start bit of a field slice from its name
    """
    if ':' in addr:
        # try splitting with ':'
        word_list = addr.split(":")
        if len(word_list) != 0:
            start = int(word_list[1][:-1], 10)
            return start
    # maybe the field has a alias. try splitting with '_'
    word_list = [int(s) for s in addr.split('_') if s.isdigit()]
    if len(word_list) == 0 or len(word_list) > 2:
        logger.error("Invalid name %s for key field", addr)
        assert (0)
    return word_list[1]


def getEndBitFromName(addr):
    """ @brief This returns the end bit of a field slice from its name
    """
    if ':' in addr:
        # try splitting with ':'
        word_list = addr.split(":")
        if len(word_list) != 0:
            end = int(word_list[0][word_list[0].find('[') + 1:], 10)
            return end
    # maybe the field has a alias. try splitting with '_'
    word_list = [int(s) for s in addr.split('_') if s.isdigit()]
    if len(word_list) == 0 or len(word_list) > 2:
        logger.error("Invalid name %s for key field", addr)
        assert (0)
    return word_list[0]


def getFieldSliceByteWidthFromName(addr):
    """ @brief This returns the byte width of a field slice
    """
    start = getStartBitFromName(addr)
    end = getEndBitFromName(addr)
    return ((end - start + 1 + 7) // 8)


def getFieldSliceBitWidthFromName(addr):
    """ @brief This returns the bit width of a field slice 
    """
    start = getStartBitFromName(addr)
    end = getEndBitFromName(addr)
    return end - start + 1


def getFieldSliceValue(full_field_value, start, end):
    """ @brief This extracts the value from the full_field_value lying between the start
    and end bit 
    """
    mask = ((1 << (end - start + 1)) - 1) << start
    return (full_field_value & mask) >> start


def getIpv6AddrFieldSliceValue(field_slice_name, ip_addr):
    """ @brief Given a ipv6 addr, this extracts the value of the field slice from it 
    """
    start = getStartBitFromName(field_slice_name)
    end = getEndBitFromName(field_slice_name)
    octets = ip_addr.split(":")
    new_val = 0
    for octet in octets:
        new_val = int(octet, 16) | (new_val << 16)

    return getFieldSliceValue(new_val, start, end)


def garbleBits(val, index_set):
    """ @brief Given a value and a set of indices, this function inverts the bits in the value           
    at all the indices in the set
    """
    mask = 0
    for index in index_set:
        mask |= (1 << index)

    return (val ^ mask)


def generateIpv6Addr():
    """ @brief This generates a random ipv6 address
    """
    ip_addr = "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x" % tuple(random.randint(0, 65535) for x in range(8))
    return ip_addr


def getDataObj(table_obj, eg_port):
    """ @brief Given a table object and a port, this returns a data obj
    """
    data_obj = table_obj.make_data(
        [gc.DataTuple("port", eg_port)],
        "SwitchIngress.hit")
    return data_obj


class SingleFieldSingleSliceTernaryTest(BfRuntimeTest):
    """ @brief This test adds a field slice entry in the table, sends packets and verifies,
    reads back the entry and verifies and then finally deletes the entry
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_single_slice_ternary"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.tcam_field_name = "hdr.ipv6.dst_addr[16:7]"
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]

        dmac = '22:22:22:22:22:22'

        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        num_entries = 100

        table_obj = bfrt_info.table_get(self.table_name)

        for entry_idx in range(num_entries):
            logger.info("Processing entry %d", entry_idx)
            try:
                # Generate ipv6 addr and mask
                ipv6_dst_addr = generateIpv6Addr()
                ipv6_dst_addr_mask = generateIpv6Addr()
                logger.debug("Ipv6 addr is : " + ipv6_dst_addr)
                logger.debug("Ipv6 addr mask is :" + ipv6_dst_addr_mask)

                pkt = testutils.simple_tcpv6_packet(eth_dst=dmac, ipv6_dst=ipv6_dst_addr)
                exp_pkt = pkt

                target = gc.Target(device_id=0, pipe_id=0xffff)

                # Extract the tcam val and mask that is going to be programmed from the
                # generated ipv6 addr and mask
                tcam_field_val = getIpv6AddrFieldSliceValue(self.tcam_field_name, ipv6_dst_addr)
                logger.debug("TCAM val is " + hex(tcam_field_val))
                tcam_field_mask = getIpv6AddrFieldSliceValue(self.tcam_field_name, ipv6_dst_addr_mask)
                logger.debug("TCAM mask is " + hex(tcam_field_mask))
                tcam_field_width_byte = getFieldSliceByteWidthFromName(self.tcam_field_name)

                # Insert the entry
                logger.info("Insert the entry")
                table_key = table_obj.make_key(
                    [gc.KeyTuple(self.port_field_name, ig_port),
                     gc.KeyTuple(self.tcam_field_name, tcam_field_val, tcam_field_mask),
                     gc.KeyTuple("$MATCH_PRIORITY", 1)])
                table_obj.entry_add(
                    target,
                    [table_key],
                    [getDataObj(table_obj, eg_port)])

                testutils.send_packet(self, ig_port, pkt)

                logger.info("Expecting packet on port %d", eg_port)
                testutils.verify_packet(self, exp_pkt, eg_port)
                testutils.verify_no_other_packets(self)

                # Get the entry
                resp = table_obj.entry_get(target, None, {"from_hw": True})
                # For tcam, the mask is applied on the value before programming it in the hardware
                # Thus form the expected tcam value to be read from the hw
                tcam_field_val_hw = (tcam_field_val & tcam_field_mask)
                for data, key in resp:
                    data_dict = data.to_dict()
                    key_dict = key.to_dict()
                    assert key_dict[self.port_field_name]['value'] == ig_port
                    assert key_dict[self.tcam_field_name]['value'] == tcam_field_val_hw
                    assert key_dict[self.tcam_field_name]['mask'] == tcam_field_mask
            finally:
                # Delete the entry
                logger.info("Delete the entry")
                table_obj.entry_del(
                    target,
                    [table_key])

                testutils.send_packet(self, ig_port, pkt)
                logger.info("Packet is expected to get dropped.")
                testutils.verify_no_other_packets(self)


class SingleFieldSingleSliceExmTest(BfRuntimeTest):
    """ @brief This test adds a field slice entry in the table, sends packets and verifies,
    reads back the entry and verifies and then finally deletes the entry
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_single_slice"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.exm_field_name = "hdr.ipv6.dst_addr[15:7]"
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]

        dmac = '22:22:22:22:22:22'

        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        num_entries = 100

        table_obj = bfrt_info.table_get(self.table_name)
        table_key = None

        for entry_idx in range(num_entries):
            logger.info("Processing entry %d", entry_idx)
            try:
                # Generate ipv6 addr
                ipv6_dst_addr = generateIpv6Addr()
                logger.debug("Ipv6 addr is " + ipv6_dst_addr)

                pkt = testutils.simple_tcpv6_packet(eth_dst=dmac, ipv6_dst=ipv6_dst_addr)
                exp_pkt = pkt

                target = gc.Target(device_id=0, pipe_id=0xffff)

                # Extract the exact match val that is going to be programmed from the
                # generated ipv6 addr
                exm_value = getIpv6AddrFieldSliceValue(self.exm_field_name, ipv6_dst_addr)
                logger.debug("EXM val is " + hex(exm_value))

                # Insert the entry
                logger.info("Insert the entry")
                table_key = table_obj.make_key(
                    [gc.KeyTuple(self.port_field_name, ig_port),
                     gc.KeyTuple(self.exm_field_name, exm_value)])
                table_obj.entry_add(
                    target,
                    [table_key],
                    [getDataObj(table_obj, eg_port)])

                testutils.send_packet(self, ig_port, pkt)

                logger.info("Expecting packet on port %d", eg_port)
                testutils.verify_packet(self, exp_pkt, eg_port)
                testutils.verify_no_other_packets(self)

                # Get the entry
                resp = table_obj.entry_get(target, None, {"from_hw": True})
                for data, key in resp:
                    data_dict = data.to_dict()
                    key_dict = key.to_dict()
                    assert key_dict[self.port_field_name]['value'] == ig_port
                    assert key_dict[self.exm_field_name]['value'] == exm_value

            finally:
                # Delete the entry
                if table_key:
                    logger.info("Delete the entry")
                    table_obj.entry_del(
                        target,
                        [table_key])

                testutils.send_packet(self, ig_port, pkt)
                logger.info("Packet is expected to get dropped")
                testutils.verify_no_other_packets(self)


def SingleFieldSingleSliceLpmHelper(self, table_obj, lpm_field_name):
    """ @brief This is a helper function that adds a field slice entry in the table, sends packets and verifies, reads back the 
    entry and verifies and then finally deletes the entry
    """

    ig_port = swports[1]
    eg_port = swports[2]

    dmac = '22:22:22:22:22:22'
    num_entries = 100

    for entry_idx in range(num_entries):
        logger.info("Processing entry %d", entry_idx)
        try:
            # Generate ipv6 addr
            ipv6_dst_addr = generateIpv6Addr()
            logger.debug("Ipv6 addr is " + ipv6_dst_addr)

            pkt = testutils.simple_tcpv6_packet(eth_dst=dmac, ipv6_dst=ipv6_dst_addr)
            exp_pkt = pkt

            target = gc.Target(device_id=0, pipe_id=0xffff)

            # Extract the lpm val, lpm field width in bits and bytes that is going to be programmed from the
            # generated ipv6 addr
            lpm_field_val = getIpv6AddrFieldSliceValue(lpm_field_name, ipv6_dst_addr)
            lpm_field_width_byte = getFieldSliceByteWidthFromName(lpm_field_name)
            lpm_value_bit_width = getFieldSliceBitWidthFromName(lpm_field_name)

            # Choose a prefix len between 1 and lpm field bit width
            p_len = random.randint(1, lpm_value_bit_width)
            logger.debug("LPM val is " + hex(lpm_field_val))
            logger.debug("LPM prefix len is " + hex(p_len))

            # Insert the entry
            logger.info("Insert the entry")
            table_key = table_obj.make_key(
                [gc.KeyTuple(self.port_field_name, ig_port),
                 gc.KeyTuple(lpm_field_name, lpm_field_val, prefix_len=p_len)])
            table_obj.entry_add(
                target,
                [table_key],
                [getDataObj(table_obj, eg_port)])

            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)
            testutils.verify_no_other_packets(self)

            # Get the entry
            resp = table_obj.entry_get(target, None, {"from_hw": True})
            # for lpm, the mask is applied on the value before programming it in the hardware
            # thus get the expected lpm field val to be read from hw
            lpm_field_val_hw = (((lpm_field_val) >> (lpm_value_bit_width - p_len)) << (lpm_value_bit_width - p_len))
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                assert key_dict[self.port_field_name]['value'] == ig_port
                assert key_dict[lpm_field_name]['value'] == lpm_field_val_hw
                assert key_dict[lpm_field_name]['prefix_len'] == p_len

        finally:
            # Delete the entry
            logger.info("Delete the entry")
            table_obj.entry_del(
                target,
                [table_key])

            testutils.send_packet(self, ig_port, pkt)
            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class SingleFieldSingleSliceLpmTest(BfRuntimeTest):
    """ @brief This test adds a field slice entry in the table, sends packets and verifies,
    reads back the entry and verifies and then finally deletes the entry
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_single_slice_lpm"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.lpm_field_name = "hdr.ipv6.dst_addr[94:45]"
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)
        SingleFieldSingleSliceLpmHelper(self, table_obj, self.lpm_field_name)


class SingleFieldSingleSliceLpmAliasTest(BfRuntimeTest):
    """ @brief This test adds a field slice entry in the table, sends packets and verifies,
    reads back the entry and verifies and then finally deletes the entry
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_single_slice_lpm_alias"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.lpm_field_name = "ipv6.dstaddr_127_89_"
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)
        SingleFieldSingleSliceLpmHelper(self, table_obj, self.lpm_field_name)


class SingleFieldSingleSliceRangeTest(BfRuntimeTest):
    """ @brief This test adds a field slice entry in the table, sends packets and verifies,
    reads back the entry and verifies and then finally deletes the entry
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_single_slice_range"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.range_field_name = "hdr.ipv6.dst_addr[77:66]"

    def runTest(self):
        ig_port = swports[1]
        eg_port = swports[2]

        dmac = '22:22:22:22:22:22'
        logger.info("Range field slices are not supported yet")
        # FIXME This test fails
        return

        self.set_bfrt_info(self.parse_bfrt_info(self.get_bfrt_info("tna_field_slice")))
        target = self.Target(device_id=0, pipe_id=0xffff)
        num_entries = 100

        for entry_idx in range(num_entries):
            logger.info("Processing entry %d", entry_idx)
            try:
                ipv6_dst_addr = generateIpv6Addr()
                pkt = testutils.simple_tcpv6_packet(eth_dst=dmac, ipv6_dst=ipv6_dst_addr)
                logger.debug("Ipv6 addr is " + ipv6_dst_addr)
                exp_pkt = pkt
                range_bit_width = getFieldSliceBitWidthFromName(self.range_field_name)
                range_byte_width = getFieldSliceByteWidthFromName(self.range_field_name)
                range_start = 1
                range_end = 0
                while range_end < range_start:
                    range_start = random.randint(0, pow(2, range_bit_width))
                    range_end = random.randint(0, pow(2, range_bit_width))
                pkt_range_value = getIpv6AddrFieldSliceValue(self.range_field_name, ipv6_dst_addr)
                range_start = pkt_range_value - 1
                range_end = pkt_range_value + 1
                should_range_match = False
                if pkt_range_value >= range_start and pkt_range_value <= range_end:
                    logger.info("Entry is expected to hit")
                    should_range_match = True
                else:
                    logger.info("Entry is expected to miss")
                logger.debug("Range start is " + hex(range_start))
                logger.debug("Range end is " + hex(range_end))
                logger.debug("Pkt Range value  is " + hex(pkt_range_value))
                logger.info("Insert the entry")
                self.insert_table_entry(
                    target,
                    self.table_name,
                    [self.KeyField('ig_intr_md.ingress_port', self.to_bytes(ig_port, 2)),
                     self.KeyField(self.range_field_name,
                                   low=self.to_bytes(range_start, range_byte_width),
                                   high=self.to_bytes(range_end, range_byte_width)),
                     self.KeyField("$MATCH_PRIORITY",
                                   self.to_bytes(1, 4))],
                    'SwitchIngress.hit',
                    [self.DataField('port', self.to_bytes(eg_port, 2))])

                testutils.send_packet(self, ig_port, pkt)

                if should_range_match == True:
                    logger.info("Expecting packet on port %d", eg_port)
                    testutils.verify_packet(self, exp_pkt, eg_port)
                testutils.verify_no_other_packets(self)

                # Get key field ids
                key_port_field_id = self.get_key_field(self.table_name, "ig_intr_md.ingress_port")
                key_ipv6_dst_field_id = self.get_key_field(self.table_name, self.range_field_name)

                # Get all the entries
                resp = self.get_table_entry(target, self.table_name, None, {"from_hw": True})
                key_dict = {}
                entry_iter = self.parseEntryGetResponse(resp, key_dict)
                for data_dict in entry_iter:
                    assert key_dict[key_port_field_id]['value'] == self.to_bytes(ig_port, 2)
                    assert key_dict[key_ipv6_dst_field_id]['low'] == self.to_bytes(range_start, range_byte_width)
                    assert key_dict[key_ipv6_dst_field_id]['high'] == self.to_bytes(range_end, range_byte_width)
            finally:
                logger.info("Delete the entry")
                self.delete_table_entry(
                    target,
                    self.table_name,
                    [self.KeyField('ig_intr_md.ingress_port', self.to_bytes(ig_port, 2)),
                     self.KeyField(self.range_field_name,
                                   low=self.to_bytes(range_start, range_byte_width),
                                   high=self.to_bytes(range_end, range_byte_width)),
                     self.KeyField("$MATCH_PRIORITY",
                                   self.to_bytes(1, 4))])

                testutils.send_packet(self, ig_port, pkt)
                logger.info("Packet is expected to get dropped.")
                testutils.verify_no_other_packets(self)


def MultiSliceTestHelper(self, table_obj, exm_0_field_name, tcam_field_name, lpm_field_name, exm_1_field_name,
                         is_multi_field):
    """ @brief
    This is a helper function that 
    - Adds multiple field sliced entry in the table
    - For some runs, purposely creates discrepancies between the key fields that are already programmed and the values in 
      the sent pkts and ensures that the packet doesn't hit the entry. The way this is done is, the ipv6 address is first
      generated. Then the value of the field slice is extraced from that address. Then a bit which we expect to match on
      is flipped in the extracted value. Thus there will be a mismatch between the value present in the packet and the value
      installed in the hardware which will cause the packet to miss
    - Always scrambles the bits which are not expected to match on
    - Reads back the entry and verifies
    - Sends packets and verifies, 
      - If the installed key fields match the ones in the pkt, then the pkt should hit the entry
      - If they don't match then the pkt should miss the entry
    - Finally deletes the entry
    """
    ig_port = swports[1]
    eg_port = swports[2]

    dmac = '22:22:22:22:22:22'

    target = gc.Target(device_id=0, pipe_id=0xffff)
    num_entries = 100

    for entry_idx in range(num_entries):
        logger.info("Processing entry %d ", entry_idx)
        try:
            # Generate ipv6 dst addr and mask
            ipv6_dst_addr = generateIpv6Addr()
            ipv6_dst_addr_mask = generateIpv6Addr()
            logger.debug("Generated dst v6 addr is " + ipv6_dst_addr)
            logger.debug("Generated dst v6 mask is " + ipv6_dst_addr_mask)

            # Generate ipv6 src addr and mask
            ipv6_src_addr = generateIpv6Addr()
            ipv6_src_addr_mask = generateIpv6Addr()
            logger.debug("Generated src v6 addr is " + ipv6_src_addr)
            logger.debug("Generated src v6 mask is " + ipv6_src_addr_mask)

            # Extract all the field values from the dst addr
            tcam_v6_addr = ipv6_dst_addr
            tcam_v6_addr_mask = ipv6_dst_addr_mask
            lpm_v6_addr = ipv6_dst_addr
            exm_0_v6_addr = ipv6_dst_addr
            exm_1_v6_addr = ipv6_dst_addr

            if is_multi_field == True:
                # This indicates that the total 4 field slices are from 2 different fields, (2 slices from 1 field each)
                # Thus, the first exm field slice and tcam field slice are actually slices of a ipv6 src addr (Refer the p4
                # program for deduction). Hence set them accordingly
                tcam_v6_addr = ipv6_src_addr
                tcam_v6_addr_mask = ipv6_src_addr_mask
                exm_0_v6_addr = ipv6_src_addr

            pkt = testutils.simple_tcpv6_packet(eth_dst=dmac, ipv6_dst=ipv6_dst_addr, ipv6_src=ipv6_src_addr)
            exp_pkt = pkt

            # Start off by expecting the sent packet to match on all the fields
            should_exm_match_0 = True
            should_exm_match_1 = True
            should_tcam_match = True
            should_lpm_match = True

            # For different iterations of the test, we want to tamper with different fields individually
            if entry_idx % 5 == 1:
                should_exm_match_0 = False
            elif entry_idx % 5 == 2:
                should_exm_match_1 = False
            elif entry_idx % 5 == 3:
                should_tcam_match = False
            elif entry_idx % 5 == 4:
                should_lpm_match = False

            # We use this set to keep track of bits we want to flip/garble within a field slice
            garble_bit_index = set()

            # exm
            exm_val_0 = getIpv6AddrFieldSliceValue(exm_0_field_name, exm_0_v6_addr)
            exm_field_byte_width_0 = getFieldSliceByteWidthFromName(exm_0_field_name)
            exm_field_bit_width_0 = getFieldSliceBitWidthFromName(exm_0_field_name)
            logger.debug("EXM_0 value before scrambling " + hex(exm_val_0))
            # Scramble bits if we want the pkt to miss in this iteration because of a mismatch on this field
            if should_exm_match_0 == False:
                # Flip any bit which we expect to match on
                index = random.randint(0, exm_field_bit_width_0 - 1)
                exm_val_0 = garbleBits(exm_val_0, [index])
            logger.debug("EXM_0 value after scrambling " + hex(exm_val_0))

            # ternary
            tcam_val = getIpv6AddrFieldSliceValue(tcam_field_name, tcam_v6_addr)
            tcam_mask = getIpv6AddrFieldSliceValue(tcam_field_name, tcam_v6_addr_mask)
            tcam_field_bit_width = getFieldSliceBitWidthFromName(tcam_field_name)
            tcam_field_byte_width = getFieldSliceByteWidthFromName(tcam_field_name)
            logger.debug("TCAM value before scrambling " + hex(tcam_val))
            # Scramble bits if we want the pkt to miss in this iteration because of a mismatch on this field
            if should_tcam_match == False:
                # Flip any bit which we expect to match on
                index = tcam_field_bit_width
                for i in range(tcam_field_bit_width):
                    if ((tcam_mask >> i) & 0x01) == 1:
                        index = i
                if index == tcam_field_bit_width:
                    # indicates that the generated  mask is all zeros. Thus ternary field is always going to match
                    # Hence flip the flag accordingly
                    should_tcam_match = True
                else:
                    tcam_val = garbleBits(tcam_val, [index])
            # Always scramble bits which are don't care
            garble_bit_index.clear()
            for i in range(tcam_field_bit_width):
                bit_val = (tcam_mask >> i) & 1
                if bit_val == 0:
                    garble_bit_index.add(i)
            tcam_val = garbleBits(tcam_val, garble_bit_index)
            logger.debug("TCAM value after scrambling " + hex(tcam_val))
            # We never scramble the tcam mask
            logger.debug("TCAM mask " + hex(tcam_mask))

            # lpm
            lpm_value_bit_width = getFieldSliceBitWidthFromName(lpm_field_name)
            lpm_value_byte_width = getFieldSliceByteWidthFromName(lpm_field_name)
            p_len = random.randint(1, lpm_value_bit_width)
            lpm_value = getIpv6AddrFieldSliceValue(lpm_field_name, lpm_v6_addr)
            logger.debug("LPM value before scrambling " + hex(lpm_value))
            logger.debug("LPM prefix width " + hex(p_len))
            # Scramble bits if we want the pkt to miss in this iteration because of a mismatch on this field
            if should_lpm_match == False:
                # Flip any bit within the prefix length so that the value in the packet
                # won't match with the value programmed. We are always flipping the bit
                # at index p_len
                lpm_value = garbleBits(lpm_value, [lpm_value_bit_width - p_len])
            if p_len != lpm_value_bit_width:
                # Always scramble the bits which are not supposed to be looked up anyway
                garble_bit_index.clear()
                for i in range(lpm_value_bit_width - 1 - p_len):
                    garble_bit_index.add(i)
                lpm_value = garbleBits(lpm_value, garble_bit_index)
            logger.debug("LPM value after scrambling " + hex(lpm_value))

            # exm
            exm_val_1 = getIpv6AddrFieldSliceValue(exm_1_field_name, exm_1_v6_addr)
            exm_field_byte_width_1 = getFieldSliceByteWidthFromName(exm_1_field_name)
            exm_field_bit_width_1 = getFieldSliceBitWidthFromName(exm_1_field_name)
            logger.debug("EXM_1 value before scrambling " + hex(exm_val_1))
            # Scramble bits if we want the pkt to miss in this iteration because of a mismatch on this field
            if should_exm_match_1 == False:
                # Flip any bit which we expect to match on
                index = random.randint(0, exm_field_bit_width_1 - 1)
                exm_val_1 = garbleBits(exm_val_1, [index])
            logger.debug("EXM_1 value after scrambling " + hex(exm_val_1))

            logger.info("Insert the entry")
            table_key = table_obj.make_key(
                [gc.KeyTuple(self.port_field_name, ig_port),
                 gc.KeyTuple(exm_0_field_name, exm_val_0),
                 gc.KeyTuple(tcam_field_name, tcam_val, tcam_mask),
                 gc.KeyTuple(lpm_field_name, lpm_value, prefix_len=p_len),
                 gc.KeyTuple(exm_1_field_name, exm_val_1),
                 gc.KeyTuple("$MATCH_PRIORITY", 1)])
            table_obj.entry_add(
                target,
                [table_key],
                [getDataObj(table_obj, eg_port)])

            # Get the entry
            resp = table_obj.entry_get(target, None, {"from_hw": True})
            # for lpm and tcam, the mask is applied on the value before programming it in the hardware
            # thus get the expected lpm and tcam val to be read from hw
            lpm_val_hw = (((lpm_value) >> (lpm_value_bit_width - p_len)) << (lpm_value_bit_width - p_len))
            tcam_value_hw = tcam_val & tcam_mask
            for data, key in resp:
                data_dict = data.to_dict()
                key_dict = key.to_dict()
                assert key_dict[self.port_field_name]['value'] == ig_port
                assert key_dict[exm_0_field_name]['value'] == exm_val_0
                assert key_dict[tcam_field_name]['value'] == tcam_value_hw
                assert key_dict[tcam_field_name]['mask'] == tcam_mask
                assert key_dict[lpm_field_name]['value'] == lpm_val_hw
                assert key_dict[lpm_field_name]['prefix_len'] == p_len
                assert key_dict[exm_1_field_name]['value'] == exm_val_1

            # Send the packet
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Should Exm_0 match %d", should_exm_match_0)
            logger.info("Should Tcam match %d", should_tcam_match)
            logger.info("Should Lpm match %d", should_lpm_match)
            logger.info("Should Exm_1 match %d", should_exm_match_1)
            if should_exm_match_0 == 1 and should_tcam_match == 1 and should_lpm_match == 1 and should_exm_match_1 == 1:
                # For this iteration we expect the pkt to match on all the field slices
                logger.info("Expecting packet on port %d after installing entry %d", eg_port, entry_idx)
                testutils.verify_packet(self, exp_pkt, eg_port)
            else:
                # For this iteration we expect the pkt to miss on any one of the fields and hence to get dropped
                logger.info("Expecting the packet to get dropped after installing entry %d", entry_idx)
                testutils.verify_no_other_packets(self)

        finally:
            # Delete the entry
            logger.info("Delete the entry")
            table_obj.entry_del(
                target,
                [table_key])

            testutils.send_packet(self, ig_port, pkt)
            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class SingleFieldMultiSliceTest(BfRuntimeTest):
    """ @brief This test adds entries to the table, sends packets which should miss and hit the entries and verifies, reads back the
    entries and verifies and finally deletes the entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_multi_slice"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)

        MultiSliceTestHelper(self, table_obj, exm_0_field_name="hdr.ipv6.dst_addr[41:5]",
                             tcam_field_name="hdr.ipv6.dst_addr[127:54]", lpm_field_name="hdr.ipv6.dst_addr[53:42]",
                             exm_1_field_name="hdr.ipv6.dst_addr[4:0]", is_multi_field=False)


class SingleFieldMultiSliceAliasTest(BfRuntimeTest):
    """ @brief This test adds entries to the table, sends packets which should miss and hit the entries and verifies, reads back the
    entries and verifies and finally deletes the entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_single_field_multi_slice_alias"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)

        MultiSliceTestHelper(self, table_obj, exm_0_field_name="ipv6.dstaddr_23_9_",
                             tcam_field_name="ipv6.dstaddr_57_28_", lpm_field_name="ipv6.dstaddr_100_70_",
                             exm_1_field_name="hdr.ipv6.dst_addr[7:3]", is_multi_field=False)


class MultiFieldMultiSliceTest(BfRuntimeTest):
    """ @brief This test adds entries to the table, sends packets which should miss and hit the entries and verifies, reads back the
    entries and verifies and finally deletes the entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_multi_field_multi_slice"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)

        MultiSliceTestHelper(self, table_obj, exm_0_field_name="hdr.ipv6.src_addr[84:50]",
                             tcam_field_name="hdr.ipv6.src_addr[43:0]", lpm_field_name="hdr.ipv6.dst_addr[127:16]",
                             exm_1_field_name="hdr.ipv6.dst_addr[5:5]", is_multi_field=True)


class MultiFieldMultiSliceAliasTest(BfRuntimeTest):
    """ @brief This test adds entries to the table, sends packets which should miss and hit the entries and verifies, reads back the
    entries and verifies and finally deletes the entries
    """

    def setUp(self):
        client_id = 0
        p4_name = "tna_field_slice"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        self.table_name = "SwitchIngress.forward_multi_field_multi_slice_alias"
        seed = random.randint(1, 65535)
        random.seed(seed)
        logger.info("seed used is %d", seed)
        self.port_field_name = "ig_intr_md.ingress_port"

    def runTest(self):
        bfrt_info = self.interface.bfrt_info_get("tna_field_slice")
        table_obj = bfrt_info.table_get(self.table_name)

        MultiSliceTestHelper(self, table_obj, exm_0_field_name="hdr.ipv6.src_addr[53:19]",
                             tcam_field_name="ipv6.srcaddr_106_80_", lpm_field_name="hdr.ipv6.dst_addr[81:60]",
                             exm_1_field_name="ipv6.dstaddr_23_9_", is_multi_field=True)
