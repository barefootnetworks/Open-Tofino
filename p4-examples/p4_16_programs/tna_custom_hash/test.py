# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import logging
import struct
import zlib
import crcmod
import random

from ptf import config, mask
import ptf.testutils as testutils
from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.client as gc

dev_id = 0
p4_program_name = "tna_custom_hash"

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()


class TestCustomHash(BfRuntimeTest):
    """@brief Send values encoded in MAC addresses and verify that the returned
    hashes match the expected values.
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
            random.seed(1)
            num_trials = 20
            logger.info(("\nInject {} packets with random values provided in the eth" +
                         " dst field.").format(num_trials))
            logger.info("Tofino computes the crc32 and crc-bzip2 hashes of the " +
                        "random value and stores them in the dst and src mac " +
                        "fields. We verify the hash value each packet.")

            # crc32 function for verification
            hash1_func = zlib.crc32
            # crc32-bzip2 function for verification
            hash2_func = crcmod.predefined.mkCrcFun('crc-32-bzip2')

            for i in range(num_trials):
                # random_value = random.randint(0, 2^32-1)
                random_value = random.randint(0, 2 ** 32 - 1)
                random_hex = "{:012x}".format(random_value)
                random_mac = ":".join(random_hex[t:t + 2] for t in range(0, 12, 2))
                ipkt = testutils.simple_udp_packet(eth_dst=str(random_mac),
                                                   eth_src=str(random_mac),
                                                   ip_src='1.2.3.4',
                                                   ip_dst='100.99.98.97',
                                                   ip_id=101,
                                                   ip_ttl=64,
                                                   udp_sport=0x1234,
                                                   udp_dport=0xabcd)

                testutils.send_packet(self, swports[0], ipkt)
                (rcv_dev, rcv_port, rcv_pkt, pkt_time) = \
                    testutils.dp_poll(self, dev_id, swports[1], timeout=2)
                nrcv = ipkt.__class__(rcv_pkt)

                hash1_p4_value = int(str(nrcv[Ether].dst).replace(":", ""), 16)
                hash2_p4_value = int(str(nrcv[Ether].src).replace(":", ""), 16)

                hash1_exp_value = hash1_func(struct.pack("!I", random_value)) & 0xffffffff
                hash2_exp_value = hash2_func(struct.pack("!I", random_value)) & 0xffffffff

                # verify the hash
                logger.info("Random value        : {:x}".format(random_value))
                logger.info("Random value mac    : {}".format(random_mac))
                logger.info("Hash1 P4 value       : {:x}".format(hash1_p4_value))
                logger.info("Hash1 expected value : {:x}".format(hash1_exp_value))
                logger.info("Hash2 P4 value       : {:x}".format(hash2_p4_value))
                logger.info("Hash2 expected value : {:x}".format(hash2_exp_value))
                assert hash1_p4_value == hash1_exp_value
                assert hash2_p4_value == hash2_exp_value

        finally:
            table_output_port.default_entry_reset(target)
