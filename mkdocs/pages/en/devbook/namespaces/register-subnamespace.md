---
title: Register Subnamespace
---

# Registering a Subnamespace

Subnamespaces (also called "child" namespaces) extend the hierarchical structure of <namespaces:>.

This tutorial shows how to register a subnamespace under an existing <root namespace:>.

## Prerequisites

Before you start, make sure to:

- Set up your development environment.
  See [Setting Up a Development Environment](../start/setup.md).
- Have <account:> with an existing root namespace.
  See [Registering a Root Namespace](./register-root-namespace.md).

    !!! note
        The examples in this tutorial use a root namespace named `ns_root`.
        Make sure to update the code to use your own root namespace name.
- Obtain <XYM:> to pay for the transaction and lease fees.
  See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).

Additionally, review the [Transfer transaction](../transactions/transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/register-subnamespace', ['py', 'js']) }}

## Code Explanation

Most of the code follows the same pattern as registering a root namespace.
This section focuses on the key differences.

For detailed explanations of the common steps (setting up the account, fetching network time and fees, and announcing),
see [Registering a Root Namespace](./register-root-namespace.md).

### Building the Transaction

{{ tutorial.code_snippet(['py:47:66', 'js:45:65']) }}

The main difference when registering a subnamespace is in the transaction descriptor:

* **Registration type:** The value `child` indicates a subnamespace (child namespace) is being created.

* **Parent ID**: Instead of specifying a duration, you provide the namespace ID of the parent namespace.
    The parent must be either a root namespace or another child namespace (if creating a third-level namespace).

    The parent namespace ID is calculated from its name using <dy:IdGenerator.generateNamespaceId>.

* **Name:** The name of the subnamespace.
    This name follows the same rules as root namespace names: lowercase letters, numbers, hyphens, and underscores,
    starting with a letter or number, up to 64 characters.

    Note that this is just the name of the subnamespace, not the full path.
    For example, to create `company.product`, where `company` is the root, you would set `name: 'product'` and
    `parent_id: generateNamespaceId('company')`.

    To ensure the subnamespace name is unique across multiple runs, the example appends a timestamp to the name.

The subnamespace automatically inherits the expiration time of its root namespace.
When the root expires, all subnamespaces expire with it.

!!! note "Subnamespace lease fees"

    Registering a subnamespace requires a lease fee in addition to the transaction fee.
    Unlike root namespaces, this fee is fixed regardless of duration.
    For details, see [Lease fee](../../textbook/namespaces.md#lease-fee).

The transaction is then signed, announced, and confirmed following the same process as in
[Registering a Root Namespace](./register-root-namespace.md#submitting-the-transaction).

### Retrieving the Subnamespace

{{ tutorial.code_snippet(['py:109:133', 'js:125:154']) }}

To verify the subnamespace was registered, the code retrieves it from the network
using the <get:/namespaces/{namespaceId}> endpoint and displays its properties.

The subnamespace ID is computed using <dy:IdGenerator.generateNamespaceId>.
This function takes both the subnamespace name and the parent ID,
applying a deterministic hashing algorithm to produce the child namespace ID.

The response confirms:

* **Registration type:** Shows `1` for a subnamespace.
* **Owner address:** The account that registered the namespace (same as the root namespace owner).
* **Parent ID:** The namespace ID of the parent namespace.
* **Depth:** The depth in the namespace hierarchy (1 for direct children of root, 2 for grandchildren).
* **Levels:** The full hierarchical path. `level0` contains the root namespace ID, `level1` contains the child ID,
  and `level2` contains the grandchild ID (if applicable).
* **Start and end heights:** Inherited from the root namespace.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/namespaces/register-subnamespace.log'
```

The transaction hash printed in the output can be used to search for the transaction
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                           | Related documentation                 |
| -------------------------------------------------------------- | ------------------------------------- |
| [Generate namespace ID](#building-the-transaction)             | <dy:IdGenerator.generateNamespaceId>  |
| [Build a subnamespace registration](#building-the-transaction) | <dy:SymbolTransactionFactory.create>  |
| [Retrieve the namespace](#retrieving-the-namespace)            | <get:/namespaces/{namespaceId}>       |

## Next Steps

Now that you have a subnamespace, you can:

- Register additional subnamespaces to expand your hierarchical structure
- Link your namespace to a mosaic or account to create an alias
