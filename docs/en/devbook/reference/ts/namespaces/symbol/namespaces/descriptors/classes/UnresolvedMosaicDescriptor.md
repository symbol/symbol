# Class: UnresolvedMosaicDescriptor

Type safe descriptor used to generate a descriptor map for UnresolvedMosaicDescriptor.

A quantity of a certain mosaic, specified either through a MosaicId or an alias.

## Constructors

### new UnresolvedMosaicDescriptor()

```ts
new UnresolvedMosaicDescriptor(mosaicId, amount): UnresolvedMosaicDescriptor
```

Creates a descriptor for UnresolvedMosaic.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Unresolved mosaic identifier. |
| `amount` | [`Amount`](../../models/classes/Amount.md) | Mosaic amount. |

#### Returns

[`UnresolvedMosaicDescriptor`](UnresolvedMosaicDescriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.amount` | [`Amount`](../../models/classes/Amount.md) |
| `rawDescriptor.mosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
