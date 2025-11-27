# Class: MosaicGlobalRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicGlobalRestrictionTransactionV1Descriptor.

Set global rules to transfer a restrictable mosaic (V1, latest).

## Constructors

### new MosaicGlobalRestrictionTransactionV1Descriptor()

```ts
new MosaicGlobalRestrictionTransactionV1Descriptor(
   mosaicId, 
   referenceMosaicId, 
   restrictionKey, 
   previousRestrictionValue, 
   newRestrictionValue, 
   previousRestrictionType, 
   newRestrictionType): MosaicGlobalRestrictionTransactionV1Descriptor
```

Creates a descriptor for MosaicGlobalRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Identifier of the mosaic being restricted. The mosaic creator must be the signer of the transaction. |
| `referenceMosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Identifier of the mosaic providing the restriction key. The mosaic global restriction for the mosaic identifier depends on global restrictions set on the reference mosaic. Set `reference_mosaic_id` to **0** if the mosaic giving the restriction equals the `mosaic_id`. |
| `restrictionKey` | `bigint` | Restriction key relative to the reference mosaic identifier. |
| `previousRestrictionValue` | `bigint` | Previous restriction value. |
| `newRestrictionValue` | `bigint` | New restriction value. |
| `previousRestrictionType` | [`MosaicRestrictionType`](../../models/classes/MosaicRestrictionType.md) | Previous restriction type. |
| `newRestrictionType` | [`MosaicRestrictionType`](../../models/classes/MosaicRestrictionType.md) | New restriction type. |

#### Returns

`MosaicGlobalRestrictionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |
| `rawDescriptor.newRestrictionType` | [`MosaicRestrictionType`](../../models/classes/MosaicRestrictionType.md) |
| `rawDescriptor.newRestrictionValue` | `bigint` |
| `rawDescriptor.previousRestrictionType` | [`MosaicRestrictionType`](../../models/classes/MosaicRestrictionType.md) |
| `rawDescriptor.previousRestrictionValue` | `bigint` |
| `rawDescriptor.referenceMosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |
| `rawDescriptor.restrictionKey` | `bigint` |
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
