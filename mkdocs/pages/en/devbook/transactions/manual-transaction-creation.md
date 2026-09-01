---
title: Manual Transaction Creation
tutorial_level: intermediate
---

# Creating Transactions Manually

Most tutorials create transactions from descriptors using <dy:SymbolFacade.createTransactionFromTypedDescriptor>.
This is the recommended approach: in languages that support typed descriptors, it provides type safety and better
editor support, while also calculating the deadline and transaction fees for you.

For completeness, this tutorial shows the lower-level alternative: creating the transaction manually with
<dy:SymbolTransactionFactory.create>.

The example mirrors the [Transfer Transaction](./transfer.md) tutorial, but replaces descriptor-based transaction
creation with manual field assignment, including explicit deadline and fee handling.

The remaining steps are briefly summarized.
For full details, refer to the [Transfer Transaction](./transfer.md) tutorial.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:>, either [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
* Obtain <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/transactions/manual_transaction_creation', ['py', 'js', 'java']) }}

## Code Explanation

### Setting Up an Account

{{ tutorial.code_snippet_tagged('step-1') }}

The signer account is loaded from the `SIGNER_PRIVATE_KEY` environment variable.
If not provided, a test key is used as default.

### Fetching Network Time

{{ tutorial.code_snippet_tagged('step-2') }}

Manual transaction creation requires an absolute deadline expressed in <network time:>.
Network time is measured in milliseconds since the <nemesis block:>.

When using descriptor-based transaction creation, the SDK accepts a deadline duration in seconds from now instead so
fetching the current network time is not necessary.

The snippet fetches the current network time from <get:/node/time> and stores it so the transaction deadline can be
set later.
Applications do not need to query network time before every transaction: it can be fetched once and then adjusted
using the local system clock.

!!! info "Deadline checks"

    If a transaction's deadline is earlier than the current network time or too far in the future, the transaction is
    rejected.

### Fetching Recommended Fees

{{ tutorial.code_snippet_tagged('step-3') }}

Transactions on Symbol must pay a fee to incentivize nodes to include them in blocks.
The snippet fetches the recommended fee multiplier from <get:/network/fees/transaction> and stores it for use after
the transaction is created.

### Building the Transaction

{{ tutorial.code_snippet_tagged('step-4') }}

The transaction is created with <dy:SymbolTransactionFactory.create>, which accepts a plain descriptor object.
Unlike <dy:SymbolFacade.createTransactionFromTypedDescriptor>, the manual factory does not fill in common transaction
fields or calculate the fee.

The descriptor passed to <dy:SymbolTransactionFactory.create> contains:

* {{ tutorial.var('type') }}: Use <ser:TransferTransactionV1|transfer_transaction_v1>.
* {{ tutorial.var('signer_public_key') }}: The account that signs the transaction and pays the fee.
    In a transfer transaction, it is also the source of the transferred mosaics.
* {{ tutorial.var('deadline') }}: The absolute deadline in network time.
* {{ tutorial.var('recipient_address') }}: In this example, the recipient is the same as the sender.
* {{ tutorial.var('mosaics') }}: The mosaics to send.
    The example sends 1 XYM, expressed as `1_000_000` atomic units because XYM has <divisibility:> 6.

After the transaction is created, its size is known.
The final fee is calculated using <dy:FeeCalculator.calculateTransactionFee>, which multiplies that size by
the recommended fee multiplier, and assigned to the transaction's `fee` field.

### Signing and Serializing

{{ tutorial.code_snippet_tagged('step-5') }}

The transaction is signed with <dy:SymbolFacade.signTransaction>.
The signature is then attached with <dy:SymbolTransactionFactory.attachSignature>, producing the JSON payload that can
be announced to a node.

### Announcing the Transaction

{{ tutorial.code_snippet_tagged('step-6') }}

The transaction is announced by sending the signed payload to <put:/transactions>.

### Waiting for Confirmation

{{ tutorial.code_snippet_tagged('step-7') }}

After announcement, the transaction status is monitored using <get:/transactionStatus/{hash}> until it is confirmed or
fails.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="2 4 8 13 14 19 21"
--8<-- 'devbook/transactions/manual_transaction_creation.log'
```

Key points in the output:

* **Line 2:** The code explicitly fetches the current network time.
* **Line 4:** The code fetches the recommended fee multiplier.
* **Line 8** (`signature`): The signature is already attached before the transaction is printed.
* **Line 13** (`fee`): The fee was calculated after the transaction was created.
* **Line 14** (`deadline`): The deadline is an absolute network-time timestamp.
* **Line 19:** The signed payload is announced to the network.
* **Line 21:** The transaction hash can be used to look up the transaction in the
    [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to create a transaction manually:

| Step                                                        | Related documentation                                                               |
| ----------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| [Fetch network time](#fetching-network-time)                | <get:/node/time>                                                                    |
| [Fetch recommended fees](#fetching-recommended-fees)        | <get:/network/fees/transaction>                                                     |
| [Build the transaction](#building-the-transaction)          | <dy:SymbolTransactionFactory.create>                                                |
| [Calculate the fee](#building-the-transaction)              | <dy:FeeCalculator.calculateTransactionFee>                                          |
| [Sign and serialize](#signing-and-serializing)              | <dy:SymbolFacade.signTransaction><br/><dy:SymbolTransactionFactory.attachSignature> |
| [Announce and confirm](#announcing-the-transaction)         | <put:/transactions><br/><get:/transactionStatus/{hash}>                             |
