---
title: Sending Funds and Messages
---

# Sending Funds and Messages

This page explains how to send funds and messages from an account in the Symbol Mobile Wallet App.

Funds and messages are sent using a <transfer transaction:>.
A transfer transaction can send one or more <mosaics:> to another account, optionally with a
[message](../../textbook/transfer_transactions.md#optional-message).
Messages can also be sent without transferring any mosaic.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* The sending account must have enough balance to cover the amount being sent and the transaction fee.

## How to Send Funds and Messages

Follow these steps to send a transfer transaction:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the **:fontawesome-regular-paper-plane: SEND** button
in the middle of the account box.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-1.webp") }}
Fill in the transfer form.  
Paste the recipient's address into the **RECIPIENT** box or use the **:fontawesome-regular-address-book: address book**
button inside the box to select one of your accounts.  
Select the mosaic to send, enter the amount, and optionally add a message.
Check **ENCRYPTED** if the message should be encrypted for the recipient.  
The **SEND** button remains disabled until the required fields are complete.  
Continue with the next step to see the completed form.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-2.webp") }}
After the required fields are filled in, the fee slider appears.  
Move the slider to choose the fee and confirmation speed.  
Smaller fees typically result in slower transactions but actual confirmation time depends on network conditions.  
Tap **SEND**.
The next screen will ask for confirmation.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-3.webp") }}
Review the transaction details.

If everything is correct, tap **CONFIRM**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-4.webp") }}
Wait while the app creates, signs, and announces the transaction.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-5.webp") }}
The app shows a success message when the transfer transaction is confirmed.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## Checking the Transaction History

You can check the history to see what happened.

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap **:fontawesome-solid-history: HISTORY** at the bottom.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-6.webp") }}
The history shows past transactions involving the selected account.  
In this example, **MY ACCOUNT** is selected, so the top transaction is the one just sent.  
Tap the transaction to open its details.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-7.webp") }}
Review the transaction details.  
The details show the transaction status, date, amount, sender, recipient, fee, and transaction hash.  
To see the message, tap the long arrow to expand the details.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-9.webp") }}
The expanded details show the message, the transferred mosaics, and the full sender and recipient addresses.  
To check the history from the recipient's point of view, you need to have it in your wallet.  
Return to the transaction history using the **:material-arrow-left: back arrow** and select the recipient account
from the account dropdown at the top.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-8.webp") }}
The history now shows transactions involving **Second account**.  
In this example, it only shows the received transfer.  
Tap the transaction to open its details.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-7.webp") }}
The transaction details are the same because both accounts are viewing the same transfer transaction.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
