---
title: Creating a Profile
---

# Creating a new Profile

## What Is a Profile

The Symbol Desktop Wallet uses profiles to group and manage related <account:|accounts>.  
For example, you can use different profiles to keep personal and work accounts separate.

Each profile is protected by its own password and stores the <key pair:|keys> and metadata needed to access its accounts.  
The wallet supports multiple profiles on the same device.

The Symbol Desktop Wallet is an <HD Wallet:>, which means it can generate multiple accounts from a single secret phrase.  
This phrase, also called a seed, is unique to each profile and is created during the profile setup process.

## Prerequisites

* Make sure you have installed the Symbol Desktop Wallet.  
If you haven’t done that yet, see the [Installing the Wallet](./install.md) guide.

* This page explains how to create a **new** profile from scratch.  
If you have a backup of a previous profile and want to restore it, follow the [Importing a Profile](./import-profile.md) tutorial instead.

## How to Create a Profile

Follow these steps to create a new profile in the Symbol Desktop Wallet:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin() }}
Open the Symbol Desktop Wallet and click **Create a new profile?**
{{ tutorial.step_end("screenshots/desktop-create-account-0.png") }}

{{ tutorial.step_begin() }}
Click **Create Mnemonic**.

To create a new profile, you need a new seed, which will generate the accounts in your profile.  
The wallet will generate a unique secret phrase (also called a mnemonic) for you.
{{ tutorial.step_end("screenshots/desktop-create-account-1.png") }}

{{ tutorial.step_begin() }}
Fill in your profile details:

* Give your profile a name.  
  This is just for your own reference to help you keep multiple profiles organized.
* Select the network type (usually `Mainnet`).
* Enter and confirm a password.
* Optionally, add a password hint.  
  This hint can help jog your memory if you forget the password.
* Click **Next**.
{{ tutorial.step_end("screenshots/desktop-create-account-2.png") }}

{{ tutorial.step_begin() }}
Move your mouse around the screen until the progress bar reaches 100%.

This motion is used as a source of randomness (entropy) to help generate a truly unique secret phrase.  
It ensures that no two users will ever end up with the same phrase by chance.
{{ tutorial.step_end("screenshots/desktop-create-account-3.png") }}

{{ tutorial.step_begin() }}
Click **Display mnemonic words** to view your secret phrase.
{{ tutorial.step_end("screenshots/desktop-create-account-4.png") }}

{{ tutorial.step_begin() }}
Click the copy button :octicons-copy-24: to save the phrase.

Paste the phrase somewhere secret and secure, such as an encrypted notes app or a password manager.

Ideally, also copy the phrase by hand onto a piece of paper and store it in a safe place.  
Avoid keeping it only on a digital device, which could be lost or compromised.
{{ tutorial.step_end("screenshots/desktop-create-account-5.png") }}

{{ tutorial.step_begin() }}
Confirm the phrase by selecting the words in the correct order. Click **Next** when finished.

This step checks that you have written down the phrase correctly and in the right order.
{{ tutorial.step_end("screenshots/desktop-create-account-6.png") }}

{{ tutorial.step_begin() }}
Read the safety tips, accept the Terms and Conditions, and click **Finish**.
{{ tutorial.step_end("screenshots/desktop-create-account-7.png") }}

{{ tutorial.list_end() }}

You will be taken to the main screen of the wallet:

![Profile created successfully](screenshots/desktop-create-account-8.jpg)

## Next Steps

Your new profile is now ready to use.

* To learn how to add more accounts to the profile, see [Creating an Account](./create-account.md).
