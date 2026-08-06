---
title: Creating an Account
---

# Creating an Account

This page explains how to add another <account:> to the active wallet in the Symbol Mobile Wallet App.

Accounts created this way are **seed accounts**.
They are derived from the wallet's <mnemonic phrase:>, so the same wallet backup can recover them later.

If you want to bring an existing account from another wallet, you will need its <private key:> and the
[Importing an Account](./import-account.md) tutorial instead.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

## How to Create an Account

Follow these steps to add a seed account to the active wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the account dropdown at the top-left.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-account-1.webp") }}
Tap the **Add Account** button in the bottom-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-account-2.webp") }}
Enter a name for the new account.  
This label helps you recognize the account later.  
Select one of the available seed accounts.
All seed accounts shown here are derived from the wallet's mnemonic phrase.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The new account will be added to the account list:

![Account created successfully](create-account-3.webp){ .tutorial-result }

If the account has any balance, it will be shown in the account list.

## Next Steps

Your new account is now ready to use.
Select it to start using it.

* You can switch between accounts from the account dropdown on the **:material-home: HOME** screen.
* Follow [Deleting an Account](./delete-account.md) if you want to hide a seed account from the account list.
* Follow [Exporting a Wallet](./export-wallet.md) if you still need to back up the wallet's mnemonic phrase.
