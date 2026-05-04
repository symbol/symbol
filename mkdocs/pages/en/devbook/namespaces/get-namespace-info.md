---
title: Get Namespace Information
tutorial_level: beginner
---

# Getting Namespace Information

This tutorial shows how to retrieve a <namespace:>'s properties and the <mosaic:> or <account:> it points to.

## Prerequisites

This tutorial only reads data from the network. No account is required.

Before you start, make sure to [set up your development environment](../start/setup.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/get-namespace-info', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default <testnet:> node is used.

The `NAMESPACE_NAME` environment variable specifies which namespace to query.
If not set, it defaults to `symbol.xym`, the namespace linked to the network's native currency <XYM:>.

## Code Explanation

### Generating the Namespace ID

{{ tutorial.code_snippet(['py:16:20', 'js:11:15']) }}

Namespace IDs are computed locally from the namespace name using <dy:IdGenerator.generateNamespacePath>.
This function takes a fully qualified name like `symbol.xym`, splits it by `.`, and returns an array of namespace IDs
for each level in the hierarchy.
The last element is the ID of the deepest namespace.

### Fetching Namespace Information

{{ tutorial.code_snippet(['py:22:44', 'js:17:46']) }}

The <get:/namespaces/{namespaceId}> endpoint retrieves the current properties of a namespace, including:

* **Registration type:** The value `0` indicates a <root namespace:> and `1` indicates a <subnamespace:>.

* **Owner address:** The account that [registered the namespace](../../textbook/namespaces.md#ownership).

* **Depth:** The number of levels in the namespace hierarchy.
    For example, `foo` has a depth of `1`, `foo.bar` has a depth of `2`, and `foo.bar.baz` has a depth of `3`.

* **Levels:** The namespace IDs for each level in the hierarchy.
    `level0` is always the root namespace ID. `level1` and `level2` appear for deeper hierarchies.

* **Start and end heights:** The <block:> range during which the [namespace is active](../../textbook/namespaces.md#duration).

### Checking the Alias

{{ tutorial.code_snippet(['py:46:58', 'js:48:59']) }}

Each level in a namespace hierarchy is an independent namespace that can have its own
[alias](../../textbook/namespaces.md#linking).
The response includes alias information for the queried level, indicating whether it is linked to a mosaic or an
account:

* **Alias type `0`:** No alias is linked.
* **Alias type `1`:** The namespace is linked to a mosaic. The response includes the linked <mosaic ID:>.
* **Alias type `2`:** The namespace is linked to an account. The response includes the linked <address:>.

## Output

The output shown below corresponds to a typical run of the program, querying the `symbol.xym` namespace on testnet.

```text linenums="1" hl_lines="3 6 7 8 9 10 11 12 13 14"
--8<-- 'devbook/namespaces/get-namespace-info.log'
```

Some highlights from the output:

* **Namespace ID** (line 3): The computed ID for `symbol.xym` is `0xe74b99ba41f4afee`.

* **Registration type** (line 6): The value `1` confirms this is a subnamespace (child of `symbol`).

* **Owner address** (line 7): The account that registered the `symbol` namespace hierarchy.

* **Depth** (line 8): The value `2` indicates a two-level hierarchy: `symbol` (level 0) and `xym` (level 1).

* **Level IDs** (lines 9-10): `level0` is the root `symbol` namespace ID (`A95F1F8A96159516`), and `level1` is the `xym`
    subnamespace ID (`E74B99BA41F4AFEE`).
    The last level ID matches the namespace ID on line 3, confirming it is the namespace being queried.

* **End height** (line 12): The value `18446744073709551615` (`0xFFFFFFFFFFFFFFFF`) means this namespace never expires.

* **Alias** (lines 13-14): The alias type `1` confirms the namespace is linked to the XYM mosaic (`72C0212E67A08BCE`).

## Conclusion

This tutorial showed how to:

| Step                                                          | Related documentation                  |
| ------------------------------------------------------------- | -------------------------------------- |
| [Generate namespace ID](#generating-the-namespace-id)         | <dy:IdGenerator.generateNamespacePath> |
| [Fetch namespace properties](#fetching-namespace-information) | <get:/namespaces/{namespaceId}>        |
| [Check namespace alias](#checking-the-alias)                  | <get:/namespaces/{namespaceId}>        |
