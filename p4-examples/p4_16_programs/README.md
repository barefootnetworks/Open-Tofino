
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


Barefoot Networks Software Development Environment
     (c) Barefoot Networks Inc, 2019-present

# Table of contents

  1.  Summary
  2.  Compiling
  3.  Testing
  4.  Examples Table
  5.  Troubleshooting

# 1. Summary

* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)
* Purpose: Demonstrate the usage of TNA and T2NA features and verify the P4 Studio SDE installation

The examples demonstrate how to use selected features of TNA and T2NA. 
Furthermore, the example programs serve as a test vehicle to verify that that the Barefoot P4 Studio 
SDE environment has been properly installed and configured to enable running packet tests (PTF) against 
the Tofino ASIC model for the P4-16 software stack. 

The examples in this directory are tested and verified to be functional. 
Each directory is self-contained -- it contains a readme file, the p4 file(s), and the 
PTF tests associated with the example. The P4 file with the same name as the directory 
(with a .p4 extension) is typically the main P4 file. 
Each example's readme file contains a description of what it does and special instructions on how to build or run the example, if any.

Most examples are tested on both, TNA and T2NA. Each example's readme file lists the architectures it is tested on.

Please check the corresponding readme file of an example before running it.

# 2.  Compiling

All examples are automatically built and installed by default during the P4 Studio installation process.
If needed, they can be built and installed through the P4 Studio Build tool.
The following instructions are needed only when an existing example was modified, or a new example was added.
To run an example included and built with P4 Studio, the please continue with Section 3.

An example program can be compiled by using the provided build scripts by using
the instructions below if nothing different is stated in its readme file:

```
export SDE=/path/to/sde/
export SDE_INSTALL=$SDE/install
export PATH=$SDE_INSTALL/bin:$PATH
cd $SDE/pkgsrc/p4-build
./autogen.sh
cd $SDE/build/p4-examples
```

For Tofino:
```
$SDE/pkgsrc/p4-build/configure --with-tofino --with-p4c=p4c --prefix=$SDE_INSTALL \
      --bindir=$SDE_INSTALL/bin \
      P4_NAME=<program name> \
      P4_PATH=$SDE/pkgsrc/p4-examples/p4_16_programs/<program name>/<program name>.p4 \
      P4_VERSION=p4-16 P4_ARCHITECTURE=tna \
      LDFLAGS="-L$SDE_INSTALL/lib"
```

For Tofino2:
```
$SDE/pkgsrc/p4-build/configure --with-tofino2 --with-p4c=p4c --prefix=$SDE_INSTALL \
      --bindir=$SDE_INSTALL/bin \
      P4_NAME=<program name> \
      P4_PATH=$SDE/pkgsrc/p4-examples/p4_16_programs/<program name>/<program name>.p4 \
      P4_VERSION=p4-16 P4_ARCHITECTURE=t2na \
      LDFLAGS="-L$SDE_INSTALL/lib"
```

Then, for both Tofino and Tofino2:
```
make
make install
```


# 3.  Testing

To run an example, follow these steps if nothing different is stated in its readme file:

```
export SDE=/path/to/sde/
export SDE_INSTALL=$SDE/install
sudo $SDE_INSTALL/bin/veth_setup.sh
```

In three separate terminal windows:
```
cd $SDE
./run_tofino_model.sh --arch <tofino|tofino2> -p <program name>
```
```
cd $SDE
./run_switchd.sh --arch <tofino|tofino2> -p <program name>
```
```
cd $SDE
./run_p4_tests.sh --arch <tofino|tofino2> -p <program name>
```

## Example test output:
```
[...]
Setting up veth interfaces
No of Veths is 64
Adding CPU veth
Arch is tofino
Target is asic-model
Connecting to localhost port 7777 to check status on these devices: [0]
Waiting for device 0 to be ready
Started port clean script
WARNING: No route found for IPv6 destination :: (no default route?)
test.DynHashingTest ... Subscribe attempt #1
Subscribe response received 0
Binding with p4_name tna_dyn_hashing
Binding with p4_name tna_dyn_hashing successful!!
Received tna_dyn_hashing on GetForwarding
=============== Testing <test case> entry operation===============
<per test case details>
ok

  ----------------------------------------------------------------------
<summary of all testcases>

OK
```


# 4.  Example List

The table provides an overview on the features demonstrated in the examples.
For each major TNA/T2NA feature, the example demonstrating is usage is listed.

| Feature                                      | Example Name                |
|----------------------------------------------|-----------------------------|
| Action profiles                              | tna_action_profile          |
| Action selectors                             | tna_action_selector         |
| Batch read error reporting                   | tna_lpm_match               |
| Batch write error reporting                  | tna_ternary_match           |
| BRI and hardware interaction                 | tna_operations              |
| BRI and pdfixed over Thrift in one test      | bri_with_pdfixed_thrift     |
| Bridged metadata                             | tna_bridged_md              |
| Checksum verification and update             | tna_checksum                |
| Configure ports                              | tna_ports                   |
| Counters                                     | tna_counter                 |
| Custom hash                                  | tna_custom_hash             |
| Digest                                       | tna_digest                  |
| Dynamic hash selection                       | tna_dyn_hashing             |
| Dynamic key mask                             | tna_dkm                     |
| Dynamically set pipeline program             | bri_set_forwarding_pipeline |
| Exact match                                  | tna_exact_match             |
| L2/L3 Switching                              | tna_simple_switch           |
| Longest prefix match (algorithmic-lpm-based) | tna_lpm_match               |
| Longest prefix match (TCAM-based)            | tna_lpm_match               |
| Low pass filter (LPF)                        | tna_meter_lpf_wred          |
| Match-action table entry scopes              | tna_exact_match             |
| Meter byte counter adjustment                | tna_meter_bytecount_adjust  |
| Meters                                       | tna_meter_lpf_wred          |
| Mirror                                       | tna_mirror                  |
| Multicast                                    | tna_multicast               |
| Multiple pipelines in one program            | tna_32q_2pipe               |
| Multiple programs on one device              | tna_32q_multiprogram        |
| P4Runtime basic example                      | p4rt_utests                 |
| P4Runtime comprehensive example              | onf_fabric                  |
| Packet generator                             | tna_pktgen                  |
| Parser value sets (PVS)                      | tna_pvs                     |
| Pipeline snapshots                           | tna_snapshot                |
| Port metadata                                | tna_port_metadata           |
| Port metadata using an extern                | tna_port_metadata_extern    |
| Proxy hash                                   | tna_proxy_hash              |
| Random                                       | tna_random                  |
| Range Match                                  | tna_range_match             |
| Register table entry scopes                  | tna_register                |
| Registers and register actions               | tna_register                |
| Resubmit                                     | tna_resubmit                |
| Symmetric Hash                               | tna_symmetric_hash          |
| Table default entry handling                 | tna_exact_match             |
| Table entry idle timeout                     | tna_idletimeout             |
| Table key field slicing                      | tna_field_slice             |
| Table pipe scope handling                    | tna_exact_match             |
| Ternary match (algorithmic-TCAM-based)       | tna_ternary_match           |
| Ternary match (TCAM-based)                   | tna_ternary_match           |
| Timestamps                                   | tna_timestamp               |
| WRED                                         | tna_meter_lpf_wred          |


# 5.  Troubleshooting

Please contact support@barefootworks.com for issues related to this program.
To quickly resolve your issue, please include with your issue report:
  1. The P4 program and PTF test you exercised.
  2. The P4 Studio SDE version.
  3. The OS, Virtual Machine, or Container environment you are using.
  4. The steps followed to exercise the test.
  5. A description of the error encountered and/or unexpected behavior and any
     relevant execution artifacts, e.g. a console log with an error message,
     a driver log, and/or compiler output artifacts.

