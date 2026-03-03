---
title: Prove Transaction Inclusion
---

# Proving a Transaction's Inclusion in a Block

Each Symbol <block:> records its transactions in a
<merkle tree:> whose root, the `transactionsHash`,
is stored in the block header.
A transaction can be verified against this root to prove it was included in a block without having to download all the
block's transactions.

This tutorial shows how to fetch a merkle proof from the API and verify that a specific transaction is part of a block.

## Prerequisites

Before you start:

* [Set up your development environment](../start/setup.md).
* Review how [state hashes](../../textbook/blocks.md#state-hashes) work, in particular the `transactionsHash`
    merkle tree.

This tutorial only reads data from the network. No <account:> or <XYM:> balance is required.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/chain/prove-transaction', ['py', 'js']) }}

The snippet reads the hash of the transaction to prove from the `TRANSACTION_HASH` environment variable.
If not set, a known transaction from block `55` of the Symbol testnet is used as the default.

## Code Explanation

### Fetching the Confirmed Transaction

{{ tutorial.code_snippet(['py:17:26', 'js:13:22']) }}

The code fetches the confirmed transaction from the <get:/transactions/confirmed/{transactionId}> endpoint.

The `meta.height` field is the block height where the transaction was confirmed, needed in the following step to
retrieve the block header.

The response also contains the `merkleComponentHash`, which is the leaf hash used in the block's merkle tree.
For regular transactions, this value equals the transaction hash.
For <aggregate transactions:>, it is computed as the SHA3-256 hash of the transaction hash concatenated with the
public keys of the cosignatories.

### Fetching the Block Header

{{ tutorial.code_snippet(['py:28:35', 'js:24:32']) }}

The <get:/blocks/{height}> endpoint returns block metadata, including the `transactionsHash` field.
This hash is the root of the merkle tree built from the
`merkleComponentHash` of each transaction in the block.

The code wraps the hex string in a `Hash256` object, which is the format expected by the <dy:proveMerkle> function.

### Fetching the Merkle Proof Path

{{ tutorial.code_snippet(['py:37:50', 'js:34:48']) }}

The <get:/blocks/{height}/transactions/{hash}/merkle> endpoint returns a **merkle proof path**:
the minimum set of intermediate hashes needed to recompute the `transactionsHash` starting from the
`merkleComponentHash` (one per level of the merkle tree).

Each item in the path contains:

* **hash**: An intermediate hash needed to recompute the next level of the tree.
* **position**: Whether this hash sits to the `left` or `right` when combined with the previous result.

The code converts each item into a pair of hash and boolean (`true` if the hash is on the left), to match the format
expected by the <dy:proveMerkle> function.

### Verifying the Proof

{{ tutorial.code_snippet(['py:52:64', 'js:50:62']) }}

<dy:proveMerkle> recomputes the merkle root by iteratively combining the `merkleComponentHash` with each intermediate
hash in the proof path, following the specified position order.
If the computed root matches the block's `transactionsHash`, the transaction is proven to be part of the block.

## Output

The following output shows a typical run of the program:

```text linenums="1" hl_lines="4 6 9 10"
--8<-- 'devbook/chain/prove-transaction.log'
```

Some highlights from the output:

* **Block height** (line 4): The transaction was confirmed in block `55`.

* **Block transactions hash** (line 6): The `transactionsHash` from block `55` is the merkle root for all
    transactions confirmed in that block.

* **Merkle path length** (line 9): A path with `4` entries means the tree has 4 levels, so the block contains up to
    2⁴ = 16 transactions.

* **Proof result** (line 10): The computed root matched the `transactionsHash`, confirming the transaction is genuinely
    part of block `55`.

To inspect the transaction or its block in the explorer, visit the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/) and enter the transaction hash or block height.

## Conclusion

This tutorial showed how to:

| Step                                                               | Related documentation                             |
| ------------------------------------------------------------------ | ------------------------------------------------- |
| [Fetch confirmed transaction](#fetching-the-confirmed-transaction) | <get:/transactions/confirmed/{transactionId}>     |
| [Fetch the block header](#fetching-the-block-header)               | <get:/blocks/{height}>                            |
| [Fetch the merkle proof path](#fetching-the-merkle-proof-path)     | <get:/blocks/{height}/transactions/{hash}/merkle> |
| [Verify the proof](#verifying-the-proof)                           | <dy:proveMerkle>                                  |

## Next Steps

The same process can be used to prove receipts by using the <get:/blocks/{height}/statements/{hash}/merkle> endpoint
and verifying against the block `receiptsHash` instead.
