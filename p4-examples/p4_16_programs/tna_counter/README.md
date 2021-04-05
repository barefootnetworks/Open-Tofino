
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_counter
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Counters keep track of the packets and bytes processed by match tables. Counters can 
either be directly associated with a table entry or can be addressed by a counter id; 
the first approach is called direct counter, the latter is called indirect counter. 
This program services as an example for both types of counters. The example program 
consists of a chain of three lookup tables that match for src and dst MAC addresses 
and count the matching packets.