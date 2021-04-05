
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_snapshots
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The MAU snapshot feature allows user to analyze packets as they move through 
different MAU stages in the chip. It is a very useful tool for debugging forwarding 
issues. It is embedded within the forwarding pipeline, and it can capture a 
packet processing state in real-time without disruptions to performance or 
control-plane resources. It helps to answer questions such as:

 - Did the packet reach the pipeline processing engine?
 - How do the packet headers appear (e.g. Layer 2 to Layer 4 data)
 - How is the packet altered in each stage, and where is it sent?

Tofino has the ability to capture a snapshot based on a local stage trigger or 
from a trigger that was hit in some previous stage. As part of a snapshot 
capture, Tofino captures the output PHV’s from a stage. Tofino also captures 
some control information that describes the stage environment when the capture 
happened. 

The example shows how to set a snapshot trigger and how to extract the snapshot
data from the chip through BRI.
