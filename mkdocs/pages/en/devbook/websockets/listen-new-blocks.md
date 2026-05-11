---
title: New Blocks
tutorial_level: beginner
---

# Listening to New Blocks

The <ws:block> and <ws:finalizedBlock> WebSocket channels send real-time notifications when a new <block:> is produced
or [finalized](../../textbook/consensus.md#finalization).
Compared to polling the <get:/chain/info> endpoint, WebSockets push updates as they happen without the overhead of
repeated API calls.

This tutorial shows how to subscribe to both channels and display each update as it arrives.

!!! note

    For a polling-based approach, see the
    [Querying Chain and Finalization Height](../chain/chain-heights.md) tutorial.

## Prerequisites

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

{{ tutorial.code_full_tagged('devbook/websockets/listen_new_blocks', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API <node:>.
If no value is provided, a default one is used.
The WebSocket URL is derived from `NODE_URL` by replacing the HTTP protocol with the WebSocket protocol and appending
`/ws`.

The program runs until interrupted with `Ctrl+C`, which triggers the unsubscribe step before closing the connection.

## Code Explanation

### Connecting to the WebSocket

{{ tutorial.code_snippet_tagged('step-1') }}

The first step is to open a WebSocket connection to the node's `/ws` endpoint.
Upon connecting, the server sends a message containing a unique identifier (`uid`) that must be included in all subsequent
subscription requests.

See the [WebSocket reference](../reference/websockets/index.md) for details on the connection protocol.

### Subscribing to Channels

{{ tutorial.code_snippet_tagged('step-2') }}

The code subscribes to two channels:

* <ws:block>: Notifies every time a new block is produced (approximately every 30 seconds).
* <ws:finalizedBlock>: Notifies every time a finalization round completes (approximately every 10 to 20 minutes).

Each subscription message includes the `uid` received during the connection step and the name of the channel.

### Handling Messages

{{ tutorial.code_snippet_tagged('step-3') }}

The code listens for incoming messages until the program is interrupted.
Each message includes a `topic` field identifying the channel and a `data` object with the event payload.

For `block` messages, the payload follows the [BlockInfoDTO](../reference/rest/symbol.md#model/BlockInfoDTO) schema.
This tutorial uses two of them to identify each block:

* `data.block.height`: The height of the new block.
* `data.meta.hash`: The hash of the new block.

For `finalizedBlock` messages, the payload follows the
[FinalizedBlockDTO](../reference/rest/symbol.md#model/FinalizedBlockDTO) schema.
This tutorial uses:

* `data.height`: The finalized block height.
* `data.hash`: The hash of the finalized block.

The chain height increases each time a new block is produced.
The finalized height lags behind the chain tip because finalization typically occurs 10 to 20 minutes after block
production.

See the [Consensus](../../textbook/consensus.md) section in the textbook for details on how <voting nodes:> drive
this process.

### Unsubscribing on Exit

{{ tutorial.code_snippet_tagged('step-4') }}

When the program is interrupted (`Ctrl+C`), the code sends unsubscribe messages for both channels before closing the
connection.
This ensures a clean disconnection from the node.

## Output

The following output shows a typical run listening to new blocks and finalization events:

```text linenums="1" hl_lines="2 3 4 5 8 11"
--8<-- 'devbook/websockets/listen_new_blocks.log'
```

The output shows:

* **Connection** (line 2): The WebSocket connection is established to the `wss://` URL and the server returns a unique
    `uid`.
* **Subscriptions** (lines 3-4): Both the `block` and `finalizedBlock` channels are subscribed.
* **New blocks** (lines 5-7, 9-10): New block notifications arrive approximately every 30 seconds.
* **Finalization** (line 8): A finalization notification arrives when a finalization round completes,
    covering multiple blocks at once.
* **Unsubscribe** (line 11): On `Ctrl+C`, the code unsubscribes from both channels.

## Conclusion

This tutorial showed how to:

| Step                                                       | Related documentation                                                          |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------ |
| [Subscribe to block channel](#subscribing-to-channels)     | <ws:block>                                                                     |
| [Subscribe to finalized channel](#subscribing-to-channels) | <ws:finalizedBlock>                                                            |
| [Handle block messages](#handling-messages)                | [BlockInfoDTO](../reference/rest/symbol.md#model/BlockInfoDTO)                 |
| [Handle finalized messages](#handling-messages)            | [FinalizedBlockDTO](../reference/rest/symbol.md#model/FinalizedBlockDTO)       |
