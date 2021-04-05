
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_bridged_md
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Metadata can be passed from the ingress pipeline to the egress pipeline using
bridged metadata. In P4_14 all metadata in the ingress pipeline was implicitly
bridged to the egress pipeline. In P4_16, this has bo be is done explicitly in
each program. 

The example code illustrates how metadata can be forwarded from the ingress 
pipeline to the egress pipeline and how custom headers can be used for this 
process.
