# Class: UnresolvedMosaicDescriptor

Type safe descriptor used to generate a descriptor map for UnresolvedMosaicDescriptor.

A quantity of a certain mosaic, specified either through a MosaicId or an alias.

## Constructors

### Constructor

```ts
new UnresolvedMosaicDescriptor(mosaicId, amount): UnresolvedMosaicDescriptor;
```

Creates a descriptor for UnresolvedMosaic.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`UnresolvedMosaicId`](../models/UnresolvedMosaicId.md) | Unresolved mosaic identifier. |
| `amount` | [`Amount`](../models/Amount.md) | Mosaic amount. |

#### Returns

`UnresolvedMosaicDescriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.amount` | [`Amount`](../models/Amount.md) |
| `rawDescriptor.mosaicId` | [`UnresolvedMosaicId`](../models/UnresolvedMosaicId.md) |

## Methods

### toMap()

```ts
toMap(): object;
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
