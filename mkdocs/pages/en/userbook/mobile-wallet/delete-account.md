---
title: Deleting an Account
---

# Deleting an Account

This page explains how to hide or delete an <account:> from the active wallet in the Symbol Mobile Wallet App.

The wallet's main account, which is the first seed account, cannot be deleted.
Other seed accounts can be hidden from the account list, but they cannot be removed permanently because they are
derived from the wallet's <mnemonic phrase:>.
You can recover a hidden seed account later by following [Creating an Account](./create-account.md).

Only imported external accounts can be truly deleted.
If an imported account's <private key:> is not backed up, deleting it can make the account permanently inaccessible.
Follow [Exporting an External Account](./export-account.md) first if you need to back it up from the app.

## Prerequisites

* Make sure you have installed the Symbol Mobile Wallet.
    If you have not done that yet, see the [Installing the App](./install.md) guide.

* You must already have a wallet set up and unlocked in the app.
    See [Creating a Wallet](./create-wallet.md) or [Importing a Wallet](./import-wallet.md) if needed.

* If you are deleting an imported account, make sure you have backed up its private key.

!!! warning "Imported accounts need a separate backup"

    Imported accounts are not included in the wallet's mnemonic phrase backup.
    If you delete an imported account without backing up its private key, you can lose access to it.

## How to Hide or Delete an Account

Follow these steps to hide a seed account or delete an imported account:

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
From the wallet's **:material-home: HOME** screen, tap the account dropdown at the top-left.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("import-account-3.webp") }}
Tap the button next to the account icon on the right of the account you want to remove.

For seed accounts, :material-eye-off: hides the account.
For imported accounts, :material-delete: removes the account from the app.
{{ tutorial.step_end() }}

{{ tutorial.step_begin("delete-account-2.webp") }}
If you are removing an imported account, review the confirmation message.

If the private key is backed up, tap **CONFIRM**.
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

You will be taken back to the updated account list:

![Account removed from the list](create-account-3.webp){ .tutorial-result }

## Next Steps

The account is no longer shown in the account list.

* To show a hidden seed account again, follow [Creating an Account](./create-account.md).
* To add an external account again, follow [Importing an Account](./import-account.md).
