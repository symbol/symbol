# Exporting a Profile

This page explains how to export a <profile:> and all the accounts it contains from the Symbol Desktop Wallet.  
Exporting a profile lets you create a backup or use the same accounts on another device by [importing the profile](./import-profile.md) there.

The Symbol Desktop Wallet exports profiles as paper wallets:

Paper Wallet
:   A printable document containing information to restore a wallet profile and all its accounts.

    The information is intended to be printed on paper and may include QR codes, <key pair:|private keys>,
    <mnemonic phrase:|mnemonic phrases>, or all of them.

    Paper wallets are less convenient than digital backups because the information must be entered manually.  
    However, they are more secure since they are not stored or accessed online.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you haven’t done that yet, see the [Installing the Wallet](./install.md) guide.

* You must already have a profile set up in the Symbol Desktop Wallet, either by [creating a new one](./create-profile.md) or [importing an existing one](./import-profile.md).

## How to Export a Profile

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-0.jpg") }}
Open the Symbol Desktop Wallet and log in to the profile you want to export.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
Select the **Accounts** tab.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-2.jpg") }}
Click **:material-download: Backup Profile**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-3.jpg") }}
Enter your profile password to confirm access.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-4.jpg") }}
Click the **:material-download: Download** button.

You will receive a PDF document containing a <paper wallet:>.  
This document should be printed and then deleted from your device.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-5.jpg") }}
This image shows an example of a paper wallet containing an exported <mnemonic phrase:>.

You can use this phrase to restore all accounts derived from it.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-6.jpg") }}
This image shows an example of a paper wallet containing an individual account.

The account may have been generated from a mnemonic phrase or imported directly from a private key.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## Next Steps

All the accounts in your profile are now safely backed up in the paper wallet.

You can:

* Keep the backup in a safe place in case you ever need to restore access.
* Follow the [Importing a Profile](./import-profile.md) tutorial to restore it on a new device or
    installation of the Symbol Desktop Wallet.
