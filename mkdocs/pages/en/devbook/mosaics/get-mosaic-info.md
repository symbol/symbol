---
title: Get Mosaic Information
tutorial_level: beginner
---

# Getting Mosaic Information

Every <mosaic:> on Symbol has a set of on-chain properties such as supply, divisibility, and behavior flags.

This tutorial shows how to retrieve a mosaic's properties and any <namespace:> aliases linked to it.

## Prerequisites

This tutorial only reads data from the network. No <account:> is required.

Before you start, make sure to [set up your development environment](../start/setup.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/mosaics/get_mosaic_info', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default <testnet:> node is used.

The `MOSAIC_ID` environment variable specifies which mosaic to query.
If not set, it defaults to the <XYM:> <mosaic ID:> on testnet (`72C0212E67A08BCE`).

## Code Explanation

### Fetching Mosaic Information

{{ tutorial.code_snippet_tagged('step-1') }}

The <get:/mosaics/{mosaicId}> endpoint retrieves the current properties of a mosaic, including:

* **Supply:** The total number of [atomic](../../textbook/mosaics.md#divisibility) units currently in circulation.
    Do not confuse with the [initial supply](../../textbook/mosaics.md#initial-supply).
* **Divisibility:** The number of <divisibility:|decimal places> the mosaic supports.
    For example, XYM has a divisibility of `6`, meaning 1 XYM equals 1,000,000 atomic units.
* **Flags:** A bitmask encoding the mosaic's behavior restrictions.
    Each flag occupies a single bit:
    [`supply_mutable`](../../textbook/mosaics.md#supply-mutability) (1),
    [`transferable`](../../textbook/mosaics.md#transferability) (2),
    [`restrictable`](../../textbook/mosaics.md#restrictability) (4),
    and [`revokable`](../../textbook/mosaics.md#revocability) (8).
    Multiple flags combine additively. For example, a value of `6` means `transferable` (2) + `restrictable` (4).
* **Duration:** The [number of blocks](../../textbook/mosaics.md#duration) the mosaic remains active.
    A value of `0` means the mosaic never expires.
* **Start height:** The <block:> height at which the mosaic was created.
* **Revision:** Incremented each time the mosaic definition is modified.

### Formatting the Supply

{{ tutorial.code_snippet_tagged('step-2') }}

The supply value returned by the API is expressed in <divisibility:|atomic> units.
To convert it to whole units, the code divides the supply into whole and fractional parts
using the mosaic's divisibility.

For XYM (divisibility `6`), a supply of `8325447775994408` atomic units equals `8325447775.994408` whole units.

### Fetching Namespace Aliases

{{ tutorial.code_snippet_tagged('step-3') }}

Mosaics can be linked to human-readable namespace aliases.
The <post:/namespaces/mosaic/names> endpoint accepts mosaic IDs and returns any namespace names currently linked to
them.

A mosaic can have multiple namespace aliases if different namespaces link to the same mosaic.
If no namespace is linked, the response indicates that no aliases exist.

## Output

The output shown below corresponds to a typical run of the program, querying the XYM mosaic on testnet.

```text linenums="1" hl_lines="5 6 7 8 9 10 11 13 16"
--8<-- 'devbook/mosaics/get_mosaic_info.log'
```

Some highlights from the output:

* **Mosaic ID** (line 5): The XYM mosaic identifier on testnet (`72C0212E67A08BCE`).

* **Supply** (line 6): The total supply in atomic units.

* **Divisibility** (line 7): The value `6` means 1 XYM = 1,000,000 (10^6^) atomic units.

* **Flags** (line 8): The value `2` resolves to `transferable`, meaning XYM can be freely sent between accounts.

* **Duration** (line 9): The value `0` means XYM never expires.

* **Supply in whole units** (line 13): The supply converted from atomic units to whole units using the mosaic's
divisibility.

* **Namespace alias** (line 16): The mosaic is linked to the `symbol.xym` namespace.

## Conclusion

This tutorial showed how to:

| Step                                                      | Related documentation           |
| --------------------------------------------------------- | ------------------------------- |
| [Fetch mosaic properties](#fetching-mosaic-information)   | <get:/mosaics/{mosaicId}>       |
| [Fetch namespace aliases](#fetching-namespace-aliases)    | <post:/namespaces/mosaic/names> |

## Next Steps

* [Prove a mosaic definition](../chain/prove-mosaic-definition.md) to verify the data matches what is recorded on chain
* [Query an account balance](../accounts/query-balance.md) to see how much of a mosaic an account holds
