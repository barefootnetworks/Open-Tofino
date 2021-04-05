
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_dkm
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Dynamic table key masks (DKMs) enable runtime programmable masks for table key 
fields. The compiler pragma specifies that the associated exact match-action table, 
supports runtime programmable masks for fields in the table key. A value of 1 turns 
this feature on.  All other values are ignored.  It is an error if this pragma is 
applied to a table that must be implemented in TCAM resources or as an algorithmic TCAM.

The example illustrates the usage of DKM using a simple router implementation.