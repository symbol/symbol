# Security

Cryptocurrency users are frequently targeted by scammers, hackers, and other malicious actors.
Some attacks use familiar techniques such as impersonation or misinformation.
Others exploit the unique properties of blockchain systems.

This page summarizes common threats and practical security considerations for protecting Symbol <accounts:> and assets.

!!! warning "This list is not exhaustive"
    Users are encouraged to research security practices further and remain vigilant when managing their accounts.

## Key Points

* **Be extremely cautious when signing transactions during delegated harvesting activation**

    A [sweeping](#sweeping-with-aggregate-bonded-transactions) attack has previously targeted users
    activating delegated harvesting.
    Carefully review any transaction before signing it, particularly if it includes transfers to
    unfamiliar addresses.

    Never sign a transaction that is not fully understood.

* **Never share your <private key:> or <mnemonic phrase:>**

    Also known as a recovery phrase or seed phrase.
    These credentials provide full control over an account.
    Anyone with access to them can transfer all assets from the account.

    Don't share it with anyone, and never enter them on any website or unknown wallet.

* **The Symbol Syndicate will never contact users privately for support**

    Community members and maintainers will never ask for private keys or mnemonic phrases.

* **Blockchain <transactions:> are irreversible**

    Once confirmed, transactions cannot be reversed and funds cannot be recovered from malicious recipients.

* **Store backups securely and offline**

    Any backup of private keys or mnemonic phrases should be protected from theft, loss, or unauthorized access.

## Sweeping with Aggregate Bonded Transactions

### Aggregate transactions

An <aggregate transaction:> can include multiple embedded transactions signed by different participants.
In particular, <bonded aggregate transactions:> require additional signatures before they are confirmed.
Users must therefore carefully review the entire transaction before adding their <cosignature:>.

### Sweeping Attacks

A sweeping attack uses automated scripts that monitor transactions announced to the network.

When a target action is detected, such as <delegated harvesting:> activation, the attacker quickly
announces a malicious <bonded aggregate transaction:>.
This transaction may appear related to the original action but includes an embedded transfer that
sends <XYM:> or other mosaics to the attacker.

If the victim signs the transaction, the attacker can empty their account.

Because the transaction appears to be part of a legitimate process, victims may initially assume
that the balance change is related to delegated harvesting activation or another pending operation.

### Example Attack Scenario

* A user begins delegated harvesting activation by announcing the Link all keys transaction.
* The attacker detects this transaction and announces a malicious bonded aggregate transaction.
* The transaction may include a harvesting-related message such as "Delegated-№234567", together
    with a transfer of XYM to the attacker.
* The user's wallet detects the bonded transaction and prompts the user to review and cosign it.
* If the user signs the transaction, the XYM balance is transferred to the attacker.

Different wallets handle bonded transactions differently:

* Some wallets prevent users from cosigning aggregate transactions.
* Some hide transactions originating from unknown addresses.
* Others prompt the user to sign without displaying the full contents.

For additional verification, aggregate transactions can be inspected in the [Symbol Explorer](https://symbol.fyi)
before signing.

Below is an example of an aggregate transaction containing two embedded transactions.
Both must be reviewed and understood before signing.

![Aggregate transaction](aggregate-transaction.png)

## Spoofing

### Description

Spoofing attacks disguise a malicious actor as a trusted individual or organization.

The goal is usually to obtain a user's <private key:> or <mnemonic phrase:>,
which allows the attacker to restore the user's wallet and gain full control of the account.

Symbol wallets are non-custodial, meaning that users are solely responsible for protecting their keys.

### Example Attack Scenario

* A user asks a question on [𝕏](https://x.com/SymbolSyndicate) or in the [Symbol Discord](https://discord.gg/J38KwW5ZuG).
* A malicious account sends a private message impersonating an official support member.
* The attacker attempts to gain trust and requests the user's private key or mnemonic phrase.
* If the victim provides these credentials, the attacker can transfer all assets from the account.

This scenario is just an example, and similar events could play out across any social media platform, messaging service,
or forum on which you share information publicly.

### Protection Measures

The Symbol Syndicate will **never** contact users privately for support and will never request private keys or
mnemonic phrases.

Common warning signs include:

* Requests for personal or wallet information.
* Impersonation accounts with slightly altered usernames.
* Discord profiles posing as support staff.
* Links to websites claiming to “validate” or “repair” wallets.
* Requests to move conversations to private messages.

If a message appears suspicious, it should be ignored or reported.

Most importantly, **never share private keys or mnemonic phrases**.

## Clipboard hacking

### Description

Symbol <addresses:> are long and typically copied and pasted rather than typed manually.

Clipboard-hijacking malware monitors clipboard activity and replaces copied addresses with the attacker's address.
When the user pastes the address into a transaction, the malicious address is used instead.

### Protection Measures

Always verify the destination address before confirming a transaction.

Using up-to-date anti-malware software can also help detect clipboard-hijacking malware before it affects crypto
activity.

## Compromised Backups

Any copy of a <private key:> or <mnemonic phrase:> can provide access to the associated accounts.

Backups must therefore be protected from theft, loss, or compromise, including exposure through
cloud storage services or insecure devices.
