---
title: Bridging Tokens
---

# Bridging Tokens

This page explains how to use the Symbol Mobile Wallet App to bridge tokens between Symbol and Ethereum.

Bridging lets you move value from one blockchain ecosystem to another.
For example, you can bridge `XYM` to Ethereum as `bXYM`, so it can be used in the wider Ethereum ecosystem.
The bridge can also swap <XYM:> for <ETH:>.

For the full explanation of bridge workflows, fees, processing time, and risks, see
[Bridging Symbol to Ethereum](../../textbook/bridge.md).

!!! warning "Bridge requests take time and include fees"

    A bridge request must be confirmed on the source network, detected by the bridge, processed, and paid out on the
    target network.
    Fees are deducted from the amount sent, and the final amount received can vary.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* The source account must have enough balance to cover the amount being bridged and the fees.

## How to Bridge Tokens

Follow these steps to bridge tokens:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("harvesting-0.webp") }}
From the wallet's **:material-home: HOME** screen, tap **ACTIONS** in the bottom-right corner.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-1.webp") }}
Tap **Network Bridge**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-2.webp") }}
If the wallet does not have an Ethereum account yet, a popup prompts you to create one.
This may happen the first time you use the bridge.

Tap **CONFIRM**.

If the wallet already has an Ethereum account, skip to step 6.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-3.webp") }}
Tap **ACTIVATE ACCOUNT**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-4.webp") }}
The Ethereum account is now available.

Tap the back arrow to return to the **ACTIONS** screen, then tap **Network Bridge** again.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-5.webp") }}
The **Swap Tokens** screen lets you choose what to bridge or swap.

Use the top dropdown to select the source token and the bottom dropdown to select the target token.
See [Bridging Symbol to Ethereum](../../textbook/bridge.md) for the available options.

Enter the amount.
The summary box shows:

* **You Send**: The amount sent from the source account.
* **Transaction Fee**: Estimate of the fee paid on the source network.
* **Bridge Fee**: Estimate of the fee deducted by the bridge.
* **You Receive**: The final amount received on the target network.

Fees are subtracted from the amount sent, and their final value is only known when the bridge processes the request.
Additionally, the bridge does not provide <slippage:> protection.

If the amount is not expected to cover the fees, **Amount too low** is shown and the **SEND** button is disabled.
Fees can be substantial on Ethereum.

Enter a valid amount and tap **SEND**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-6.webp") }}
Review the confirmation screen.

For example, when bridging `XYM` to `bXYM`, **SIGNER ADDRESS** is your source account and
**RECIPIENT ADDRESS** is your Ethereum account.

The **FEE** shown here is only the transaction fee, not the bridge fee.

If everything is correct, tap **CONFIRM**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
Enter your PIN code to unlock the wallet and authorize the transaction.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-7.webp") }}
Wait while the app creates, signs, announces, and confirms the transaction containing your request.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-8.webp") }}
The transaction is confirmed.

Tap **OK**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-9.webp") }}
The request has been sent to the bridge and is now pending.

Tap the transaction box at the bottom to see the details.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-10.webp") }}
The **Swap Details** screen shows the bridge request status.

At this point, the request is waiting for the bridge to process it, which requires the transaction to be finalized
first.
The process can take about 20 minutes.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-11.webp") }}
The bridge has picked up the request and is processing it.

This involves waiting for a transaction to confirm on the target network.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-12.webp") }}
The tokens are now available in the wallet.
Tap the back arrow to return to the **ACTIONS** screen.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-1.webp") }}
Tap **External Account**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-4.webp") }}
To see tokens stored in the external account other than <ETH:>, tap the account box to open its details.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-13.webp") }}
The **Bridge Account Details** screen shows the tokens held by the Ethereum account.

In this example, you can see the received `bXYM`.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
