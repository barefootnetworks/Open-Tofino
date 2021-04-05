
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_digest
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Digest generation is a mechanism to send a message from the data plane to the 
control plane without sending the entire packet to the CPU. A digest for one 
packet includes only selected header fields to be sent to and processed by the
control plane processor.

This program serves as an example of generating digests for the purpose of MAC 
learning. The P4 program consists of a simple lookup table that triggers a MAC 
learning digest if a source MAC address is unknown or if it is known but attached 
to a different port.
