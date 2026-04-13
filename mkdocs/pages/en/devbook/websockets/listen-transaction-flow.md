---
title: Transaction Flow
---

# Listening to Transaction Flow

Symbol provides WebSocket channels that send real-time notifications as a <transaction:> moves through the confirmation
process for a specific <account:>.
Compared to polling the <get:/transactionStatus/{hash}> endpoint, WebSockets push updates as they happen without the
overhead of repeated API calls.

This tutorial shows how to subscribe to transaction channels, announce a minimal
[Transfer Transaction](../transactions/transfer.md), and wait for its confirmation using WebSockets.

!!! note

    For a polling-based approach, see the
    [Monitoring Transaction Status](../transactions/monitoring-status.md) tutorial.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
* Have the address of the account to monitor.
* Have an account with enough balance for transaction fees.
  See [Creating an Account from a Private Key](../accounts/create-from-private-key.md) or
  [Creating an Account by Using a Wallet](../../userbook/wallet/create-account.md).

Additionally, install the language-specific WebSocket library:

=== ":simple-python: Python"

    Install the `websockets` library:

    ```bash
    pip install websockets
    ```

=== ":simple-javascript: JavaScript"

    This tutorial uses the native `WebSocket` API available in Node.js 22 or later.
    No additional packages are required.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/websockets/listen-transaction-flow', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API <node:>.
If no value is provided, a default one is used.
The WebSocket URL is derived from `NODE_URL` by replacing the HTTP protocol with the WebSocket protocol and appending
`/ws`.

## Code Explanation

### Setting Up the Monitored Address and Signer

{{ tutorial.code_snippet(['py:16:27', 'js:9:17']) }}

Each transaction WebSocket channel is scoped to a specific address.
The `MONITOR_ADDRESS` environment variable sets the address to watch.
The channel sends a notification whenever this address is involved in a transaction, whether as sender, recipient,
or any other role derived from the transaction's content (for example, signer of an embedded transaction in an
<aggregate transaction:>).

To trigger notifications, this tutorial sends a transfer transaction to the monitored address.
The sender's private key is read from `SIGNER_PRIVATE_KEY`.

If any of these environment variables is not provided, the tutorial provides default values that correspond to
the same account.

### Connecting to the WebSocket

{{ tutorial.code_snippet(['py:31:35', 'js:20:28']) }}

The code opens a WebSocket connection to the node's `/ws` endpoint.
Upon connecting, the server sends a message containing a unique identifier (`uid`) that must be included in all
subsequent subscription requests.

See the [WebSocket reference](../reference/websockets/index.md) for details on the connection protocol.

### Subscribing to Channels

{{ tutorial.code_snippet(['py:37:48', 'js:30:40']) }}

The code subscribes to three address-scoped channels, appending the monitored address to each channel name:

* <ws:unconfirmedAdded&#47;{address}>: Notifies when a transaction enters the <unconfirmed pool:>,
    waiting to be included in a block.
* <ws:unconfirmedRemoved&#47;{address}>: Notifies when a transaction leaves the unconfirmed state
    (either confirmed or expired).
* <ws:confirmedAdded&#47;{address}>: Notifies when a transaction involving the address is included in a <block:>.

Each subscription message includes the `uid` received during the connection step and the full channel name with the
monitored address.

### Building and Signing a Transfer Transaction

{{ tutorial.code_snippet(['py:50:75', 'js:42:65']) }}

This tutorial builds a minimal [Transfer Transaction](../transactions/transfer.md) to the monitored address, with no
mosaics and no message.
A transfer is used for simplicity, but any transaction type triggers the same WebSocket notifications.

The transaction is built as usual: fetching the network time and fee multiplier, creating the transaction descriptor,
and signing it.
The hash is computed locally so it can be matched against incoming WebSocket messages later.

### Announcing and Waiting for Confirmation

{{ tutorial.code_snippet(['py:77:98', 'js:67:95']) }}

The code announces the transaction and then listens for incoming messages, printing each one.

!!! warning "Announce after subscribing to channels"

    Always announce the transaction **after** subscribing to the WebSocket channels to ensure the listener is ready.
    Otherwise, notifications could arrive before the WebSocket is listening.

Each message includes a `topic` field identifying the channel and a `data` object with the event payload.

For `confirmedAdded` and `unconfirmedAdded` messages, the payload follows the
[TransactionInfoDTO](../reference/rest/symbol.md#model/TransactionInfoDTO) schema.
For `unconfirmedRemoved` messages, the payload contains only the transaction hash (`meta.hash`).

When a `confirmedAdded` message arrives whose hash matches the announced transaction, the program prints a confirmation
message and exits.

The expected sequence for a successful transaction is described in the
[Transaction Lifecycle](../../textbook/transactions.md#transaction-lifecycle) section:

1. `unconfirmedAdded`: The transaction enters the unconfirmed pool.
2. `unconfirmedRemoved`: The transaction leaves the unconfirmed pool.
3. `confirmedAdded`: The transaction is confirmed in a block.

### Unsubscribing from Channels

{{ tutorial.code_snippet(['py:100:106', 'js:97:102']) }}

After confirmation, the code sends unsubscribe messages for all three channels before closing the connection.

## Output

```text linenums="1" hl_lines="2 3 4-6 7 8-10 11 12"
--8<-- 'devbook/websockets/listen-transaction-flow.log'
```

The output shows:

* **Address** (line 2): The monitored address.
* **Connection** (line 3): The WebSocket connection is established and the server returns a unique `uid`.
* **Subscriptions** (lines 4-6): All three transaction channels are subscribed.
* **Announcement** (line 7): The transaction is announced and its hash is printed.
* **Transaction flow** (lines 8-10): The transaction moves from `unconfirmedAdded` to `unconfirmedRemoved` to
    `confirmedAdded`, showing the full confirmation lifecycle.
* **Confirmation** (line 11): The hash from `confirmedAdded` matches the announced transaction, confirming success.
* **Unsubscribe** (line 12): The code unsubscribes from all channels.

## Conclusion

This tutorial showed how to:

| Step                                                                    | Related documentation                                                      |
|-------------------------------------------------------------------------|----------------------------------------------------------------------------|
| [Subscribe to unconfirmedAdded](#subscribing-to-channels)               | <ws:unconfirmedAdded&#47;{address}>                                        |
| [Subscribe to unconfirmedRemoved](#subscribing-to-channels)             | <ws:unconfirmedRemoved&#47;{address}>                                      |
| [Subscribe to confirmedAdded](#subscribing-to-channels)                 | <ws:confirmedAdded&#47;{address}>                                          |
| [Handle transaction messages](#announcing-and-waiting-for-confirmation) | [TransactionInfoDTO](../reference/rest/symbol.md#model/TransactionInfoDTO) |

## Next Steps

To detect rejected transactions and their error codes, see the [Listening to Transaction Errors](listen-transaction-error.md) tutorial.
