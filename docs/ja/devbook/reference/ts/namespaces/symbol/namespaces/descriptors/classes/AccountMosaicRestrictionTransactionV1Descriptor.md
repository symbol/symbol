# Class: AccountMosaicRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountMosaicRestrictionTransactionV1Descriptor.

Allow or block incoming transactions containing a given set of mosaics (V1, latest).

## Constructors

### new AccountMosaicRestrictionTransactionV1Descriptor()

```ts
new AccountMosaicRestrictionTransactionV1Descriptor(
   restrictionFlags, 
   restrictionAdditions?, 
   restrictionDeletions?): AccountMosaicRestrictionTransactionV1Descriptor
```

Creates a descriptor for AccountMosaicRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `restrictionFlags` | [`AccountRestrictionFlags`](../../models/classes/AccountRestrictionFlags.md) | Type of restriction being applied to the listed mosaics. |
| `restrictionAdditions`? | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md)[] | Array of mosaics being added to the restricted list. |
| `restrictionDeletions`? | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md)[] | Array of mosaics being removed from the restricted list. |

#### Returns

[`AccountMosaicRestrictionTransactionV1Descriptor`](AccountMosaicRestrictionTransactionV1Descriptor.md)

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
