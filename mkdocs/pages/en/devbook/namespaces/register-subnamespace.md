---
title: Register Subnamespace
tutorial_level: intermediate
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

{{ tutorial.code_full_tagged('devbook/namespaces/register_subnamespace', ['py', 'js', 'java']) }}

## Code Explanation

The code follows the same pattern as the [Registering a Root Namespace](./register-root-namespace.md) tutorial.
This section focuses only on the key differences.

For detailed explanations of the common steps (setting up the account, fetching recommended fees, and announcing)
and the transaction descriptor fields shared with a root namespace,
see [Registering a Root Namespace](./register-root-namespace.md).

### Choosing the Subnamespace Name

{{ tutorial.code_snippet_tagged('step-1') }}

A subnamespace is identified by its full name, which joins the parent namespace name and the child name with a dot,
such as `company.product`.
See [Name](../../textbook/namespaces.md#name) in the Textbook for the naming rules.

To avoid collisions across multiple runs of the tutorial, a timestamp is added to the child name.
In practice, however, programs would use a fixed name for their subnamespaces.
You can force the tutorial to use fixed names through the `ROOT_NAMESPACE` and `SUBNAMESPACE` environment variables.

The parent namespace ID is derived from the parent name using <dy:IdGenerator.generateNamespaceId>.
A parent namespace is referenced by this ID rather than by its name.

!!! warning "Use a parent namespace owned by the signer"

    By default, the code uses the test account referenced by `SIGNER_PRIVATE_KEY` and a parent namespace named
    `ns_root`.

    If you come from the [Registering a Root Namespace](./register-root-namespace.md) tutorial, set the
    `SIGNER_PRIVATE_KEY` and `ROOT_NAMESPACE` environment variables to match the account and namespace you created
    there, or any other namespace that the signer owns.

### Building the Transaction

{{ tutorial.code_snippet_tagged('step-2') }}

The main difference when registering a subnamespace is in the transaction descriptor:

* {{ tutorial.var('registration_type') }}: The value `child` indicates a subnamespace is being created.
    Use `root` to [register a root namespace](./register-root-namespace.md) instead.

* {{ tutorial.var('parent_id') }}: Instead of specifying a duration, you provide the namespace ID of the parent
    namespace, derived in the previous step.
    It can be a root namespace or another subnamespace.

* {{ tutorial.var('name') }}: The name of the subnamespace, chosen in the previous step.

    Note that this is just the name of the subnamespace, not the full path.
    For example, to create `company.product`, where `company` is the root, you would set `name: 'product'` and
    {{ tutorial.var("`parent_id: generate_namespace_id('company')`") }}.

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

{{ tutorial.code_snippet_tagged('step-3') }}

To verify the subnamespace was registered, the code retrieves it from the network
using the <get:/namespaces/{namespaceId}> endpoint and displays its properties.

The subnamespace ID is computed using <dy:IdGenerator.generateNamespaceId>.
This function takes both the subnamespace name and the parent ID,
applying a deterministic hashing algorithm to produce the subnamespace ID.

A successful response confirms the subnamespace was registered and is active on the network.

!!! info "Subnamespace registered but not linked yet"

    A subnamespace becomes useful when it serves as an alias for a <mosaic:> or an <account:>.
    Link the subnamespace to an identifier using the guides in [Next Steps](#next-steps).

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="5 6 14 17 19 28 31 32 33 34 35 36 37 38"
--8<-- 'devbook/namespaces/register_subnamespace.log'
```

Some highlights from the output:

* **Full namespace path** (line 5): The subnamespace `ns_root.sub_1766533103` shows the full name.

* **Parent namespace ID** (lines 6, 33): The parent ID `0xB0786316D5C1D9DD` links this subnamespace to its root.

* **Fee** (line 14): The transaction fee of 0.016 XYM is calculated as the transaction size
    multiplied by the fee multiplier.
    The [lease fee](../../textbook/namespaces.md#lease-fee) is deducted separately by the network when the transaction
    is confirmed.

* **ID and name** (lines 17, 19): The `id` field shows the subnamespace ID as a decimal number, while `name` contains
    only the child portion encoded as hexadecimal (for example, `7375625f...` decodes to `sub_1...`).

* **Subnamespace ID** (line 28): Shows both decimal and hexadecimal representations to match the `id` field on
    line 17.

* **Registration type** (line 31): The value `1` indicates a subnamespace (versus `0` for root namespaces).

* **Owner address** (line 32): The account that registered the namespace, which must be the same as the root
    namespace owner.

* **Depth** (line 34): The depth of `2` indicates there are 2 levels in the namespace hierarchy.
    Level 0 is the root namespace, and level 1 is this subnamespace.

* **Levels** (lines 35-36): The full hierarchical path. `level0` contains the root namespace ID, and `level1`
    contains the child ID.
    If depth were 3, `level2` would contain the grandchild ID.

* **Start and end heights** (lines 37-38): These values are inherited from the root namespace, not set independently.

The transaction hash printed in the output can also be used to search for the transaction
in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

## Conclusion

This tutorial showed how to:

| Step                                                                       | Related documentation                                                                            |
| -------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| [Generate namespace ID](#choosing-the-subnamespace-name)                   | <dy:IdGenerator.generateNamespaceId>                                                             |
| [Build a subnamespace registration transaction](#building-the-transaction) | <dy:SymbolFacade.createTransactionFromTypedDescriptor>, <ser:NamespaceRegistrationTransactionV1> |
| [Retrieve the subnamespace](#retrieving-the-subnamespace)                  | <get:/namespaces/{namespaceId}>                                                                  |

## Next Steps

Now that you have a subnamespace, you can:

* Register additional subnamespaces to expand your hierarchical structure
* [Link your namespace to a mosaic](./link-namespace-to-mosaic.md) or
    [to an account](./link-namespace-to-address.md) to create an alias
