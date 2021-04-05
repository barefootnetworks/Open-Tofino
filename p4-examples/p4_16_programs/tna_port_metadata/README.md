
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_port_metadata
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Each ingress hardware port can be associated with a small amount of static data 
that is prepended to each packet in the ingress pipeline. At a low level, this 
data is just a collection of bytes, but it’s often convenient to expose it to 
the control plane with a higher-level API. The drivers can expose a table-like 
control-plane API for reading and writing this data, which is referred to as 
a “phase0 table”.

The "phase0 table" can match e.g. for the ingress port number and add a custom
per-port metadata header to each packet. This example demonstrates how this
is done using a corresponding parser state.

Please note that the recommended method for extracting the port metadata is to
use the "port_metadata_unpack" extern, which is demonstrated in the 
tna_port_metadata_extern example.
