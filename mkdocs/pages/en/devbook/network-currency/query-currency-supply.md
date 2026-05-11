---
title: Query Currency Supply
tutorial_level: beginner
---

# Querying Currency Supply

Exchanges and market data aggregators need accurate supply figures to display market capitalization and token metrics.

The Symbol network exposes the maximum, total, and circulating supply of <XYM:>, the native currency, through dedicated
REST endpoints.

This tutorial shows how to query each value and derive additional metrics from them.

## Prerequisites

This tutorial uses the [Symbol REST API](../reference/rest/symbol.md) without requiring an SDK.
You only need a way to make HTTP requests.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/network-currency/query_currency_supply', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default <testnet:> node is used.

!!! warning "Using a mainnet node"
    The default node points to testnet.
    For production supply data, set `NODE_URL` to a <mainnet:> node.
    For a list of available mainnet nodes, see [symbol.fyi/nodes](https://symbol.fyi/nodes).

## Code Explanation

### Fetching Supply Values

{{ tutorial.code_snippet_tagged('step-1') }}

Each supply value is available through a dedicated endpoint:

* <get:/network/currency/supply/max>: the hard cap for XYM, as configured in the network properties.
* <get:/network/currency/supply/total>: the total amount of XYM minted to date.
    New XYM is minted gradually through <inflation:> rewards with each new <block:>.
* <get:/network/currency/supply/circulating>: the total supply minus the balances held by the nemesis account,
    treasury accounts, and the sink accounts that collect [harvest](../../textbook/harvesting.md#sink),
    [mosaic rental](../../textbook/mosaics.md#lease-fee), and [namespace rental](../../textbook/namespaces.md#lease-fee)
    fees.

All three endpoints return a plain-text number (not JSON), already expressed in whole units with decimal places
(e.g. `8999999999.000000`), not in <divisibility:|atomic> units.

!!! warning "Circulating supply is node-dependent"
    The list of non-circulating accounts is configured by each node operator (in the node's `rest.json` file),
    so different nodes could report different circulating supply values.
    If you are integrating supply data, ensure you query a trusted node with the
    [default configuration](https://github.com/symbol/symbol/blob/dev/client/rest/resources/rest.json).

### Deriving Additional Metrics

{{ tutorial.code_snippet_tagged('step-2') }}

After fetching all three values, the code derives two additional metrics:

* **Non-circulating:** The difference between total and circulating supply.
* **Unminted:** The difference between maximum and total supply, representing the XYM that remains to be minted.

## Output

The following output shows a typical run querying the currency supply:

```text
--8<-- 'devbook/network-currency/query_currency_supply.log'
```

These values come from a <testnet:> node and do not reflect mainnet supply figures.

The output shows the full breakdown of the XYM supply:

* The **maximum supply** is the hard cap for XYM.
* The **total supply** is lower, because not all XYM has been minted yet.
* The **circulating supply** is lower still, because some minted XYM is held by non-circulating accounts.
* The **non-circulating** value accounts for the difference between total and circulating supply.
* The **unminted** value shows the remaining XYM that will be gradually minted through inflation rewards.

## Conclusion

This tutorial showed how to:

| Step                                                      | Related documentation                      |
| ----------------------------------------------------------| ------------------------------------------ |
| [Fetch maximum supply](#fetching-supply-values)           | <get:/network/currency/supply/max>         |
| [Fetch total supply](#fetching-supply-values)             | <get:/network/currency/supply/total>       |
| [Fetch circulating supply](#fetching-supply-values)       | <get:/network/currency/supply/circulating> |
| [Derive additional metrics](#deriving-additional-metrics) | -                                          |

## Next steps

To check a specific account's XYM balance, see the [Query Account Balance](../accounts/query-balance.md) tutorial.
