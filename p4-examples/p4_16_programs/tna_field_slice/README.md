
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_field_slice
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The Tofino architecture supports field slicing for match keys for exact, 
ternary, and longest prefix match kinds. The example demonstrates how to use 
this feature for every match kind as well as algorithmic longest prefix match 
implementations. Furthermore, it demonstrates the usage of multiple slices of 
one header field used with different match kinds.
