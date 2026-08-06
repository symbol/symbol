---
title: Exporting a Wallet
---

# Exporting a Wallet Backup Phrase

This page explains how to view and back up the <mnemonic phrase:> for the active wallet in the Symbol Mobile Wallet App.
You can use this phrase later to [import the wallet](./import-wallet.md) on the same device or another device.

!!! danger "Keep your mnemonic phrase secret"

    Anyone with your mnemonic phrase can control the accounts derived from it.

    Store it somewhere secure and offline, and never share it with anyone.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the Wallet](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* You need the wallet PIN code.

## How to Export a Wallet Backup Phrase

Follow these steps to view the mnemonic phrase for the active wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's main screen, tap the **:material-cog: Settings** button in the top-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-1.webp") }}
Tap **Security**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-2.webp") }}
Tap **SHOW MNEMONIC PHRASE**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
Enter your PIN code to unlock the wallet.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The mnemonic phrase will be shown on screen.
Write the words down in the exact order shown and store them somewhere safe, such as an offline backup location.

![Mnemonic phrase shown](export-wallet-4.webp){ .tutorial-result }

## Next Steps

Your wallet backup phrase is now backed up.

* Keep the backup secure and offline.
* Use the phrase to [import the wallet](./import-wallet.md) if you need to restore access later.
* Review the [Security](../security.md) guide for more advice on protecting wallet backups.
