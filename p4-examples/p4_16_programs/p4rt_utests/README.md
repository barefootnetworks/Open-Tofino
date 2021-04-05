
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: p4rt_utests
* P4 version: P4_16
* Architectures: P4.org v1model
* Programming stack: P4Runtime

This P4 program is used to test the P4Runtime implementation for Tofino ASICs,
or more precisely the Barefoot PI implementation (PI is the library used to
support P4Runtime on different vendor SDKs).

The PTF tests that are included with this program demonstrate how to:
 * use the P4Runtime `SetForwardingPipelineConfig` RPC to update the P4 pipeline
   running on a Tofino ASIC.
 * generate P4Runtime `Read` and `Write` RPCs by using the helper methods
   defined in the `P4RuntimeTest` base PTF class.

Please note that the PTF tests do NOT demonstrate how to use P4Runtime in 
conjunction with BRI "Fixed".

## How to run
This example requires a special configuration to build and run that is described 
in the `P4Runtime API Guide`.
