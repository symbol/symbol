---
title: Extend Root Namespace
---

# Extending a Root Namespace

This tutorial shows how to extend a root <namespace:> before it expires.

## Prerequisites

- An <account:> that owns a root namespace.
- <XYM:> to pay for the transaction and lease fees.

Review the [Registering a Root Namespace](./register-root-namespace.md) tutorial
for the complete registration process.

## Step by Step

To extend a namespace, follow the [registration process](./register-root-namespace.md) with these parameters:

1. Use the **same namespace name** as the existing namespace.
2. Set `registration_type` to `root`.
3. Specify the **duration** (in blocks) to add to the current lease.

The account signing the transaction must be the namespace owner.

When you extend a root namespace, all subnamespaces under it are automatically extended.

For details on duration limits and the grace period,
see [Duration](../../textbook/namespaces.md#duration) in the Textbook.
