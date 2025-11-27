# Class: AccountOperationRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountOperationRestrictionTransactionV1Descriptor.

Allow or block outgoing transactions depending on their transaction type (V1, latest).

## Constructors

### new AccountOperationRestrictionTransactionV1Descriptor()

```ts
new AccountOperationRestrictionTransactionV1Descriptor(
   restrictionFlags, 
   restrictionAdditions?, 
   restrictionDeletions?): AccountOperationRestrictionTransactionV1Descriptor
```

Creates a descriptor for AccountOperationRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `restrictionFlags` | [`AccountRestrictionFlags`](../../models/classes/AccountRestrictionFlags.md) | Type of restriction being applied to the listed transaction types. |
| `restrictionAdditions`? | [`TransactionType`](../../models/classes/TransactionType.md)[] | Array of transaction types being added to the restricted list. |
| `restrictionDeletions`? | [`TransactionType`](../../models/classes/TransactionType.md)[] | Array of transaction types being rtemoved from the restricted list. |

#### Returns

`AccountOperationRestrictionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.restrictionFlags` | [`AccountRestrictionFlags`](../../models/classes/AccountRestrictionFlags.md) |
| `rawDescriptor.type` | `string` |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
