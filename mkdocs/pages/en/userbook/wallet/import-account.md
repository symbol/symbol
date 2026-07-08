---
title: Importing an Account
---

# Importing an Existing Account

This page explains how to add an existing <account:> to your profile by importing its private key.  
Imported accounts are not derived from the profile’s <mnemonic phrase:>, so they must be backed up separately.

You might want to import an account if:

* You created it in another wallet or on another device.
* You are recovering an account that was not generated from your profile's seed.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you have not done that yet, see the [Installing the Wallet](./install.md) guide.

* You must already have a profile set up and logged in.  
See [Creating a Profile](./create-profile.md) or [Importing a Profile](./import-profile.md) if needed.

## How to Import an Account

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
From the wallet’s main screen, go to the **Accounts** tab.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-1.jpg") }}
Click **:material-plus-circle: Add an account** at the bottom of the screen.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-account-2.jpg") }}
In the **Select the Type of Account** dropdown, choose **"I want to import an existing account private key"**.

Enter a name in the **New Account Name** field to help you recognize the account later.

Paste the private key into the **Enter Your Private Key** field.

Enter your profile password into the **Password** field to authorize the import.

Then click **Confirm** to import the account.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The imported account will be added to your list of managed accounts, in the **Private key accounts** section.

![New account created](screenshots/import-account-3.jpg)

!!! warning
    Imported accounts are not recoverable from your <mnemonic phrase:>.

    Be sure to back up your profile now so their private keys are included in the <paper wallet:>.

## Next Steps

You can now:

* [Export the profile](./export-profile.md) to back up both seed and imported accounts in a single paper wallet.
* Use the imported account to receive and send transactions.
