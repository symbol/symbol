---
title: Fund via Faucet
---

# Getting Testnet Funds from the Faucet

The Symbol testnet provides a faucet that distributes free <XYM:> to developer <accounts:> for testing purposes.
This guide explains how to claim testnet funds using the web-based faucet.

!!! note
    Testnet XYM has no real-world value.
    It exists only to let you experiment with Symbol features without using real currency.

    If you need mainnet XYM, see available
    [exchanges](https://coinmarketcap.com/currencies/symbol/#Markets).

## Prerequisites

Before you start, make sure to:

- Create a testnet <account:> to receive funds.
  See [Creating an Account from a Private Key](../accounts/create-from-private-key.md).
- Have an X account to verify your identity with the faucet.

## How to Claim Testnet Funds

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("../../images/faucet-open.png") }}
Open your web browser and navigate to the Symbol testnet faucet at
**<https://testnet.symbol.tools/>**.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-sign-in.png") }}
Click **Sign in with Twitter** (now X) and follow the authentication flow.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-authorize.png") }}
After signing in, X will ask you to authorize the faucet application to access
your account information.

Review the permissions and click **Authorize app** to continue.
Once authorized, you will be redirected again to the faucet.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-address.png") }}
Enter your testnet address in **Your Testnet Address** field.
The address should start with `T` for testnet accounts.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-xym.png") }}
In the **XYM Amount** field, specify how much XYM you want to claim.
The maximum amount per request is 10,000 XYM.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-claim.png") }}
Click **Claim** to submit your request. If the request is successful, the faucet will transfer specified amount of XYM
to your address.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-view-explorer.png") }}
Click **View in Explorer** in the top-right corner notification to verify that the transaction was processed.

The explorer will display the transaction details, including its confirmation status.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## Returning Funds to the Faucet

When you are done testing, consider returning unused XYM back to the faucet.
The faucet address is the same address that sent you the funds.

You can find the sender address by checking the transaction in the blockchain explorer
or by searching your account transaction history.

Better yet, use the faucet address as the recipient for your test transactions.
This way, you practice sending transactions while helping keep the faucet stocked for other developers.

## Next Steps

Why not try [sending a transfer transaction](../transactions/transfer.md)?
