---
title: Create from Mnemonic
---

# Creating Accounts from Mnemonics

This tutorial shows how to create <accounts:> for the Symbol blockchain using a <mnemonic phrase:>.

This approach is commonly used by <HD wallets:> to manage multiple accounts from a single seed.

## Prerequisites

If you have not done so already, start with the [Hello World](../start/hello-world.md) tutorial to make sure your
development environment is set up correctly.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/accounts/create-from-mnemonic', ['py', 'js']) }}

## Code Explanation

### Initializing the Facade

{{ tutorial.code_snippet(['py:5:6', 'js:5:6']) }}

The <dy:SymbolFacade> provides access to Symbol's cryptographic operations and network utilities.
It is initialized with a network name (`testnet` or `mainnet`) to ensure that network-specific values,
such as <address:>, are generated correctly.

### Defining a Mnemonic

{{ tutorial.code_snippet(['py:8:16', 'js:8:17']) }}

The example checks for an existing mnemonic in the `MNEMONIC` environment variable.
If the variable is set, the mnemonic is loaded from it.
Otherwise, a new random mnemonic is generated using <dy:Bip32.random>.

Symbol uses the [BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) standard, which represents
mnemonics as 24 English words selected from a standardized word list.
These words encode entropy that serves as the foundation for deriving private keys.

!!! warning "Store your mnemonic securely"
    The mnemonic can be used to regenerate all derived accounts and private keys.
    Anyone with access to it can control your accounts, and losing it means losing access permanently.

    Never share your mnemonic with anyone, and always store it in a secure location.

### Deriving the Root Node

{{ tutorial.code_snippet(['py:18:23', 'js:19:24']) }}

After defining the mnemonic, <dy:Bip32.fromMnemonic> converts the mnemonic and a password into a root node,
which serves as the starting point for deriving child accounts.

The password (sometimes called a "25th word") is an optional string that extends the mnemonic seed.
It can be left empty or set to any value.
When used, it adds another layer of security. Different passwords with the same mnemonic produce completely different
accounts.

!!! note "Password security"
    The password is part of the account derivation.
    Both the mnemonic and password are required to regenerate the accounts.
    If you lose or forget either one, you lose access to all derived accounts.

In this example, the password is loaded from the `PASSWORD` environment variable.
If not set, the snippet defaults to `correcthorsebatterystaple` for demonstration purposes only.

### Deriving the Child Account

{{ tutorial.code_snippet(['py:25:27', 'js:26:28']) }}

The root node can derive multiple child accounts using a hierarchical path.
This allows a single mnemonic to manage many accounts while keeping them cryptographically isolated.

<dy:SymbolFacade.bip32Path> generates the derivation path for a specific account index.
In this example, the account at index `0` is derived, but you can derive additional accounts by using different
indices (e.g., `1`, `2`, `3`).

<dy:Bip32Node.derivePath> follows the specified path from the root node to produce a child node that represents
the account.

### Creating the Account

{{ tutorial.code_snippet(['py:29:38', 'js:30:39']) }}

Once the child node is derived, it is converted into a usable key pair and address.

1. **Key pair creation:** <dy:SymbolFacade.bip32NodeToKeyPair> extracts the <private key:> and <public key:> from the
   child node.
   The private key must remain secret, while the public key can be safely shared.

2. **Address derivation:** <dy:network.publicKeyToAddress> converts the public key into an <address:>, a shorter,
   human-readable, network-specific identifier for the account.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/accounts/create-from-mnemonic.log'
```

Each time the code runs without environment variables, it generates a different random mnemonic and account.
If the same mnemonic and password are provided, the same account is always derived for the given account index.

## Conclusion

This tutorial showed how to:

| Step                                                       | Related documentation                    |
| ---------------------------------------------------------- | ---------------------------------------- |
| [Define a mnemonic](#defining-a-mnemonic)                  | <dy:Bip32.random>                        |
| [Derive a root node](#deriving-the-root-node)              | <dy:Bip32.fromMnemonic>                  |
| [Derive a child account](#deriving-the-child-account)      | <dy:Bip32Node.derivePath>                |
| [Get the key pair](#creating-the-key-pair-and-address)     | <dy:SymbolFacade.bip32NodeToKeyPair>     |
| [Get the address](#creating-the-key-pair-and-address)      | <dy:network.publicKeyToAddress>          |
