# WebSockets

To get **live updates** when an event occurs on the blockchain, Symbol publishes
[WebSockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API).

Client applications can open a WebSocket connection and get a unique identifier.
With this identifier, applications can subscribe to any of the available channels instead of needing to constantly
poll the [REST API](../rest/symbol.md) for updates.

When an event occurs in a channel, the [REST Gateway](../../../textbook/nodes.md#rest-gateway) sends a notification to
every subscribed client in real-time.

WebSocket URIs share the same host and port as the HTTP requests URIs, but use the ``ws://`` protocol.
The endpoint is `/ws`, for example: `ws://localhost:3001/ws`

Both outgoing subscription messages and incoming updates use the JSON format and are described next.

!!! warning

    The WebSocket connection is dropped silently if idle for too long.

    Channels are not automatically resubscribed on reconnection.

## Response Format

All channels share the same response format, which is:

```json title="Response body"
{
    "topic": "{subscribed-channel}",
    "data": { ... }
}
```

* ``topic`` contains the name of the subscribed channel, so the same websocket can be used to monitor multiple channels
    (``topic`` matches the ``subscribe`` field provided in the request body when subscribing).
* ``data`` is a channel-specific object. Each channel listed below describes the data object it returns.

## Channels

### `block`

ws:block
:   Notifies subscribed clients every time **a new block is created**.
    Each returned message contains information about one block.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "block"
    }
    ```

=== "Response body"

    [BlockInfoDTO](../rest/symbol.md#model-BlockInfoDTO)

### `finalizedBlock`

ws:finalizedBlock
:   Notifies subscribed clients every time a set of blocks is <finalization|finalized>.
    Each returned message contains information about the **highest block** in the finalization round.
    All blocks with a smaller height are assumed finalized.

=== "Request body"

    ```json
        {
            "uid": "{uid}",
            "subscribe": "finalizedBlock"
        }
    ```

=== "Response body"

    [FinalizedBlockDTO](../rest/symbol.md#model-FinalizedBlockDTO)

### `confirmedAdded`

ws:confirmedAdded&#47;{address}
:   Notifies subscribed clients when a transaction related to the given address is included in a block.
    Each returned message contains information about one confirmed transaction.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "confirmedAdded/{address}"
    }
    ```

=== "Response body"

    [TransactionInfoDTO](../rest/symbol.md#model-TransactionInfoDTO)

### `unconfirmedAdded`

ws:unconfirmedAdded&#47;{address}
:   Notifies subscribed clients when a transaction related to the given address enters the unconfirmed state,
    waiting to be included in a block.
    Each returned message contains information about one unconfirmed transaction.

Possible scenarios when this message is received are:
a transaction is announced to the network via the <put:/transactions> HTTP endpoint or a <bonded aggregate transaction:>
has all required cosigners and changes its state from `partial` to `unconfirmed`.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "unconfirmedAdded/{address}"
    }
    ```

=== "Response body"

    [TransactionInfoDTO](../rest/symbol.md#model-TransactionInfoDTO)

### `unconfirmedRemoved`

ws:unconfirmedRemoved&#47;{address}
:   Notifies subscribed clients when a transaction related to the given address exits the `unconfirmed` state.
    Each returned message contains a no-longer-unconfirmed transaction hash.

Possible scenarios when this message is received are:
the transaction is now confirmed, or the deadline was reached and the transaction was not included in a block.

=== "Request body"

    ```json
    {
        "uid":"{uid}",
        "subscribe":"unconfirmedRemoved/{address}"
    }
    ```

=== "Response body"

    Hash of the transaction.

### `partialAdded`

ws:partialAdded&#47;{address}
:   Notifies subscribed clients when a <bonded aggregate transaction:> related to the given address enters the
    `partial` state, waiting for all required cosignatures to complete.
    Each returned message contains information about one added partial transaction.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "partialAdded/{address}"
    }
    ```

=== "Response body"

    [TransactionInfoDTO](../rest/symbol.md#model-TransactionInfoDTO)

### `partialRemoved`

ws:partialRemoved&#47;{address}
:   Notifies subscribed clients when a <bonded aggregate transaction:> related to the given address exits the
    `partial` state.
    Each returned message contains one removed partial transaction hash.

Possible scenarios when this message is emitted are:
all required <cosignatures:> were received and the transaction is now `unconfirmed`,
or the deadline was reached and the transaction was not included in a block.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "partialRemoved/{address}"
    }
    ```

=== "Response body"

    Hash of the transaction.

### `cosignature`

ws:cosignature&#47;{address}
:   Notifies subscribed clients when a <cosignature:> related to the given address is added to a
    <bonded aggregate transaction:> in the `partial` state.
    Each returned message contains one cosignature-signed transaction.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "cosignature/{address}"
    }
    ```

=== "Response body"

    [CosignatureDTO](../rest/symbol.md#model-CosignatureDTO)

### `status`

ws:status&#47;{address}
:   Notifies subscribed clients when a transaction related to the given address signals an error.
    Each returned message contains one error message and a transaction hash.

=== "Request body"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "status/{address}"
    }
    ```

=== "Response body"

    [TransactionStatusDTO](../rest/symbol.md#model-TransactionStatusDTO)
