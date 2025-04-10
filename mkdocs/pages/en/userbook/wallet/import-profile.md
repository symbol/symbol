---
title: Importing a Profile
---

# Importing an Existing Profile

This page explains how to restore an **existing** profile from a previous installation of the Symbol Desktop Wallet, or even from another compatible wallet.  
To learn what is a profile, or to create a new one from scratch, follow the [Creating a Profile](./create-profile.md) tutorial instead.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you haven’t done that yet, see the [Installing the Wallet](./install.md) guide.

* Make sure you understand what is a <profile:>.

* You will need the <mnemonic phrase:> for the profile you want to restore.  
Check the documentation for the wallet you used to create it.  
If the profile was originally created in the Symbol Desktop Wallet, you can find the instructions in the [Exporting a Profile](./export-profile.md) tutorial.

## How to Import an Existing Profile

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-0.jpg") }}
Open the Symbol Desktop Wallet and click **Create a new profile?**
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-import-profile-1.jpg") }}
Select **Import Profile**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-import-profile-2.jpg") }}
Fill in your profile details:

Give your profile a name.  
This is just for your own reference to help you keep multiple profiles organized.

Select the network type (usually `Mainnet`).

Enter and confirm a password.

Optionally, add a password hint.

Click **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-import-profile-3.jpg") }}
Enter the mnemonic phrase for the profile to restore.

This is the secret phrase you received when you first created the profile.  
Make sure the words are in the correct order and separated by spaces, or the Next button will not be enabled.

Click **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-import-profile-4.jpg") }}
Select which accounts to restore.

The wallet will scan the mnemonic for associated accounts and let you choose which ones to include in the profile.

Some of the associated accounts might have never been used, so, to help you decide which ones to recover,
their current XYM balances are shown.

The ones you do not select now can be added later.
See the [Creating an Account](./create-account.md) tutorial.

Click **Next** when you are done selecting accounts.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-import-profile-5.jpg") }}
Read the safety tips, accept the Terms and Conditions, and click **Finish**.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken to the main screen of the wallet:

![Profile created successfully](screenshots/desktop-create-profile-8.jpg)

## Next Steps

Your imported profile is now ready to use.

* To learn how to add more accounts to the profile, see [Creating an Account](./create-account.md).
