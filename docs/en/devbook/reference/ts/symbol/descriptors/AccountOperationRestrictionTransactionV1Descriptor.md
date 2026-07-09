# Class: AccountOperationRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountOperationRestrictionTransactionV1Descriptor.

Allow or block outgoing transactions depending on their transaction type (V1, latest).

## Constructors

### Constructor

```ts
new AccountOperationRestrictionTransactionV1Descriptor(
   restrictionFlags, 
   restrictionAdditions?, 
   restrictionDeletions?): AccountOperationRestrictionTransactionV1Descriptor;
```

Creates a descriptor for AccountOperationRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `restrictionFlags` | [`AccountRestrictionFlags`](../models/AccountRestrictionFlags.md) | Type of restriction being applied to the listed transaction types. |
| `restrictionAdditions?` | [`TransactionType`](../models/TransactionType.md)[] | Array of transaction types being added to the restricted list. |
| `restrictionDeletions?` | [`TransactionType`](../models/TransactionType.md)[] | Array of transaction types being rtemoved from the restricted list. |

#### Returns

`AccountOperationRestrictionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.restrictionFlags` | [`AccountRestrictionFlags`](../models/AccountRestrictionFlags.md) |
| `rawDescriptor.type` | `string` |

## Methods

### toMap()

```ts
toMap(): object;
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
