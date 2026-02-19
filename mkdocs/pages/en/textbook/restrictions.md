# Restrictions

Restriction
:   An advanced feature, also referred to as a _rule_, that allows fine-grained control over how specific
    <accounts:> or <mosaics:> can be used.

If a <transaction:> violates any active restriction, it is rejected during validation and never confirmed.
All restrictions are stored on-chain and validated by all network <nodes:>.

They are also enforced within <aggregate transactions:>.
If any <embedded transaction:> violates the applicable rules, the entire aggregate transaction is rejected.

Restrictions are created and managed using specialized restriction transactions, and are stored on-chain and therefore
publicly visible.
This allows external systems to evaluate eligibility or compliance without needing to trust off-chain logic.

There are two main types of restrictions:

* <Account restrictions:>
* <Mosaic restrictions:>

!!! info
    Originally designed for specialized use cases requiring strict controls,
    such as compliance requirements or internal policy enforcement,
    restrictions have also been adopted for other purposes, including enhancing security and limiting spam.

## Account-Based Restrictions

Account Restriction
:   A type of <restriction:> that limits what an individual <account:> can do, such as which addresses it can interact with,
    which mosaics it can receive, or what transaction types it can initiate.

Account restrictions can only be created, modified, or removed by the account itself.
If the account is part of a <multisignature account:>, any changes must be approved by the required number of cosigners.

### Account Address Restrictions

Account address restriction
:   A type of <account restriction:> that controls which other addresses a given account is allowed to interact with.

This restriction allows either **blocking** (blocklisting) or **allowing** (allowlisting) specific addresses,
but not both modes simultaneously.

The restriction can be applied to **incoming** transactions, **outgoing** transactions, or **both**.

!!! example "Examples"

    An account can be configured to:

    * Reject any transactions sent to blocklisted addresses.
        * Set an **outgoing** address restriction in **blocklist** mode, and include the unwanted addresses.
        {.operation-item}

    * Ignore any transactions from unapproved sources.
        * Set an **incoming** address restriction in **allowlist** mode, and list only the trusted source accounts.
        {.operation-item}

    * Allow interactions only with a predefined set of trusted accounts.
        This is useful, for example, when an account is restricted to internal use within an organization.

        * Set both **incoming** and **outgoing** address restrictions in **allowlist** mode, using the same list of approved accounts.
        {.operation-item}

### Account Mosaic Restrictions

Account mosaic restriction
:   A type of <account restriction:> that limits which <mosaics:> an account is allowed to receive.

This restriction supports either **blocking** (blocklisting) or **allowing** (allowlisting) incoming transactions
containing specific mosaics (but not both modes simultaneously).

Outgoing transactions are not affected.
There is no equivalent restriction for mosaics sent from the account.

If an incoming transaction contains multiple mosaics and any of them violate this restriction,
the entire transaction is rejected.

Note that this restriction applies to any transaction type that results in mosaics being transferred to the account,
not just to <transfer transactions:>.

!!! example "Examples"

    An account can be configured to:

    * Block the reception of certain unwanted mosaics, for example, those used by spammers.
        * Set an account mosaic restriction in **blocklist** mode and list the mosaics to block.
        {.operation-item}

    * Allow only a specific set of approved mosaics.
        This is useful, for example, when an account's contents are publicly visible and should remain uncluttered
        by unsolicited or irrelevant tokens.

        * Set an account mosaic restriction in **allowlist** mode and include only the allowed mosaics.
        {.operation-item}

### Account Operation Restrictions

Account operation restriction
:   A type of <account restriction:> that limits which types of <transactions:> an account is allowed to initiate.

This restriction supports either **blocking** (blocklisting) or **allowing** (allowlisting) the transaction types
that an account is allowed to perform (but not both modes simultaneously).

!!! warning

    If the `ACCOUNT_OPERATION_RESTRICTION` transaction type is omitted from the allowlist,
    the account will no longer be able to modify or remove this restriction,
    effectively locking itself into the allowed set of operations permanently.

    To avoid this risk, the [Symbol Desktop Wallet](../userbook/wallet/install.md) does not allow blocking the
    operation restriction transaction.

The restriction can be used to prevent accidental use of the wrong account,
but it does not prevent the account owner from lifting the restriction
(except in the case detailed in the warning box above).

All transaction types can be restricted.
See the list of supported types in the [Transactions page](./transactions.md#supported-transaction-types).

!!! example "Examples"

    * Limit an account to namespace registration and renewal only.
        * Set an account operation restriction in **allowlist** mode that allows only namespace-related and
        restriction-related functions.
        {.operation-item}

    * Do not allow moving funds from an account.
        * Set an account operation restriction in **blocklist** mode that forbids <transfer transactions:>.
        {.operation-item}

## Mosaic-Based Restrictions

Mosaic Restriction
:   A type of <restriction:> that defines rules accounts must satisfy in order to transact with a given <mosaic:>.

Unlike <account restrictions:>, which limit what a specific account can do, mosaic restrictions impose rules from
the mosaic's perspective.
Any account wishing to use the mosaic must comply with them.

These restrictions are created and managed by the mosaic creator.
Other accounts cannot set or modify restrictions on mosaics they do not own.
The restrictions do not apply to the mosaic creator.

!!! note

    Restrictions can only be applied to mosaics that have been created with the [Restrictable](./mosaics.md#restrictability)
    flag.

To apply a restriction to a mosaic, two components must be defined:

1. A <mosaic global restriction:>, which specifies the condition that accounts must meet in order to interact with the mosaic.
2. A <mosaic address restriction:>, which assigns a value to each account that will be compared against the condition.

**Both parts are required for the restriction to take effect.**

This dependency is unique to mosaic restrictions.
<Account restrictions:|Account restrictions>, by contrast, do not require multiple parts to work.

### Mosaic Global Restrictions

Mosaic Global Restriction
:   One of the two components required to apply a <mosaic restriction:>.
    It defines a rule that all accounts must satisfy in order to interact with a mosaic.

These restrictions are expressed as _key–value conditions_, set by the mosaic creator.
Each condition includes:

* A <mosaic ID:>: identifies the mosaic being restricted.
    Only its owner can restrict it.

* A **restriction key**: an arbitrary number defined by the creator that identifies this particular restriction.

    This is necessary because a mosaic may require multiple restrictions,
    but keys are independent for each mosaic.

    The key can be derived from a more memorable string (e.g. `KYC`) using a hash function.

* A **restriction value**: the threshold against which each account's value will be compared.

* A **comparison operator**: defines how each account's value is compared to the threshold
    (e.g. equal, not equal, greater than).

!!! example

    A mosaic that requires each account to have a value of `1` for the key `KYC` can use the following
    configuration:

    <div class="centered" markdown>

    |        Field | Value                    |
    | -----------: | :----------------------- |
    | <Mosaic ID:> | Mosaic being restricted. |
    |          Key | Hash of `KYC`            |
    |        Value | `1`                      |
    |     Operator | `Equal`                  |

    </div>

Once a global rule is set, the mosaic owner must explicitly assign values to accounts using
<mosaic address restrictions:> (see next section), which are then evaluated against the global condition.

Additionally, a mosaic global restriction can depend on the restriction values of another mosaic,
known as the _reference mosaic_.
The two mosaics do not need to be owned by the same account, allowing restriction logic to be delegated to third parties.

### Mosaic Address Restrictions

Mosaic Address Restriction
:   One of the two components required to apply a <mosaic restriction:>.
    It assigns _restriction attributes_ to a specific account, which are then evaluated against the condition
    defined in the corresponding <mosaic global restriction:>.

These attributes are set by the mosaic creator and act as account-specific metadata.
An account is allowed to transact with a mosaic if all the global restrictions for the mosaic are satisfied
by the restriction attributes set on the account.

To configure a mosaic address restriction, the following attributes must be provided:

* A <mosaic ID:>: identifies the mosaic being restricted.
    Only its owner can issue this restriction.
* A **Target account**: the address the restriction applies to.
* A **Restriction key**: selects an active global restriction.
* A **Restriction value**: a value to assign to the account, which will be compared to the value in the
    selected global restriction, according to the comparison operator.

!!! example

    Continuing the example from the previous section, to allow an account to transact with the restricted mosaic,
    the mosaic creator needs to submit the following mosaic address restriction:

    <div class="centered" markdown>

    |          Field | Value              |
    | -------------: | :----------------- |
    |   <Mosaic ID:> | Restricted mosaic. |
    | Target Account | `TABC123...`       |
    |            Key | Hash of `KYC`      |
    |          Value | `1`                |

    </div>

    The mosaic global restriction with the key `KYC` will be selected, and its value `1` compared for equality with
    the value `1` in the mosaic address restriction.

    Since the values are equal, the account `TABC123...` is allowed to transact with this mosaic.
