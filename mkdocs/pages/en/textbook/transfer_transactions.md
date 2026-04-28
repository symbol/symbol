# Transfer Transactions

Transfer Transaction
:   A <transaction:> that allows sending mosaics and optional messages from one account to another.

Transfer transactions are the most common type of transaction on Symbol,
enabling both asset transfers and simple communication.

## Key Features

* **Mosaic Transfer**

    You can attach one or more <mosaics:> to a transfer transaction.
    All mosaics in a transfer transaction are sent from one sender to one recipient.
    This makes transfer transactions ideal for simple, direct asset transfers.

* **Message Support**

    Optional plaintext or encrypted messages can be included.
    A transfer transaction does not require mosaics, so messages can also be sent on their own.
    This allows for simple communication alongside the mosaic transfers.

* **Aggregate Compatibility**

    Multiple transfer transactions can be wrapped in an <aggregate transaction:>,
    allowing multiple senders and multiple recipients in a single transaction.

* **Multisig Compatibility**

    Transfer transactions, like all other transactions, support <multisignature accounts:>.
    This allows them to require authorization from more than one account, allowing complex governance schemes.

## Structure

Besides the [common transaction structure](./transactions.md#common-transaction-structure),
a transfer transaction contains the following attributes:

| Attribute                       | Description                                           |
| ------------------------------- | ----------------------------------------------------- |
| **Recipient's address**         | Address or namespace alias identifying the recipient. |
| **List of transferred mosaics** | Zero or more mosaics to transfer.                     |
| **Optional message**            | Plaintext or encrypted message, up to 1024 bytes.     |

### Recipient's Address

The recipient can be specified as an <address:> or a <namespace:> alias.

!!! warning "Assets sent to an unowned address are lost"

    It is possible to send mosaics to an address that has never appeared on-chain.

    This behavior is normal and supported,
    but care must be taken to ensure the recipient controls the destination address,
    because if no one has the <private key:> corresponding to it,
    the transferred mosaics will be permanently locked.

### List of Transferred Mosaics

This list includes the <mosaic ID:> (or <namespace:> alias) and number of units for each transferred mosaic.

Naturally, the sender must own enough units of each mosaic, or the transaction will be rejected.

The list may also be empty, allowing messages to be sent without transferring any assets.

### Optional Message

A message of up to 1024 bytes may be included in the transaction.

The Symbol protocol does not define a standard encoding for messages.
It is up to the sender and recipient to agree on a format or encoding scheme (e.g., UTF-8, JSON, hex).
By convention, most wallets and applications assume UTF-8 encoding.

!!! note "Encryption Convention"

    In order to support encryption, applications built on top of Symbol developed a _convention_ in which
    the first byte of the message indicates whether the rest is encrypted or not:

    * If the first byte is `0x00`, the following bytes contain an unencrypted message.
    * If the first byte is `0x01`, the following bytes contain a message encrypted using
        Bouncy Castle's AES block cipher in [CBC mode](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#CBC)
        and the recipient's public key, so that only the recipient can decrypt it.

    Note that this prefix byte reduces the maximum message length to 1023 bytes.

    This convention is supported by the [Symbol Desktop Wallet](../userbook/wallet/install.md),
    the [Symbol Explorer](https://symbol.fyi), and many other applications,
    but it is not part of the Symbol protocol itself.
