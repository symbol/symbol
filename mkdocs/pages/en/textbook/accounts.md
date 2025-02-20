# Accounts

Account
:   A secure place where digital assets like cryptocurrencies or <NFT:|NFTs> can be stored.
    It functions similarly to a safe deposit box in traditional banking.

In the case of blockchain, accounts are secured by a <key pair:>: assets can only be **transferred out** of an account by using its private key, but the public key can be shared freely in order to **receive** assets.

Public keys are commonly shared as an <address:> for convenience, so the terms "account" and "address" are used as synonyms.

Besides managing digital assets, accounts also represent the ownership of a private key, and act as a form of digital identity.
On a blockchain, accounts can authorize transactions, configure permissions, and participate in consensus mechanisms.

!!! note "Account Lifecycle"

    Accounts become active the first time they interact with the blockchain, for example, by receiving assets.
    Prior to activation, no information about them is recorded on-chain and they do not appear in block explorers.

    Once activated, an account can be emptied of assets, but it cannot be deleted from the blockchain.

## Restrictions

Restriction
:   Rule that filter what types of transactions are allowed to or from an account.

Account restrictions can be of three types:

* **Address restrictions**: Block or allow transactions from specific <address:|addresses>.
* **Mosaic restrictions**: Block or allow transactions involving certain mosaics.
* **Operation restrictions**: Block or allow specific transaction types (e.g. transfers, metadata updates).

Restrictions help prevent spam, misconfiguration, and unauthorized access.
They are configured using dedicated transaction types and can only be modified by the account owner or cosignatories in the case of multisig accounts.

## Mnemonics

Mnemonic Phrase
:   A human-readable representation of a <key pair:|private key>, typically shown as a list of 12 or 24 random words.

    It is also called just Mnemonic, and often used when creating or restoring accounts in <HD Wallet:|HD wallets>.

Symbol uses the [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) standard that requires 24 English words.

!!! warning "Treat mnemonics as if they were private keys"

    Access to a mnemonic phrase provides full access to all accounts generated from it.
    Never share it, and avoid storing it unencrypted in digital form.

## Wallets

Wallet
:   An application used to manage Symbol accounts.

    It stores <key pair:|private keys> or <Mnemonic Phrase:|mnemonic phrases>, and uses them to signs transactions.
    More broadly, wallets provide tools for exploring and interacting with the blockchain.

Wallets can be:

* :material-application-outline: **Software wallets**

    Applications installed on desktop or mobile devices.

    These typically offer the full range of functionality, at an increased security risk:
    the software wallet must be online in order to interact with the blockchain, exposing the stored private keys to potential compromise, even if protected by a password.

* :material-integrated-circuit-chip: **Hardware wallets**

    External physical devices that store keys offline.

    These are designed primarily for secure transaction signing and must be connected to a software wallet to operate.

    The private keys they contain never leave the device except when explicitly backed up, making hardware wallets significantly more secure.

Most wallets allow managing multiple accounts, QR code scanning (for signing and requesting transaction signatures), metadata entry, and <multisignature account:|multisig> configuration.
Accounts can be also imported or exported using either <key pair:|private keys> or <mnemonic phrase:|mnemonic phrases>.

## HD Wallets

HD Wallet
:   A Hierarchical Deterministic (HD) <wallet:> derive a series of <account:|accounts> from a single seed, which allows handling a group of accounts using a single seed instead of multiple <key pair:|key pairs>.

    This greatly simplifies the management of multiple accounts, but extra caution must be taken to keep the seed safe because compromising the seed compromises all the accounts derived from it.
    The seed is typically a <mnemonic phrase:>.

    Most wallets are HD wallets.

Symbol uses the [BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki) standard to generate accounts from the seed.

## Metadata

Metadata
:   Structured data attached an <account:> or to other blockchain entities like mosaics and namespaces.

    Metadata on Symbol consists of key–value pairs and is stored on-chain.

Since metadata cannot be attached to an account without its explicit approval, its use cases include:

* Tagging accounts with identity information.
* Adding public contact or usage data.
* Certifying purpose or affiliations.

## Multisignature Accounts

Multisignature Account
:   An <account:> (called **multisig**) requiring signature from multiple parties (called **cosignatories**) to approve transactions.

Multisig accounts are configured by:

* Defining the list of cosignatories.
* Setting approval thresholds, i.e., how many signatures (**M**) out of the total number of cosignatories (**N**) are required to authorize an operation.

    This is known as an **M-of-N** multisig, and can be configured differently depending on the type of operation to authorize:

    * **Approval threshold**: number of required signatures to approve a regular transaction.
    * **Removal threshold**: number of required signatures to remove cosignatories from the multisig.

Example use cases:

* **Shared control over funds or functionality**.

    No action can be performed on the account without approval from all configured cosignatories.

    This also mitigates the risk of one of the accounts being compromised.

* **Multifactor authorization**.

    As a security measure, users can create a multisig so that they need to approve transactions from multiple devices.

* **Account ownership transfer**.

    Transferring private keys is not a viable mechanism to change ownership of an account, because the receiver can never be sure that the sender has deleted their copy of the keys.

    To solve this issue, the sender can configure the transferred account as a 1-of-1 multisig, and set the receiver account as the only cosignatory.

    The account can be transferred again by changing the single cosignatory as many times as needed.

Finally, cosignatories can also be other multisig accounts, enabling flexible, multi-layered authorization models.

Bear in mind the following constraints when designing multisignature solutions:

| Constraint                                        | Value |
| ------------------------------------------------- | ----: |
| Maximum number of cosignatories for an account    |    25 |
| Maximum number of accounts one account can cosign |    25 |
| Maximum layers                                    |     3 |
