---
title: Link Namespace to Address
---

# Linking and Unlinking Namespaces to Addresses

<Namespaces:> can be linked to <addresses:> to create human-readable aliases that can be used instead of long
hexadecimal addresses in transactions.

This tutorial shows how to link a namespace to an account address and how to unlink it when no longer needed.

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
- Create an <account:> that owns the namespace, either [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
- [Register a root namespace](./register-root-namespace.md) to link to an address.
- Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

!!! info "Namespace ownership required"
    Only the account that owns the namespace can link it to an address. The target address does not need to
    cosign or approve the link.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/link-namespace-to-address', ['py', 'js']) }}

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet(['py:18:26', 'js:14:22']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account must own the namespace being linked.

### Defining the Namespace and Target Address

{{ tutorial.code_snippet(['py:28:38', 'js:24:36']) }}

The code defines:

* **Namespace name:** The name of the namespace to be linked. A timestamp is appended to ensure uniqueness across runs.
* **Namespace ID:** The ID is generated from the namespace name using <dy:IdGenerator.generateNamespaceId>.
* **Target address:** The address that the namespace will point to.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:41:59', 'js:39:58']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Transaction

{{ tutorial.code_snippet(['py:61:70', 'js:60:69']) }}

The address alias transaction specifies:

* **Type:** Address alias transactions use the type `address_alias_transaction_v1`.

* **Signer public key:** The account that owns the namespace and will pay the transaction fee.

* **Namespace ID:** The identifier of the namespace being linked.

* **Address:** The target address to link to the namespace.

* **Alias action:** The value `link` creates the alias. To remove the alias later, use `unlink` instead.

!!! info "Unlinking an alias"
    To unlink a namespace from an address, announce another `address_alias_transaction_v1` transaction with the same
    namespace ID and address, but set the `alias_action` field to `unlink`.

    The unlinking process does not remove the namespace itself, only the association between the namespace and the
    address. After unlinking, the namespace can be linked to a different address or mosaic.

### Submitting the Transaction

{{ tutorial.code_snippet(['py:72:91', 'js:71:91']) }}

The transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:93:111', 'js:93:127']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Verifying the Alias

{{ tutorial.code_snippet(['py:113:126', 'js:129:145']) }}

To verify the alias was created, the code retrieves the namespace information from the network
using the <get:/namespaces/{namespaceId}> endpoint.

The response includes the alias type (`address`) and the aliased address, confirming the namespace now points to the
specified address.

### Using the Alias

{{ tutorial.code_snippet(['py:128:152', 'js:147:179']) }}

Once the namespace is linked to an address, the namespace can be used in place of the address in transactions.
The code demonstrates creating a <transfer transaction:> using the namespace as the recipient address instead of
the full hexadecimal address.

To use a namespace as a recipient address, the namespace ID must be encoded into a 24-byte address. The namespace ID
is obtained using `generate_namespace_path()[-1]`, which correctly handles both root namespaces and subnamespaces by
computing the hierarchical namespace ID.

The namespace ID is then encoded into a 24-byte address with the following structure:

* **Byte 0:** Network byte with alias flag set (`network_identifier | 0x01`)
* **Bytes 1-8:** Namespace ID in little-endian byte order
* **Bytes 9-23:** Zero padding

For more details on how to announce transfer transactions, see the
[Transfer Transaction](../transactions/transfer.md) tutorial.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="3 5 23 32 33 36"
--8<-- 'devbook/namespaces/link-namespace-to-address.log'
```

Some highlights from the output:

* **Namespace and target** (lines 3, 5): The namespace `nsaddr_1769457266` is being linked to the target address
    `TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI`.

* **Transaction hash** (line 23): The transaction hash can be used to search for the transaction in the
    [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

* **Alias verification** (lines 32-33): The namespace information confirms the alias type is `2` (address) and
    shows the linked address.

* **Using the alias** (line 36): A transfer transaction is created using the namespace alias as the recipient,
    demonstrating that the alias can be used in place of the full address.

    !!! note "Different recipient address"
        The recipient address differs from the target address because it is the
        [encoded namespace ID](#using-the-alias), not the target address itself.
        The network resolves the alias to the linked address when processing the transaction.

## Conclusion

This tutorial showed how to:

| Step                                                                | Related documentation                         |
| ------------------------------------------------------------------- | --------------------------------------------- |
| [Generate namespace ID](#defining-the-namespace-and-target-address) | <dy:IdGenerator.generateNamespacePath>        |
| [Build an address alias transaction](#building-the-transaction)     | <dy:SymbolTransactionFactory.create>          |
| [Verify the alias](#verifying-the-alias)                            | <get:/namespaces/{namespaceId}>               |
