---
title: Transfer
tutorial_level: beginner
---

# Creating a Transfer Transaction

<Transfer transactions:|Transfer transactions> are the most basic type of Symbol transaction.
They allow sending <XYM:> or any other type of <mosaic:> from one <account:> to another, optionally including a message.

This tutorial shows how to create, sign, and announce a transfer transaction, and then poll the transaction's status
until it is confirmed.
The recommended fee multiplier is fetched from the network so the SDK can calculate an appropriate transaction fee.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create an <account:> to send the transfer transaction, either
    [from code](../accounts/create-from-private-key.md) or
    [by using a wallet](../../userbook/wallet/create-account.md).
* Obtain <XYM:> to pay for the transaction fee and transfer amount.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/transactions/transfer', ['py', 'js']) }}

The whole code is wrapped in a single `try` block to provide simple error handling,
but applications will probably want to use more fine-grained control.

## Code Explanation

### Setting Up the Account

{{ tutorial.code_snippet_tagged('step-1') }}

The signer account is loaded from the `SIGNER_PRIVATE_KEY` environment variable.
If not provided, a test key is used as default.

### Fetching Recommended Fees

{{ tutorial.code_snippet_tagged('step-2') }}

Transactions on Symbol must pay a fee to incentivize nodes to include them in blocks.
If the fee is too low, no node may include the transaction.
If it is too high, the sender wastes funds.
In addition, each node may enforce a minimum fee threshold for incoming transactions.

The optimal fee depends on the current state of the network,
particularly the number of transactions being submitted and the fees they are offering.
To support fee estimation, Symbol provides the <get:/network/fees/transaction>
endpoint that returns a _recommended fee multiplier_ based on recent transaction activity.

The final fee is calculated by multiplying the recommended multiplier by the transaction's size in bytes.
When creating transactions from descriptors as done in all tutorials, this operation is performed by the SDK.
If you create transactions manually using <dy:SymbolTransactionFactory.create>, you need to calculate the final fee
yourself.

Although applications can use a fixed fee for simplicity, it is more efficient to follow the network recommendation.
There is no need to query the multiplier for every transaction, but it should be refreshed regularly.

The snippet above takes the greater of the network's recommended multiplier (`medianFeeMultiplier`) and the
minimum multiplier (`minFeeMultiplier`) required by the node where the transactions will be sent.
The result is stored for later use once the transaction size is known.

### Building the Transaction

{{ tutorial.code_snippet_tagged('step-3') }}

The transfer transaction is created from the transaction's descriptor.
The descriptor contains the transaction-specific fields, while the creation method receives the common fields used
to finish the transaction.

<dy:SymbolFacade.createTransactionFromTypedDescriptor> receives:

* The transaction's descriptor: Defines <ser:TransferTransactionV1> and the transfer fields.
* The signer public key: The signer is the account that will pay the fee.
    In a transfer transaction, it is also the source of the transferred mosaics.
* The fee multiplier: Used to calculate the transaction fee.
* The deadline duration: Set to two hours from the current time.

    !!! info "Deadlines and network time"

        Transactions on Symbol must include a deadline, which defines how long the network should attempt to confirm the
        transaction before discarding it.
        Deadlines are expressed in <network time:>, measured from the <nemesis block:>.

        If a transaction's deadline is earlier than the current network time or more than six hours in the future,
        the transaction will be rejected.

        When creating transactions from descriptors, the SDK takes care of network time and accepts a relative deadline
        duration in seconds from now.

        If you create transactions manually using <dy:SymbolTransactionFactory.create>, you need to provide the absolute
        deadline yourself.
        You can fetch the current network time from <get:/node/time>.
        Applications do not need to query network time before every transaction: it can be fetched once and then adjusted
        using the local system clock when needed.

The transaction's descriptor contains:

* {{ tutorial.var('recipient_address') }}: In this example, the recipient is the same as the sender,
    which is useful for demonstration but not terribly practical.

* `mosaics`: This is an array, because a transfer transaction can send multiple mosaics at once.
    Each entry includes a <mosaic ID:> and an amount.

    In the example, the mosaic ID for <XYM:> is obtained using its alias, `symbol.xym`, which is easier to remember
    than the full hexadecimal ID.

    Amounts are expressed in atomic units, which depend on the mosaic's <divisibility:>.
    For XYM, the divisibility is 6, so 1 XYM must be expressed as `1_000_000`.

The descriptor does not include common transaction fields such as the signer public key, deadline, or fee.
<dy:SymbolFacade.createTransactionFromTypedDescriptor> fills them in, taking care of network time for the relative
deadline and calculating the fee from the fee multiplier.

!!! info "Including a message in the transaction"

    Transactions can optionally include a free-form message.
    The [Sending Messages with Transfer Transactions](./messages.md) explains how to do this.

### Signing and Serializing

{{ tutorial.code_snippet_tagged('step-4') }}

Once the transaction is created, it must be signed with the signing account's private key.
Signing ensures the transaction is authentic and authorized by the sender.

<dy:SymbolFacade.signTransaction> returns a <signature:>.

<dy:SymbolTransactionFactory.attachSignature> adds the signature to the transaction and serializes it into a
JSON payload ready to be submitted directly to a node for announcement.

### Announcing the Transaction

{{ tutorial.code_snippet_tagged('step-5') }}

Announcing a transaction is a simple request to the <put:/transactions> endpoint of any Symbol <API node:>.
As long as the payload is correctly formed, the request will succeed with an HTTP 200 response.

However, this response does **not** indicate that the transaction is valid or accepted by the network.
Validation, fee checks, and other rules are applied asynchronously after the transaction is received.

To confirm that the transaction is actually accepted and included in a block, its status must be monitored separately,
as shown in the next step.

### Waiting for Confirmation

{{ tutorial.code_snippet_tagged('step-6') }}

!!! note
    This step uses polling to check whether the transaction has been confirmed.
    Polling is used here for illustration purposes, but it is not the recommended approach for real applications.

    [WebSockets](../websockets/listen-transaction-flow.md) provide a more responsive solution without the overhead of repeated API calls.

    In addition, the logic for checking transaction status is reusable.
    It can be moved into a utility function or module, since it is needed after announcing every transaction.

The snippet above repeatedly queries the <get:/transactionStatus/{hash}> endpoint using the hash of the submitted
transaction.
The response may take one of several forms:

* An HTTP error, indicating that the node has not yet started processing the transaction.
* A valid JSON object containing the transaction status.

If the status group is `confirmed`, the transaction has been accepted and included in a block.

If the status group is `failed`, the transaction has been rejected, for example, due to insufficient funds.

In any other case, the code waits one second and tries again, up to a maximum of 60 times.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="3 7 11 13 14 18 25"
--8<-- 'devbook/transactions/transfer.log'
```

Some highlights from the output:

* **Fee multiplier** (line 3): The recommended multiplier fetched from the network, used together with the
    transaction size to compute the fee.

* **Signer public key** (line 7): The account that signs the transaction and sends the mosaics.

* **Transaction fee** (line 11): `17600` atomic units (0.0176 XYM), derived from the fee multiplier and the
    transaction's size in bytes.

* **Recipient address** (line 13): The account that receives the mosaics.
    It looks different from the address used in the code because the transaction format encodes it in its raw
    hexadecimal form rather than the Base32 text.

* **Mosaics** (line 14): The assets transferred.
    Here, `1000000` atomic units of the mosaic aliased by `symbol.xym` (<XYM:>), equal to 1 XYM.

* **Announcement response** (line 18): The node accepted the payload.
    This does not yet mean the transaction is valid or included in a block.

* **Confirmed status** (line 25): The transaction has been accepted and included in a block.

The number of status checks before confirmation can vary based on network conditions,
and the initial `unknown` status may or may not appear,
depending on how quickly the node begins processing the transaction.

To see the transaction from the network's perspective, you can visit the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/) and search for the transaction hash.
The hash is printed in the line that says `Waiting for confirmation from /transactionStatus/...`.
You should see the transaction move through the confirmation process in real time.

Alternatively, you can search for the `signerPublicKey` to view the transaction in the history of the signer account.

## Conclusion

This tutorial showed how to:

| Step                                                    | Related documentation                                                               |
| ------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| [Obtain fee information](#fetching-recommended-fees)    | <get:/network/fees/transaction>                                                     |
| [Build a transaction](#building-the-transaction)        | <dy:SymbolFacade.createTransactionFromTypedDescriptor>, <ser:TransferTransactionV1> |
| [Sign the transaction](#signing-and-serializing)        | <dy:SymbolFacade.signTransaction><br/><dy:SymbolTransactionFactory.attachSignature> |
| [Announce the transaction](#announcing-the-transaction) | <put:/transactions>                                                                 |
| [Wait for confirmation](#waiting-for-confirmation)      | <get:/transactionStatus/{hash}>                                                     |

Other transaction types follow the same general process.
