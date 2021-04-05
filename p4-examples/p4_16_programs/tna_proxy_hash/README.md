
Copyright (c) Intel Corporation
SPDX-License-Identifier: CC-BY-ND-4.0


## Summary

* Name: tna_proxy_hash
* P4 version: P4_16
* Architectures: Tofino Native Architecture (TNA), Tofino2 Native Architecture (T2NA)
* Programming stack: Barefoot Runtime Interface (BRI)

Proxy hash is a mechanism to reduce the RAMs consumption of exact match tables.
When using Cuckoo Hashing, each table entry's match key is stored in the table 
to verify that not only the hashes by also the actual values of the search 
value and the match key are identical. With proxy hashing, the match key
value is hashed as well to reduce its memory footprint. The downside of this
approach is that it introduces a small probability for false positives in the 
table lookup process.
The savings of using proxy hash depends on the match key size and the table size.
In general, the larger the match keys and the tables are, the higher the memory
savings are going to be.
 