
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_timestamp
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The TNA provides a total of six timestamps per packet throughout the packet 
processing pipeline. This example demonstrates how to access each one of these
by writing their values into the payload of a UDP packet.

The list of available timestamps:

 - ingress_intrinsic_metadata_t.ingress_mac_tstamp
 - ingress_intrinsic_metadata_from_parser_t.global_tstamp
 - egress_intrinsic_metadata_t.enq_tstamp
 - egress_intrinsic_metadata_t.deq_timedelta
 - egress_intrinsic_metadata_from_parser_t.global_tsamp
 - egress_intrinsic_metadata_for_output_port_t.capture_tstamp_on_tx

Please note that the MAC tx timestamp works on hardware only and is not
supported in the model at this time.
