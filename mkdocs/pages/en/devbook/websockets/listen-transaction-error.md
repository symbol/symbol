---
title: Transaction Errors
---

# Listening to Transaction Errors

The <ws:status&#47;{address}> WebSocket channel sends real-time notifications when a <transaction:> related to a
specific <account:> is rejected by the network.
Instead of polling the <get:/transactionStatus/{hash}> endpoint, the `status` channel pushes error details as soon as
the network rejects a transaction.

This tutorial shows how to subscribe to the `status` channel and handle rejection notifications.
To test the listener, the code deliberately sends an invalid transaction that the network will reject.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
* Have the address of the account to monitor.
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

{{ tutorial.code_full('devbook/websockets/listen-transaction-error', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API <node:>.
If no value is provided, a default one is used.
The WebSocket URL is derived from `NODE_URL` by replacing the HTTP protocol with the WebSocket protocol and appending
`/ws`.

## Code Explanation

### Setting Up the Monitored Address and Signer

{{ tutorial.code_snippet(['py:17:27', 'js:14:22']) }}

The `status` channel is scoped to a specific address.
The `MONITOR_ADDRESS` environment variable sets the address to watch.
The channel notifies whenever the address participates in a rejected transaction, whether as sender, recipient, or any
other role derived from the transaction's content (for example, signer of an embedded transaction in an
<aggregate transaction:>).
To trigger a rejection, this tutorial sends a transfer transaction with a non-existent mosaic, signed with the
private key from `SIGNER_PRIVATE_KEY`.

If any of these environment variables is not provided, the tutorial provides default values that correspond to the same
account.

### Connecting to the WebSocket

{{ tutorial.code_snippet(['py:31:35', 'js:25:33']) }}

The code opens a WebSocket connection to the node's `/ws` endpoint.
Upon connecting, the server sends a message containing a unique identifier (`uid`) that must be included in all
subsequent subscription requests.

See the [WebSocket reference](../reference/websockets/index.md) for details on the connection protocol.

### Subscribing to the Status Channel

{{ tutorial.code_snippet(['py:37:42', 'js:35:38']) }}

The code subscribes to the <ws:status&#47;{address}> channel scoped to the monitored address.
This channel notifies whenever a transaction involving the address is rejected by the network, providing the error code
and the transaction hash.

### Building and Signing an Invalid Transfer Transaction

{{ tutorial.code_snippet(['py:44:73', 'js:40:67']) }}

This tutorial builds a [Transfer Transaction](../transactions/transfer.md) sent to the monitored address, including
a mosaic with the alias `symbol.unknown`.
Since this mosaic does not exist on the network, the transaction will be rejected.

The transaction is built as usual: fetching the network time and fee multiplier, creating the transaction descriptor,
and signing it.
The hash is computed locally so it can be matched against the incoming WebSocket error message.

### Announcing and Waiting for the Error

{{ tutorial.code_snippet(['py:75:96', 'js:69:93']) }}

The code announces the transaction and then listens for incoming messages.
Each message follows the [TransactionStatusDTO](../reference/rest/symbol.md#model/TransactionStatusDTO) schema
and contains:

* **hash:** The hash of the rejected transaction.
* **code:** The error code explaining why the transaction was rejected.
    See the [TransactionStatusEnum](../reference/rest/symbol.md#model/TransactionStatusEnum) schema for all possible
    values.

When the received hash matches the announced transaction, the program prints the error code and exits.

### Unsubscribing from the Channel

{{ tutorial.code_snippet(['py:98:102', 'js:95:98']) }}

After receiving the error, the code sends an unsubscribe message before closing the connection.

## Output

```text linenums="1" hl_lines="2 3 4 5 6 7"
--8<-- 'devbook/websockets/listen-transaction-error.log'
```

The output shows:

* **Address** (line 2): The monitored address.
* **Connection** (line 3): The WebSocket connection is established and the server returns a unique `uid`.
* **Subscription** (line 4): The `status` channel is subscribed.
* **Announcement** (line 5): The transaction is announced and its hash is printed.
* **Error** (line 6): The network rejects the transaction with `Failure_Core_Insufficient_Balance` because the sender
    does not hold the requested mosaic.
* **Unsubscribe** (line 7): The code unsubscribes from the `status` channel.

## Conclusion

This tutorial showed how to:

| Step                                                                         | Related documentation                                                          |
|------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| [Subscribe to status channel](#subscribing-to-the-status-channel)            | <ws:status&#47;{address}>                                                      |
| [Trigger a rejection](#building-and-signing-an-invalid-transfer-transaction) | [Transfer Transaction](../transactions/transfer.md)                            |
| [Handle error messages](#announcing-and-waiting-for-the-error)               | [TransactionStatusDTO](../reference/rest/symbol.md#model/TransactionStatusDTO) |
