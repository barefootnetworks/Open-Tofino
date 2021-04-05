
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_meter_lpf_wred
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Meters, low-pass filters (lpf), and weighted random early detection (WRED) are
mechanisms to determine the eligibility of packets to be dropped according to
different policing requirements. All three mechanisms are available in TNA/T2NA
as a direct and an indexed version. As with counters, when using the direct
version of the mechanism, the calling table provides an index implicitly by 
using the index of the calling table entry. In contrast to that, when using the 
indexed version, the index has to be provided by the programmer. 

This program serves as an example for both versions of the three mechanisms. 
Each mechanism is demonstrated in the program using a dedicated table.
