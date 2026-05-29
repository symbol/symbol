---
title: Bonded Transaction Flow
tutorial_level: advanced
---

# Listening to Bonded Transaction Flow

<Bonded aggregate transactions:|Bonded aggregate transactions> follow a richer lifecycle than regular transactions.
After being announced, they enter a `partial` state where the network receives <cosignatures:> from all required
participants.
Only after all cosignatures arrive does the transaction move through the standard `unconfirmed` and `confirmed` states.

This tutorial recreates the asset swap from the
[Bonded Aggregate Transaction](../transactions/bonded-aggregate.md) tutorial, but monitors the full bonded lifecycle
using [WebSocket](../reference/websockets/index.md) channels instead of polling.

Account A builds and announces the aggregate, while Account B subscribes to WebSocket channels, cosigns, and waits
for confirmation.

## Prerequisites

Before you start, make sure to set up your development environment.
See [Setting Up a Development Environment](../start/setup.md).

Additionally, install the language-specific WebSocket library:

=== ":simple-python: Python"

    Install the `websockets` library:

    ```bash
    pip install websockets
    ```

=== ":simple-javascript: JavaScript"

    This tutorial uses the native `WebSocket` API available in Node.js 22 or later.
    No additional packages are required.

You also need two <accounts:> with <XYM:> and one custom <mosaic:> to complete the swap.
Although pre-funded accounts are provided for convenience, they are not maintained and may run out of funds.

To use your own accounts, complete the following steps:

* Create an account (Account A) to initiate the aggregate transaction, either
  [from code](../accounts/create-from-private-key.md) or
  [by using a wallet](../../userbook/wallet/create-account.md).
* Create a second account (Account B) to participate in the swap.
* Obtain XYM for Account A to pay for the transaction fee, transfer amounts, and the hash lock deposit.
  See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).
* Create a mosaic owned by Account B for the swap.
  See [Creating a Mosaic](../mosaics/create-mosaic.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/websockets/listen_bonded_transaction_flow', ['py', 'js']) }}

A bonded aggregate transaction involves two distinct roles: an **initiator** (Account A) that builds, signs, and
announces the aggregate, and one or more **cosigners** (Account B and any additional cosigners) that monitor
WebSocket channels and cosign after verifying the transaction.

In practice, each role runs as a separate program on a separate machine, and all cosigners must already be
listening before the initiator submits the bonded aggregate.
This tutorial combines both roles in a single script for simplicity.

## Code Explanation

The code uses the `NODE_URL` environment variable to set the Symbol API <node:>.
If no value is provided, a default one is used.
The WebSocket URL is derived from `NODE_URL` by replacing the HTTP protocol with the WebSocket protocol and appending
`/ws`.

### Account A: Setting Up Accounts

{{ tutorial.code_snippet_tagged('step-1') }}

This example includes both <private keys:> in one script for simplicity.
In practice, each party signs on their own machine.
Account A only needs Account B's public key to build the aggregate, because B's <public key:> is required to set B as
the signer of an embedded transaction and to derive B's <address:>.

The `ACCOUNT_A_PRIVATE_KEY` and `ACCOUNT_B_PRIVATE_KEY` environment variables set the keys for each account.
If not provided, test keys are used by default.
If using your own keys, ensure Account A has XYM and Account B holds a custom mosaic for the swap.
The addresses are derived from the public keys using the facade's network configuration.

### Account A: Building the Aggregate and Announcing the Hash Lock

{{ tutorial.code_snippet_tagged('step-2') }}

Account A creates the bonded aggregate that swaps 10 XYM for 1 custom mosaic from Account B, signs it, and announces the
required hash lock, following the same pattern described in the
[Bonded Aggregate Transaction](../transactions/bonded-aggregate.md) tutorial.

The only difference is that instead of polling <get:/transactionStatus/{hash}> to confirm the hash lock, this tutorial
uses WebSockets, following the same approach described in the
[Listening to Transaction Flow](./listen-transaction-flow.md) tutorial.

### Account B: Connecting and Subscribing to Channels

{{ tutorial.code_snippet_tagged('step-3') }}

Account B opens a WebSocket connection and subscribes to channels scoped to its own address to monitor the bonded
transaction lifecycle.
Since Account B is a participant in the aggregate, the node delivers all lifecycle events for the transaction to
Account B's address.
In addition to the channels used for [regular transactions](./listen-transaction-flow.md), bonded aggregates
use extra channels:

* <ws:partialAdded&#47;{address}>: Notifies when a bonded aggregate enters the `partial` state, waiting for
    cosignatures.
* <ws:partialRemoved&#47;{address}>: Notifies when a bonded aggregate leaves the `partial` state (either all
    cosignatures were collected or the deadline expired).
* <ws:cosignature&#47;{address}>: Notifies when a cosignature is added to a partial transaction.

### Account A: Announcing the Bonded Aggregate

{{ tutorial.code_snippet_tagged('step-4') }}

Once Account B is subscribed, Account A announces the bonded aggregate to <put:/transactions/partial> (not the
regular <put:/transactions> endpoint).

### Account B: Handling WebSocket Messages and Cosigning

{{ tutorial.code_snippet_tagged('step-5') }}

Account B listens for incoming messages and dispatches them by channel.
The message schemas are the same as in the [regular transaction flow](./listen-transaction-flow.md) tutorial,
except for `cosignature` messages, which follow the
[CosignatureDTO](../reference/rest/symbol.md#model/CosignatureDTO) schema
and do not include the `meta.hash` field used by other channels.

The key action happens on `partialAdded`: when the hash matches the expected aggregate,
Account B cosigns the transaction using <dy:SymbolFacade.cosignTransactionHash> with the `detached`
parameter set to `true`, and announces the cosignature to <put:/transactions/cosignature>.
For deeper verification, Account B can fetch the full transaction from
<get:/transactions/partial/{transactionId}> and inspect its contents before deciding whether to cosign.

The expected message sequence for a successful bonded aggregate is described in the
[Transaction Lifecycle](../../textbook/transactions.md#transaction-lifecycle) section:

1. `partialAdded`: The bonded aggregate enters the partial cache, waiting for cosignatures.
2. `cosignature`: A cosignature from Account B is added.
3. `unconfirmedAdded`: The fully signed transaction enters the unconfirmed pool.
4. `partialRemoved`: The transaction leaves the partial state.
5. `unconfirmedRemoved`: The transaction leaves the unconfirmed pool.
6. `confirmedAdded`: The transaction is confirmed in a block.

### Account B: Unsubscribing from Channels

{{ tutorial.code_snippet_tagged('step-6') }}

After confirmation, Account B sends unsubscribe messages for all channels before closing the connection.

## Output

```text linenums="1" hl_lines="2-3 6 7 8-14 15 16-22"
--8<-- 'devbook/websockets/listen_bonded_transaction_flow.log'
```

The output shows:

* **Accounts** (lines 2-3): The addresses for Account A (initiator) and Account B (cosigner).
* **Hash lock** (lines 6): The bonded aggregate hash is computed, the hash lock is announced, and its confirmation
    is received via WebSocket.
* **Connection** (line 7): The WebSocket connection is established and the server returns a unique `uid`.
* **Subscriptions** (lines 8-14): All seven bonded transaction channels are subscribed (including `status`).
* **Announcement** (line 15): The bonded aggregate is announced to `/transactions/partial`.
* **Cosigning** (lines 16-18): The aggregate enters `partialAdded`, Account B submits a cosignature,
    and the `cosignature` channel confirms it was received.
* **Confirmation** (lines 19-22): The fully signed transaction enters the unconfirmed pool (`unconfirmedAdded`),
    leaves the partial state (`partialRemoved`), moves through `unconfirmedRemoved`, and is finally `confirmedAdded`.

## Conclusion

This tutorial showed how to:

| Step                                                                                        | Related documentation                                                       |
|---------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------|
| [Subscribe to partialAdded](#account-b-connecting-and-subscribing-to-channels)              | <ws:partialAdded&#47;{address}>                                             |
| [Subscribe to partialRemoved](#account-b-connecting-and-subscribing-to-channels)            | <ws:partialRemoved&#47;{address}>                                           |
| [Subscribe to cosignature](#account-b-connecting-and-subscribing-to-channels)               | <ws:cosignature&#47;{address}>                                              |
| [Handle transaction messages](#account-b-handling-websocket-messages-and-cosigning)         | [TransactionInfoDTO](../reference/rest/symbol.md#model/TransactionInfoDTO)  |
| [Handle cosignature messages](#account-b-handling-websocket-messages-and-cosigning)         | [CosignatureDTO](../reference/rest/symbol.md#model/CosignatureDTO)          |
| [Submit cosignatures on partialAdded](#account-b-handling-websocket-messages-and-cosigning) | <dy:SymbolFacade.cosignTransactionHash><br/><put:/transactions/cosignature> |
