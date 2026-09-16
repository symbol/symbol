---
title: Exporting an External Account
---

# Exporting an External Account

This page explains how to back up an <external account:> from the Symbol Mobile Wallet App by exporting its <private key:>.

<Seed accounts:|Seed accounts> are derived from the wallet's <mnemonic phrase:>, so they are backed up when you
[export the wallet backup phrase](./export-wallet.md).
External accounts are different: they are imported into the app from a private key, and they are not included in the
wallet's mnemonic phrase backup.
To back up an external account, export its private key manually as shown here.

!!! danger "Keep private keys secret"

    Anyone with an account's private key can control that account.
    Never share private keys, and store backups securely and offline.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* You need an external account in the active wallet.
    See [Importing an Account](./import-account.md) if needed.

* You need the wallet PIN code.

## How to Export an External Account

Follow these steps to reveal and copy the private key for an external account:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("export-account-1.webp") }}
From the wallet's **:material-home: HOME** screen, select the external account you want to export.

Tap **:fontawesome-regular-user: DETAILS** on the left of the account box.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-account-2.webp") }}
In the **DETAILS** box, check that **ACCOUNT TYPE** is `external`.

If it is not, follow the [Exporting a Wallet](./export-wallet.md) guide instead.

Tap **:octicons-key-24: REVEAL PRIVATE KEY**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
Enter your PIN code to unlock the wallet.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-account-4.webp") }}
The private key is shown.

Tap **:material-content-copy: COPY** to copy it.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The private key is now copied to the clipboard, but it is not backed up yet.
Store it somewhere secure and offline, or print it and keep the hardcopy in a safe place.

* Never send a private key by chat, email, or any other channel someone else can access.
* Keep private key backups in separate secure places.

## Next Steps

* Use [Importing an Account](./import-account.md) to import this private key into another wallet.
