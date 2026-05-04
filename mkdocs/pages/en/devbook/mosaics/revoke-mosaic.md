---
title: Revoke Mosaic
tutorial_level: beginner
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

{{ tutorial.code_snippet(['py:25:40', 'js:21:40']) }}

The snippet reads the signer's private key from the `SIGNER_PRIVATE_KEY` environment variable, which defaults to a test
key if not set.
The signer's address is derived from the public key.
This account must be the original creator of the mosaic with the `revokable` flag.

The `SOURCE_ADDRESS` environment variable specifies the address of the account from which mosaic units will be revoked.

The `MOSAIC_ID` environment variable specifies the hexadecimal identifier of the mosaic to revoke.
See [Querying Account Balance](../accounts/query-balance.md) to list the mosaics held by an account.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:43:61', 'js:43:61']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Checking Initial Balance

{{ tutorial.code_snippet(['py:65:69', 'js:65:70']) }}

Before revoking, the helper function `get_account_mosaics` fetches the source account's current balance for the target
mosaic from the <get:/accounts/{accountId}> endpoint.
This provides a baseline to compare against after the revocation.

### Building the Revocation Transaction

{{ tutorial.code_snippet(['py:74:84', 'js:75:85']) }}

The revocation transaction reclaims mosaic units from the source account and returns them to the creator's balance:

* **Type:** Mosaic supply revocation transactions use the type `mosaic_supply_revocation_transaction_v1`.

* **Source address:** The address of the account holding the mosaic units to revoke.
    This can be any account that currently holds units of the specified mosaic.

* **Mosaic:** An object containing the <mosaic ID:> and the amount to revoke.

* **Amount:** The number of atomic units to revoke from the source account.
    To find out the mosaic's [divisibility](../../textbook/mosaics.md#divisibility),
    query the <get:/mosaics/{mosaicId}> endpoint.
    For example, with a divisibility of `2`, an amount of `700` represents `7.00` whole units (700 / 10^2^).

!!! note "Partial revocation"

    The amount does not have to match the source account's full balance.
    Any amount up to the source's current holdings can be revoked in a single transaction.

### Submitting the Revocation

{{ tutorial.code_snippet(['py:86:106', 'js:87:104']) }}

The revocation transaction is signed and announced following the same process as in
[Creating a Transfer Transaction](../transactions/transfer.md#announcing-the-transaction).

{{ tutorial.code_snippet(['py:108:125', 'js:106:138']) }}

The code then waits for the transaction to be confirmed by polling the
<get:/transactionStatus/{hash}> endpoint until the status changes to `confirmed`.

### Verifying the Revocation

{{ tutorial.code_snippet(['py:129:133', 'js:142:147']) }}

To verify the revocation, the helper function `get_account_mosaics` fetches the source account's balance again.
The balance should be lower than the [initial balance](#checking-initial-balance) by the revoked amount.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="4 12 24 26 27 47"
--8<-- 'devbook/mosaics/revoke-mosaic.log'
```

Some highlights from the output:

* **Mosaic ID** (line 4): The mosaic ID `8857803461494335809` (`0x7aed3d514c986941`) identifies the mosaic to revoke.

* **Initial balance** (line 12): Before the revocation, the source account holds `1000` atomic units of the mosaic.

* **Source address** (line 24): The `source_address` field identifies the account from which units are revoked.
    This is the hex-encoded form of the Base32 address shown on line 3.

* **Revoked amount** (lines 26-27): The `mosaic` object specifies the mosaic ID in decimal format and the amount `700`.
    The decimal value corresponds to the hexadecimal ID shown on line 4.

* **Verified balance** (line 47): After the revocation, the source account's balance is `300`, confirming that `700`
    atomic units were successfully reclaimed.

The transaction hash printed in the output can be used to search for the transaction in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                       | Related documentation                |
| -------------------------------------------------------------------------- | ------------------------------------ |
| [Check account balance](#checking-initial-balance)                         | <get:/accounts/{accountId}>          |
| [Revoke mosaic units](#building-the-revocation-transaction)                | <dy:SymbolTransactionFactory.create> |
| [Verify the revocation](#verifying-the-revocation)                         | <get:/accounts/{accountId}>          |
