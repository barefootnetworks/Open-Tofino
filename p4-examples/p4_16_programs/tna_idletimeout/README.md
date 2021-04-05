
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_idletimeout
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The per-flow idle-timeout feature ensures that table entries are automatically
removed if they are not used for a given time interval. The feature is 
available as a table attribute for match tables, independent of the match kind 
used for the match keys. 

The example demonstrates how to use the idle timeout feature for both, normal
match tables and match tables using action profiles.