---
title: Add Account Restrictions
---

# Adding Restrictions to an Account

Accounts can enforce limits on:

- which other <accounts:> they can interact with
- which <mosaics:> they can transact with
- which types of operations they can perform

These limits are configured using <account restrictions:>.

This tutorial demonstrates how to restrict an account's **outgoing transactions**
so that it can only send transactions to a single authorized address.

If the restriction is already enabled, the tutorial instead demonstrates how to remove it.

After enabling or disabling the restriction, a test transfer transaction to an unauthorized address is announced,
showing how the network rejects it.

!!! note "Difference with Mosaic Restrictions"

    Symbol also supports <mosaic restrictions:>, which are defined at the mosaic level rather than
    at the account level as shown in this tutorial.

    These are distinct mechanisms.
    They are configured using different transaction types and operate under different rules.

    However, account restrictions can limit which mosaics an account may interact with, and
    mosaic restrictions can limit which accounts may interact with a mosaic.
    The conceptual overlap is therefore a common source of confusion.

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
- Create an <account:>: to restrict, either [from code](./create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md),
    or use the provided default account.
- Obtain <XYM:> to pay for the transaction fees.
    See [Getting Testnet Funds from the Faucet](./testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/accounts/account-restrictions', ['py', 'js']) }}

## Code Explanation

The code begins by defining two helper functions.
For details on how transactions are announced and how their confirmation is tracked, refer to
[Transfer transaction](../transactions/transfer.md) tutorial.
The remaining helper functions are described in the sections below.

The tutorial then proceeds to:

- [set up the required keys](#setting-up-the-accounts)
- [fetch the current network conditions](#fetching-network-time-and-fees)
- [detect the current restriction state](#detecting-the-restriction-state)

Depending on whether the account is already restricted,
a transaction is created to either:

- [enable the restriction](#enabling-the-restriction), or
- [remove the restriction](#removing-the-restriction)

The transaction is then [announced and confirmed](#submitting-the-transaction),
and finally, [a test transfer](#sending-a-test-transfer) is submitted.

### Setting Up the Accounts

{{ tutorial.code_snippet(['py:105:113', 'js:111:119']) }}

An account can only configure restrictions on itself, so this tutorial requires a single <private key:>.
The private key can be provided through the `SIGNER_PRIVATE_KEY` environment variable
(as a 64-character hexadecimal string).
If it is not provided, a default value is used.

The account must hold sufficient funds to announce transactions.
If the default key is used, the corresponding account may already be funded.

At this stage, the authorized address is also configured.
The restriction will later limit outgoing transactions to this address only.

### Fetching Network Time and Fees

{{ tutorial.code_snippet(['py:116:134', 'js:122:140']) }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Detecting the Restriction State

The following function retrieves the current account restrictions applied to a given address using the
<get:/restrictions/account/{address}> endpoint.
If no restrictions are configured, the function returns an empty list.

{{ tutorial.code_snippet(['py:47:60', 'js:53:66']) }}

The returned list is then evaluated to determine the tutorial's execution path.
Based on its contents, the appropriate configuration transaction is constructed,
either to enable or to remove the restriction.

{{ tutorial.code_snippet(['py:138:146', 'js:144:155']) }}

If multiple restrictions are configured on the account, only the first one returned by the endpoint is removed.
This situation should not occur in this tutorial.

### Enabling the Restriction

To restrict the list of addresses the account can interact with, an `AccountAddressRestrictionTransaction` is used.

The other two account restriction types, not covered in this tutorial, are:

- `AccountMosaicRestrictionTransaction`
- `AccountOperationRestrictionTransaction`

{{ tutorial.code_snippet(['py:63:80', 'js:69:87']) }}

The transaction includes the following fields:

- `signer_public_key`: <public key:> of the account whose restriction configuration will be modified.

- `restriction_flags`:

    - `AccountRestrictionFlags.ADDRESS` specifies that the restriction applies to addresses.
        Other possible scopes are `MOSAIC_ID` and `TRANSACTION_TYPE`.
    - `AccountRestrictionFlags.OUTGOING` specifies that only outgoing transactions are affected.
        Incoming transaction restrictions can be configured independently by omitting this flag.

    By default, the listed values form an _allowlist_.
    Only the specified addresses are allowed.

    To configure the restriction in _blocklist_ mode, where the listed addresses are forbidden,
    include the `AccountRestrictionFlags.BLOCK` flag.

    The network XOR's these flags with the current value, which at this point is 0 because the tutorial makes sure
    no restriction is present before enabling it.

- `restriction_additions`: list of addresses (or mosaic IDs, or transaction types) to be added to the restriction.

    In this case, the list contains only the authorized address.

### Removing the Restriction

Disabling the restriction requires clearing both the configured flags and the listed addresses.

{{ tutorial.code_snippet(['py:83:101', 'js:90:107']) }}

The same `restriction_flags` values used when enabling the restriction are provided again.
Because the flags are XOR'ed by the network, supplying the same values toggles them off,
effectively clearing the restriction.

The addresses currently configured in the restriction are supplied in the `restriction_deletions` field
so they can be removed from the configuration.

The <dy:Address.fromDecodedAddressHexString> method converts the hexadecimal string format returned by the REST API
into the address representation expected when constructing a transaction.

### Submitting the Transaction

The constructed transaction is signed, announced and confirmed as described in the
[Transfer transaction](../transactions/transfer.md) tutorial.

{{ tutorial.code_snippet(['py:148:154', 'js:157:164']) }}

### Sending a Test Transfer

A test transfer is then attempted to an unauthorized address.

{{ tutorial.code_snippet(['py:156:170', 'js:166:180']) }}

If the restriction has been enabled, the transfer fails with an `Address_Interaction_Prohibited` error.
If the restriction has been removed, the transfer is confirmed successfully.

The restriction configuration transaction and the test transfer are announced and confirmed independently.
Each requires its own confirmation, which may increase the total execution time.

The process could be optimized by embedding both transactions in a single <aggregate transaction:>
and announcing them together.

## Output

The output shown below corresponds to two typical runs of the program.

=== ":material-lock-plus: Enabling the Restriction"

    ```text linenums="1" hl_lines="2-3 9 21-24 41"
    --8<-- 'devbook/accounts/account-restrictions-enable.log'
    ```

    Key points in the output:

    - **Lines 2-3**: Addresses of the involved accounts.
    - **Line 9** (`Response: No restrictions found`): No restrictions are currently configured.
    - **Line 21** (`"restriction_flags": 16385`): `0x4001` corresponds to the combination of `ADDRESS` and `OUTGOING`.
    - **Line 22-24** (`"restriction_additions"`): List of allowed addresses, in decoded hexadecimal format.
        The value corresponds to the address shown in line 3.
    - **Line 41** (`test transfer failed`): The unauthorized recipient address results in an
        `Address_Interaction_Prohibited` error, as expected.

=== ":material-lock-open: Removing the Restriction"

    ```text linenums="1" hl_lines="2-3 9 21 23-25 44"
    --8<-- 'devbook/accounts/account-restrictions-disable.log'
    ```

    Key points in the output:

    - **Lines 2-3**: Addresses of the involved accounts.
    - **Line 9** (`Response: [ ... ]`): Existing restrictions are detected.
    - **Line 21** (`restriction_flags`): Same flag value used when enabling the restriction.
    - **Line 23-25** (`restriction_deletions`): The previously configured address is removed.
    - **Line 44** (`test transfer confirmed`): The transfer is confirmed successfully because the restriction has been lifted.

The transaction hashes shown in the output can be used to look up the transactions in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                               | Related documentation                        |
|------------------------------------------------------------------------------------|----------------------------------------------|
| [Retrieve the current restriction configuration](#detecting-the-restriction-state) | <get:/restrictions/account/{address}>        |
| [Enable a restriction](#enabling-the-restriction)                                  | `AccountAddressRestrictionTransaction`       |
| [Remove a restriction](#removing-the-restriction)                                  | `AccountAddressRestrictionTransaction`       |
