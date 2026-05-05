---
title: Modify Mosaic Definition
tutorial_level: intermediate
---

# Modifying a Mosaic Definition

As long as no units of a <mosaic:> exist on the network, its creator can still change its definition properties
(flags, divisibility, and duration) by sending a second definition transaction with the same identifier.

For example, adding a flag that was not set initially, changing its divisibility, or extending its duration before
distribution.

!!! warning "Zero supply requirement"

    A mosaic definition can only be modified when its total supply is `0`.
    No units of the mosaic can exist anywhere on the network, otherwise the transaction fails with
    `Failure_Mosaic_Modification_Disallowed`.

This tutorial shows how to modify an existing mosaic's flags.
To change the mosaic's supply instead of its definition, see [Changing Mosaic Supply](./change-mosaic-supply.md).

!!! tip "Consider creating a new mosaic instead"

    [Creating a new mosaic](./create-mosaic.md) is easier than modifying an existing one.
    Understanding how modification works is still important to know when a mosaic's definition can no longer be edited.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Have a mosaic with **zero supply**.
    Either [create a new mosaic](./create-mosaic.md) without minting supply,
    or [decrease the supply](./change-mosaic-supply.md) of an existing mosaic to `0`.
* Obtain <XYM:> to pay for the transaction and lease fees.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/mosaics/modify-mosaic-definition', ['py', 'js']) }}

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet_tagged('step-1') }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a
test key if not set.
The signer's address is derived from the public key.
This account must be the original creator of the mosaic.

### Fetching Network Time and Fees

{{ tutorial.code_snippet_tagged('step-2') }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Modification Transaction

Modifying a mosaic definition requires knowing its current values (flags, divisibility, and duration),
because the network combines them with the values in the transaction rather than replacing them.
The mosaic can be retrieved using the <get:/mosaics/{mosaicId}> endpoint or looked up in the
[Symbol Explorer](https://testnet.symbol.fyi/).

In this tutorial, the current values are already known because the mosaic was just
[created and retrieved](./create-mosaic.md#retrieving-the-mosaic) in the previous tutorial.

{{ tutorial.code_snippet_tagged('step-3') }}

The `MOSAIC_NONCE` environment variable specifies the <nonce:> of the mosaic to modify.
The nonce must match the one used when [creating the mosaic](./create-mosaic.md) to target the same mosaic.

The modification transaction uses the same `mosaic_definition_transaction_v1` type as the original creation.
The key difference is that **the nonce targets an existing mosaic** instead of creating a new one.

When processing the transaction, each property is combined with the mosaic's current value using the following rules:

* **Flags** are XOR'd with the current flags.
    Setting a flag that is already active removes it; setting a flag that is not active adds it.
    For a description of each available flag, see
    [Building the Mosaic Definition Transaction](./create-mosaic.md#building-the-mosaic-definition-transaction).
* **Divisibility** is XOR'd with the current divisibility.
    The resulting value must be between `0` and `6`.
* **Duration** is added to the current remaining duration.
    Duration can only be extended, not reduced, because the field is unsigned.
    A value of `0` leaves the duration unchanged.
    Eternal mosaics (duration `0`) cannot have their duration modified.
    The resulting duration cannot exceed 10,512,000 blocks (approximately 10 years).

In this example, the existing mosaic has flags `transferable restrictable`
([numeric value](./create-mosaic.md#conclusion) `6`).
The modification sets `flags: 'revokable'` (numeric value `8`).
XOR produces 6 ⊕ 8 = 14, which corresponds to `transferable restrictable revokable`.

The following table illustrates how XOR affects individual flags:

| Flag            | Current | Modification | Result (XOR) |
| --------------- | ------- | ------------ | ------------ |
| `transferable`  | on      | off          | on           |
| `restrictable`  | on      | off          | on           |
| `revokable`     | off     | on           | on           |

Setting `divisibility` to `0` results in 2 ⊕ 0 = 2 (no change), and `duration: 0` adds nothing to the current
duration.

!!! note "Lease fee"

    Each mosaic definition transaction incurs the full [lease fee](../../textbook/mosaics.md#lease-fee) paid in
    <XYM:>, whether creating or modifying a mosaic.
    This is the same fee every time, in addition to the standard [transaction fee](#fetching-network-time-and-fees).
    The lease fee amount can be queried from the <get:/network/fees/rental> endpoint
    (`effectiveMosaicRentalFee` property).

### Submitting the Modification

{{ tutorial.code_snippet_tagged('step-4') }}

The modification transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet_tagged('step-5') }}

The code then waits for the transaction to be confirmed by polling the <get:/transactionStatus/{hash}> endpoint
until the status changes to `confirmed`.

### Retrieving the Mosaic

{{ tutorial.code_snippet_tagged('step-6') }}

To verify the modification was applied, the code retrieves the mosaic from the network using the
<get:/mosaics/{mosaicId}> endpoint and displays its updated properties.

A successful response confirms the mosaic now has the expected flags value.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="7 8 19 21 22 36 37 38"
--8<-- 'devbook/mosaics/modify-mosaic-definition.log'
```

Some highlights from the output:

* **Mosaic nonce** (line 7): The nonce `1770998662` matches the nonce used when the mosaic was created, targeting
    the same mosaic for modification.

* **Mosaic ID** (line 8): The mosaic ID `0x5bdd3795f7a8b3af` is derived from the nonce and signer address, confirming
    the correct mosaic is being modified.

* **Transaction properties** (lines 19, 21-22): The modification sets `duration` to `0` (no change),
    `flags` to `8` (`revokable`), and `divisibility` to `0` (no change).

* **Updated properties** (lines 36-38): The mosaic's divisibility remains `2` (unchanged by XOR with `0`).
    The flags are now `14`, which corresponds to `transferable` (2) + `restrictable` (4) + `revokable` (8).
    The duration remains `0` (eternal, unchanged).

The transaction hash printed in the output can be used to search for the transaction in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                        | Related documentation                |
| --------------------------------------------------------------------------- | ------------------------------------ |
| [Generate mosaic ID](#building-the-modification-transaction)                | <dy:IdGenerator.generateMosaicId>    |
| [Modify mosaic flags](#building-the-modification-transaction)               | <dy:SymbolTransactionFactory.create> |
| [Verify the updated mosaic](#retrieving-the-mosaic)                         | <get:/mosaics/{mosaicId}>            |
