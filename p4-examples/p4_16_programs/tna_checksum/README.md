
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_checksum
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Checksums are crucial for reliable network communications. In this example, the
checksum feature of the TNA is demonstrated using the TCP and UDP over IPv4.
The P4 program detects checksum errors of incoming packets, can apply source 
network and port address translations, and is able to update the checksums if 
required. The address translation is applied transparently, without routing 
the packet and rewriting the MAC addresses.
