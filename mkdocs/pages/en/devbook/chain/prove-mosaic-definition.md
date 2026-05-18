---
title: Prove Mosaic Definition
tutorial_level: intermediate
---

# Prove a Mosaic's Definition

Every Symbol <block:> header has a `stateHash` that covers the full chain state, including all <mosaic:> definitions.
By requesting a proof and verifying it locally, you can confirm that the data a node returns matches what is actually
recorded on chain, without having to trust the node or run one yourself.

This tutorial shows how to fetch a mosaic's definition from the API, serialize it into the same binary format used by
the chain, and verify the result against the block's `stateHash` using a <Patricia tree:> proof.

## Prerequisites

Before you start, [set up your development environment](../start/setup.md).
This tutorial only reads data from the network. No <account:> or <XYM:> balance is required.

Additionally, review how [block hashes](../../textbook/blocks.md#block-hashes) work, in particular the state hash
section.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/chain/prove_mosaic_definition', ['py', 'js']) }}

This example verifies the network's currency mosaic (<XYM:> on mainnet), whose ID is discovered automatically from
<get:/network/properties>.
The currency mosaic is a convenient choice because it exists on every Symbol network, but the same process works
for any mosaic by replacing the ID.

## Code Explanation

### Fetching the Mosaic Definition

{{ tutorial.code_snippet_tagged('step-1') }}

The code starts by fetching the network currency mosaic ID from <get:/network/properties>.
The `currencyMosaicId` field is a hex string with embedded apostrophes (e.g. `0x72C0212E'67A08BCE`), so the code strips
them before parsing.

The mosaic's full definition is then fetched from <get:/mosaics/{mosaicId}>.
The response includes all the fields that make up the mosaic's definition: `version`, `id`, `supply`,
`startHeight`, `ownerAddress`, `revision`, `flags`, `divisibility`, and `duration`.

### Computing the Key and Value Hashes

{{ tutorial.code_snippet_tagged('step-2') }}

To verify the mosaic's definition, the code must reproduce the exact hash that the chain stores internally.
This requires serializing all fields into a binary buffer in the exact field order and sizes defined by the
<ser:MosaicEntry> schema, then computing the SHA3-256 hash of the result.

!!! note "Nested structures"

    <ser:MosaicEntry> contains a <ser:MosaicDefinition> structure, which in turn contains a <ser:MosaicProperties>
    structure.
    The code serializes fields from all three levels, so the full field list is longer than what `MosaicEntry` alone
    shows.

The mosaic [sub-cache Patricia tree](../../textbook/blocks.md#state-hash) stores each mosaic as a key-value pair in a
leaf node.
The **value** is the hashed value (SHA3-256 of the serialized definition).
The **key** is computed by hashing just the mosaic ID (8 bytes, little-endian) with SHA3-256, and is used to locate
the mosaic's leaf node in the tree.

### Fetching the Block State Hash

{{ tutorial.code_snippet_tagged('step-3') }}

The code fetches the current chain height from <get:/chain/info>, then uses it to retrieve the corresponding block
header from <get:/blocks/{height}>.

The block's `stateHash` field is the SHA3-256 hash of all sub-cache Merkle roots concatenated together.
The `stateHashSubCacheMerkleRoots` array contains the individual root hash for each sub-cache
(accounts, mosaics, namespaces, and so on).

### Fetching the Tree Path

{{ tutorial.code_snippet_tagged('step-4') }}

The <get:/mosaics/{mosaicId}/merkle> endpoint returns the raw path through the mosaic sub-cache, from the root down to
the leaf that stores the mosaic's hashed value.
`deserializePatriciaTreeNodes` converts this raw binary into a list of tree nodes.

For educational purposes, the code then walks the deserialized path and prints each node for inspection.
This step is not required for verification, as the SDK handles it internally in the next step.
Branch nodes show their active links and which [nibble](../../textbook/blocks.md#state-hash) was followed to reach the
next node.
The leaf node at the end stores the remaining path nibbles and the hashed value.

### Verifying the Proof

{{ tutorial.code_snippet_tagged('step-5') }}

<dy:Merkle.provePatriciaMerkle> takes five arguments, each computed in an earlier step:

| Parameter                          | Source                                                     | Role                                                 |
| -----------------------------------| -----------------------------------------------------------| ---------------------------------------------------- |
| {{ tutorial.var('encoded_key') }}  | SHA3-256 of the mosaic ID                                  | Identifies which leaf to look up in the tree         |
| {{ tutorial.var('hashed_value') }} | SHA3-256 of the serialized definition                      | The expected value stored in the leaf                |
| {{ tutorial.var('merkle_path') }}  | <get:/mosaics/{mosaicId}/merkle>                           | The chain of branch and leaf nodes from root to leaf |
| {{ tutorial.var('state_hash') }}   | `stateHash` from <get:/blocks/{height}>                    | The block header's hash of all chain state           |
| {{ tutorial.var('roots') }}        | `stateHashSubCacheMerkleRoots` from <get:/blocks/{height}> | The individual root hash of each sub-cache           |

The function then verifies the proof in three stages:

1. **Link to the block:** Checks that `SHA3-256(roots)` matches {{ tutorial.var('state_hash') }}, confirming the
    sub-cache roots are genuine.
    Then checks that the hash of the first node in {{ tutorial.var('merkle_path') }} matches one of those roots
    (the mosaic sub-cache).
2. **Walk the tree**: Follows {{ tutorial.var('merkle_path') }} from root to leaf, checking that each node's hash
    appears among its parent's 16 links (one per nibble value `0`–`F`).
3. **Match the leaf:** Checks that the leaf's value matches {{ tutorial.var('hashed_value') }} and that the path
    through the tree reconstructs {{ tutorial.var('encoded_key') }}.

If all checks pass, the result is `0x0001` (`VALID_POSITIVE`), confirming that the mosaic definition returned by the API
is exactly what is recorded in the chain at the given height.

See <js:PatriciaMerkleProofResult> for the full set of possible result codes.

!!! warning "Height consistency"

    The mosaic definition, block header, and tree path must all reflect the same chain state.
    If a new block is confirmed between requests, the state hash will have changed and the proof will fail.
    When this happens, re-fetch all three pieces of data and try again.
    If the proof still fails after retrying, the node may be serving incorrect data.

## Output

The following output shows a typical run of the program:

```text linenums="1" hl_lines="3-14 15 16 19 22-26 27"
--8<-- 'devbook/chain/prove_mosaic_definition.log'
```

Some highlights from the output:

* **Mosaic definition** (lines 3-14): The full mosaic definition as returned by <get:/mosaics/{mosaicId}>, showing all
    fields that are serialized for the proof.

* **Hashed value and encoded key** (lines 15-16): The SHA3-256 hashes used as the value and encoded key in the Patricia
    tree.

* **State hash** (line 19): The block header's hash of all chain state.
    The proof checks that the tree path traces back to this hash, which ties the verification to a specific block.

* **Tree path** (lines 22-26): The deserialized path from root to leaf.
    Each branch node shows its links and which nibble was followed (`-> follow`).
    Every branch node consumes one nibble (hex digit) of the encoded key, and the leaf stores the remaining nibbles
    that were not consumed by any branch.
    Concatenating the nibbles `3`, `A`, `4`, `C` with the leaf path `540D7E...559E` reconstructs the full encoded key
    from line 16.
    The leaf's value matches the hashed value from line 15.
    See the [state hash](../../textbook/blocks.md#state-hash) diagram in the Textbook for a visual representation
    of this tree structure.

* **Proof result** (line 27): The proof succeeded, confirming that the mosaic definition served by the node is identical to
    what is stored on chain at height `3220296`.

## Conclusion

This tutorial showed how to:

| Step                                                                    | Related documentation                                                     |
| ----------------------------------------------------------------------- | --------------------------------------------------------------------------|
| [Fetch mosaic definition](#fetching-the-mosaic-definition)              | <get:/mosaics/{mosaicId}>                                                 |
| [Compute the key and value hashes](#computing-the-key-and-value-hashes) | <ser:MosaicEntry>                                                         |
| [Fetch the block state hash](#fetching-the-block-state-hash)            | <get:/chain/info>, <get:/blocks/{height}>                                 |
| [Fetch the tree path](#fetching-the-tree-path)                          | <get:/mosaics/{mosaicId}/merkle>, `deserializePatriciaTreeNodes`          |
| [Verify the proof](#verifying-the-proof)                                | <dy:Merkle.provePatriciaMerkle>                                           |

## Next Steps

The same technique applies to any [sub-cache](../../textbook/blocks.md#state-hash) in the state hash, not just
mosaics.

Each sub-cache has its own [catbuffer schema](../reference/serialization/index.md) that defines the binary layout
(e.g. <ser:AccountState>, <ser:RootNamespaceHistory>, <ser:MetadataEntry>) and a corresponding `/merkle` endpoint for
fetching the tree path.

To verify that a specific transaction is part of a block instead, see the
[Prove Transaction Inclusion](prove-transaction.md) tutorial.
