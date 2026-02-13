---
title: Extend Root Namespace
---

# Extending a Root Namespace

<Root namespaces:> are leased for a limited [duration](../../textbook/namespaces.md#duration), up to a maximum of 5
years per registration.
If you want to keep a namespace beyond its initial lease, you need to extend it.

This tutorial shows how to extend a root namespace.

## Prerequisites

- An <account:> that owns an active root namespace.
    See [Registering a Root Namespace](./register-root-namespace.md).
- <XYM:> to pay for the transaction and lease fees.

## When to Extend

You can extend a namespace in two situations:

* **While active:** The specified duration is added to the current lease,
    pushing the expiration date further into the future.

* **During the grace period:** The namespace has expired but is still within the [grace period](../../textbook/namespaces.md#duration),
    extending it restores the namespace to active status immediately.

!!! note "Extending Subnamespaces"

    Only root namespaces need to be extended.
    <Subnamespaces:> inherit the duration of their root namespace, so when you extend a root namespace, all its
    subnamespaces are automatically extended as well.

## Step by Step

To extend a namespace, repeat the [registration process](./register-root-namespace.md) with these parameters:

1. Use the **same namespace name** as the existing namespace.
2. Set `registration_type` to `root`.
3. Specify the **duration** (in blocks) to add to the current lease.

The account signing the transaction must be the namespace owner.

## Duration and Limits

Each extension can add up to 5,256,000 blocks (approximately 5 years).
However, the total remaining duration cannot exceed 5 years into the future.
For example, if a namespace expires in 3 years, you can only add up to 2 more years.

To maintain a namespace indefinitely, extend it periodically.

For more details, see [Duration](../../textbook/namespaces.md#duration) in the Textbook.
