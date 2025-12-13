---
title: Account Metadata
---

# Adding Metadata to an Account

<Accounts:> can store <metadata:> as key-value pairs.

This tutorial shows how to add metadata to an account, retrieve metadata from the network, and update existing values.

In this example, an account adds a username to itself and then modifies it.

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
- Create an <account:> to add metadata to, either
  [from code](./create-from-private-key.md) or
  [by using a wallet](../../userbook/wallet/create-account.md).
- Obtain <XYM:> to pay for the transaction fee.
  See [Getting Testnet Funds from the Faucet](./testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed, and the
[Complete Aggregate transaction](../transactions/complete-aggregate.md) tutorial to understand how aggregate
transactions work.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/transactions/account-metadata', ['py', 'js']) }}

## Code Explanation

This tutorial demonstrates adding new metadata to an account and then updating that metadata.

### Setting Up the Account

{{ tutorial.code_snippet(['py:53:61', 'js:51:59']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.

In this tutorial, the signer adds metadata to their own account.
Adding metadata to a different account requires the target to cosign the transaction.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:64:82', 'js:62:80']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively, following the process described in the [Transfer Transaction](./transfer.md) tutorial.

### Defining the Metadata

{{ tutorial.code_snippet(['py:84:87', 'js:82:85']) }}

Each metadata entry is identified by a **scoped metadata key**, a 64-bit value chosen by the metadata creator.
The combination of the signer's address, the target account's address, and the scoped metadata key uniquely
identifies the metadata entry.

!!! note "Multiple entries with the same key"

    Because the signer's address is part of the unique identifier, multiple accounts can add metadata with the same
    scoped key to a target (e.g., Account A and Account B can add a different `username` to Account C).
    Each entry remains separate and can only be updated by its signer.

The SDK provides a <dy:metadataGenerateKey> helper function that generates a key from a
human-readable string using SHA3-256 hashing.
This approach makes keys more meaningful and reduces the chance of collisions.

In this example, the key is derived from the string `username`.
To ensure the metadata key is unique across multiple runs of the tutorial, a timestamp is appended to the key string.
In practice, you would use a stable key that identifies the specific metadata entry you want to create or update.

The metadata value can be any byte sequence.
In this example, the username `alice` is UTF-8 encoded.

### Creating the Embedded Metadata Transaction

{{ tutorial.code_snippet(['py:89:101', 'js:87:100']) }}

Symbol requires metadata transactions to be inside an <aggregate transaction:>.

For this reason, the code defines the metadata update as an <embedded transaction:>.

The account metadata transaction specifies:

* **Type:** Use `account_metadata_transaction_v1`.

* **Signer public key:** The account creating the metadata entry.

* **Target address:** The account to attach the metadata to.
  When the target differs from the signer, the target account must cosign the aggregate transaction.

* **Scoped metadata key:** The 64-bit identifier for this metadata entry.

* **Value size delta:** When creating **new** metadata, set this to the byte length of the value.
  When **updating** existing metadata, set this to the difference between the new and current value lengths.

* **Value:** The metadata content as bytes.
  When creating new metadata, provide the raw value.
  When updating, provide a computed value (explained in the
  [Modifying Existing Metadata](#modifying-existing-metadata) section).

### Building and Announcing the Aggregate Transaction

{{ tutorial.code_snippet(['py:103:115', 'js:102:114']) }}

The code adds the embedded transaction to a complete <aggregate transaction:>.

Since the signer is modifying their own account, the transaction does not need cosignatures
and the aggregate can be signed and announced immediately.

!!! note "Adding metadata to a different account"

    If the target account is different from the signer, the target must cosign the aggregate transaction to approve the
    metadata entry.
    This requirement prevents third parties from attaching unwanted metadata to an account without permission.

    For details on collecting <cosignatures:>, see the [Bonded Aggregate](../transactions/bonded-aggregate.md)
    tutorial.

{{ tutorial.code_snippet(['py:117:125', 'js:116:125']) }}

The transaction is then signed and announced following the same process as in
[Creating a Complete Aggregate Transaction](../transactions/complete-aggregate.md#building-the-aggregate-transaction).

### Retrieving Metadata

{{ tutorial.code_snippet(['py:130:151', 'js:130:148']) }}

Updating an existing metadata entry requires the current value from the network.

The code queries the <get:/metadata> endpoint allows searching for metadata entries by filtering on
`sourceAddress`, `targetAddress`, `scopedMetadataKey`, and `metadataType` (`0` for account metadata).

It then searches the response for the entry that matches the scoped metadata key.

### Modifying Existing Metadata

{{ tutorial.code_snippet(['py:153:167', 'js:150:165']) }}

Symbol requires updates to be computed via bitwise XOR between current and the new value.

The XOR operation compares each byte: matching bytes become zero, and differing bytes capture the change.

The SDK provides a <dy:metadataUpdateValue> helper function for this calculation.

The transaction uses the same scoped metadata key and target address as the original entry.
The `value_size_delta` field reflects the change in size:

* **Positive** if the new value is longer than the current value.
* **Negative** if the new value is shorter than the current value.
* **Zero** if both values have the same length.

{{ tutorial.code_snippet(['py:183:192', 'js:182:192']) }}

The transaction is then signed and announced as in
[Creating a Complete Aggregate Transaction](../transactions/complete-aggregate.md#building-the-aggregate-transaction).

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="14 15 16 27 52 62 71 72"
--8<-- 'devbook/transactions/account-metadata.log'
```

Key points in the output:

* **Line 14** (`"scoped_metadata_key"`): The 64-bit key generated from the input string using SHA3-256 hashing.
* **Line 15** (`"value_size_delta": 5`): When creating new metadata, this equals the byte length of the value
  (`"alice"` = 5 bytes).
* **Line 16** (`"value": "616c696365"`): The metadata value encoded as hexadecimal (`"alice"` in UTF-8).
* **Line 27** (`"transactions_hash"`): The hash of the embedded transactions for the metadata creation.
* **Line 52** (`Current value: alice`): Retrieved from the network before updating.
* **Line 62** (`"transactions_hash"`): The hash of the embedded transactions for the metadata update.
* **Line 71** (`"value_size_delta": -2`): Negative because the new value (`"bob"` = 3 bytes) is shorter than the
  current value (5 bytes). The difference is -2.
* **Line 72** (`"value": "03030b6365"`): The XOR'd value computed from the current and new values, not the raw new
  value.

The transaction hashes printed in the confirmation messages can be used to search for the transactions
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                                  | Related documentation                              |
| ------------------------------------------------------------------------------------- | -------------------------------------------------- |
| [Define metadata key and value](#defining-the-metadata)                               | <dy:metadataGenerateKey>                           |
| [Create an account metadata transaction](#creating-the-embedded-metadata-transaction) | <dy:SymbolTransactionFactory.createEmbedded>       |
| [Retrieve metadata](#retrieving-metadata)                                             | <get:/metadata>                                    |
| [Modify existing metadata](#modifying-existing-metadata)                              | <dy:metadataUpdateValue>                           |

