
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_32q_2pipe
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Tofino can run separate control flows on each of the chip's hardware pipelines. The 
example illustrates this capability by using two different pipeline definitions within 
a single program using a single control plane interface.

Please note that this program contains specific configuration parameters that
work only on the model and the 32Q Tofino chip. All features are available on 
the other Tofino configurations as well. However, the example will only work on a
32Q chip.

## How to run
This examples requires a special configuration to run.

### Model
A special *multipipe\_custom\_bfrt.conf* file is required with the model and the 
*int-port-loop* parameter to configure the model to behave like a 32 queue chip.
```
run_tofino_model.sh --arch <tofino|tofino2> -p tna_32q_2pipe -c p4_examples/p4_16_programs/<tofino|tofino2>/multipipe_custom_bfrt.conf --int-port-loop 0xa
```

### switchd

```
run_switchd.sh -p tna_32q_2pipe
```

### test
```
run_p4_tests.sh -p tna_32q_2pipe
```


