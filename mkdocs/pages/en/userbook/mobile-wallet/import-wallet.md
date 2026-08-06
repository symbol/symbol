---
title: Importing a Wallet
---

# Importing an Existing Wallet

This page explains how to import an **existing** wallet into the Symbol Mobile Wallet App using a
<mnemonic phrase:>.
If you want to create a new wallet from scratch, follow the [Creating a Wallet](./create-wallet.md) guide instead.

See [the introduction](./intro.md) for more about wallets and accounts in the app.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* Make sure you have the mnemonic phrase for the wallet you want to import.

    Symbol uses the [BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) standard, which represents
    mnemonics as 24 English words selected from a standardized word list.

!!! warning "Only one wallet can be active at a time"

    The Symbol Mobile Wallet App handles one wallet at a time.
    If a wallet is already active, the option to import a wallet will not be shown.

    To import a different wallet, first [log out](./logout-wallet.md) from the current one.
    Before doing that, make sure you have [backed up](./export-wallet.md) the current wallet's
    <mnemonic phrase:> so you can recover it later if needed.
    If it contains <external accounts:>, also make sure you have [backed them up](./export-account.md).

## How to Import a Wallet

Follow these steps to import an existing wallet into the Symbol Mobile Wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-0.webp") }}
Open the Symbol Mobile Wallet and tap **Import Wallet**.

If you do not see the welcome screen, a wallet is already active and you need to [log out](./logout-wallet.md)
or [create an account](./create-account.md) inside it instead.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("import-wallet-1.webp") }}
Enter the mnemonic phrase for the wallet you want to import.

Make sure the words are in the correct order and separated by spaces.

Tap **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-wallet-3.webp") }}
Create a PIN code and confirm it.

This PIN protects access to the wallet on your device.

Choose one you can remember, but do not share it with anyone.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("import-wallet-2.webp") }}
Wait a few seconds while the app finishes setting up the wallet.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken to the **:material-home: HOME** screen:

![Wallet imported successfully](create-wallet-5.webp){ .tutorial-result }

Your imported wallet is ready to use.
You can share an account address to receive funds or use the app to send transactions.

## Next Steps

Your wallet is now available on this device.

* Keep your mnemonic phrase backup secure.
* You can [add more accounts](./create-account.md) to this wallet if needed.
* To restore external accounts, [import them separately](./import-account.md).
* Review the [Security](../security.md) guide before receiving or sending funds.
