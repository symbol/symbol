---
title: Typed Descriptors
---

# Creating a Transfer Transaction Using Typed Descriptors

<TS:SymbolFacade.createTransactionFromTypedDescriptor> is a typed alternative to
<TS:SymbolTransactionFactory.create>.
It uses structured, type-safe parameters, making it easier to construct transactions correctly and reduce the
chance of mistakes.

For example, the snippet above could use the <TS:{{descriptor}}> to define the transaction in a type-safe way.

---

**Caution**:
The `deadline` parameter of the typed version is relative to the local system time, not the network time.
This removes the need to fetch the network time, but can lead to drift if the system clock is inaccurate.

Applications should periodically synchronize with the network time to ensure deadlines are calculated correctly.