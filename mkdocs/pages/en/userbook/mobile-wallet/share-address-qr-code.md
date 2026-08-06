---
title: Sharing Your Address via a QR Code
---

# Sharing Your Address via a QR Code

This page explains how to share an <account:> address from the Symbol Mobile Wallet App using a
[QR code](https://en.wikipedia.org/wiki/QR_code).

An account's <address:> can be conveniently converted into a QR code that a second Symbol Mobile Wallet can scan.
The scanned address can then be used as the recipient for a one-off transaction, or added to the scanning wallet's
address book for later use.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* The second device must also have the Symbol Mobile Wallet installed.

## How to Share Your Address via a QR Code

Follow these steps to show an account address as a QR code and scan it with another device:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("share-address-0.webp") }}
From the wallet's **:material-home: HOME** screen, select the account you want to share from the account dropdown.  
In this example, **Second account** is selected.  
Tap **:fontawesome-regular-user: DETAILS** on the left of the account box.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-1.webp") }}
On the account details screen, tap **:octicons-download-24: RECEIVE**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-2.webp") }}
A QR code is shown for the account address.  
Keep it on screen.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
On the second device, go to the **:material-home: HOME** screen, tap **:material-line-scan: SCAN** at the bottom,
and point the camera at the QR code.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-3.webp") }}
The **SHARED INFORMATION** screen appears on the second device, showing the shared address and a list of available
actions.  
Tap **:fontawesome-regular-paper-plane: Send Transaction**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-4.webp") }}
The **TRANSFER** form is shown with the **RECIPIENT** field already filled in with the shared address.  
This guide stops here but you can complete the transaction by following
[Sending Funds and Messages](./send-funds-and-messages.md).
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
