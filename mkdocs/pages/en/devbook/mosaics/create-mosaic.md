---
title: Create Mosaic
---

# Creating a Mosaic

<Mosaics:|Mosaics> represent assets on the Symbol blockchain, such as currencies, collectibles, or access rights.
Unlike tokens on other platforms, Symbol mosaics are supported directly at the protocol level
and require no additional coding to use.
Their properties are configurable to support various use cases, from simple currencies to restricted tokens.

This tutorial shows how to create a mosaic and mint its initial supply.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:> to create the mosaic, either
    [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
* Obtain <XYM:> to pay for the transaction and lease fees.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/mosaics/create-mosaic', ['py', 'js']) }}

## Code Explanation

Creating a mosaic requires announcing two transactions:

1. A **mosaic definition** transaction to register the mosaic and its properties.
2. A **mosaic supply change** transaction to mint the initial units.

### Setting Up the Account

{{ tutorial.code_snippet(['py:50:59', 'js:50:58']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account will own the created mosaic.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:62:80', 'js:61:79']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Mosaic Definition Transaction

{{ tutorial.code_snippet(['py:85:100', 'js:84:100']) }}

The mosaic definition transaction registers a new mosaic on the network with the following properties:

* **Type:** Mosaic definition transactions use the type `mosaic_definition_transaction_v1`.

* **Duration:** The number of blocks the mosaic will remain active. A value of `0` means the mosaic never expires.
    If a duration is provided, the maximum allowed value is
    approximately [10 years](../../textbook/mosaics.md#duration) (3,650 days or approximately 10,512,000 blocks
    with the default 30-second block target).

    !!! warning "Expiring mosaics become unusable"

        When an expiring mosaic reaches its duration, it can no longer be transferred or used in transactions.
        Balances remain in accounts but are effectively frozen.
        Before setting a duration, consider whether your use case truly requires it.

* **Divisibility:** The number of decimal places the mosaic supports.
    For example, a value of `2` means each whole unit can be divided into 100 (10^2^) atomic units.
    For more details, see [Divisibility](../../textbook/mosaics.md#divisibility) in the Textbook.

* **Nonce:** An arbitrary 32-bit unsigned integer (0 to 4,294,967,295) that acts as a locally unique identifier
    for mosaics created by the same account.
    The <mosaic ID:> is derived deterministically from the owner's address and the <nonce:> using
    <dy:IdGenerator.generateMosaicId>, so each unique nonce produces a different mosaic.

    !!! note "Nonce choice in this tutorial"

        This tutorial uses the current timestamp as the nonce to ensure each run creates a unique mosaic.
        The `& 0xFFFFFFFF` bitmask truncates the value to fit in 32 bits.
        In practice, any value works as long as the same account has not already used that nonce.

* **Flags:** A space-separated set of behavior restrictions for the mosaic.
    Multiple flags can be combined in a single string, for example `'transferable restrictable'`.

    The available flags are:

    * `transferable`: the mosaic can be freely sent between any accounts.
    * `supply_mutable`:  the total supply can be changed after creation.
    * `restrictable`:  the owner can apply
        [mosaic restrictions](../../textbook/restrictions.md#mosaic-based-restrictions) to control which accounts
        can hold or transfer the mosaic.
    * `revokable`: the creator can reclaim units from any account.

    In this example, `transferable restrictable` is used.

!!! note "Lease fee"

    In addition to the standard [transaction fee](#fetching-network-time-and-fees),
    creating a mosaic requires a one-time lease fee paid in <XYM:>.

    Unlike the transaction fee, the lease fee is **not** included in the transaction request.
    It is deducted automatically by the network from the signer's account
    when the mosaic definition transaction is confirmed.

    The amount of the lease fee can be queried from the <get:/network/fees/rental> endpoint
    (`effectiveMosaicRentalFee` property).

### Submitting the Mosaic Definition

{{ tutorial.code_snippet(['py:102:113', 'js:102:115']) }}

The mosaic definition transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Building the Mosaic Supply Change Transaction

{{ tutorial.code_snippet(['py:118:126', 'js:120:129']) }}

Once the mosaic definition is confirmed, a second transaction increases the mosaic's supply:

* **Type:** Mosaic supply change transactions use the type `mosaic_supply_change_transaction_v1`.

* **Mosaic ID:** The identifier of the mosaic, computed from the signer's address and nonce using
    <dy:IdGenerator.generateMosaicId>.

* **Action:** The value `increase` mints new units directly into the signer's account.

* **Delta:** The number of atomic units to add.
    Since the mosaic has a [divisibility](../../textbook/mosaics.md#divisibility) of `2`, a delta of `10000`
    results in `100.00` whole units (10000 / 10^2^).

### Submitting the Supply Change

{{ tutorial.code_snippet(['py:128:140', 'js:131:143']) }}

The mosaic supply change transaction is signed and announced following the same process as the mosaic definition
transaction.

!!! tip "Combining both transactions"

    Instead of announcing the definition and supply change as two separate transactions, you can submit them
    together in a single [Complete Aggregate Transaction](../transactions/complete-aggregate.md).

    This ensures both operations are confirmed atomically in the same block.

Even without the `supply_mutable` flag, supply changes are allowed as long as the owner holds the entire supply.
Once any units are distributed to other accounts, the supply becomes permanently fixed.

### Retrieving the Mosaic

{{ tutorial.code_snippet(['py:145:156', 'js:148:159']) }}

To verify the mosaic was created successfully, the code retrieves it from the network
using the <get:/mosaics/{mosaicId}> endpoint and displays its properties.

A successful response confirms the mosaic exists on the network with the expected supply and divisibility.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="9 10 18 20 21 23 24 46 60 61 62 63 64"
--8<-- 'devbook/mosaics/create-mosaic.log'
```

Some highlights from the output:

* **Mosaic ID** (line 10): The nonce is combined with the signer's address
    to derive the mosaic ID `0x736fec06ed1daa73`.

* **Fee** (line 18): The transaction fee of 0.015 XYM is calculated as the transaction size
    multiplied by the fee multiplier. The [lease fee](../../textbook/mosaics.md#lease-fee) is deducted separately
    by the network when the transaction is confirmed.

* **Mosaic ID** (line 20): The `id` field is automatically computed by the transaction factory from the nonce
    and the signer's address, matching the value printed on line 10.

* **Mosaic properties** (lines 21, 23-24): Flags are stored as a bitmask, where each flag occupies a single bit:
    `supply_mutable` (1), `transferable` (2), `restrictable` (4), and `revokable` (8).
    The value `6` equals `transferable` (2) + `restrictable` (4).
    The divisibility is `2` and the duration `0` means the mosaic never expires.

* **Supply delta** (line 46): The delta of `10000` atomic units represents `100.00` whole units
    given the mosaic's divisibility of `2`.

* **Verified properties** (lines 60-64): The mosaic is retrieved from the network, confirming
    the expected supply, flags, divisibility, and duration.

The transaction hashes printed in the output can be used to search for the transactions
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                     | Related documentation                |
| ------------------------------------------------------------------------ | ------------------------------------ |
| [Generate mosaic ID](#building-the-mosaic-definition-transaction)        | <dy:IdGenerator.generateMosaicId>    |
| [Define the mosaic](#building-the-mosaic-definition-transaction)         | <dy:SymbolTransactionFactory.create> |
| [Mint mosaic supply](#building-the-mosaic-supply-change-transaction)     | <dy:SymbolTransactionFactory.create> |
| [Retrieve the mosaic](#retrieving-the-mosaic)                            | <get:/mosaics/{mosaicId}>            |

## Next Steps

Now that you have created a mosaic, you can:

* [Link a namespace to your mosaic](../namespaces/link-namespace-to-mosaic.md) to create a human-readable alias
* [Send your mosaic with a transfer transaction](../transactions/transfer.md) to distribute it to other accounts
* [Change the mosaic supply](./change-mosaic-supply.md) to increase or decrease the total supply for `supply_mutable`
    mosaics
* [Modify the mosaic definition](./modify-mosaic-definition.md) to change its flags or duration before distributing
