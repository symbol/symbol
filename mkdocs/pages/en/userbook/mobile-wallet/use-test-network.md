---
title: Using the Test Network
---

# Using the Test Network

This page explains how to switch the Symbol Mobile Wallet App between <mainnet:> and <testnet:>.

Mainnet is the live Symbol network, where transactions use funds with real value.
Testnet is a separate network used for testing and learning.
It works like mainnet, but uses test funds that have no real-world value.

Using testnet is a good way to get acquainted with the blockchain without risking real funds.
You can get testnet <XYM:> from the [Getting Testnet Funds from the Faucet](../../devbook/accounts/testnet-faucet.md) guide.

!!! warning "Mainnet and testnet are separate"

    Mainnet and testnet are independent networks.
    Your accounts, balances, and transaction history on one network will not be the same on the other.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

## How to Use a Test Network

Follow these steps to switch between mainnet and testnet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the **:material-cog: Settings** button in the top-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-1.webp") }}
Tap **:octicons-database-24: Network**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("network-2.webp") }}
Tap the **NETWORK TYPE** dropdown.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("network-4.webp") }}
Select the network you want to use.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

After a few seconds, you are taken back to the **Network** screen.
The **CONNECTED NODE INFO** box will show updated information for the selected network.

Your mainnet and testnet accounts may be different, even when they come from the same wallet.
They belong to different networks and have independent balances and transaction histories.
