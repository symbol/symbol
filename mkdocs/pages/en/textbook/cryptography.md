# Basic Cryptography

These are the basic cryptography concepts that underpin all of Symbol's technology.

## Hashes

Hash
:   A cryptographic hash is a fixed-size string of characters produced by a mathematical function
    (called a _hash function_) that maps input data of any size to a unique output.

Several such functions exist, such as [SHA‑512](https://en.wikipedia.org/wiki/SHA-2) or
[RIPEMD‑160](https://en.wikipedia.org/wiki/RIPEMD), but they all share the same essential properties:

* **Determinism**: The same input always produces the same hash.
* **Collision resistance**: It is extremely difficult to find two different inputs that produce the same hash.
* **Irreversibility**: The original input cannot be reconstructed from the hash.

These properties are critical for ensuring data integrity, verify authenticity,
and linking <blocks:> together in a blockchain.

## Keys

Private Key
:   A very long, secret number.
    The actual value of the private key is meaningless, and it is meant to be kept secret.
    It should be impossible to guess by unauthorized parties, and, although it is commonly randomly-generated,
    it is extremely unlikely that the same number is generated twice by chance.

Public Key
:   A very long number that serves as the public identifier of a <private key:> and can be disseminated widely.
    It can be used to prove that the private key is known without revealing it.

    Although mathematically derived from the private key, the reverse operation is practically impossible with
    current technology.

Key Pair
:   A matched set consisting of a <private key:> and its corresponding <public key:>.
    The private key is kept secret by the owner, while the public key is distributed openly.
    Together, they enable secure cryptographic operations such as digital signatures and encryption.

Symbol uses key pairs in different places, for different purposes:

Main Key
:   <Key pair:|Key pair> associated with every <account:>, identifying its owner.

Remote Key
:   <Key pair:|Key pair> associated with every <remote harvesting:> account.

VRF Key
:   <Key pair:|Key pair> used by <harvesting:> nodes to make their output unpredictable.

Voting Key
:   <Key pair:|Key pair> required for nodes participating in the <finalization:> process.

Transport Key
:   <Key pair:|Key pair> used by nodes for secure transport over [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security).

??? warning "Key Security"

    The **private key** in any key pair should be kept secret at all times.

    However, the severity of having a secret key revealed depends on the purpose of that key:

    | Key           | Severity | Impact |
    | ------------- | -------- | ------ |
    | **Main**      | 🔴 HIGH  | Assets can be transferred out of the account. |
    | **Remote**    | 🟠 MED   | Harmless to the account or the node. Easily reverted by linking another remote account. An attacker grabbing a large number of remote keys could gain a lot of harvesting power, influencing which blocks are added to the blockchain. |
    | **VRF**       | 🟡 LOW   | Harmless without the key used for harvesting. |
    | **Voting**    | 🟠 MED   | Harmless to the account or the node. Easily reverted by linking another voting account. An attacker grabbing more than 50% of the network's voting keys could influence block finalization. |
    | **Transport** | 🟡 LOW   | An attacker could steal harvesting delegations away from the node, but harmless otherwise. |

On Symbol, both the private and the public key are 256-bit (32-byte) integers.
The public key is obtained via [Elliptic Curve Cryptography](https://en.wikipedia.org/wiki/Elliptic-curve_cryptography)
using the [twisted Edwards curve](https://en.wikipedia.org/wiki/Twisted_Edwards_curve).

## Signatures

Signature
:   A digital attachment to a document that certifies that the document is approved by a given <account:>.

The signature is obtained by processing the document with the <private key:> of the account,
so that anybody can use the associated public key to verify that the signature matches the document,
but only the owner of the private key can produce an identical signature.

All transactions on Symbol are signed, but the signatures required depend on the transaction type and its participants.
For example, transferring assets from a single-owner account to another only requires the signature of the
source account's private key.

However, transferring assets from a <multisignature account:|multiple-owner account> requires the approval of all
preconfigured signers, and must therefore gather multiple signatures before it is considered valid.

Signatures on Symbol are 512-bit (64-byte) long and are generated using the [Ed25519](https://ed25519.cr.yp.to) and
[SHA‑512](https://en.wikipedia.org/wiki/SHA-2) algorithms.

## Addresses

Address
:   A convenient, shorter form of a <public key:>, that simplifies sharing it by requiring only
    letters and numbers. It's typically a synonym for <account:>.

Keys, both public and private, are binary data which is hard to print and share, whereas addresses are made up of
only latin letters and numbers.

Moreover, Symbol keys require 32 bytes of binary data, or 64 hexadecimal characters.
Addresses, on the other hand, only require 39 characters, reaching a compromise between length and practicality.

On Symbol, addresses are obtained from public keys by:

1. Generating a 24-byte **raw address** by joining:

    * A network ID byte: `N` for Symbol's main network, or `T` for Symbol's test network.
    * A 160-bit (20-byte) [RIPEMD‑160](https://en.wikipedia.org/wiki/RIPEMD) <hash:> of the public key.
    * A 3-byte checksum to detect mistyped addresses.

    Example: `0x78,0xD0,0x44,0xED,0xC3,0xDC,0x8B,0x86...`

2. Generating a 39-character **encoded address** by [Base32-encoding](https://en.wikipedia.org/wiki/Base32) the raw address.

    The encoded address is the most common way of sharing addresses because it only uses uppercase letters and numbers.

    Example: `PDIEJ3OD3SFYNZCQUSEWKY4NRRZUI5LMJPSVLPQ`

3. Optionally, for easier reading, hyphens can be added every 6 characters to create a 45-character **pretty address**.

    Example: `PDIEJ3-OD3SFY-NZCQUS-EWKY4N-RRZUI5-LMJPSV-LPQ`

!!! note "Address generation is an offline process"

    Note that address generation does not require interaction with the blockchain.

    In fact, Symbol only tracks addresses and associated public keys when they first appear in a transaction.

## Vanity addresses

While keys, and therefore <addresses:> too, are normally generated randomly, it is possible to create **vanity addresses**
that include specific patterns or prefixes.

This involves generating <key pairs:> repeatedly until one produces an address that meets the desired criteria.
The process usually requires substantial time and computation depending on the complexity of the pattern.

Vanity addresses can be useful for branding, visibility, or personal preference, but they offer no security advantage.
