
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_32q_multiprogram
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Tofino is capable to run separate control flows on each hardware pipeline of
the chip. The example illustrates this capability by using two different 
pipeline definitions used by two different P4 programs using a separate control 
plane interface each.

Please note that this program contains specific configuration parameters that
work only on the model and the 32Q Tofino chip. All features are available on 
the other Tofino configurations as well. However, the example will only work on a
32Q chip.

## Build requirements
This example relies on two different p4 programs: program\_a and program\_b. 
They both are compiled separately using the commands specified in the main readme file but with the following
values for the P4\_NAME and P4\_PATH variables in the configure command:

Configuration of program\_a:
```
P4_NAME=tna_32q_multiprogram_a 
P4_PATH=$SDE/pkgsrc/p4-examples/p4_16_programs/tna_32q_multiprogram/program_a/tna_32q_multiprogram_a.p4
```

Configuration of program\_b:
```
P4_NAME=tna_32q_multiprogram_b 
P4_PATH=$SDE/pkgsrc/p4-examples/p4_16_programs/tna_32q_multiprogram/program_b/tna_32q_multiprogram_b.p4
``` 
