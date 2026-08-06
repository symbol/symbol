---
title: Creating an Account
---

# Creating a New Account

This page explains how to add a new <account:> to an existing <profile:> in the Symbol Desktop Wallet.  
You can manage multiple accounts within the same profile, all protected by a single password.

This is useful if you want to organize assets or separate different use cases, for example.

Creating accounts does not require spending any currency, and new accounts will not be reflected on the blockchain
until they participate in a transaction.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you have not done that yet, see the [Installing the Wallet](./install.md) guide.

* You must already have a profile set up and logged in.  
See [Creating a Profile](./create-profile.md) or [Importing a Profile](./import-profile.md) if needed.

## How to Create a New Account

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
From the wallet’s main screen, go to the **Accounts** tab.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-1.jpg") }}
Click **:material-plus-circle: Add an account** at the bottom of the screen.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-2.jpg") }}
Configure the new account by filling out the form.

Leave the default option selected in **Select the Type of Account**:
**"I want to create a seed account for my profile"**.

Enter a name in the **New Account Name** field.  
This label will help you recognize the account later.

In the **Password** field, enter your profile password to authorize the operation.

Then click **Confirm** to create the account.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The new account will be generated from your profile’s mnemonic phrase and added to your list of managed accounts,
in the **Seed accounts** section.

![New account created](screenshots/create-account-3.jpg)

Since this is a seed account, it can always be regenerated from any backup of the mnemonic phrase,
so its private key does not need to be backed up independently.

[Imported accounts](./import-account.md), on the other hand, are not derived from the mnemonic phrase
and must have their private keys backed up separately to avoid losing access.

[Exporting a profile](./export-profile.md) will back up both seed accounts and imported accounts in a
single <paper wallet:>.

## Next Steps

You can now:

* Share the new account’s address to receive funds.
* Use the account to send <transactions:> or interact with Symbol features like <mosaics:> and namespaces.
