
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_custom_hash
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

This example exercises the custom hash TNA feature. TNA allows users to define 
any custom CRC polynomial with the CRCPolynomial extern.

tna_random.p4 configures two custom hash functions with the parameters for the
CRC32 and CRC32-bzip2 algorithms. The P4 program computes the hash values from 
the values provided in each packet's MAC address fields and stores the resulting
hash value the same MAC address field.

The test encodes random numbers in the MAC address fields of packet, sends them
to the P4 program and compares the hash values computed by the P4 program with 
the values computed by the python zlib and crcmod modules for verification.
