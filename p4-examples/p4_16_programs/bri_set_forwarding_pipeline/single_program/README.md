
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


# SetForwardingPipeline - Single Program
This example test requires building some other programs in the examples directory mentioned below. 
The variables **base\_pick\_path** and **base\_put\_path** used in the test are to simulate the path 
on the controller and the path on the switch respectively. **base\_pick\_path** is the path from where 
the test picks up the data files, chip binary etc. **base\_put\_path** is the path where the gRPC 
server puts these files on the switch and hence the path from where switchd picks up the data files 
and loads them.

## Build requirements
The following P4 program needs to be built in order to run this particular test example:

 * p4\_examples/p4\_16\_examples/tna\_register

## How to run
This examples requires a special configuration to run.

### Model
A special *tofino\_skip\_p4.conf.in* file is required with model and switchd which contains platform dependencies.
```
cd $SDE
./run_tofino_model.sh --arch <tofino|tofino2> -p dummy -c ./pkgsrc/p4-examples/<tofino|tofino2>/<tofino|tofino2>_skip_p4.conf.in 
```

### switchd
```
cd $SDE
./run_switchd.sh --skip-p4 --arch <tofino|tofino2> -c ./pkgsrc/p4-examples/<tofino|tofino2>/<tofino|tofino2>_skip_p4.conf.in
```

### test
```
cd $SDE
./run_p4_tests.sh --arch <tofino|tofino2> -p bri_set_forwarding_pipeline/single_program
```
