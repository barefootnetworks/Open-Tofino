# 
# Copyright (c) Intel Corporation
# SPDX-License-Identifier: CC-BY-ND-4.0
#


import ptf
import grpc
from ptf import config
import ptf.testutils as testutils
from ptf.testutils import *
from bfruntime_client_base_tests import BfRuntimeTest, BaseTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import sys
import threading
import time
import random
try:
    import queue as q
except ImportError as e:
    import Queue as q

test_dir = sys.argv[sys.argv.index("--test-dir") + 1]
sys.path.append(test_dir + "/../../")
from tna_32q_multiprogram.test import *
del(MultiProgramFixedComponentTest)

logger = logging.getLogger('Test')
if not len(logger.handlers):
    formatter = logging.Formatter('[%(levelname)s] (%(threadName)-10s) %(message)s')
    sh = logging.StreamHandler()
    sh.setFormatter(formatter)
    logger.addHandler(sh)

base_pick_path = testutils.test_param_get("base_pick_path")
binary_name = testutils.test_param_get("arch")
if binary_name is not "tofino2" and binary_name is not "tofino":
    assert 0, "%s is unknown arch" % (binary_name)

if not base_pick_path:
    base_pick_path = "install/share/" + binary_name + "pd/"

base_put_path = testutils.test_param_get("base_put_path")
if not base_put_path:
    base_put_path = "install/share/" + binary_name + "pd/forwarding"

logger.info("\nbase_put_path=%s \nbase_pick_path=%s", base_pick_path, base_put_path)

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


def create_path_bf_rt(base_path, p4_name_to_use):
    return base_path + "/" + p4_name_to_use + "/bf-rt.json"


def create_path_context(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/context.json"


def create_path_tofino(base_path, p4_name_to_use, profile_name):
    return base_path + "/" + p4_name_to_use + "/" + profile_name + "/" + binary_name + ".bin"


def set_fwd_config(test, client_id, action, bind=True):
    """@brief Helper function to send a set fwd config msg for both the programs
        tna_32q_multiprogram_a and tna_32q_multiprogram_b.
        Note, this function cannot be used for sending WARM_INIT_END, since
        it sends a config with it as well. WARM_INIT_END cannot accept a config.

        @param test The test object which has its own interface (Program A)
        @param clinet_id client ID
        @param action proto enum of VERIFY_AND_WARM_INIT_BEGIN or 
        VERIFY_AND_WARM_INIT_BEGIN_AND_END
        @param bind If this client needs to bind to the P4 as well
    """
    p4_name_to_put = p4_name_to_pick = p4_name = "tna_32q_multiprogram_a"
    profile_name_to_put = profile_name_to_pick = "profile_a"

    config_list = [
        gc.ForwardingConfig(p4_name_to_put,
                            create_path_bf_rt(base_pick_path, p4_name_to_pick),
                            [gc.ProfileInfo(profile_name_to_put,
                                            create_path_context(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                                            create_path_tofino(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                                            external_pipes)]
                            )
    ]

    p4_name_to_put = p4_name_to_pick = "tna_32q_multiprogram_b"
    profile_name_to_put = profile_name_to_pick = "profile_b"
    config_list.append(
        gc.ForwardingConfig(p4_name_to_put,
                            create_path_bf_rt(base_pick_path, p4_name_to_pick),
                            [gc.ProfileInfo(profile_name_to_put,
                                            create_path_context(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                                            create_path_tofino(base_pick_path, p4_name_to_pick, profile_name_to_pick),
                                            internal_pipes)]
                            )
    )

    success = test.interface.send_set_forwarding_pipeline_config_request(
        action,
        base_put_path,
        config_list)
    if not success:
        raise RuntimeError("Failed to get response for setfwd")
    test.p4_name = p4_name
    if bind:
        test.interface.bind_pipeline_config(p4_name)


class MultipleProgramNoReplayTest(MultiProgramTest):
    """
    Case 4 Starting Program_a and Program_b on device from scratch.
    We have only overloaded the functions which need to be different from
    MultiProgramTest. All the other ones will be defaulted to their base class
    automatically.
    This test does the following

    Client_1.setUp()
    1. Client 1 Subscribes to server 
    2. Send a VERIFY_AND_WARM_INIT_BEGIN_AND_END SetForwardingPipelineConfig msg
    which includes the entire device's config
    3. Wait for WARM_INIT_FINISHED
    4. Client 1 BINDS to program_a

    Client_2.setUp()
    1. Client 2 initiates a subscribe and BINDS to program_b. Here in this 
    test, client_2 subscribes after warm_init is finished, but even if that 
    wasn't the case and it was bound to the same name earlier, it would have
    continued to be connected and bound.

    multiple.setUp():
    1. Client_1.setUp()
    2. Client_2.setUp()

    multiple.runTest()
    1. Run the test which uses all the pipeline_profiles
    """

    class ProgramA(MultiProgramTest.ProgramA):
        def setUp(self, client_id, p4_name="tna_32q_multiprogram_a"):
            # Setup and don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

            # Send a Verify and Warm_init_begin and end
            logger.info("Sending Verify and warm init being and end")
            set_fwd_config(self, client_id,
                           bfruntime_pb2.SetForwardingPipelineConfigRequest.
                           VERIFY_AND_WARM_INIT_BEGIN_AND_END)
            self.bfrt_info = self.interface.bfrt_info_get(p4_name)


class MultipleProgramWithReplayTest(MultiProgramTest):
    """
    Case 5 Starting Program_a and Program_b on device from scratch but with replays
    We have only overloaded the functions which need to be different from
    MultiProgramTest. All the other ones will be defaulted to their base class
    automatically.
    This test does the following

    Client_1.setUp()
    1. Client 1 Subscribes to server 
    2. Send a VERIFY_AND_WARM_INIT_BEGIN SetForwardingPipelineConfig msg
    which includes the entire device's config
    3. Wait for WARM_INIT_STARTED
    4. Client 1 BINDS to program_a

    Client_2.setUp()
    1. Client 2 initiates a subscribe and BINDS to program_b. Here in this 
    test, client_2 subscribes after warm_init is finished, but even if that 
    wasn't the case and it was bound to the same name earlier, it would have
    continued to be connected and bound. Regardles of when this client would
    have bound, the server refreshes after WARM_INIT_STARTED

    multiple.setUp():
    1. Client_1.setUp()
    2. Client_2.setUp()

    multiple.runTest()
    1. Replay entries on both Program_a and Program_b
    2. Issue WARM_INIT_END request from Client 1.
    3. Wait for WARM_INIT_FINISHED in order to continue
    """

    class ProgramA(MultiProgramTest.ProgramA):
        def setUp(self, client_id, p4_name="tna_32q_multiprogram_a"):
            # Setup and don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

            self.client_id = client_id

            # Send a Verify and Warm_init_begin
            logger.info("Sending Verify and warm_init begin")
            set_fwd_config(self, client_id,
                           bfruntime_pb2.SetForwardingPipelineConfigRequest
                           .VERIFY_AND_WARM_INIT_BEGIN)
            self.bfrt_info = self.interface.bfrt_info_get(p4_name)

    def setUp(self):
        super(MultipleProgramWithReplayTest, self).setUp()

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

        # Replay entries
        logger.info("Replaying entries")
        program_entries(self, target, ig_port, int_port, eg_port, port_meta_a, port_meta_b, tag, dmac, dip, ttl)

        # Send a WARM_INIT_END message from client 1 
        # Don't add any config info to this request since it will be denied
        logger.info("Sending warm_init_end")
        import time
        time.sleep(3)
        try:
            success = set_fwd_config(self.a, self.a.client_id,
                                     bfruntime_pb2.SetForwardingPipelineConfigRequest.
                                     WARM_INIT_END)
        except gc.BfruntimeRpcException as e:
            # expect an invalid arg error since we are trying to send config with
            # warm_init_end
            if e.grpc_error_get().code() != grpc.StatusCode.INVALID_ARGUMENT:
                raise e
            pass

        success = self.a.interface.send_set_forwarding_pipeline_config_request(
            bfruntime_pb2.SetForwardingPipelineConfigRequest.WARM_INIT_END)

        if not success:
            raise RuntimeError("Failed to get response for setfwd")
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


class MultipleProgramToSingleProgramTest(MultiProgramTest):
    """
    Case 6 Starting with Program_a and Program_b on device. Moving to
    a single program on the device.
    This test does the following

    Client_1.setUp()
    1. Client 1 Subscribes to server 
    2. Send a VERIFY_AND_WARM_INIT_BEGIN_AND_END SetForwardingPipelineConfig msg
    which includes the entire device's config
    3. Wait for WARM_INIT_FINISHED
    4. Client 1 BINDS to program_a

    Client_2.setUp()
    1. Client 2 initiates a subscribe and BINDS to program_b.

    multiple.setUp():
    1. Client_1.setUp()
    2. Client_2.setUp()

    multiple.runTest()
    1. Issue VERIFY_AND_WARM_INIT_BEGIN_AND_END from client 1 for tna_ternary_match
    2. Both client 1 and client 2 should be kicked immediately after receving
    WARM_INIT_STARTED since they were bound to invalid programs now.
    3. Subscribe from client 1
    4. BIND from client 1.
    """

    class ProgramA(MultiProgramTest.ProgramA):

        def setUpBind(self, client_id, p4_name):
            # Setup with a p4 name in order to subscribe and BIND
            BfRuntimeTest.setUp(self, client_id, p4_name)

        def setUp(self, client_id, p4_name="tna_32q_multiprogram_a"):
            # Setup and don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

            # Send a Verify and Warm_init_begin and end
            logger.info("Sending Verify and warm init being and end")
            set_fwd_config(self, client_id,
                           bfruntime_pb2.SetForwardingPipelineConfigRequest.
                           VERIFY_AND_WARM_INIT_BEGIN_AND_END)
            self.bfrt_info = self.interface.bfrt_info_get(p4_name)

    def setUp(self):
        super(MultipleProgramToSingleProgramTest, self).setUp()

    def runTest(self):
        p4_name_to_put = p4_name_to_pick = new_p4_name = "tna_ternary_match"
        profile_name_to_put = profile_name_to_pick = "pipe"
        # Send a Verify and warm_init_begin and warm_init_end
        logger.info("Sending Verify and warm_init_begin and warm_init_end for %s", p4_name_to_put)
        action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN_AND_END
        success = self.a.interface.send_set_forwarding_pipeline_config_request(
            action,
            base_put_path,
            [gc.ForwardingConfig(p4_name_to_put,
                                 create_path_bf_rt(base_pick_path, p4_name_to_pick),
                                 [gc.ProfileInfo(profile_name_to_put,
                                                 create_path_context(base_pick_path, p4_name_to_pick,
                                                                     profile_name_to_pick),
                                                 create_path_tofino(base_pick_path, p4_name_to_pick,
                                                                    profile_name_to_pick),
                                                 [0, 1, 2, 3])]
                                 )])
        if not success:
            raise RuntimeError("Failed to setFwd")

        # Check if both the clients have dropped out
        exception = self.a.interface.exception_q.get(timeout=5)
        if exception.grpc_error_get().code() != grpc.StatusCode.CANCELLED:
            logger.error("Did not receive cancelled for a")
            assert (0)
        exception = self.b.interface.exception_q.get(timeout=5)
        if exception.grpc_error_get().code() != grpc.StatusCode.CANCELLED:
            logger.error("Did not receive cancelled for b")
            assert (0)

        # Initiate subscribe and BIND for client 1
        self.a.setUpBind(1, new_p4_name)

        # Try inserting a table entry in the new Program and test it
        target = gc.Target(device_id=0, pipe_id=0xffff)
        ig_port = get_port_from_pipes(external_pipes)
        eg_port = get_port_from_pipes(external_pipes)
        dip = '10.10.0.1'
        pkt = testutils.simple_tcp_packet(ip_dst=dip)
        exp_pkt = pkt
        self.a.bfrt_info = self.a.interface.bfrt_info_get(new_p4_name)
        self.a.forward_table = self.a.bfrt_info.table_get("SwitchIngress.forward")
        self.a.forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
        try:
            key = self.a.forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', 1),
                                                gc.KeyTuple('vrf', 0),
                                                gc.KeyTuple('hdr.ipv4.dst_addr',
                                                            dip,
                                                            '255.255.0.0')])
            data = self.a.forward_table.make_data([gc.DataTuple('port', eg_port)],
                                                'SwitchIngress.hit')
            self.a.forward_table.entry_add(target, [key], [data])

            # check get
            resp = self.a.forward_table.entry_get(target, [key], {"from_hw": True})

            data_dict = next(resp)[0].to_dict()
            recv_port = data_dict["port"]
            if (recv_port != eg_port):
                logger.error("Error! port sent = %s received port = %s", str(eg_port), str(recv_port))
                assert 0

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Expecting packet on port %d", eg_port)
            testutils.verify_packet(self, exp_pkt, eg_port)
            testutils.verify_no_other_packets(self, timeout=2)
        except Exception as e:
            logger.info("!!!! Test Failed!!!!")
            import traceback
            traceback.print_exc()
            raise e

        finally:
            self.a.forward_table.entry_del(
                target,
                [self.a.forward_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', 1),
                                                gc.KeyTuple('vrf', 0),
                                                gc.KeyTuple('hdr.ipv4.dst_addr',
                                                            dip,
                                                            '255.255.0.0')])])

            logger.info("Sending packet on port %d", ig_port)
            testutils.send_packet(self, ig_port, pkt)

            logger.info("Packet is expected to get dropped.")
            testutils.verify_no_other_packets(self)


class MultipleProgramToMultipleProgramTest(MultiProgramTest):
    """
    Case 7 Starting with Program_a and Program_b on device. Moving to
    Program_c and Program_b on device. The second client should be kicked out
    This test does the following

    Client_1.setUp()
    1. Client 1 Subscribes to server 
    2. Send a VERIFY_AND_WARM_INIT_BEGIN_AND_END SetForwardingPipelineConfig msg
    which includes the entire device's config- Program_a and Program_b
    3. Wait for WARM_INIT_FINISHED
    4. Client 1 BINDS to program_a

    Client_2.setUp()
    1. Client 2 initiates a subscribe and BINDS to program_b.

    multiple.setUp():
    1. Client_1.setUp()
    2. Client_2.setUp()

    multiple.runTest()
    1. Issue VERIFY_AND_WARM_INIT_BEGIN_AND_END from client 1 for Program_c and Program_b
    2. Only client 1 should be kicked out after WARM_INIT_STARTED since program_a doesn't
    exist anymore. Client 2 should remain undisturbed and should only receive WARM_INIT_STARTED
    and WARM_INIT_FINISHED messages
    3. Subscribe from client 1
    4. BIND from client 1.
    """

    class ProgramA(MultiProgramTest.ProgramA):

        def setUpBind(self, client_id, p4_name):
            # Setup with a p4 name in order to subscribe and BIND
            BfRuntimeTest.setUp(self, client_id, p4_name)

        def setUp(self, client_id, p4_name="tna_32q_multiprogram_a"):
            # Setup and don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

            # Send a Verify and Warm_init_begin and end
            logger.info("Sending Verify and warm init being and end")
            set_fwd_config(self, client_id,
                           bfruntime_pb2.SetForwardingPipelineConfigRequest.
                           VERIFY_AND_WARM_INIT_BEGIN_AND_END)
            self.bfrt_info = self.interface.bfrt_info_get(p4_name)

    def setUp(self):
        super(MultipleProgramToMultipleProgramTest, self).setUp()

    def runTest(self):
        # Only changing the name of the program from a to c
        # So it will be picked from a but put into c for testing purposes
        # The server should treat them as different programs
        p4_name_to_put = new_p4_name = "tna_32q_multiprogram_c"
        p4_name_to_pick = "tna_32q_multiprogram_a"
        profile_name_to_put = "profile_c"
        profile_name_to_pick = "profile_a"

        # Send a Verify and warm_init_begin and warm_init_end
        logger.info("Client 1:")
        action = bfruntime_pb2.SetForwardingPipelineConfigRequest.VERIFY_AND_WARM_INIT_BEGIN_AND_END
        config_list = [
            gc.ForwardingConfig(p4_name_to_put,
                                create_path_bf_rt(base_pick_path, p4_name_to_pick),
                                [gc.ProfileInfo(profile_name_to_put,
                                                create_path_context(base_pick_path, p4_name_to_pick,
                                                                    profile_name_to_pick),
                                                create_path_tofino(base_pick_path, p4_name_to_pick,
                                                                   profile_name_to_pick),
                                                external_pipes)]
                                )
        ]

        p4_name_to_put = p4_name_to_pick = "tna_32q_multiprogram_b"
        profile_name_to_put = profile_name_to_pick = "profile_b"
        config_list.append(
            gc.ForwardingConfig(p4_name_to_put,
                                create_path_bf_rt(base_pick_path, p4_name_to_pick),
                                [gc.ProfileInfo(profile_name_to_put,
                                                create_path_context(base_pick_path, p4_name_to_pick,
                                                                    profile_name_to_pick),
                                                create_path_tofino(base_pick_path, p4_name_to_pick,
                                                                   profile_name_to_pick),
                                                internal_pipes)]
                                )
        )

        success = self.a.interface.send_set_forwarding_pipeline_config_request(
            action,
            base_put_path,
            config_list)
        if not success:
            raise RuntimeError("Failed to get response for setfwd")

        # Check if client 1 has dropped out
        exception = self.a.interface.exception_q.get(timeout=5)
        if exception.grpc_error_get().code() != grpc.StatusCode.CANCELLED:
            logger.error("Did not receive cancelled for a")
            assert (0)
        # client 2 should not drop out
        logger.info("Client 2:")
        is_queue_empty = False
        try:
            exception = self.b.interface.exception_q.get(timeout=5)
        except q.Empty:
            logger.info("Did not receive cancelled for b. Expected behavior")
            is_queue_empty = True
            pass
        if not is_queue_empty:
            logger.error("Exception_queue was supposed to be empty")
            assert (0)

        # Check for WARM_INIT_FINISHED on client 2.
        success = self.b.interface.is_set_fwd_action_done(
            bfruntime_pb2.SetForwardingPipelineConfigResponseType.
            Value("WARM_INIT_FINISHED"), 5, 5)
        if not success:
            raise RuntimeError("Failed to receive set fwd response")

        # Initiate subscribe and BIND for client 1
        logger.info("Client 1:")
        self.a.setUpBind(1, new_p4_name)


class MultipleProgramStressTest(BaseTest):
    """
    Starting Program_a and Program_b on device from scratch. We will create
    m:  1 client. It subscribes and then keeps pushing the same
        program in a loop. Run in main thread
    g:  2 clients which without BIND, just try to get the device's config using
        GetForwardingPipelineConfigRequest. Run in separate threads.
    w:  2 clients to keep subscribing and BIND to a program and then write to
        a table. Run in separate threads.


    Client_m.setUp():
    1. Client tries subscribe till it succeeds.

    Client_m.runTest():
    1. Send a VERIFY_AND_WARM_INIT_BEGIN_AND_END SetForwardingPipelineConfig msg
       which includes the entire device's config
    2. Wait for WARM_INIT_FINISHED
    3. Go back to step 1

    Client_g.setUp():
    1. Client tries subscribe till it succeeds.

    Client_g.runTest():
    1. Get device config. Retry till success
    2. go back to step 1

    Client_w1.setUp():
    1. Client tries subscribe till it succeeds.

    Client_w1.runTest():
    1. BIND to program a. Retry till success
    2. Write entries to table
    3. go back to step 1

    Client_w2.setUp():
    1. Client tries subscribe till it succeeds.

    Client_w2.runTest():
    1. BIND to program b. Retry till success
    2. Write entries to table
    3. go back to step 1

    multiple.runTest()
    1. Start Client_w1 thread
    2. Start CLient_w2 thread
    3. Start a few Client_g threads
    4. Make a Client_m and let it verify and warm_init_begin and warm_init_end
    5. Stop all the threads and ensure no errors occurred
    """

    class ProgramM(BfRuntimeTest):
        def setUp(self, client_id):
            # Setup and don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)
            self.client_id = client_id

        def runTest(self):
            # Send a Verify and warm_init_begin and warm_init_end
            logger.info("Sending Verify and warm_init_begin and warm_init_end")
            set_fwd_config(self, self.client_id,
                           bfruntime_pb2.SetForwardingPipelineConfigRequest.
                           VERIFY_AND_WARM_INIT_BEGIN_AND_END, False)

    class ProgramG(BfRuntimeTest):
        def setUp(self, client_id):
            # Setup don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

        def runTest(self):
            logger.info("Client G: getting fwd config")
            try:
                self.interface.bfrt_info_get("tna_32q_multiprogram_b")
            except gc.BfruntimeRpcException as e:
                # Errors tolerated ->
                # 1. Device is locked
                if e.grpc_error_get().code() == grpc.StatusCode.UNAVAILABLE:
                    logger.info("Failed to GetFwd because device is locked")
                    logger.info(e)
                else:
                    logger.error("Failed to GetFwdConfig")
                    raise e
            except Exception as e:
                raise e
            logger.info("BfRtInfo GetFwdConfig successful!")

    class ProgramW1(BfRuntimeTest):
        def setUp(self, client_id):
            # Setup don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

        def runTest(self):
            self.p4_name = "tna_32q_multiprogram_a"
            # Keep BINDing to the program till success or 5 tries
            while (True):
                try:
                    time.sleep(1)
                    logger.info("Client W1: Binding to %s", self.p4_name)
                    self.interface.bind_pipeline_config("tna_32q_multiprogram_a")
                    self.bfrt_info = self.interface.bfrt_info_get(self.p4_name)
                    break
                except gc.BfruntimeRpcException as e:
                    # Errors tolerated ->
                    # 1. Device is locked
                    # 2. Already_exists because this client is already bound
                    # to the same P4
                    if e.grpc_error_get().code() == grpc.StatusCode.UNAVAILABLE:
                        logger.info("Failed to BIND because device is locked %s",
                                    e.grpc_error_get().code())
                        continue
                    elif e.grpc_error_get().code() == grpc.StatusCode.ALREADY_EXISTS:
                        logger.info("This client is already connected to this P4 %s",
                                    e.grpc_error_get().code())
                        break
                    logger.error("Failed to BIND %s", e.grpc_error_get().code())
                    raise e
                except Exception as e:
                    raise e
            # Write a few entries
            logger.info("Writing to Table: SwitchIngress_a.storm_control")
            target = gc.Target(device_id=0, pipe_id=0xffff)
            self.storm_control_table = self.bfrt_info.table_get("SwitchIngress_a.storm_control")
            for i in range(10):
                try:
                    self.storm_control_table.entry_add(
                        target,
                        [self.storm_control_table.make_key(
                            [gc.KeyTuple('ig_intr_md.ingress_port', i)])],
                        [self.storm_control_table.make_data(
                            [gc.DataTuple('index', i)],
                            'SwitchIngress_a.set_color')]
                    )
                except gc.BfruntimeRpcException as e:
                    # All errors tolerated here since we just want to try and write entries
                    logger.info("%s", e)
                    pass

    class ProgramW2(BfRuntimeTest):
        def setUp(self, client_id):
            # Setup don't perform bind
            BfRuntimeTest.setUp(self, client_id, perform_bind=False)

        def runTest(self):
            self.p4_name = "tna_32q_multiprogram_b"
            # Keep BINDing to the program till success
            while (True):
                try:
                    time.sleep(1)
                    logger.info("Client W2: Binding to %s", self.p4_name)
                    self.interface.bind_pipeline_config("tna_32q_multiprogram_b")
                    self.bfrt_info = self.interface.bfrt_info_get(self.p4_name)
                    break
                except gc.BfruntimeRpcException as e:
                    # Errors tolerated ->
                    # 1. Device is locked, try again
                    # 2. Already_exists because this client is already bound
                    # to the same P4, break
                    if e.grpc_error_get().code() == grpc.StatusCode.UNAVAILABLE:
                        logger.info("Failed to BIND because device is locked %s", e.grpc_error_get().code())
                        continue
                    elif e.grpc_error_get().code() == grpc.StatusCode.ALREADY_EXISTS:
                        logger.info("This client is already connected to this P4 %s", e.grpc_error_get().code())
                        break
                    logger.error("Failed to BIND %s", e.grpc_error_get().code())
                    raise e
                except Exception as e:
                    raise e
            # Write a few entries
            logger.info("Writing to Table: SwitchIngress_b.forward")
            target = gc.Target(device_id=0, pipe_id=0xffff)
            dip = "5.6.7.8"
            self.e_forward_table = self.bfrt_info.table_get("SwitchEgress_b.forward")
            self.e_forward_table.info.key_field_annotation_add("hdr.ipv4.dst_addr", "ipv4")
            for i in range(10):
                try:
                    self.e_forward_table.entry_add(
                        target,
                        [self.e_forward_table.make_key(
                            [gc.KeyTuple('hdr.ipv4.dst_addr', dip, prefix_len=31),
                             gc.KeyTuple('hdr.ipv4.ttl', i),
                             gc.KeyTuple('hdr.custom_metadata.custom_tag', i)])],
                        [self.e_forward_table.make_data([],
                                                        'SwitchEgress_b.hit')]
                    )
                except gc.BfruntimeRpcException as e:
                    # All errors tolerated here since we just want to try and write entries
                    logger.info("%s", e)
                    pass
                except Exception as e:
                    raise e

    class ClientThread(threading.Thread):
        def __init__(self, client_id, client, max_sleep_time, name):
            super(MultipleProgramStressTest.ClientThread, self).__init__(name=name)
            self.client_id = client_id
            self.client = client
            self.max_sleep_time = max_sleep_time
            self._exception_q = q.Queue()
            self.stop_event = threading.Event()

        def run(self):
            self.client.setUp(self.client_id)
            while not self.stop_event.wait(1):
                sleep_time = random.randint(0, self.max_sleep_time)
                time.sleep(sleep_time)
                try:
                    self.client.runTest()
                except gc.BfruntimeRpcException as e:
                    # Check if there are any exceptions
                    self._exception_q.put(e)
                    raise e
                except Exception as e:
                    self._exception_q.put(e)
                    raise e

            logger.info("Client-thread %d STOPPED", self.client_id)

        def stop(self):
            self.stop_event.set()
            self.join()
            self.client.tearDown()
            if (not self._exception_q.empty()):
                exception = self._exception_q.get()
                raise exception

    def setUp(self):
        # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()

    def runTest(self):
        # Run client once in order to ensure that the device has something
        # to start with. We don't want to ignore every error on the client threads
        m = self.ProgramM()
        m.setUp(1)
        m.runTest()

        # Start thread for client_w1
        w1 = self.ClientThread(2, self.ProgramW1(), 5, name='Writer-1')
        w1.start()

        # Start thread for client_w2
        w2 = self.ClientThread(3, self.ProgramW2(), 4, name='Writer-2')
        w2.start()

        # Start threads for client_g
        g_client_list = []
        for client_id in range(4, 6):
            g = self.ClientThread(client_id, self.ProgramG(), 3, name='GetFwd-' + str(client_id - 3))
            g.start()
            g_client_list.append(g)

        # Run the client several times on this thread itself
        for i in range(5):
            time.sleep(4)
            m.runTest()

        m.tearDown()
        # Let the test run for another 10 seconds
        logger.info("Stopping all theads!!!!")
        time.sleep(10)

        w1.stop()
        w2.stop()
        for i in g_client_list:
            i.stop()

        logger.info("TEST OVER!!!!!!!!")

    def tearDown(self):
        BaseTest.tearDown(self)


# Remove all the classes from global space which would have been added as part of the multiprogram test
# so that they are not run as part of this suite
globals().pop("MultiProgramTest")
