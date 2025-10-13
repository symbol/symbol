---
title: Messages
---

# Sending Messages with Transfer Transactions

<Transfer transactions:> can include an optional message field, which allows attaching up to 1,024 bytes of data to the transaction.
Messages can be sent as plain text or encrypted using the recipient's public key, ensuring only the intended recipient can read them.

This tutorial shows how to send both plain and encrypted messages, and demonstrates how to decrypt received messages.

## Prerequisites

If you have not done so already, start with the [Hello World](../start/hello-world.md) tutorial to make sure your development environment is set up correctly.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/transactions/messages', ['py', 'js']) }}

## Code Explanation

### Setting Up Accounts

{{ tutorial.code_snippet(['py:14:32', 'js:10:28']) }}

The tutorial uses two accounts: a sender and a recipient.
The sender is loaded from the environment variable `PRIVATE_KEY`, while the recipient's public key is loaded from the environment variable `RECIPIENT_PUBLIC_KEY` (both use test keys by default).

In a real application, you typically have the recipient's address rather than their public key. To encrypt messages, you need to obtain their public key by querying the network using the <get:/accounts/{accountId}> endpoint. The public key is only available for accounts that have previously sent transactions. 

### Sending a Plain Text Message

{{ tutorial.code_snippet(['py:35:88', 'js:31:84']) }}

To attach a plain text message to a transfer transaction, encode the message as bytes and include it in the `message` field.

The transaction follows the same structure as a [basic transfer transaction](transfer.md), with the addition of the message field:

* **Message format:** Plain text messages are encoded directly as UTF-8 bytes.
* **Maximum size:** Messages cannot exceed 1,024 bytes. The network will reject transactions with larger messages.

    !!! tip "Handling larger data"

        For applications requiring more than 1,024 bytes of data, consider storing the data off-chain and including a hash or reference in the message field. This approach maintains data integrity verification while keeping transaction sizes manageable.

* **Network visibility:** Plain text messages are publicly visible on the blockchain. Anyone can read them by querying the transaction.


This example sends 1 XYM along with the message `"Hello, Symbol!"` to demonstrate that messages can be combined with mosaic transfers.

!!! info "Messages without mosaics"

	You can send a message without transferring any mosaics by providing an empty mosaics array.

### Sending an Encrypted Message

{{ tutorial.code_snippet(['py:93:139', 'js:89:133']) }}

Encrypted messages provide confidentiality by encrypting the message content using a shared secret derived from the sender's private key and the recipient's public key.
Both the sender and recipient can decrypt the message using their own private key and the other party's public key.

The encryption process uses the <dy:MessageEncoder> class:

1. **Create a MessageEncoder** with the sender's key pair.
2. **Encode the message** using the recipient's public key and the message bytes.
3. **Attach the encrypted payload** to the transaction's `message` field.

By convention, encrypted messages begin with a `0x01` byte to indicate encryption, followed by the encrypted content. This reduces the effective message size to 1,023 bytes.

!!! info "Message encryption is a convention, not a protocol feature"

	The Symbol protocol treats the message field as a byte array (max 1,024 bytes) without validating its content.
	
	By convention, applications use a prefix byte to indicate message encoding:
	
	* `0x00` - Unencrypted message
	* `0x01` - Encrypted message (using Bouncy Castle's AES block cipher in CBC mode)
	
	The SDK's `MessageEncoder` handles this convention, allowing both sender and recipient to decrypt messages using each other's public keys.
	
	See [Transfer Transactions](../../textbook/transfer_transactions.md#optional-message) in the textbook for details.

### Decrypting a Received Message

{{ tutorial.code_snippet(['py:141:154', 'js:135:149']) }}

To decrypt a message, create a <dy:MessageEncoder> with your key pair and use the <dy:MessageEncoder.tryDecode> method with the other party's public key and the encrypted payload.

In this example, the sender decrypts using their own key pair and the recipient's public key to verify the encryption.
The recipient would decrypt using their own key pair and the sender's public key (available in the transaction's `signerPublicKey` field).

The method returns a tuple `(is_decoded, message)` indicating whether decryption was successful.

If decryption succeeds, the message contains the original plaintext bytes, which you can decode as UTF-8.

If decryption fails, possible causes include:

* The message was encrypted for a different recipient.
* The message is corrupted or tampered with.
* The message is plain text, not encrypted.
* An incorrect public key was used for the other party.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/transactions/messages.log'
```

You can view the transactions on the [Symbol Testnet Explorer](https://testnet.symbol.fyi/) by searching for the transaction hashes printed in the output.

When you view an encrypted message transaction in the explorer, the message field will appear as hexadecimal data beginning with `01`.
The explorer cannot decrypt the message because it doesn't have access to the private keys.

## Conclusion

This tutorial showed how to:

| Step                                                           | Related documentation                     |
| -------------------------------------------------------------- | ----------------------------------------- |
| [Send a plain text message](#sending-a-plain-text-message)     | <dy:TransferTransaction> (`messsage` field) |
| [Send an encrypted message](#sending-an-encrypted-message)     | <dy:MessageEncoder.encode>                |
| [Decrypt a received message](#decrypting-a-received-message)   | <dy:MessageEncoder.tryDecode>             |

