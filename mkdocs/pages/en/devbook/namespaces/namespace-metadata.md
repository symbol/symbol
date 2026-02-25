---
title: Add Namespace Metadata
---

# Adding Metadata to a Namespace

<Namespaces:|Namespaces>, like <accounts:> and <mosaics:>, can store <metadata:> as key-value pairs.

This tutorial shows how to add metadata to a namespace, retrieve it from the network, and update existing values.

In this example, the pair `description = My first namespace` is attached to a namespace and then changed to
`Updated namespace`:

```dot
digraph {
    layout="neato";
    Namespace [label="Namespace\ntestnamespace" tooltip="Namespace" pos="0,0!"];
    Metadata [
        style=filled
        class=metadata
        label=<<table border="0"><tr><td><b>Key</b></td><td><b>Value</b></td></tr><tr><td>description</td><td>My first namespace</td></tr></table>>
        tooltip="Metadata entry"
        shape=note
        pos="2.5,0.5!"];
}
```

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:> to own the namespace, either
    [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
* [Register a namespace](./register-root-namespace.md) owned by the signer account.
* Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed, and the
[Complete Aggregate transaction](../transactions/complete-aggregate.md) tutorial to understand how
<aggregate transactions:> work.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/namespace-metadata', ['py', 'js']) }}

## Code Explanation

This tutorial demonstrates adding new metadata to a namespace and then updating that metadata.

### Setting Up the Account and Namespace

{{ tutorial.code_snippet(['py:54:68', 'js:52:67']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.

The namespace name is read from the `NAMESPACE_NAME` environment variable,
which defaults to `testnamespace` if not set.
The namespace ID is computed from the name using <dy:IdGenerator.generateNamespaceId>.

!!! note "Namespace must exist"

    The namespace must already be registered and owned by the signer account,
    or the transaction adding the metadata will be rejected.

    See [Registering a Root Namespace](./register-root-namespace.md) to learn how to do it.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:71:89', 'js:70:88']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Defining the Metadata

Each namespace metadata entry is uniquely identified by:

* The **signer's address**: the account adding the metadata
* The **target account's address**: the namespace owner, whose signature is required
* The **target namespace ID**
* A **scoped metadata key**: a 64-bit value chosen by the metadata creator

    The SDK provides a <dy:Metadata.metadataGenerateKey> helper function that generates this key from a
    human-readable string using SHA3-256 hashing.
    This approach makes keys more meaningful and reduces the chance of collisions.

{{ tutorial.code_snippet(['py:94:97', 'js:93:96']) }}

In this example, the key is derived from the string `description`.
For demonstration purposes, a timestamp is appended to the key string,
so each time the code is executed a new entry is added to the namespace.
In practice, you would use a fixed key that identifies the specific metadata entry you want to create or update.

The metadata value can be any sequence of up to 1024 bytes.
In this example, the value is the string `My first namespace` encoded in UTF-8.

!!! note "Multiple entries with the same key"

    The key is only one of the 4 parts that identify a metadata entry,
    so a change in any part produces a different entry.

    For example, different accounts can use the same scoped metadata key on the same namespace without conflict,
    because the signer's address is different.

    Each entry is independent and can only be updated by the account that originally created it.

### Creating the Embedded Namespace Metadata Transaction

{{ tutorial.code_snippet(['py:99:112', 'js:98:112']) }}

A namespace metadata transaction attaches a key-value pair to a namespace on the blockchain.
The same transaction type handles both adding new metadata entries and updating existing ones.

Symbol requires these transactions to be inside an <aggregate transaction:> that includes both
the signer account and the namespace owner's signature.
This prevents unwanted metadata from being attached to a namespace without its owner's permission.

In this tutorial, the signer is also the namespace owner so only one signature is needed.
However, the transaction still needs to be inside an aggregate,
so the code defines the namespace metadata transaction as an <embedded transaction:> with these properties:

* **Type:** Use `namespace_metadata_transaction_v1`.

* **Signer public key:** The account creating the metadata entry.

* **Target address:** The namespace owner's address.
    When the signer differs from the namespace owner, the owner must <cosignature:|cosign> the aggregate transaction.

* **Target namespace ID:** The namespace to attach the metadata to.

* **Scoped metadata key:** The 64-bit key used to identify this metadata entry.

* **Value size delta:** When creating new metadata, set this to the byte length of the value.
    When updating existing metadata, set this to the difference between the new and current value lengths.

* **Value:** The metadata content as bytes.
    When creating new metadata, provide the raw value.
    When updating, provide a computed value (explained in the
    [Modifying Existing Metadata](#modifying-existing-metadata) section).

### Building the Aggregate Transaction

{{ tutorial.code_snippet(['py:114:124', 'js:114:124']) }}

The code adds the embedded namespace metadata transaction to an <aggregate transaction:>.

Since the signer is the namespace owner, no <cosignatures:> are required and the aggregate can be created as
<complete aggregate transaction:|complete>, allowing it to be signed and announced immediately.

!!! note "Adding metadata by a different account"

    If the signer is different from the namespace owner, the owner must cosign the aggregate transaction to approve the
    metadata entry.

    For details on collecting cosignatures on-chain, see the [Bonded Aggregate](../transactions/bonded-aggregate.md)
    tutorial.

### Submitting the Aggregate Transaction

{{ tutorial.code_snippet(['py:126:135', 'js:126:137']) }}

The aggregate transaction is signed and announced following the same process as in
[Creating a Complete Aggregate Transaction](../transactions/complete-aggregate.md#building-the-aggregate-transaction).

### Retrieving Metadata

{{ tutorial.code_snippet(['py:140:158', 'js:142:163']) }}

To retrieve the current value of a metadata entry, the code uses the <get:/metadata> endpoint
with filters for `sourceAddress`, `targetAddress`, `scopedMetadataKey`, `targetId` (the namespace ID), and
`metadataType` (`2` for namespace metadata).

The endpoint returns the list of entries matching the filters, which in this case contains a single item.

### Modifying Existing Metadata

{{ tutorial.code_snippet(['py:160:177', 'js:165:183']) }}

Updating an existing metadata entry requires the current value, retrieved from the network as previously shown.

To demonstrate updating metadata, the code changes the description from `My first namespace` to `Updated namespace`
by creating another `namespace_metadata_transaction_v1` transaction with the same scoped metadata key.

Modifying an existing metadata value differs from creating a new one in that the updated value must be defined
in terms of the current value, using the following fields:

* `value_size_delta`: The difference in length between the new and current values.
    In this example, the delta is `-1` because `Updated namespace` (17 bytes) is one byte shorter than
    `My first namespace` (18 bytes).

* `value`: The XOR'd bytes computed by comparing the current and new values byte-by-byte.

    The SDK provides a <dy:Metadata.metadataUpdateValue> helper function that handles the XOR calculation.
    The XOR operation compares each byte: matching bytes become zero, and differing bytes capture the change.

Note that `value_size_delta` represents the difference in final value lengths (new vs current),
not the length of the XOR'd bytes themselves.

!!! tip "Deleting a metadata entry"

    To delete a metadata entry, set `value_size_delta` to the negative of the current value length and provide the
    current value as `value`. The XOR produces an empty result, which removes the entry from the network.

As with the [initial metadata creation](#building-the-aggregate-transaction), this metadata modification is wrapped
in an aggregate transaction and then signed and announced.

{{ tutorial.code_snippet(['py:179:201', 'js:185:210']) }}

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="3 4 18 19 20 23 37 47 48 50"
--8<-- 'devbook/namespaces/namespace-metadata.log'
```

Key points in the output:

* **Line 3** (`Namespace name`): The namespace receiving the metadata.
* **Line 4** (`Namespace ID`): The computed namespace ID in decimal and hexadecimal formats.
* **Line 18** (`"scoped_metadata_key"`): The 64-bit key generated from the input string using SHA3-256 hashing.
* **Line 19** (`"target_namespace_id"`): The namespace receiving the metadata.
* **Line 20** (`"value_size_delta": 18`): When creating new metadata, this equals the byte length of the value
    (`"My first namespace"` = 18 bytes).
* **Line 23**: The transaction hash for looking up the metadata creation in the explorer.
* **Line 37** (`Current value: My first namespace`): Retrieved from the network before updating.
* **Line 47** (`"value_size_delta": -1`): Negative because the new value (`"Updated namespace"` = 17 bytes) is shorter
    than the current value (18 bytes).
* **Line 48** (`"value"`): The XOR'd value computed from the current and new values, not the raw new value.
* **Line 50**: The transaction hash for looking up the metadata update in the explorer.

The transaction hashes can be used to search for the transactions in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                                             | Related documentation                        |
|--------------------------------------------------------------------------------------------------|----------------------------------------------|
| [Define metadata key and value](#defining-the-metadata)                                          | <dy:Metadata.metadataGenerateKey>            |
| [Create a namespace metadata transaction](#creating-the-embedded-namespace-metadata-transaction) | <dy:SymbolTransactionFactory.createEmbedded> |
| [Retrieve metadata](#retrieving-metadata)                                                        | <get:/metadata>                              |
| [Modify existing metadata](#modifying-existing-metadata)                                         | <dy:Metadata.metadataUpdateValue>            |
