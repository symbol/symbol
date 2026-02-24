---
title: Resolve Namespace from Receipt
---

# Resolving a Namespace from a Receipt

<namespace:|Namespace> aliases can [change over time](../../textbook/namespaces.md#duration), so the current value of a
namespace may not match the value it had when a <transaction:> was confirmed.

Every time a transaction uses an alias instead of a raw <address:> or <mosaic ID:>, the network stores a <receipt:>
with the actual value the alias pointed to at confirmation time.
The type of receipt that captures namespace resolutions is called a
[resolution statement](../../textbook/blocks.md#resolution-statements).

This tutorial shows how to query resolution statements to find the real address and mosaic ID behind a namespace
alias used in a confirmed transaction.

## Prerequisites

Before you start, [set up your development environment](../start/setup.md).

This tutorial only reads data from the network. It does not submit any transaction, so no <account:> or <XYM:>
balance is needed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/resolve-namespace-from-receipt', ['py', 'js']) }}

## Code Explanation

### Defining the Transaction Hash

{{ tutorial.code_snippet(['py:11:14', 'js:7:10']) }}

The code defines the hash of a confirmed transaction that used namespace aliases instead of a plain address and
mosaic ID.
This hash is read from the `TRANSACTION_HASH` environment variable, which defaults to a known transaction on the
Symbol <testnet:>.

### Retrieving the Confirmed Transaction

{{ tutorial.code_snippet(['py:17:29', 'js:13:26']) }}

The confirmed transaction is fetched from the <get:/transactions/confirmed/{transactionId}> endpoint using its hash.

The response includes a `meta` object with the <block:> `height` where the transaction was confirmed.
This height is needed to query the resolution statements for that specific block.

The transaction metadata also includes the `index` field, which is the 0-based position of the transaction within
the block.
The code converts it to a 1-based `primaryId` (`index + 1`) to match against the resolution entries later.

{{ tutorial.code_snippet(['py:31:35', 'js:28:32']) }}

The retrieved transaction contains the `recipientAddress` field.
If the transaction used a namespace alias as recipient, this field holds the unresolved value (the encoded
namespace ID) instead of a real address.

The code checks whether the recipient is an alias by inspecting the lowest bit of the first byte:

* If the bit is `1`, the address is an encoded namespace alias, as described in the
    [Linking Namespaces to Addresses](./link-namespace-to-address.md#using-the-alias) tutorial.
* If the bit is `0`, the recipient is a regular address and no resolution is needed.

{{ tutorial.code_snippet(['py:37:45', 'js:34:43']) }}

The code iterates through the transaction's `mosaics` array and checks whether each mosaic ID is a namespace alias
by inspecting bit 63 (the highest bit) of its 64-bit value:

* If the bit is `1`, the value is a namespace ID used as a mosaic alias, as described in the
    [Linking Namespaces to Mosaics](./link-namespace-to-mosaic.md#using-the-alias) tutorial.
* If the bit is `0`, the value is a regular mosaic ID and no resolution is needed.

### Querying Address Resolution Statements

{{ tutorial.code_snippet(['py:48:58', 'js:46:57']) }}

Using the block height from the transaction metadata, the code queries the
<get:/statements/resolutions/address> endpoint to retrieve the address resolution statements for that block.

Each resolution statement contains:

* **Height:** The block where the resolution occurred.
* **Unresolved:** The namespace alias (encoded as an address) that was used in the transaction.
* **Resolution entries:** An array mapping the alias to the actual address at the time of confirmation.

{{ tutorial.code_snippet(['py:60:73', 'js:59:78']) }}

The endpoint returns all address resolution statements for the block.
The code skips any statement whose `unresolved` field does not match the transaction's `recipientAddress`,
since a single block can contain multiple resolution statements if different namespace aliases were used.

Each statement can also have multiple entries if the alias was re-linked to a different value between transactions
in the same block.
If consecutive transactions resolve to the same value, only a single entry is stored.

Each resolution entry contains:

* **Source:** Indicates from which transaction the resolved value applies, using a `primaryId`
    (1-based index of the transaction in the block) and `secondaryId`
    (1-based index within an <aggregate transaction:>, or `0` for standalone transactions).
* **Resolved:** The actual address the alias pointed to from the indicated source onward.

To determine the resolved value for the analyzed transaction, the code finds the last entry whose `primaryId` is
less than or equal to the transaction's `primaryId`.
This is the value the network used for that specific transaction, regardless of what the alias currently points to,
even if the alias was defined multiple times in the same block.

### Querying Mosaic Resolution Statements

{{ tutorial.code_snippet(['py:76:99', 'js:81:110']) }}

The same transaction also used `symbol.xym` as a mosaic alias instead of a raw mosaic ID.
The code queries the <get:/statements/resolutions/mosaic> endpoint with the same block height to retrieve
the mosaic resolution statements.

As with the address resolution, the code skips statements whose `unresolved` field does not match the aliased
mosaic IDs from the transaction.

The response follows the same structure as the address resolution, but the `resolved` field contains a
mosaic ID instead of an address.
The same matching logic applies: the last entry whose `primaryId` is less than or equal to the transaction's
`primaryId` gives the resolved mosaic ID.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="2 4 5 6 7 8 9 15 16 22 23"
--8<-- 'devbook/namespaces/resolve-namespace-from-receipt.log'
```

Some highlights from the output:

* **Transaction hash** (line 2): The hash of the confirmed transaction being analyzed.

* **Block height** (line 4): The block where the transaction was confirmed, used to query resolution statements.

* **Transaction index** (line 5): The 0-based position of the transaction in the block, converted to a 1-based
    `primaryId` for matching against resolution entries.

* **Recipient and alias checks** (lines 6-9): The recipient field contains the encoded namespace alias, and the alias
    flag check confirms it is a namespace alias rather than a regular address.
    The mosaic alias check confirms that `E74B99BA41F4AFEE` is also a namespace alias.

* **Resolved address** (lines 15-16): The address resolution shows that the namespace alias resolved to
    `TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI` for this transaction.

* **Resolved mosaic ID** (lines 22-23): The mosaic resolution shows that the unresolved namespace ID
    `E74B99BA41F4AFEE` resolved to mosaic ID `72C0212E67A08BCE` for this transaction.

!!! note "Resolution statements only contain numeric IDs"
    Resolution statements contain namespace IDs, not human-readable names like `symbol.xym`.
    To retrieve the readable name from an ID, use <post:/namespaces/names>.

## Conclusion

This tutorial showed how to:

| Step                                                                      | Related documentation                         |
| ------------------------------------------------------------------------- | --------------------------------------------- |
| [Retrieve a confirmed transaction](#retrieving-the-confirmed-transaction) | <get:/transactions/confirmed/{transactionId}> |
| [Query address resolutions](#querying-address-resolution-statements)      | <get:/statements/resolutions/address>         |
| [Query mosaic resolutions](#querying-mosaic-resolution-statements)        | <get:/statements/resolutions/mosaic>          |
