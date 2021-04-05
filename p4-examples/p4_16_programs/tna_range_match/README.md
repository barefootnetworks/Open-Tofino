
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_range_match
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Range match is one of two additional match kinds provided by TNA. In TNA, table 
keys can match on values within a range (start, end). The range is inclusive at 
the start and exclusive at the end, meaning any value between 
start, start + 1, …, end - 1 will result in a match. 

Please be aware that the implementation of range match consumes multiple TCAM 
entries per range entry and refer to the documentation for details.

The example shows how to use the range match kind.
