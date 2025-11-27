# Class: MosaicDescriptor

Type safe descriptor used to generate a descriptor map for MosaicDescriptor.

A quantity of a certain mosaic.

## Constructors

### new MosaicDescriptor()

```ts
new MosaicDescriptor(mosaicId, amount): MosaicDescriptor
```

Creates a descriptor for Mosaic.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaicId` | [`MosaicId`](../../models/classes/MosaicId.md) | Mosaic identifier. |
| `amount` | [`Amount`](../../models/classes/Amount.md) | Mosaic amount. |

#### Returns

`MosaicDescriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.amount` | [`Amount`](../../models/classes/Amount.md) |
| `rawDescriptor.mosaicId` | [`MosaicId`](../../models/classes/MosaicId.md) |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
