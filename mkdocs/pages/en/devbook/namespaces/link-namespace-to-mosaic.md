---
title: Link Namespace to Mosaic
---

# Linking and Unlinking Namespaces to Mosaics

<Namespaces:> can be linked to <mosaics:> to create human-readable aliases that can be used instead of long
hexadecimal mosaic IDs in transactions.

This tutorial shows how to link a namespace to a mosaic identifier and how to unlink it when no longer needed.

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
- Create an <account:> that owns the namespace, either [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
- [Register a root namespace](./register-root-namespace.md) to link to a mosaic.
- Have a mosaic ID to link the namespace to. You can create one or use an existing mosaic.
- Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

!!! info "Namespace and mosaic ownership required"
    Only the account that owns both the namespace and the mosaic can link them together.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/link-namespace-to-mosaic', ['py', 'js']) }}

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet(['py:17:25', 'js:14:22']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account must own both the namespace and the mosaic being linked.

### Defining the Namespace and Target Mosaic

{{ tutorial.code_snippet(['py:27:35', 'js:24:35']) }}

The code defines:

* **Namespace name:** The name of the namespace to be linked. A timestamp is appended to ensure uniqueness across runs.
* **Namespace ID:** The ID is generated from the namespace name using <dy:IdGenerator.generateNamespacePath>,
    which returns an array of IDs for each level in the hierarchy.
    The last element (`[-1]`) is selected to get the final namespace ID.

    For a root namespace like `foo`, the array contains one element.
    For a subnamespace like `symbol.xym`, it contains two elements, and the last one is the ID of `xym` under `symbol`.

* **Mosaic ID:** The hexadecimal identifier of the mosaic that the namespace will point to.
    In this example, a test mosaic ID is used, but any valid mosaic ID can be specified.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:38:56', 'js:38:57']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Transaction

{{ tutorial.code_snippet(['py:58:67', 'js:59:68']) }}

The mosaic alias transaction specifies:

* **Type:** Mosaic alias transactions use the type `mosaic_alias_transaction_v1`.

* **Signer public key:** The account that owns the namespace and mosaic, and will pay the transaction fee.

* **Namespace ID:** The identifier of the namespace being linked.

* **Mosaic ID:** The identifier of the mosaic to link to the namespace.

* **Alias action:** The value `link` creates the alias. To remove the alias later, use `unlink` instead.

!!! info "Unlinking an alias"
    To unlink a namespace from a mosaic, announce another `mosaic_alias_transaction_v1` transaction with the same
    namespace ID and mosaic ID, but set the `alias_action` field to `unlink`.

    The unlinking process does not remove the namespace or the mosaic, only the association between them.
    After unlinking, the namespace can be linked to a different mosaic or address.

### Submitting the Transaction

{{ tutorial.code_snippet(['py:69:88', 'js:70:90']) }}

The transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:90:108', 'js:92:126']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Verifying the Alias

{{ tutorial.code_snippet(['py:110:122', 'js:128:141']) }}

To verify the alias was created, the code retrieves the namespace information from the network
using the <get:/namespaces/{namespaceId}> endpoint.

The response includes the alias type (`mosaic`) and the aliased mosaic ID, confirming the namespace now points to the
specified mosaic.

### Using the Alias

{{ tutorial.code_snippet(['py:124:144', 'js:143:162']) }}

Once the namespace is linked to a mosaic, the namespace can be used in place of the mosaic ID in transactions.
The code demonstrates creating a <transfer transaction:> using the namespace in the mosaics array instead of
the full hexadecimal mosaic ID.

To use a namespace as a mosaic ID, the namespace name is converted to its mosaic alias ID using
<dy:IdGenerator.generateMosaicAliasId>.

For more details on how to announce transfer transactions, see the
[Transfer Transaction](../transactions/transfer.md) tutorial.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="3 5 23 32 33 36"
--8<-- 'devbook/namespaces/link-namespace-to-mosaic.log'
```

Some highlights from the output:

* **Namespace and target** (lines 3, 5): The namespace `nsmos_1769457459` is being linked to the target mosaic ID.

* **Transaction hash** (line 23): The transaction hash can be used to search for the transaction in the
    [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

* **Alias verification** (lines 32-33): The namespace information confirms the alias type is `1` (mosaic) and
    shows the linked mosaic ID.

* **Using the alias** (lines 36): A transfer transaction is created using the mosaic alias in the mosaics array,
    demonstrating that the alias can be used in place of the full mosaic ID.

    !!! note "Different mosaic ID"
        The mosaic ID used in the transfer differs from the original mosaic ID because it is the
        [encoded namespace ID](#using-the-alias), not the mosaic ID itself.
        The network resolves the alias to the linked mosaic when processing the transaction.

## Conclusion

This tutorial showed how to:

| Step                                                              | Related documentation                         |
| ----------------------------------------------------------------- | --------------------------------------------- |
| [Generate namespace ID](#defining-the-namespace-and-target-mosaic)| <dy:IdGenerator.generateNamespacePath>        |
| [Build a mosaic alias transaction](#building-the-transaction)     | <dy:SymbolTransactionFactory.create>          |
| [Verify the alias](#verifying-the-alias)                          | <get:/namespaces/{namespaceId}>               |
| [Use the alias in a transfer](#using-the-alias)                   | <dy:IdGenerator.generateMosaicAliasId>        |
