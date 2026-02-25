---
title: Register Subnamespace
---

# Registering a Subnamespace

<Subnamespaces:> (also called "child" namespaces) extend the hierarchical structure of <namespaces:>.

This tutorial shows how to register a subnamespace under an existing <root namespace:>.

Once registered, additional steps are required to link the namespace to a mosaic or account,
as explained in [Next Steps](#next-steps).

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
* Have an <account:> with an existing root namespace.
  See [Registering a Root Namespace](./register-root-namespace.md).

    !!! note
        The examples in this tutorial use a root namespace named `ns_root`.
        Make sure to update the code to use your own root namespace name.

* Obtain <XYM:> to pay for the transaction and lease fees.
  See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/register-subnamespace', ['py', 'js']) }}

## Code Explanation

The code follows the same pattern as the [Registering a Root Namespace](./register-root-namespace.md) tutorial.
This section focuses only on the key differences.

For detailed explanations of the common steps (setting up the account, fetching network time and fees, and announcing),
see [Registering a Root Namespace](./register-root-namespace.md).

### Building the Transaction

{{ tutorial.code_snippet(['py:48:67', 'js:45:65']) }}

The main difference when registering a subnamespace is in the transaction descriptor:

* **Registration type:** The value `child` indicates a subnamespace (child namespace) is being created.
    Use `root` to [register a root namespace](./register-root-namespace.md) instead.

* **Parent ID**: Instead of specifying a duration, you provide the namespace ID of the parent namespace.
    The parent must be either a root namespace or another child namespace (if creating a third-level namespace).

    The parent namespace ID is calculated from its name using <dy:IdGenerator.generateNamespaceId>.

* **Name:** The name of the subnamespace.
    This name follows the same rules as root namespace names: lowercase letters, numbers, hyphens, and underscores,
    starting with a letter or number, up to 64 characters.

    Note that this is just the name of the subnamespace, not the full path.
    For example, to create `company.product`, where `company` is the root, you would set `name: 'product'` and
    `parent_id: generateNamespaceId('company')`.

    To ensure the subnamespace name is unique across multiple runs of the tutorial, a timestamp is added to the name.
    In practice, programs would use a fixed name for their namespaces.

The subnamespace automatically inherits the expiration time of its root namespace.
When the root expires, all subnamespaces expire with it.

!!! note "Subnamespace lease fees"

    In addition to the standard transaction fee,
    registering a subnamespace requires a [lease fee](../../textbook/namespaces.md#lease-fee).

    Unlike the transaction fee, the lease fee is **not** included in the transaction request.

    For subnamespaces, this fee is fixed regardless of duration.
    The network deducts the lease fee automatically when the transaction is confirmed,
    so you do not need to specify it in the transaction.

The transaction is then signed, announced, and confirmed following the same process as in
[Registering a Root Namespace](./register-root-namespace.md#submitting-the-transaction).

### Retrieving the Subnamespace

{{ tutorial.code_snippet(['py:110:136', 'js:125:157']) }}

To verify the subnamespace was registered, the code retrieves it from the network
using the <get:/namespaces/{namespaceId}> endpoint and displays its properties.

The subnamespace ID is computed using <dy:IdGenerator.generateNamespaceId>.
This function takes both the subnamespace name and the parent ID,
applying a deterministic hashing algorithm to produce the child namespace ID.

A successful response confirms the subnamespace was registered and is active on the network.

!!! info "Subnamespace registered but not linked yet"

    A subnamespace becomes useful when it serves as an alias for a <mosaic:> or an <account:>.
    Link the subnamespace to an identifier using the guides in [Next Steps](#next-steps).

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="7 8 16 19 21 30 33 34 35 36 37 38 39 40"
--8<-- 'devbook/namespaces/register-subnamespace.log'
```

Some highlights from the output:

* **Full namespace path** (line 7): The subnamespace `ns_root.sub_1766533103` shows the full hierarchical name.

* **Parent namespace ID** (lines 8, 33): The parent ID `0xb0786316d5c1d9dd` links this subnamespace to its root.

* **Fee** (line 16): The transaction fee of 0.016 XYM is calculated as the transaction size
    multiplied by the fee multiplier. The [lease fee](../../textbook/namespaces.md#lease-fee) is deducted separately
    by the network when the transaction is confirmed.

* **ID and name** (lines 19, 21): The `id` field shows the subnamespace ID as a decimal number, while `name` contains
    only the child portion encoded as hexadecimal (for example, `7375625f...` decodes to `sub_1...`).

* **Child namespace ID** (line 30): Shows both decimal and hexadecimal representations to match the `id` field on
    line 19.

* **Registration type** (line 34): The value `1` indicates a subnamespace (versus `0` for root namespaces).

* **Owner address** (line 35): The account that registered the namespace, which must be the same as the root
    namespace owner.

* **Depth** (line 36): The depth of `2` indicates there are 2 levels in the namespace hierarchy.
    Level 0 is the root namespace, and level 1 is this subnamespace.

* **Levels** (lines 37-38): The full hierarchical path. `level0` contains the root namespace ID, and `level1`
    contains the child ID. If depth were 3, `level2` would contain the grandchild ID.

* **Start and end heights** (lines 39-40): These values are inherited from the root namespace, not set independently.

The transaction hash printed in the output can also be used to search for the transaction
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                       | Related documentation                 |
| -------------------------------------------------------------------------  | ------------------------------------- |
| [Generate namespace ID](#building-the-transaction)                         | <dy:IdGenerator.generateNamespaceId>  |
| [Build a subnamespace registration transaction](#building-the-transaction) | <dy:SymbolTransactionFactory.create>  |
| [Retrieve the subnamespace](#retrieving-the-subnamespace)                  | <get:/namespaces/{namespaceId}>       |

## Next Steps

Now that you have a subnamespace, you can:

* Register additional subnamespaces to expand your hierarchical structure
* [Link your namespace to a mosaic](./link-namespace-to-mosaic.md) or
    [to an account](./link-namespace-to-address.md) to create an alias
