
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_symmetric_hash
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The symmetric hash makes pairs of hash fields order independent. This means that
a pair of values will result in the same hash value, independent of the order
they are provided to the hash engine.
This feature is useful for load balancing, e.g. ECMP to ensure that packets of
a single flow takes the same path for independent of their direction, i.e. the
order of the input values for the hash engine.

The example demonstrates how to use the @symmetric pragma for multiple pairs
of values.
