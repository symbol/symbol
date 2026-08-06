---
title: Logging Out
---

# Logging Out from a Wallet

This page explains how to log out from the active wallet in the Symbol Mobile Wallet App.

Logging out removes the wallet data from this device.
The accounts and assets still exist on the blockchain, but you will need the wallet's <mnemonic phrase:>
to recover access later.

!!! warning "Back up your wallet before logging out"

    Make sure you have backed up the wallet's <mnemonic phrase:> before logging out.

    If you do not have a backup, follow the [Exporting a Wallet](./export-wallet.md) guide first.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* You need the wallet PIN code.

## How to Log Out from a Wallet

Follow these steps to log out from the active wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the **:material-cog: Settings** button in the top-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-1.webp") }}
Tap **Logout**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("logout-wallet-2.webp") }}
Read the confirmation message.

If your mnemonic phrase is backed up, tap **Confirm**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
Enter your PIN code to unlock the wallet and confirm the logout.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken back to the welcome screen:

![Welcome screen](create-wallet-0.webp){ .tutorial-result }

## Next Steps

You are now logged out from the wallet on this device.

* To restore the same wallet later, follow [Importing a Wallet](./import-wallet.md).
* To start fresh, follow [Creating a Wallet](./create-wallet.md).
