---
title: Link Namespace to Address
tutorial_level: intermediate
---

# Linking and Unlinking Namespaces to Addresses

<Namespaces:> can be linked to <addresses:> to create human-readable aliases that can be used instead of long
hexadecimal addresses in transactions.

This tutorial shows how to link a namespace to an account address and how to unlink it when no longer needed.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:> that owns the namespace, either [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
* [Register a root namespace](./register-root-namespace.md) to link to an address.
* Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

!!! info "Namespace ownership required"
    Only the account that owns the namespace can link it to an address.
    The target address does not need to cosign or approve the link.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/namespaces/link_namespace_to_address', ['py', 'js']) }}

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet_tagged('step-1') }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account must own the namespace being linked.

### Defining the Namespace and Target Address

{{ tutorial.code_snippet_tagged('step-2') }}

The code defines:

* **Namespace name:** The namespace to link, read from the `NAMESPACE_NAME` environment variable,
    which defaults to `my_namespace` if not set.
    This name must match a namespace that your account already owns.
* **Namespace ID:** The ID is generated from the namespace name using <dy:IdGenerator.generateNamespacePath>,
    which returns an array of IDs for each level in the hierarchy.
    The last element is selected to get the final namespace ID.
    Taking the last element works for both root namespaces and subnamespaces.

    For a root namespace like `foo`, the array contains one element.
    For a subnamespace like `symbol.xym`, it contains two elements, and the last one is the ID of `xym` under `symbol`.

    !!! info "Subnamespace IDs are unique"
        Subnamespace IDs are derived hierarchically, so two subnamespaces with the same leaf name but different parents
        produce different IDs.
        For example, the last element of the path for `foo.xym` and `bar.xym` will be different.

* **Target address:** The address that the namespace will point to, read from the `TARGET_ADDRESS`
    environment variable. If not set, a default test address is used.

### Fetching Network Time and Fees

{{ tutorial.code_snippet_tagged('step-3') }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Transaction

{{ tutorial.code_snippet_tagged('step-4') }}

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
    address.
    After unlinking, the namespace can be linked to a different address or mosaic.

### Submitting the Transaction

{{ tutorial.code_snippet_tagged('step-5') }}

The transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet_tagged('step-6') }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Verifying the Alias

{{ tutorial.code_snippet_tagged('step-7') }}

To verify the alias was created, the code retrieves the namespace information from the network
using the <get:/namespaces/{namespaceId}> endpoint.

The response includes the alias type (`address`) and the linked address, confirming the namespace now points to the
specified address.

### Using the Alias

{{ tutorial.code_snippet_tagged('step-8') }}

Once the namespace is linked to an address, the namespace can be used in place of the address in transactions.
The code demonstrates creating a <transfer transaction:> using the alias as the recipient address instead of
the full hexadecimal address.

For simplicity, this example creates the transaction but does not announce it or wait for its confirmation.

To use a namespace as a recipient address, the namespace ID is converted into a 24-byte address using
<dy:Address.fromNamespaceId>.
As described in the [previous section](#defining-the-namespace-and-target-address), the last component of the
namespace path is used as the namespace ID.

For more details on how to announce transfer transactions, see the
[Transfer Transaction](../transactions/transfer.md) tutorial.

!!! note "Address Resolution Receipt"
    When the network processes a transaction that uses a namespace alias as a recipient address, it generates an
    **Address Resolution Receipt**.
    This receipt records the actual address the alias pointed to at the time the transaction was confirmed.

    This is important for historical auditability: since aliases can be changed or removed at any time, the receipt
    ensures that the resolved address can always be verified, even if the alias has since been updated.

    Resolution receipts can be queried using the <get:/statements/resolutions/address> endpoint.
    For more details on receipts, see the [Resolution Statements](../../textbook/blocks.md#resolution-statements)
    section in the Textbook.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="3 5 23 32 33 36"
--8<-- 'devbook/namespaces/link_namespace_to_address.log'
```

Some highlights from the output:

* **Namespace and target** (lines 3, 5): The namespace `nsaddr_1770541301` is being linked to the target address
    `TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI`.

* **Transaction hash** (line 23): The transaction hash can be used to search for the transaction in the
    [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

* **Alias verification** (lines 32-33): The namespace information confirms the alias type is `2` (address) and
    shows the linked address.

* **Using the alias** (line 36): A transfer transaction is created using the alias as the recipient,
    demonstrating that it can be used in place of the full address.

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
| [Use the alias in a transfer](#using-the-alias)                     | <dy:Address.fromNamespaceId>                  |
| [Query address resolution receipts](#using-the-alias)               | <get:/statements/resolutions/address>         |
