---
title: Register Root Namespace
---

# Registering a Root Namespace

<Namespaces:> provide human-readable aliases for <accounts:> and <mosaics:>,
which can be used instead of long addresses and hexadecimal mosaic IDs.

This tutorial shows how to register a <root namespace:> and set its lease [duration](../../textbook/namespaces.md#duration).

Once registered, additional steps are required to link the namespace to a mosaic or account,
as explained in [Next Steps](#next-steps).

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
- Create an <account:> to register the namespace, either
  [from code](../accounts/create-from-private-key.md) or
  [by using a wallet](../../userbook/wallet/create-account.md).
- Obtain <XYM:> to pay for the transaction and lease fees.
  See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/register-root-namespace', ['py', 'js']) }}

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet(['py:17:25', 'js:13:21']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account will own the registered namespace.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:28:46', 'js:24:43']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Transaction

{{ tutorial.code_snippet(['py:48:60', 'js:45:57']) }}

The namespace registration transaction specifies:

* **Type:** Namespace registration transactions use the type `namespace_registration_transaction_v1`.

* **Registration type:** The value `root` indicates a root namespace is being created.
    Use `child` to [register a subnamespace](./register-subnamespace.md) instead.

* **Duration:** The number of blocks for which the namespace will be leased.
    The minimum duration is 86,400 blocks (approximately 30 days), and the maximum is
    5,256,000 blocks (approximately 5 years).

* **Name:** The name of the root namespace.
    Names can only contain lowercase letters, numbers, hyphens, and underscores, must start with a letter or number,
    and can be at most 64 characters long.

    To ensure the namespace name is unique across multiple runs of the tutorial, a timestamp is added to the name.
    In practice, programs would use a fixed name for their namespaces.

!!! note "Namespace lease fees"

    In addition to the standard [transaction fee](#fetching-network-time-and-fees),
    registering a namespace requires a [lease fee](../../textbook/namespaces.md#lease-fee) proportional to
    the requested duration.

    Unlike the transaction fee, the lease fee is **not** included in the transaction request.
    It is calculated and deducted automatically by the network from the **transaction signer’s account**
    when the registration transaction is confirmed.

    The amount of the lease fee can be calculated beforehand using the <get:/network/fees/rental> endpoint.

### Submitting the Transaction

{{ tutorial.code_snippet(['py:62:81', 'js:59:79']) }}

The transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:83:101', 'js:81:115']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Retrieving the Namespace

{{ tutorial.code_snippet(['py:103:120', 'js:117:138']) }}

To verify the namespace was registered, the code retrieves it from the network
using the <get:/namespaces/{namespaceId}> endpoint and displays its properties.

The namespace ID is computed using <dy:IdGenerator.generateNamespaceId>.
This function applies a deterministic hashing algorithm to the namespace name,
producing the ID needed to query the namespace information.

A successful response confirms the namespace was registered and is active on the network.

!!! info "Namespace registered but not linked yet"

    A namespace becomes useful when it serves as an alias for a <mosaic:> or an <account:>.
    Link the namespace to an identifier using the guides in [Next Steps](#next-steps).

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="7 15 18 20 30 33 34 35 36"
--8<-- 'devbook/namespaces/register-root-namespace.log'
```

Some highlights from the output:

* **Namespace name** (line 7): The chosen name `ns_1766533079` includes a timestamp to ensure uniqueness.
    Search for this name in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/) to view the namespace details.

* **Fee** (line 15): The transaction fee of 0.0159 XYM is calculated as the transaction size
    multiplied by the fee multiplier. The [lease fee](../../textbook/namespaces.md#lease-fee) is deducted separately
    by the network when the transaction is confirmed.

* **ID and name** (lines 18, 20): The `id` field shows the namespace ID as a decimal number, while `name` contains
    the namespace name encoded as a hexadecimal string. For example, `6e735f...` decodes to `ns_1...`.

* **Namespace ID** (line 30): Shows both decimal and hexadecimal representations to match the `id` field on line 18.

* **Registration type** (line 33): The value `0` indicates a root namespace (versus `1` for subnamespaces).

* **Owner address** (line 34): The account that registered and owns the namespace.

* **Start and end heights** (lines 35-36): The namespace is active from block `2984442` to block `3073722`.
    The end height includes a [grace period](../../textbook/namespaces.md#duration) (1 day on <testnet:>,
    30 days on <mainnet:>) beyond the requested duration, giving owners time to renew before the namespace
    becomes available to others.

The transaction hash printed in the output can also be used to search for the transaction
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                          | Related documentation                              |
| ------------------------------------------------------------- | -------------------------------------------------- |
| [Generate namespace ID](#building-the-transaction)            | <dy:IdGenerator.generateNamespaceId>               |
| [Build a namespace registration transaction](#building-the-transaction)   | <dy:SymbolTransactionFactory.create>   |
| [Retrieve the namespace](#retrieving-the-namespace)           | <get:/namespaces/{namespaceId}>                    |

## Next Steps

Now that you have a root namespace, you can:

- [Link your namespace to a mosaic](./link-namespace-to-mosaic.md) or [to an account](./link-namespace-to-address.md) to create an alias
- [Register a subnamespace](./register-subnamespace.md) to create a hierarchical structure
- [Extend the namespace](./extend-root-namespace.md) before it expires to keep it active
