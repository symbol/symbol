---
title: Change Mosaic Supply
tutorial_level: beginner
---

# Changing Mosaic Supply

<Mosaics:|Mosaics> created with the `supply_mutable` flag can have their total supply increased or decreased after
creation.

This tutorial shows how to change a mosaic's supply.

## Prerequisites

Before you start, make sure to have:

* An <account:> that owns a mosaic with the `supply_mutable` flag set.
    See the [Creating a Mosaic](./create-mosaic.md) tutorial.
* <XYM:> to pay for the transaction fee.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

For more details, see [Supply Mutability](../../textbook/mosaics.md#supply-mutability) in the Textbook.

## Increasing Supply (Minting)

To mint new units, reuse the [supply change step](./create-mosaic.md#building-the-mosaic-supply-change-transaction)
from the Creating a Mosaic tutorial with these parameters:

1. Set `action` to `increase`.
2. Set `delta` to the number of atomic units to add.
    Remember that the delta is expressed in atomic units, so the mosaic's
    [divisibility](../../textbook/mosaics.md#divisibility) determines the conversion to whole units.

New units are added to the mosaic creator's account balance.

## Decreasing Supply (Burning)

To burn existing units, use the same <ser:MosaicSupplyChangeTransactionV1> type with these parameters:

1. Set `action` to `decrease`.
2. Set `delta` to the number of atomic units to remove.
    As with minting, the delta is expressed in atomic units based on the mosaic's
    [divisibility](../../textbook/mosaics.md#divisibility).

Units are removed from the mosaic creator's account balance.
If the creator does not hold enough units, the transaction fails with a validation error.
