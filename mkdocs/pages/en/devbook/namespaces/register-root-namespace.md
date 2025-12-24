---
title: Register Root Namespace
---

# Registering a Root Namespace

<Namespaces:> provide human-readable aliases for <mosaics:> and <accounts:>.

This tutorial shows how to register a root namespace and set its
<lease duration:../../textbook/namespaces.md#duration>.

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

{{ tutorial.code_snippet(['py:16:24', 'js:13:21']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account will own the registered namespace.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:27:45', 'js:24:43']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](./transfer.md) tutorial.

### Building the Transaction

{{ tutorial.code_snippet(['py:47:59', 'js:45:57']) }}

The namespace registration transaction specifies:

* **Type:** Use `namespace_registration_transaction_v1`.

* **Registration type:** The value `root` indicates a root namespace is being created.

* **Duration:** The number of blocks for which the namespace will be leased.
    The minimum duration is 86,400 blocks (approximately 30 days on mainnet), and the maximum is
    15,552,000 blocks (approximately 5 years).

* **Name:** The name of the root namespace.
    Names can only contain lowercase letters, numbers, hyphens, and underscores, must start with a letter or number,
    and can be at most 64 characters long.

    To ensure the namespace name is unique across multiple runs, the example appends a timestamp to the name.

!!! note "Namespace lease fees"

    Registering a root namespace requires a lease fee in addition to the transaction fee.
    The lease fee is proportional to the requested duration.
    For details, see [Lease Fee](../../textbook/namespaces.md#lease-fee).

### Submitting the Transaction

{{ tutorial.code_snippet(['py:61:80', 'js:59:79']) }}

The transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:82:100', 'js:81:115']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Retrieving the Namespace

{{ tutorial.code_snippet(['py:102:117', 'js:117:134']) }}

To verify the namespace was registered, the code retrieves it from the network
using the <get:/namespaces/{namespaceId}> endpoint and displays its properties.

The namespace ID is computed using <dy:IdGenerator.generateNamespaceId>.
This function applies a deterministic hashing algorithm to the namespace name,
producing the ID needed to query the namespace information.

The response includes:

* **Registration type:** Shows `0` for a namespace.
* **Owner address:** The account that registered and owns the namespace.
* **Start height:** The block height when the namespace was registered.
* **End height:** The block height when the namespace will expire.

For root namespaces, you can calculate the remaining duration by subtracting the current block height from the end
height.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/namespaces/register-root-namespace.log'
```

The transaction hash printed in the output can be used to search for the transaction
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                          | Related documentation                              |
| ------------------------------------------------------------- | -------------------------------------------------- |
| [Generate namespace ID](#building-the-transaction)            | <dy:IdGenerator.generateNamespaceId>               |
| [Build a namespace registration](#building-the-transaction)   | <dy:SymbolTransactionFactory.create>               |
| [Retrieve the namespace](#retrieving-the-namespace)           | <get:/namespaces/{namespaceId}>                    |

## Next Steps

Now that you have a root namespace, you can:

- Link your namespace to a mosaic or account to create an alias
- [Register a subnamespace](./register-subnamespace.md) to create a hierarchical structure
- [Extend the namespace](./extend-root-namespace.md) before it expires to keep it active
