---
title: Chain and Finalization Height
tutorial_level: beginner
---

# Querying Chain and Finalization Height

The <get:/chain/info> endpoint returns the current chain height and the latest <finalization:> height.
Comparing both values shows how far behind the finalized state is from the chain tip, which is useful for
applications that need to confirm transactions are irreversible.

This tutorial shows how to poll chain state in a loop and display how long ago each height last changed.

## Prerequisites

This tutorial uses the [Symbol REST API](../reference/rest/symbol.md) without requiring an SDK.
You only need a way to make HTTP requests.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/chain/chain_heights', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default one is used.

The program runs in an infinite loop, printing a status line every second.
A keyboard interrupt (`Ctrl+C`) stops the loop.

## Code Explanation

### Fetching Chain Information

{{ tutorial.code_snippet_tagged('step-1') }}

On each iteration, the code sends a `GET` request to the <get:/chain/info> endpoint.
The response contains:

* **height:** The current chain height (the latest <block:> known to the node).
* **latestFinalizedBlock:** An object with details about the most recently finalized block, including:
    * **height:** The finalized block height.
    * **finalizationEpoch:** The finalization epoch number.
    * **finalizationPoint:** The finalization point within the epoch.
    * **hash:** The hash of the finalized block.

The chain height increases each time a new block is produced (approximately every 30 seconds).

The finalized height lags behind the chain tip because a block is typically finalized 10 to 20 minutes after it is
produced.
When a finalization round completes, the finalized height jumps forward by many blocks at once rather than
advancing one block at a time.
See the [Consensus](../../textbook/consensus.md#finalization) textbook section for details on how
<voting nodes:> drive this process.

### Tracking Height Changes

{{ tutorial.code_snippet_tagged('step-2') }}

To show how long ago each height last changed, the code stores the previous values and their timestamps.
When a height differs from the previous value, the timestamp is updated to the current time.

Until a change is observed, the timestamp remains unset and the output displays `-` instead of a number.
Once a change occurs, the counter starts from `0s ago` and increments each second until the next change.

### Polling Loop

{{ tutorial.code_snippet_tagged('step-3') }}

Each iteration prints a single status line showing:

* The current chain height and how many seconds have elapsed since it last changed.
* The finalized height and how many seconds have elapsed since it last changed.

The loop then sleeps for one second between iterations.

## Output

The following output shows a typical run monitoring the chain and finalization heights:

```text
--8<-- 'devbook/chain/chain_heights.log'
```

The output shows:

1. The chain height advances 1 block, from `3,159,411` to `3,159,412`.
    At that point, the change counter starts from `0s`.
2. Later, the finalized height catches up and advances 23 blocks, from `3,159,388` to `3,159,411`.
    Its change counter starts from `0s` too.
3. Both heights initially show `-` because no change has been observed yet.

The gap between the chain height and the finalized height is normal.
A transaction included in a block at the chain tip is confirmed but not yet irreversible.
Once the finalized height reaches or exceeds that block, the transaction is guaranteed to remain in the chain.

## Conclusion

This tutorial showed how to:

| Step                                                   | Related documentation                                      |
| ------------------------------------------------------ | ---------------------------------------------------------- |
| [Fetch chain information](#fetching-chain-information) | <get:/chain/info>                                          |

## Next steps

For an event-driven approach to monitoring new blocks and finalization, see the
[Listening to New Blocks](../websockets/listen-new-blocks.md) WebSocket tutorial.
