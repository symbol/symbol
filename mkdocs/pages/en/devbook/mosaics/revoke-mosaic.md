---
title: Revoke Mosaic
---

# Revoking a Mosaic

<Mosaics:|Mosaics> created with the `revokable` flag allow their creator to reclaim units from any <account:>,
returning them to the creator's own account balance.
This is useful for enforcing contractual terms, reclaiming unused tokens, or correcting erroneous distributions.

This tutorial shows how to revoke mosaic units from another account.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Have an <account:> that owns a mosaic with the `revokable` flag set.
    See the [Creating a Mosaic](./create-mosaic.md) tutorial.
* Have transferred some mosaic units to another account.
    See the [Transfer Transaction](../transactions/transfer.md) tutorial.
* Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how transactions
are announced and confirmed.

For more details on revocability, see [Revocability](../../textbook/mosaics.md#revocability) in the Textbook.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/mosaics/revoke-mosaic', ['py', 'js']) }}

## Code Explanation

### Setting Up the Accounts

{{ tutorial.code_snippet(['py:15:27', 'js:12:25']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a test
key if not set.
The signer's address is derived from the public key.
This account must be the original creator of the mosaic with the `revokable` flag.

The `SOURCE_ADDRESS` environment variable specifies the address of the account from which mosaic units will be revoked.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:29:47', 'js:28:46']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Building the Revocation Transaction

{{ tutorial.code_snippet(['py:52:66', 'js:51:65']) }}

The `MOSAIC_ID` environment variable specifies the hexadecimal identifier of the mosaic to revoke.
The mosaic ID can be obtained from the output of the [Creating a Mosaic](./create-mosaic.md) tutorial
or from the [Symbol Explorer](https://testnet.symbol.fyi/).

The revocation transaction reclaims mosaic units from the source account and returns them to the creator's balance:

* **Type:** Mosaic supply revocation transactions use the type `mosaic_supply_revocation_transaction_v1`.

* **Source address:** The address of the account holding the mosaic units to revoke.
    This can be any account that currently holds units of the specified mosaic.

* **Mosaic:** An object containing the <mosaic ID:> and the amount to revoke.

* **Amount:** The number of atomic units to revoke from the source account.
    Since the mosaic in this example has a [divisibility](../../textbook/mosaics.md#divisibility) of `2`,
    an amount of `700` represents `7.00` whole units (700 / 10^2^).

!!! note "Partial revocation"

    The amount does not have to match the source account's full balance.
    Any amount up to the source's current holdings can be revoked in a single transaction.

### Submitting the Revocation

{{ tutorial.code_snippet(['py:68:88', 'js:67:84']) }}

The revocation transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:90:107', 'js:86:118']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Verifying the Revocation

{{ tutorial.code_snippet(['py:112:120', 'js:123:132']) }}

To verify the revocation, the code retrieves the source account's mosaic balances from the
<get:/accounts/{accountId}> endpoint.

A successful response shows that the revoked mosaic's balance has decreased by the specified amount.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="10 20 22 23 39"
--8<-- 'devbook/mosaics/revoke-mosaic.log'
```

Some highlights from the output:

* **Mosaic ID** (line 10): The mosaic ID `6619508144549180335` (`0x5bdd3795f7a8b3af`) identifies the mosaic to revoke.

* **Source address** (line 20): The `source_address` field identifies the account from which units are revoked.

* **Revoked amount** (lines 22-23): The `mosaic` object specifies the mosaic ID in decimal format and the amount `700`
    (representing `7.00` whole units with divisibility `2`). The decimal value corresponds to the hexadecimal ID shown
    on line 10.

* **Verified balance** (line 39): After the revocation, the source account's balance for mosaic
    `5BDD3795F7A8B3AF` is `300`, confirming that `700` atomic units were successfully reclaimed.

The transaction hash printed in the output can be used to search for the transaction in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                       | Related documentation                |
| -------------------------------------------------------------------------- | ------------------------------------ |
| [Revoke mosaic units](#building-the-revocation-transaction)                | <dy:SymbolTransactionFactory.create> |
| [Verify account balance](#verifying-the-revocation)                        | <get:/accounts/{accountId}>          |
