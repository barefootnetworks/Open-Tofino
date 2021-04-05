
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_lpm_match
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Longest prefix matching (lpm) is essential for IP routing. It matches an IP address
with the routing table entry with the longest prefix mask.

This example demonstrates how to use TCAM-based lpm match tables as well as
algorithmic-lpm-based lpm match tables.
