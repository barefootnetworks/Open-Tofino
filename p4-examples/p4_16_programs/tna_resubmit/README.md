
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_resubmit
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA)
* Programming stack: Barefoot Runtime Interface (BRI)

The example program demonstrates the resubmit feature of P4 on the TNA. When
a packet is resubmitted, it is sent back to the beginning of the ingress
pipeline after its first pass through it. On TNA, each packet can be resubmitted 
once, with or without up to 8 bytes of resubmit metadata transported by the 
resubmit header.

In this examples, after a packet was processed by the example program's pipeline, 
the dst address contains the number of times the packet did pass through the 
ingress pipeline. The output port number is provided by the resubmit header. 
It can be modified by applying a corresponding action and providing a new port 
number that is transmitted to the second pipeline pass using a resubmit header.