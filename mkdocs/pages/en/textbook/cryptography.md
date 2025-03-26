# Cryptography Primer

Symbol uses [elliptic curve cryptography](https://en.wikipedia.org/wiki/Elliptic-curve_cryptography) to verify data integrity and to authenticate a signer's identity.

## Key pairs

Elliptic curve cryptography is based on **key pairs** composed of:

* **Private key**: A random 256-bit (32-byte) number.

    Its actual value is meaningless, and it is meant to be kept secret.
    It should be impossible to guess by unauthorized parties, and it is extremely unlikely that the same number will be generated twice by chance.

* **Public key**: A 256-bit (32-byte) number derived from the private key.

    It serves as the public identifier of the key pair and can be disseminated widely.
    It can be used to prove that the private key is known without revealing it.

    The public key can be derived from the private key, but not the other way around.

In particular, Symbol uses the [twisted Edwards curve](https://en.wikipedia.org/wiki/Twisted_Edwards_curve) with the digital signature algorithm named [Ed25519](https://ed25519.cr.yp.to) and hashing algorithm [SHA-512](https://en.wikipedia.org/wiki/SHA-2).

## Accounts

As in traditional banking, an account is an identifier that allows storing assets in a secure place.

In the case of blockchain, accounts are secured by key pairs, and assets can only be removed from an account by using its private key.
This is why private addresses should be kept private at all times.

In order to _receive_ assets, though, an account number can be shared freely, which is simply its public key, or, in a more convenient form, its address ([as shown below](#addresses)).

## Types of Keys Used by Symbol

Symbol uses key pairs in different places, for different purposes:

* The **Main Key** manages a regular account, containing assets like mosaics or namespaces.

* The **Remote Key** manages a remote account, used in remote harvesting.

* The **VRF Key** is required for harvesting.

* The **Voting Key** is required for nodes participating in the finalization process.

* The **Transport Key** is used by nodes for secure transport over [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security).

??? warning "Key Security"

    As a rule of thumb, the **private key** in any key pair should be kept secret at all times.

    The severity of having a secret key revealed depends on the purpose of that key:

    | Key           | Severity | Impact |
    | ------------- | -------- | ------ |
    | **Main**      | 🔴 HIGH  | Assets can be transferred out of the account. |
    | **Remote**    | 🟠 MED   | Harmless to the account or the node. Easily reverted by linking another remote account. An attacker grabbing a large number of remote keys could gain a lot of harvesting power, influencing which blocks are added to the blockchain. |
    | **VRF**       | 🟡 LOW   | Harmless without the key used for harvesting. |
    | **Voting**    | 🟠 MED   | Harmless to the account or the node. Easily reverted by linking another voting account. An attacker grabbing more than 50% of the network's voting keys could influence block finalization. |
    | **Transport** | 🟡 LOW   | An attacker could steal harvesting delegations away from the node, but harmless otherwise. |

## Signatures

All transactions on the blockchain are signed using private keys, producing 512-bit (64-byte) signatures.

Signatures are impossible to forge, and therefore certify that the owners of the private keys approve the transaction.

The signatures required for a transaction to be considered valid depend on the transaction type and its participants.
For example, transferring assets from one regular account to another only requires the signature of the source account's private key.

However, transferring assets from a multisig account requires the approval of all preconfigured signers, and must therefore gather multiple signatures before it can be considered valid.

## Addresses

A Symbol address is a convenient, shorter form of a public key, that simplifies sharing it.

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

Hierarchical Deterministic Wallets (HD-Wallets for short) derive a series of accounts from a single seed.
This allows handling a group of accounts using a single seed instead of multiple key pairs, greatly simplifying their management.

A **Mnemonic Phrase** is a human-friendly representation for the seed, composed of 24 random English words.
Since private keys are generated from the mnemonic phrase, it must be protected just as much.
