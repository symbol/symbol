# Cryptography Primer

These are the basic cryptography concepts that underpin all of Symbol's technology.

## Public-Key Cryptography

Key pair
:   A pair formed from a very long, secret random number (the private key) and a public number derived from it (the public key).

    The actual value of the **private key** is meaningless, and it is meant to be kept secret.
    It should be impossible to guess by unauthorized parties, and it is extremely unlikely that the same number is generated twice by chance.

    The **public key** serves as the public identifier of the key pair and can be disseminated widely.
    It can be used to prove that the private key is known without revealing it.
    The public key can be derived from the private key, but not the other way around.

Symbol uses key pairs in different places, for different purposes:

* A **Main Key** is associated with every <account:>, identifying its owner.

* A **Remote Key** is associated with every remote-harvesting account.

* A **VRF Key** heightens the security of harvesting nodes.

* A **Voting Key** is required for nodes participating in the finalization process.

* A **Transport Key** is used by nodes for secure transport over [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security).

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
The public key is obtained via [Elliptic Curve Cryptography](https://en.wikipedia.org/wiki/Elliptic-curve_cryptography) using the [twisted Edwards curve](https://en.wikipedia.org/wiki/Twisted_Edwards_curve).

## Accounts

Account
:   A secure place where digital assets like cryptocurrencies or <NFT:|NFTs> can be stored.
    Similar to a deposit box in traditional banking.

In the case of blockchain, accounts are secured by a <key pair:>: assets can only be **removed** from an account by using its private key, but the public key can be shared freely in order to **receive** assets.

Public keys are commonly shared as [addresses](#addresses), for convenience.

## Signatures

Signature
:   A digital attachment to a document that certifies that the document is approved by a given <account:>.

    The signature is obtained by processing the document with the <key pair:|private key> of the account, so that anybody can use the associated public key to verify that the signature matches the document, but only the owner of the private key can produce an identical signature.

All transactions on Symbol are signed, but the signatures required depend on the transaction type and its participants.
For example, transferring assets from a single-owner account to another only requires the signature of the source account's private key.

However, transferring assets from a multiple-owner account requires the approval of all preconfigured signers, and must therefore gather multiple signatures before it can be considered valid.

Signatures on Symbol are 512-bit (64-byte) long and are generated using the [Ed25519](https://ed25519.cr.yp.to) and [SHA-512](https://en.wikipedia.org/wiki/SHA-2) algorithms.

## Addresses

Address
:   A convenient, shorter form of a <key pair:|public key>, that simplifies sharing it.

    Keys, both public and private, are binary data which is hard to print and share, whereas addresses are made up of only latin letters and numbers.

Addresses are obtained from public keys by:

1. Generating a 24-byte **raw address** by joining:

    * A network ID byte: `N` for Symbol's main network, or `T` for Symbol's test network.
    * A 160-bit (20-byte) hash of the public key.
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

## HD-Wallets and Mnemonics

HD-wallet
:   Hierarchical Deterministic Wallets derive a series of <account:|accounts> from a single seed, which allows handling a group of accounts using a single seed instead of multiple <key pair:|key pairs>.

    This greatly simplifies the management of multiple accounts, but extra caution must be taken to keep the seed safe.
    The seed is typically a <mnemonic phrase:>.

Symbol uses the [BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki) standard to generate accounts from the seed.

Mnemonic phrase
:   A human-friendly representation for the seed of an <HD-wallet:>, composed of random English words.

    Since private keys are generated from the mnemonic phrase, it must be protected just as much.

Symbol uses the [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) standard that requires 24 English words.
