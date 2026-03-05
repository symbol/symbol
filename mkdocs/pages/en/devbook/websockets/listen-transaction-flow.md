---
title: Transaction Flow
---

# Listening to Transaction Flow

Symbol provides WebSocket channels that send real-time notifications as a <transaction:> moves through the confirmation
process for a specific <account:>.
Compared to polling the <get:/transactionStatus/{hash}> endpoint, WebSockets push updates as they happen without
the overhead of repeated API calls.

This tutorial shows how to subscribe to all three channels and display updates as a transaction progresses from
unconfirmed to confirmed.

!!! note

    For a polling-based approach, see the
    [Monitoring Transaction Status](../transactions/monitoring-status.md) tutorial.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
* Have the address of the <account:> to monitor.
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

The program runs until interrupted with `Ctrl+C`, which triggers the unsubscribe step before closing the connection.

## Code Explanation

### Resolving the Monitored Address

{{ tutorial.code_snippet(['py:14:18', 'js:6:8']) }}

Transaction WebSocket channels are scoped to a specific address, so the code needs to know which <account:> to
monitor.
The channel notifies whenever the address participates in a transaction, whether as sender, recipient, or any other role
(for example, cosigner in an <aggregate transaction:>).

The address is read from the `MONITOR_ADDRESS` environment variable.

### Connecting to the WebSocket

{{ tutorial.code_snippet(['py:22:26', 'js:10:19']) }}

The code opens a WebSocket connection to the node's `/ws` endpoint.
Upon connecting, the server sends a message containing a unique identifier (`uid`) that must be included in all
subsequent subscription requests.

See the [WebSocket reference](../reference/websockets/index.md) for details on the connection protocol.

### Subscribing to Channels

{{ tutorial.code_snippet(['py:28:39', 'js:21:31']) }}

The code subscribes to three address-scoped channels, appending the monitored address to each channel name:

* <ws:confirmedAdded|confirmedAdded/{address}>: Notifies when a transaction involving the address is included in a <block:>.
* <ws:unconfirmedAdded|unconfirmedAdded/{address}>: Notifies when a transaction enters the <unconfirmed pool:>, waiting to be included
    in a block.
* <ws:unconfirmedRemoved|unconfirmedRemoved/{address}>: Notifies when a transaction leaves the unconfirmed state (either confirmed or
    expired).

Each subscription message includes the `uid` received during the connection step and the full channel name with
the monitored address.

### Handling Messages

{{ tutorial.code_snippet(['py:41:48', 'js:33:40']) }}

The code listens for incoming messages until the program is interrupted.
Each message includes a `topic` field identifying the channel and a `data` object with the event payload.

For `confirmedAdded` and `unconfirmedAdded` messages, the payload follows the
[TransactionInfoDTO](../reference/rest/symbol.md#model-TransactionInfoDTO) schema.
For `unconfirmedRemoved` messages, the payload contains only the transaction hash (`meta.hash`).

This tutorial extracts the transaction hash from each message to track the transaction's progression through states.
The expected sequence for a successful transaction is described in the
[Transaction Lifecycle](../../textbook/transactions.md#transaction-lifecycle) section:

1. `unconfirmedAdded`: The transaction enters the unconfirmed pool.
2. `unconfirmedRemoved`: The transaction leaves the unconfirmed pool.
3. `confirmedAdded`: The transaction is confirmed in a block.

### Unsubscribing on Exit

{{ tutorial.code_snippet(['py:50:57', 'js:42:50']) }}

When the program is interrupted (`Ctrl+C`), the code sends unsubscribe messages for all three channels before closing
the connection.
This ensures a clean disconnection from the node.

## Output

To test the listener, start the program in one terminal, then send a transaction involving the monitored address in a
separate terminal.
Any <transaction:> type triggers the WebSocket notifications, as long as the monitored address participates in it.

The following output shows the result of sending a [Transfer Transaction](../transactions/transfer.md) while the
listener is running:

```text linenums="1" hl_lines="2 4-6 7-9 10"
--8<-- 'devbook/websockets/listen-transaction-flow.log'
```

The output shows:

* **Address** (line 2): The monitored address.
* **Connection** (line 3): The WebSocket connection is established and the server returns a unique `uid`.
* **Subscriptions** (lines 4-6): All three transaction channels are subscribed.
* **Transaction flow** (lines 7-9): The transaction moves from `unconfirmedAdded` to `unconfirmedRemoved` to
    `confirmedAdded`, showing the full confirmation lifecycle.
* **Unsubscribe** (line 10): On `Ctrl+C`, the code unsubscribes from all channels.

## Conclusion

This tutorial showed how to:

| Step                                                        | Related documentation                                                      |
| ----------------------------------------------------------- | ---------------------------------------------------------------------------|
| [Subscribe to confirmedAdded](#subscribing-to-channels)     | <ws:confirmedAdded>                                                        |
| [Subscribe to unconfirmedAdded](#subscribing-to-channels)   | <ws:unconfirmedAdded>                                                      |
| [Subscribe to unconfirmedRemoved](#subscribing-to-channels) | <ws:unconfirmedRemoved>                                                    |
| [Handle transaction messages](#handling-messages)           | [TransactionInfoDTO](../reference/rest/symbol.md#model-TransactionInfoDTO) |

## Next Steps

To detect rejected transactions and their error codes, see the Listening to Transaction Errors tutorial.
