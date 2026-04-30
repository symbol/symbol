---
title: Query Account Balance
---

# Querying an Account Balance

<Accounts:|Accounts> on Symbol can hold <mosaics:> (fungible tokens), including the native currency <XYM:>.

This tutorial shows how to query an account's mosaic balances and display them with the appropriate number of
decimal places.
If the <mosaic ID:> has a linked <namespace:>, it is also retrieved and displayed as the mosaic's friendly name.

## Prerequisites

This tutorial uses the [Symbol REST API](../reference/rest/symbol.md) without requiring an SDK.
You only need a way to make HTTP requests.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/accounts/query-balance', ['py', 'js']) }}

The snippet uses the `NODE_URL` environment variable to set the Symbol API node.
If no value is provided, a default one is used.

The tutorial defines the following functions:

* `get_account_info()`: Fetches <account:> state by address or public key.
* `get_mosaic_names()`: Fetches <namespace:> aliases for mosaics.
* `get_mosaics_info()`: Fetches properties for multiple <mosaics:> in a single request.
* `format_amount()`: Formats amounts with the appropriate number of decimal places, according to their <divisibility:>.

## Code Explanation

### Fetching Account Information

{{ tutorial.code_snippet(['py:10:33', 'js:6:32']) }}

The <get:/accounts/{accountId}> endpoint retrieves the state of an account, including all the mosaics it holds.

You can query an account using either its <address:> or its <public key:>.

### Fetching Mosaic Names

{{ tutorial.code_snippet(['py:36:60', 'js:34:57']) }}

Mosaics are identified by 64-bit numeric <Mosaic ID:|IDs>, which can be hard to read and remember.
To improve usability, mosaics can be linked to human-readable <namespace:> aliases.

The <post:/namespaces/mosaic/names> endpoint accepts multiple mosaic IDs and returns any namespace names that are
currently linked to them.
If a mosaic has no linked namespace, it will not appear in the response.

The method returns a map because a mosaic can have multiple namespace aliases
(different namespaces can link to the same mosaic).

### Fetching Mosaic Properties

{{ tutorial.code_snippet(['py:63:87', 'js:59:82']) }}

To format mosaic balances correctly, the snippet fetches their properties from the network.
The key property required is <divisibility:>, which defines how many decimal places a mosaic supports.

You can retrieve mosaic properties using the <get:/mosaics/{mosaicId}> endpoint for individual mosaics.
The option used in this snippet is the <post:/mosaics> endpoint, which accepts multiple mosaic IDs in a single request
and returns detailed information about each mosaic, including its divisibility.

### Formatting Amounts

{{ tutorial.code_snippet(['py:90:105', 'js:84:101']) }}

This utility function converts _atomic_ amounts into human-friendly representations:

* **Atomic amount:** The raw value stored on the blockchain, expressed as an integer.
* **Formatted amount:** The display format with decimal places determined by the mosaic's divisibility.

The formatting splits the atomic amount into whole and fractional parts by dividing and taking the remainder
with respect to \(10^{\text{divisibility}}\).
The fractional part is then zero-padded to ensure it always displays the correct number of decimal places.

### Putting It All Together

{{ tutorial.code_snippet(['py:108:150', 'js:103:148']) }}

The main code reads the `ADDRESS` environment variable to determine which account to query.
If no value is provided, it uses a default sample address.

It orchestrates the helper functions to:

1. Retrieve the account information.
2. Extract the mosaic IDs from the account.
3. Fetch mosaic properties and namespace names for all mosaics.
4. Iterate through each mosaic and format the balance with the appropriate decimal places.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/accounts/query-balance.log'
```

The output displays all mosaics the account holds. Notice how different mosaics have different divisibility values:

* The first mosaic has divisibility 6, showing six decimal places (`592.589332`).
    It also has a friendly name (`symbol.xym`), which identifies it as the network's native currency.
* The second has divisibility 0, showing no decimal places (`999999`).
* The third has divisibility 2, showing two decimal places (`0.01`).

## Conclusion

This tutorial showed how to:

| Step                                                     | Related documentation                                         |
| -------------------------------------------------------- | ------------------------------------------------------------- |
| [Fetch account state](#fetching-account-information)     | <get:/accounts/{accountId}>                                   |
| [Fetch mosaic names](#fetching-mosaic-names)             | <post:/namespaces/mosaic/names>                               |
| [Fetch mosaic properties](#fetching-mosaic-properties)   | <get:/mosaics/{mosaicId}> <br> <post:/mosaics>                |
