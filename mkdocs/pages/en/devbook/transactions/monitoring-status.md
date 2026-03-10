---
title: Transaction Status
---

# Monitoring Transaction Status

After announcing a <transaction:> to the Symbol network, it remains unconfirmed until it is included in a <block:>.

Monitoring status changes is essential for building responsive applications that can react to transaction
confirmation or failure.

This tutorial shows how to monitor a transaction's status as it moves from unconfirmed to confirmed.

!!! note "Confirmed transactions can still be reversed"
    A confirmed transaction has been included in a block but is not yet irreversible.
    The final state is <finalization:>, which occurs only after the block is finalized by the network.
    Until then, <rollbacks:> are still possible.

## Prerequisites

This tutorial uses the [Symbol REST API](../reference/rest/symbol.md) without requiring an SDK.
You only need a way to make HTTP requests.

## Full Code

This tutorial uses polling to check the transaction status.
Polling is used here for illustration purposes, but it is not the recommended approach for production applications.
WebSockets provide a more responsive solution without the overhead of repeated API calls.

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/transactions/monitoring-status', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default one is used.

## Code Explanation

### Finding the Transaction Hash

{{ tutorial.code_snippet(['py:13:16', 'js:7:8']) }}

To monitor a transaction, you need its hash, which is generated after signing.
The hash uniquely identifies the transaction on the Symbol network.

This tutorial uses a sample transaction hash to demonstrate the monitoring.
You can provide your own hash by setting the `TRANSACTION_HASH` environment variable when running the code.

In practice, you would obtain this hash immediately after signing a transaction (see the
[Transfer tutorial](./transfer.md) for an example) and use it to track its status.

### Querying the Status Endpoint

{{ tutorial.code_snippet(['py:21:57', 'js:12:55']) }}

The `wait_for_transaction_confirmation` function is the core of this tutorial.
It monitors a transaction until it is confirmed or fails.

It uses a `for` loop to check the transaction status up to 60 times by default (2 minutes with 2-second intervals
between attempts).
This loop structure ensures that monitoring will eventually stop, even if the transaction never confirms.

On each attempt, the function queries the <get:/transactionStatus/{hash}> endpoint, which returns information
about the transaction's current state.

The response includes:

* **Group:** The transaction's current status group. Possible values:

    | Group         | Meaning                                                                          |
    | ------------- | -------------------------------------------------------------------------------- |
    | `unconfirmed` | The transaction is in the <unconfirmed pool:> waiting to be included in a block. |
    | `confirmed`   | The transaction has been included in a block.                                    |
    | `failed`      | The transaction failed validation and has been rejected.                         |
    | `partial`     | For <bonded aggregate transactions:> waiting for <cosignatures:>.                |

* **Code:** A status code providing more details (for example, `Success` or specific error codes).
    See the [TransactionStatusEnum](../reference/rest/symbol.md#model-TransactionStatusEnum) schema for all possible
    values.
* **Hash:** The transaction hash being monitored.
* **Deadline:** The transaction's deadline in network time.

The function displays all these fields on each polling attempt so you can see how the transaction progresses through
states.

### Checking for Confirmation

{{ tutorial.code_snippet(['py:59:62', 'js:57:61']) }}

After parsing the response, the function checks the `group` field.
If it is `confirmed`, the transaction was successfully included in a <block:> through <harvesting:>, and the function
returns successfully.

### Checking for Failure

{{ tutorial.code_snippet(['py:64:71', 'js:63:69']) }}

If the transaction status group is `failed`, the function raises an error with the status code.

Common reasons include insufficient balance, invalid <signature:|signatures>, or deadline expiration.
Failed transactions are rejected during validation and will not be included in a block.

See [TransactionStatusEnum](../reference/rest/symbol.md#model-TransactionStatusEnum) for all possible codes.

### Handling Unknown Status

{{ tutorial.code_snippet(['py:73:80', 'js:71:80']) }}

If the endpoint returns HTTP 404, the transaction status is not yet available.
This can happen immediately after announcing a transaction, before the <node:> processes it, or if the hash is invalid.
The function handles this case by logging the attempt and continuing to poll.

For any other error (such as connectivity issues or failed transactions), the function re-raises the exception
immediately.

### Waiting Between Attempts

{{ tutorial.code_snippet(['py:82:84', 'js:82:87']) }}

Between polling attempts, the function waits for a configurable delay (default: 2 seconds).
This prevents overwhelming the <node:> with requests and allows time for network processing.

### Handling Timeouts

{{ tutorial.code_snippet(['py:86:89', 'js:90:95']) }}

If the transaction is not confirmed after the specified number of attempts, the function raises a `RuntimeError`
explaining the problem.

This ensures that the calling code is aware that the transaction didn't complete in the expected timeframe and can
take appropriate action, such as:

* Retrying the transaction announcement
* Alerting the user
* Logging the issue for investigation

## Output

The following output shows a typical run monitoring a transaction as it moves from unconfirmed to confirmed:

```text
--8<-- 'devbook/transactions/monitoring-status.log'
```

The output shows:

1. The transaction hash being monitored.
2. Multiple polling attempts showing the transaction status, code, hash, and deadline.
3. The status changing from `unconfirmed` to `confirmed` between attempts.
4. A success message when the transaction is confirmed.

The number of attempts and timing vary depending on network conditions and block production rate.
On the Symbol network, blocks are typically produced every 30 seconds, so you may see several `unconfirmed` status
responses before the transaction is confirmed.

Once confirmed, you can get additional details such as the block height where the transaction was included by querying
<get:/transactions/confirmed/{transactionId}> with the transaction hash.

To see the transaction from the network's perspective, visit the
[Symbol Testnet Explorer](https://testnet.symbol.fyi/) and search for the transaction hash.

## Conclusion

This tutorial showed how to:

| Step                                                       | Related documentation                                                            |
| ---------------------------------------------------------- | -------------------------------------------------------------------------------- |
| [Query the status endpoint](#querying-the-status-endpoint) | <get:/transactionStatus/{hash}>                                                  |
| [Check for confirmation](#checking-for-confirmation)       | [TransactionStatusEnum](../reference/rest/symbol.md#model-TransactionStatusEnum) |
| [Check for failure](#checking-for-failure)                 | [TransactionStatusEnum](../reference/rest/symbol.md#model-TransactionStatusEnum) |

## Next steps

For production applications, consider these improvements:

* **Wait for finalization:** Verify that the block containing the transaction has been finalized
    to ensure it is truly irreversible.
    See [Querying Chain and Finalization Height](../chain/chain-heights.md).
* **Prove transaction inclusion:** Verify cryptographically that a transaction is part of a block using a Merkle proof.
    See [Proving a Transaction's Inclusion in a Block](../chain/prove-transaction.md).
* **Query multiple nodes:** Check status and finalization across several <nodes:> for greater reliability and
    protection against single-node issues.
* **Use WebSockets:** Replace polling with WebSocket subscriptions for real-time updates without repeated API calls.
    See the [Listening to Transaction Flow](../websockets/listen-transaction-flow.md) WebSocket tutorial.
