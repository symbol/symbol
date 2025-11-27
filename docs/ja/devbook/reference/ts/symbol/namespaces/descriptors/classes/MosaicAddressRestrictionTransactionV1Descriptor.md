# Class: MosaicAddressRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicAddressRestrictionTransactionV1Descriptor.

Set address specific rules to transfer a restrictable mosaic (V1, latest).

## Constructors

### new MosaicAddressRestrictionTransactionV1Descriptor()

```ts
new MosaicAddressRestrictionTransactionV1Descriptor(
   mosaicId, 
   restrictionKey, 
   previousRestrictionValue, 
   newRestrictionValue, 
   targetAddress): MosaicAddressRestrictionTransactionV1Descriptor
```

Creates a descriptor for MosaicAddressRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Identifier of the mosaic to which the restriction applies. |
| `restrictionKey` | `bigint` | Restriction key. |
| `previousRestrictionValue` | `bigint` | Previous restriction value. Set `previousRestrictionValue` to `FFFFFFFFFFFFFFFF` if the target address does not have a previous restriction value for this mosaic id and restriction key. |
| `newRestrictionValue` | `bigint` | New restriction value. |
| `targetAddress` | [`Address`](../../../classes/Address.md) | Address being restricted. |

#### Returns

`MosaicAddressRestrictionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |
| `rawDescriptor.newRestrictionValue` | `bigint` |
| `rawDescriptor.previousRestrictionValue` | `bigint` |
| `rawDescriptor.restrictionKey` | `bigint` |
| `rawDescriptor.targetAddress` | [`Address`](../../../classes/Address.md) |
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
