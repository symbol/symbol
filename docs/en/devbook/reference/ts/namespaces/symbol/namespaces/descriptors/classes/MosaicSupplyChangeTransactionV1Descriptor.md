# Class: MosaicSupplyChangeTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicSupplyChangeTransactionV1Descriptor.

Change the total supply of a mosaic (V1, latest).

## Constructors

### new MosaicSupplyChangeTransactionV1Descriptor()

```ts
new MosaicSupplyChangeTransactionV1Descriptor(
   mosaicId, 
   delta, 
   action): MosaicSupplyChangeTransactionV1Descriptor
```

Creates a descriptor for MosaicSupplyChangeTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Affected mosaic identifier. |
| `delta` | [`Amount`](../../models/classes/Amount.md) | Change amount. It cannot be negative, use the `action` field to indicate if this amount should be **added** or **subtracted** from the current supply. |
| `action` | [`MosaicSupplyChangeAction`](../../models/classes/MosaicSupplyChangeAction.md) | Supply change action. |

#### Returns

[`MosaicSupplyChangeTransactionV1Descriptor`](MosaicSupplyChangeTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.action` | [`MosaicSupplyChangeAction`](../../models/classes/MosaicSupplyChangeAction.md) |
| `rawDescriptor.delta` | [`Amount`](../../models/classes/Amount.md) |
| `rawDescriptor.mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |
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
