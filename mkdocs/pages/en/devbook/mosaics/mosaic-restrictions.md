---
title: Add Mosaic Restrictions
tutorial_level: advanced
---

# Adding Restrictions to a Mosaic

The owner of a <mosaic:> can restrict which accounts are allowed to transact with it.
The conditions are called <mosaic restrictions:> and are defined in two parts:

* <Mosaic global restrictions:> define the required conditions as key-value-relation tuples.
* <Mosaic address restrictions:> assign values for those keys to individual accounts.

An account can transact with the mosaic only if its assigned values satisfy all the mosaic's global conditions.

This tutorial requires a preexisting mosaic created with the [restrictable](../../textbook/mosaics.md#restrictability)
flag.
If the mosaic does not yet define any global restriction, the tutorial creates one with the configuration:

| Key              | Value | Relation         |
|------------------|-------|------------------|
| `security_level` | 1     | greater-or-equal |

This configuration means that the mosaic can only be used by accounts whose `security_level` restriction value is
**greater than or equal to 1**.

The tutorial then assigns this key to a test account, or toggles its value between 1 and 0 if it already exists,
and attempts to transfer the mosaic from its owner account to the test account.

As a result, every other run of the program fails with a **restriction violation** error.

Because configuring restrictions requires several transactions, the tutorial bundles them into a single
<complete aggregate transaction:>.
This avoids waiting for each transaction to be confirmed individually.

!!! note "Difference with Account Restrictions"

    Symbol also supports <account restrictions:>, which are defined at the account level rather than
    at the mosaic level as shown in this tutorial.

    These are distinct mechanisms.
    They are configured using different transaction types and operate under different rules.

    However, account restrictions can limit which mosaics an account may interact with, and
    mosaic restrictions can limit which accounts may interact with a mosaic.
    The conceptual overlap is therefore a common source of confusion.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:>: to own the mosaic, either [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md),
    or use the provided default account.
* Create a restrictable <mosaic:> or use the provided default one.
    See [Creating a Mosaic](./create-mosaic.md).
* Obtain <XYM:> to pay for the transaction fees.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) and
[Creating a Complete Aggregate Transaction](../transactions/complete-aggregate.md) tutorials to understand how
transactions are announced and confirmed, and how to bundle them.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/mosaics/mosaic_restrictions', ['py', 'js']) }}

## Code Explanation

The code begins by defining several helper functions.
For details on how transactions are announced and how their confirmation is tracked, refer to the
[Transfer transaction](../transactions/transfer.md) tutorial.
The remaining helper functions are described in the sections below.

The tutorial then proceeds to:

* [set up the required keys](#setting-up-the-accounts)
* [fetch the current network conditions](#fetching-network-time-and-fees)
* [enable the global restriction if necessary](#enabling-the-global-restriction)
* [toggle the address restriction](#toggling-the-address-restriction)
* [bundle all transactions](#building-the-aggregate-transaction) in a single aggregate transaction
* [announce and confirm](#submitting-the-transaction) the aggregate
* [send a test transfer](#sending-a-test-transfer)

### Setting Up the Accounts

The tutorial starts by configuring the accounts involved in the example.

{{ tutorial.code_snippet_tagged('step-1') }}

The code defines:

* the **owner account**, which controls the mosaic and is responsible for configuring its restrictions.
    Its private key can be provided through the `OWNER_PRIVATE_KEY` environment variable as a 64-character hexadecimal
    string.
* the **target account**, which will later receive authorization to transact with the mosaic.
    Its address can be provided through the `TARGET_ADDRESS` environment variable as a Symbol testnet address.
* the **mosaic identifier**, read from `MOSAIC_ID` as 16 hexadecimal characters.
* the **restriction name**, read from `RESTRICTION_NAME` as a string.
* the corresponding **restriction key**, derived from the restriction name using the SDK's
    <dy:Restriction.mosaicRestrictionGenerateKey> function, which hashes the name with SHA3-256 and takes the first
    eight bytes of the hash.
    This approach allows applications to use human-readable names while producing deterministic keys.
    Any 64-bit number can also be used directly as a restriction key.

If any of these values is not provided through an environment variable, a default value is used.

The owner account must hold sufficient funds to announce transactions.
If the default one is used, it may already be funded.

### Fetching Network Time and Fees

{{ tutorial.code_snippet_tagged('step-2') }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](../transactions/transfer.md) tutorial.

### Enabling the Global Restriction

{{ tutorial.code_snippet_tagged('step-3') }}

The code first checks whether the mosaic already defines a global restriction for the configured key:

{{ tutorial.code_snippet_tagged('step-4') }}

This is done by querying <get:/restrictions/mosaic> and filtering by `mosaicId` and `entryType=1`, which selects
**global restrictions**.
The returned entries are then filtered to keep only those involving the selected {{ tutorial.var('restriction_key') }}.

If no restriction is found, one is created by adding two transactions to the list of transactions to announce:

* a **mosaic global restriction transaction** defining the restriction condition.
    See the <ser:MosaicGlobalRestrictionTransactionV1> serialization table for details about each of its fields.

    The restriction created in this tutorial requires the value associated with the key
    `security_level` to be **greater than or equal to 1**.

* a **mosaic address restriction transaction** authorizing the owner account.
    See the <ser:MosaicAddressRestrictionTransactionV1> serialization table for details about each of its fields.

    The code assigns the value 1 to the owner's `security_level` so the owner account can continue transacting with
    its own mosaic.

    !!! note
        For simplicity, the tutorial assumes that if no global restriction exists, the owner account
        also has no address restriction.

        For this reason `0xFFFFFFFF_FFFFFFFF` is used as the previous value, indicating that no value
        was previously set.

        A more robust implementation should first query the owner's restriction state and use the
        appropriate previous value, as demonstrated below for the target account.

### Toggling the Address Restriction

With the global restriction in effect, the next step checks whether the target account already has a restriction value
defined for the key.

{{ tutorial.code_snippet_tagged('step-5') }}

As in the global restriction case, the current value is obtained by querying <get:/restrictions/mosaic> and filtering
by `mosaicId`, `targetAddress`, and `entryType=0`, which selects **address restrictions**.
The returned entries are then filtered to keep only those involving the selected {{ tutorial.var('restriction_key') }}.

Depending on the current value of the restriction for the target account,
a transaction is created that authorizes or deauthorizes the account.
This transaction is added to the list of transactions to announce.

{{ tutorial.code_snippet_tagged('step-6') }}

* If the account does not yet have a restriction value, or the value is not `1`, the code assigns the value `1`,
    authorizing it to use the mosaic.

* If the account already has the value `1`, the code replaces it with `0`, revoking the authorization.

Running the tutorial repeatedly therefore alternates between authorizing and deauthorizing the
target account.

Only the first restriction in the returned list is examined, because, after filtering by
{{ tutorial.var('restriction_key') }}, the list is either empty or contains a single entry.

The same <ser:MosaicAddressRestrictionTransactionV1> is used in both cases, changing only the value assigned to the
restriction.

When no previous restriction exists, the special value `0xFFFFFFFF_FFFFFFFF` must be used as the previous value.

### Building the Aggregate Transaction

All configuration transactions created above are bundled into a single <complete aggregate transaction:>,
so the user does not need to wait for them to be confirmed individually.

{{ tutorial.code_snippet_tagged('step-7') }}

Only the aggregate transaction pays fees, so <embedded transactions:> do not use the `fee` field.

### Submitting the Transaction

The constructed aggregate transaction is signed, announced, and confirmed as described in the
[Transfer transaction](../transactions/transfer.md) tutorial.

{{ tutorial.code_snippet_tagged('step-8') }}

### Sending a Test Transfer

Finally, the tutorial attempts to send one unit of the mosaic from the owner account to the target account
using a standard <transfer transaction:>.

{{ tutorial.code_snippet_tagged('step-9') }}

If the target account currently satisfies the restriction (`security_level ≥ 1`),
the transfer is confirmed successfully.

If the restriction value was toggled to `0`, the transaction fails with an `Account_Unauthorized` error.

Running the tutorial multiple times therefore alternates between successful and failing transfers,
demonstrating how mosaic restrictions control which accounts are allowed to transact with the mosaic.

## Output

The output shown below corresponds to two typical runs of the program.

=== ":material-lock-open: Enabling the Restriction and Authorizing the Account"

    ```text linenums="1" hl_lines="2-5 12-13 27 41-42 72"
    --8<-- 'devbook/mosaics/mosaic_restrictions_enable.log'
    ```

    Key points in the output:

    * **Lines 2-3**: Addresses of the involved accounts.
    * **Line 4**: The mosaic being restricted.
    * **Line 5**: The restriction name and its corresponding key.
    * **Line 12** (`Response: []`): The mosaic currently has no global restrictions.
    * **Line 13**: The transaction configuring the mosaic restriction.
        It includes the mosaic ID (in decimal), the restriction key (in decimal), the restriction value (`1`), and
        the restriction condition (`6`, which corresponds to the `greater-or-equal` <ser:MosaicRestrictionType>)
    * **Line 27**: The transaction authorizing the owner account.
        It includes the mosaic ID (in decimal), the restriction key (in decimal), and the necessary restriction
        value (`1`).
    * **Line 41** (`Response: []`): The target account is currently unauthorized because it has no value associated with
        the restriction key.
    * **Line 42**: The transaction authorizing the target account.
        It includes the mosaic ID (in decimal), the restriction key (in decimal), and the necessary restriction
        value (`1`).
    * **Line 72** (`test transfer confirmed`): The test transaction succeeded because both accounts satisfy the
        restriction and are therefore authorized.

=== ":material-lock-plus: Deauthorizing the Account"

    ```text linenums="1" hl_lines="2-5 12 15-16 43"
    --8<-- 'devbook/mosaics/mosaic_restrictions_disable.log'
    ```

    Key points in the output:

    * **Lines 2-3**: Addresses of the involved accounts.
    * **Line 4**: The mosaic being restricted.
    * **Line 5**: The restriction name and its corresponding key.
    * **Line 12** (`Response: [ ... ]`): Existing restrictions are detected.
    * **Line 15** (`Response: [ ... ]`): The target account has a restriction value of `1`, meaning it is authorized.
    * **Line 16**: The transaction deauthorizing the target account.
        It includes the mosaic ID (in decimal), the restriction key (in decimal), and the necessary restriction
        value (`0`).
    * **Line 43** (`test transfer failed`): The test transaction failed because the target account no longer satisfies
        the restriction, as expected.

The transaction hashes shown in the output can be used to look up the transactions in the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Troubleshooting

Transactions are rejected if they violate protocol constraints.
The following table summarizes the most common error sources:

<div class="keyed-table" markdown>
| Error message                  | Probable cause                                                                                      |
|--------------------------------|-----------------------------------------------------------------------------------------------------|
| `Mosaic_Expired`               | The mosaic does not exist, or it has [expired](../../textbook/mosaics.md#duration).                 |
| `Mosaic_Owner_Conflict`        | The account attempting to restrict the mosaic is not its owner.                                     |
| `Required_Property_Flag_Unset` | The mosaic was not created with the [restrictable](../../textbook/mosaics.md#restrictability) flag. |
| `Account_Unauthorized`         | Either the owner or the target account is not authorized to transact with the mosaic.               |
</div>

## Conclusion

This tutorial showed how to:

| Step                                                                                        | Related documentation                       |
|---------------------------------------------------------------------------------------------|---------------------------------------------|
| [Retrieve the current mosaic restriction configuration](#enabling-the-global-restriction)   | <get:/restrictions/mosaic>                  |
| [Configure a mosaic global restriction](#enabling-the-global-restriction)                   | <ser:MosaicGlobalRestrictionTransactionV1>  |
| [Retrieve an account's mosaic restriction configuration](#toggling-the-address-restriction) | <get:/restrictions/mosaic>                  |
| [Configure a mosaic address restriction](#toggling-the-address-restriction)                 | <ser:MosaicAddressRestrictionTransactionV1> |
