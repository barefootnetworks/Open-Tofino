
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_register
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Register and RegisterAction externs together form the stateful ALU portion of 
the pipeline. RegisterActions are triggered from table actions by calling their 
execute method, which causes the stateful ALU to read the specified element of 
the Register storage, run the RegisterAction code, and write back the modified 
value to the same element of the Register storage.

The example demonstrates how to use registers and indirect registers with TNA.
