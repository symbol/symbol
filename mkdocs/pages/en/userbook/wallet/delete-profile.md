# Deleting a Profile

This page explains how to permanently delete a <profile:> from the Symbol Desktop Wallet.  
This action removes access to all <account:|accounts> managed by that profile on this device.

!!! note
    You are only deleting **your access** to the accounts.  
    The accounts themselves, along with any assets they contain, remain on the blockchain.  
    If the <key pair:|private keys> to those accounts have been copied elsewhere, the assets can still be accessed from another device or wallet.

If you just want to switch to a different profile, you don’t need to delete the current one.  
The wallet supports multiple profiles. You can simply log out and log into, or create, another profile.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you haven’t done that yet, see the [Installing the Wallet](./install.md) guide.

* If you want to keep access to the accounts from the profile you are deleting,
    [export the profile](./export-profile.md) first and store the backup in a safe location.

## How to Delete a Profile

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/delete-profile-0.jpg") }}
From the wallet’s main screen, click the **:material-cog: Settings** button in the top-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-profile-1.jpg") }}
Click **Delete Profile**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-profile-2.jpg") }}
Confirm that you want to delete the profile.

This action cannot be undone.  
Make sure you have a backup before proceeding.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

After the profile is deleted, you will be taken to the login screen.

## Next Steps

After deleting the profile, you can:

* [Create a new profile](./create-profile.md) to start fresh.
* [Import an existing profile](./import-profile.md) using a saved backup.
