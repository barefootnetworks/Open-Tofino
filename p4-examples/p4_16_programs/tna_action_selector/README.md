
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_action_selector
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Action selectors implement yet another mechanism to populate table entries with
action specifications that have been defined outside the table entry. They are
more powerful than action profiles because they also provide the ability to
dynamically select the action specification to apply upon matching a table
entry. An action selector extern can be instantiated as a resource in the P4
program, similar to action profiles. 

The example program demonstrates how to use action selectors in a P4 program
as well as its configuration through the BRI.