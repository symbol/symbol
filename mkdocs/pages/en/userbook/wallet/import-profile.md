---
title: Importing a Profile
---

# Importing an Existing Profile

This page explains how to restore an **existing** <profile:> from a previous installation of the Symbol Desktop Wallet, or from another compatible wallet.  
To learn what a profile is, or to create a new one from scratch, follow the [Creating a Profile](./create-profile.md) tutorial instead.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you have not done that yet, see the [Installing the Wallet](./install.md) guide.

* Make sure you understand what a <profile:> is.

* You will need the <mnemonic phrase:> for the profile you want to restore.  
Check the documentation for the wallet you used to create it.  
If the profile was originally created in the Symbol Desktop Wallet, see the [Exporting a Profile](./export-profile.md) tutorial for instructions.

## How to Import an Existing Profile

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/create-profile-0.jpg") }}
Open the Symbol Desktop Wallet and click **Create a new profile?**
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-1.jpg") }}
Select **Import Profile**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-2.jpg") }}
Fill in your profile details:

Give your profile a name.  
This is just for your own reference to help you keep multiple profiles organized.

Select the network type (usually `Mainnet`).

Enter and confirm a password.

Optionally, add a password hint.

Click **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-3.jpg") }}
Enter the mnemonic phrase for the profile you want to restore.

This is the secret phrase you received when you first created the profile.  
Make sure the words are in the correct order and separated by spaces.  
The **Next** button will remain disabled until the phrase is valid.

Click **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-4.jpg") }}
Select which accounts to restore.

The wallet will scan the mnemonic for associated accounts and let you choose which ones to include in the profile.

Some of these accounts may have never been used.
To help you decide which ones to recover, their current XYM balances are shown.

You can add any accounts you do not select now at a later time.  
See the [Creating an Account](./create-account.md) tutorial.

Click **Next** when you are done selecting accounts.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-5.jpg") }}
Read the safety tips, accept the Terms and Conditions, and click **Finish**.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken to the main screen of the wallet:

![Profile created successfully](screenshots/create-profile-8.jpg)

## Next Steps

Your imported profile is now ready to use.

* To learn how to add more accounts to the profile, see [Creating an Account](./create-account.md).
