---
title: Importing an Account
---

# Importing an Account

This page explains how to import an existing <account:> into the active wallet in the Symbol Mobile Wallet App
using its <private key:>.

Imported accounts are <External accounts:>.
They are not derived from the wallet's <mnemonic phrase:>, so they cannot be recovered from the wallet backup.
Make sure you keep a [separate backup of the private key](./export-account.md).

If you want to create another account from the wallet's mnemonic phrase instead, follow
[Creating an Account](./create-account.md).

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* Make sure you have the private key for the account you want to import.
    Private keys are 64 hexadecimal characters: numbers and the letters `a` to `f`.
    The letters can be uppercase or lowercase.

!!! warning "Back up the private key"

    External accounts are not included in the wallet's mnemonic phrase backup.
    If you lose the private key, you can lose access to the account.

## How to Import an Account

Follow these steps to import an external account into the active wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the account dropdown at the top-left.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-account-3.webp") }}
Tap the **Add Account** button in the bottom-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-account-2.webp") }}
Tap **:octicons-key-24: ADD EXTERNAL ACCOUNT**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("import-account-2.webp") }}
Enter a name for the account and paste its private key.
The **CONFIRM** button will not be enabled if the private key has the wrong format.  
Review the warning, then tap **CONFIRM**.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The imported account will be added to the account list:

![Account imported successfully](import-account-3.webp){ .tutorial-result }

If the account has any balance, it will be shown in the account list.

## Next Steps

Your imported account is now available in the app.
Select it to start using it.

* Keep the private key backup secure and separate from the wallet's mnemonic phrase backup.
* Follow [Exporting an External Account](./export-account.md) if you need to back up this private key from the app later.
* You can switch between accounts from the account dropdown on the **:material-home: HOME** screen.
* Follow [Deleting an Account](./delete-account.md) if you want to remove an imported account from the app.
