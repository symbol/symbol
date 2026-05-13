---
title: Transfer
tutorial_level: beginner
---

# Creating a Transfer Transaction

<Transfer transactions:|Transfer transactions> are the most basic type of Symbol transaction.
They allow sending <XYM:> or any other type of <mosaic:> from one <account:> to another, optionally including a message.

This tutorial shows how to create, sign, and announce a transfer transaction, and then poll the transaction's status
until it is confirmed.
Required transaction parameters, such as the current time and fees, are fetched from the network to use the most
up-to-date values.

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

### Fetching Network Time

{{ tutorial.code_snippet_tagged('step-2') }}

Transactions on Symbol must include a deadline, which defines how long the network should attempt to confirm the
transaction before discarding it.
Deadlines are expressed in <network time:>, so the first step is to fetch the current network time from a <node:>.

If a transaction's deadline is earlier than the current network time or more than six hours in the future,
the transaction will be rejected.
To avoid this, you need to know the current network time before constructing the transaction, using the
<get:/node/time> endpoint.

However, applications do not need to query the network time before every transaction.
It can be fetched once and then adjusted using the local system clock when needed.
This provides a good balance between accuracy and performance.

### Fetching Recommended Fees

{{ tutorial.code_snippet_tagged('step-3') }}

Transactions on Symbol must pay a fee to incentivize nodes to include them in blocks.
If the fee is too low, no node may include the transaction.
If it is too high, the sender wastes funds.
In addition, each node may enforce a minimum fee threshold for incoming transactions.

The optimal fee depends on the current state of the network,
particularly the number of transactions being submitted and the fees they are offering.
To support fee estimation, Symbol provides the <get:/network/fees/transaction>
endpoint that returns a _recommended fee multiplier_ based on recent transaction activity.

The final fee is calculated by multiplying the recommended multiplier by the transaction's size in bytes.
This ensures that larger transactions pay proportionally more while smaller ones remain cost-effective.

Although applications can use a fixed fee for simplicity, it is more efficient to follow the network recommendation.
As with network time, there is no need to query the multiplier for every transaction,
but it should be refreshed regularly.

The snippet above takes the greater of the network's recommended multiplier (`medianFeeMultiplier`) and the
minimum multiplier (`minFeeMultiplier`) required by the node where the transactions will be sent.
The result is stored for later use once the transaction size is known.

### Building the Transaction

{{ tutorial.code_snippet_tagged('step-4') }}

All required transaction properties must be provided when building the transfer transaction.
The snippet includes the following fields:

* **Type**: Transfer transactions use the type `transfer_transaction_v1`.

* **Signer public key**: The signer is the account that will pay the fee.
    In a transfer transaction, it is also the source of the transferred mosaics.

* **Deadline**: This value is set to two hours after the current network time.

* **Recipient address**: In this example, the recipient is the same as the sender,
    which is useful for demonstration but not terribly practical.

* **Mosaics**: This is an array, because a transfer transaction can send multiple mosaics at once.
    Each entry includes a <mosaic ID:> and an amount.

    In the example, the mosaic ID for <XYM:> is obtained using its alias, `symbol.xym`, which is easier to remember
    than the full hexadecimal ID.

    Amounts are expressed in atomic units, which depend on the mosaic's <divisibility:>.
    For XYM, the divisibility is 6, so 1 XYM must be expressed as `1_000_000`.

Note that the `fee` field is not set in the descriptor.
Instead, the fee is calculated after the transaction is built, using the previously obtained multiplier and the
transaction's size in bytes, which is only known once the descriptor has been constructed.

!!! info "Including a message in the transaction"

    Transactions can optionally include a free-form message.
    The [Sending Messages with Transfer Transactions](./messages.md) explains how to do this.

### Signing and Serializing

{{ tutorial.code_snippet_tagged('step-5') }}

Once the transaction is created, it must be signed with the signing account's private key.
Signing ensures the transaction is authentic and authorized by the sender.

<dy:SymbolFacade.signTransaction> returns a <signature:> encoded as a hexadecimal string.

<dy:SymbolTransactionFactory.attachSignature> adds the signature to the transaction and serializes it into a
JSON payload ready to be submitted directly to a node for announcement.

### Announcing the Transaction

{{ tutorial.code_snippet_tagged('step-6') }}

Announcing a transaction is a simple request to the <put:/transactions> endpoint of any Symbol <API node:>.
As long as the payload is correctly formed, the request will succeed with an HTTP 200 response.

However, this response does **not** indicate that the transaction is valid or accepted by the network.
Validation, fee checks, and other rules are applied asynchronously after the transaction is received.

To confirm that the transaction is actually accepted and included in a block, its status must be monitored separately,
as shown in the next step.

### Waiting for Confirmation

{{ tutorial.code_snippet_tagged('step-7') }}

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

```text
--8<-- 'devbook/transactions/transfer.log'
```

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

| Step                                                     | Related documentation                                                               |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| [Obtain deadline information](#fetching-network-time)    | <get:/node/time>                                                                    |
| [Obtain fee information](#fetching-recommended-fees)     | <get:/network/fees/transaction>                                                     |
| [Create a transaction](#creating-a-transfer-transaction) | <dy:SymbolTransactionFactory.create>                                                |
| [Sign the transaction](#signing-and-serializing)         | <dy:SymbolFacade.signTransaction><br/><dy:SymbolTransactionFactory.attachSignature> |
| [Announce the transaction](#announcing-the-transaction)  | <put:/transactions>                                                                 |
| [Wait for confirmation](#waiting-for-confirmation)       | <get:/transactionStatus/{hash}>                                                     |

Other transaction types follow the same general process.
