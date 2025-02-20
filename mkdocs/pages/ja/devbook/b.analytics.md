## Analytics

### Querying Network Mosaics

:warning: what examples are desired here?

### Tutorial: Querying Total Mosaics Issued on Symbol

:warning: i don't think we can do this via REST but can via mongo

### Tutorial: Querying Total Nodes Active

:warning: a node can only return nodes it knows about from its view. is that intent or should example crawl.

### Tutorial: Working with Block (.blk) Data

Catapult is storing blocks within data directory. There are two optimizations:
 * catapult stores multiple blocks in a single file, this is dependent on `fileDatabaseBatchSize` setting within config-node.configuration, by default it is set to 100
 * catapult stores block files (and statements) within subdirectories, there are at most 10_000 _objects_ per directory, that means that with setting above, there will be 100 block files within single subirectory.

Every block file starts with a small header, that contains absolute ofsset within a file to start of a block.

Following examples, will show how to find files and start offsets of few blocks, given settings above.

Example 1. Nemesis block

 1. Nemesis block has height = 1, first height needs to be rounded to a batch file id using `fileDatabaseBatchSize`, `(1 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 0
 2. next the path to actual batch file is `<directory>/<filename>.dat`, where `directory` is `id / 10_000` and `filename` is `id % 10_000`, both are padded with additional zeroes;
In this trivial case, this results in path `00000/00000.dat`
 3. The header contains 100 8-byte words, with offsets for blocks 0-99, there is no block with height 0, so the entry contains 0, offset to nemesis block will be second word
 (and with default settings mentioned above = 0x320)

Example 2. Block 1690553 (1.0.3.4 protocol fork height)
 1. round to batch file id: `(1690553 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 1690500
 2. directory name `1690500 / 10_000`, so directory name is `00169`, filename = `1690500 % 10_000`, so filname is `00500.dat`
 3. header contains offsets for blocks 1690500-1690599, block in question will be at entry 53 (0-based)
```
00000190: 549d000000000000 34a0000000000000  T.......4.......
000001a0: 0ca4000000000000 eca6000000000000  ................
000001b0: 94ab000000000000 74ae000000000000  ........t.......
```
the offset at position 53 is 0xA6EC:
```
0000a6e0: .... .... .... .... .... .... 0003 0000              ....
0000a6f0: 0000 0000 < signature data .. .... ....  ................
0000a700: .... .... .... .... .... .... .... ....  ................
0000a710: .... .... .... .... .... .... .... ....  ................
0000a720: .... .... .... .... .... .... .... ....  ................
0000a730: .... .... < public key . .... .... ....  ................
0000a740: .... .... .... .... .... .... .... ....  ................
0000a750: .... ...> 0000 0000 0168 4381 b9cb 1900  .........hC.....
```

0x19cbb9 visible at offset 0xA75C is the block height in hex.

Example 3. Block 1835458 (latest at the moment of writing)
 1. round to batch file id: `(1835458 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 1835400
 2. directory name is `00183`, file name is `05400.dat`
 3. header contains offsets for blocks 1835400-1835499, block in question will be at entry 58
```
000001c0: c8a6000000000000 a8a9000000000000  ................
000001d0: 88ac000000000000 0000000000000000  ................
000001e0: 0000000000000000 0000000000000000  ................
```

Note: that since this is latest block, all further entries in the offset map are zeroed

Parsing block data is much simpler thanks to catbuffer generated model code.

!example BlockDigester
