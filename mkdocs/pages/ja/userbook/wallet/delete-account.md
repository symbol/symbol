---
title: Deleting an Account
---

# Deleting an Account from a Profile

This page explains how to delete an existing <account:> from a <profile:> in the Symbol Desktop Wallet.

This action removes the account from the profile, but it does not remove the account from the blockchain.
If you have backed up the account’s private key, you can import it again later.

!!! warning
    If the account’s private key has not been backed up, access to any funds or assets it contains
    **will be permanently lost**.

    Make sure the account is empty or properly backed up before deleting it!

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you have not done that yet, see the [Installing the Wallet](./install.md) guide.

* You must already have a profile set up and logged in.  
See [Creating a Profile](./create-profile.md) or [Importing a Profile](./import-profile.md) if needed.

## How to Delete an Account

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
Go to the **Accounts** tab in the Symbol Desktop Wallet.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-1.jpg") }}
Click on the account you want to delete in the list on the left.

The currently selected account is the one with a colored icon.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-2.jpg") }}
Click the **Delete Account** button in the **Account Information** panel.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-3.jpg") }}
In the confirmation dialog, check the box to confirm that you want to delete the account.

Then click **Confirm**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-4.jpg") }}
Enter your profile password to authorize the deletion, and click **Confirm** again.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

The deleted account will no longer appear in the list of managed accounts.

![Account successfully deleted](screenshots/delete-account-5.jpg)

## Next Steps

* [Import the account again](./import-account.md) using the private key, if you have a backup.
* [Create a new account](./create-account.md).
