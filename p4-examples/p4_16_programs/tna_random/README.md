
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_random
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

The random extern provides generation of pseudo-random numbers with a uniform
distribution. If one wishes to generate numbers with a non-uniform distribution,
you may do so by first generating a uniformly distributed random value, and
then using appropriate table lookups and/or arithmetic on the resulting value
to achieve the desired distribution.

```
/// TNA definition of Random number generator 
extern Random<W> {
/// Constructor
    Random();
/// Return a random number with uniform distribution. 
/// @return : random number between 0 and 2**W - 1
    W get(W max);
}			
```

This example p4 program writes 32b random number into IPv4 dst address field.
The test computes the mean and standard deviation for the random numbers send by
the P4 program and compares them to their expected values.
