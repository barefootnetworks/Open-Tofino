
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_mirror
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

This section describes packet mirroring, a mechanism to create a copy of a 
packet and send the it to a specific port. This mechanism is implemented by 
the packet replication engine. Two packet paths for packet mirroring are 
available: ingress to egress mirroring and egress to egress mirroring. 

In the ingress to egress mirroring path, the ‘original’ packet before ingress 
processing is cloned and the clone is sent to the beginning of the egress 
pipeline. In the egress to egress mirroring path, the modified packet after 
egress processing is cloned and the clone is sent to the beginning of the 
egress pipeline. 

Mirroring is partially configured by the data plane program and partially 
by control plane API. The mirroring feature also prepends a user-defined packet header 
to the replica. Furthermore, when the cloned packet is sent to the egress 
pipeline, it associates the clone with a set of attributes that control how the 
cloned packet will be processed. This set of attributes is referred to as a mirroring 
session, which is configured by the control plane API.

The examples cover ingress as well egress mirroring as well as multiple 
different mirror session configurations. Each example is described in detail in 
the corresponding test class.
