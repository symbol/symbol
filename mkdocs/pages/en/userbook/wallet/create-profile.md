---
title: Creating a Profile
---

# Creating a new Profile

This page explains what is a wallet profile and how to create a **new** one from scratch.  
If you have a backup of a previous profile and want to restore it, follow the [Importing a Profile](./import-profile.md) tutorial instead.

## What Is a Profile

Profile
:   A password-protected container that stores multiple <account:|accounts> on a given device,
    similar to how profiles work in a browser or operating system.

The Symbol Desktop Wallet is an <HD Wallet:>, which means it can generate multiple accounts from a single <mnemonic phrase:>.  
This phrase, also called a seed, is unique to each profile and created during setup.

Each profile includes:

* The mnemonic phrase, from which account keys can be derived.
* The list of managed accounts, both derived from the phrase and imported externally.
* Network settings, such as whether the profile uses Mainnet or Testnet.
* A password to protect access and encrypt the profile contents.
* Optional metadata, such as a profile name and password hint, for user convenience.

Profiles keep things organized and separate, and you can have multiple profiles on the same machine.
For example, one for personal use and one for work, each with its own accounts and settings.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you haven’t done that yet, see the [Installing the Wallet](./install.md) guide.

## How to Create a Profile

Follow these steps to create a new profile in the Symbol Desktop Wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-0.jpg") }}
Open the Symbol Desktop Wallet and click **Create a new profile?**
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-1.jpg") }}
Click **Create Mnemonic**.

To create a new profile, you need a new seed, which will generate the accounts in your profile.  
The wallet will generate a unique secret phrase (also called a mnemonic) for you.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-2.jpg") }}
Fill in your profile details:

Give your profile a name.  
This is just for your own reference to help you keep multiple profiles organized.

Select the network type (usually `Mainnet`).

Enter and confirm a password.

Optionally, add a password hint.  
This hint can help jog your memory if you forget the password.

Click **Next**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-3.jpg") }}
Move your mouse around the screen until the progress bar reaches 100%.

This motion is used as a source of randomness (entropy) to help generate a truly unique secret phrase.  
It ensures that no two users will ever end up with the same phrase by chance.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-4.jpg") }}
Click **Display mnemonic words** to view your secret phrase.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-5.jpg") }}
Click the copy button :octicons-copy-24: to save the phrase.

Paste the phrase somewhere secret and secure, such as an encrypted notes app or a password manager.

Ideally, also copy the phrase by hand onto a piece of paper and store it in a safe place.  
Avoid keeping it only on a digital device, which could be lost or compromised.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-6.jpg") }}
Confirm the phrase by selecting the words in the correct order. Click **Next** when finished.

This step checks that you have written down the phrase correctly and in the right order.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/desktop-create-profile-7.jpg") }}
Read the safety tips, accept the Terms and Conditions, and click **Finish**.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken to the main screen of the wallet:

![Profile created successfully](screenshots/desktop-create-profile-8.jpg)

## Next Steps

Your new profile is now ready to use.

* To learn how to add more accounts to the profile, see [Creating an Account](./create-account.md).
