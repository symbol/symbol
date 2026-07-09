# Class: MosaicDefinitionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicDefinitionTransactionV1Descriptor.

Create a new  [mosaic](/concepts/mosaic.html) (V1, latest).

## Constructors

### Constructor

```ts
new MosaicDefinitionTransactionV1Descriptor(
   id, 
   duration, 
   nonce, 
   flags, 
   divisibility): MosaicDefinitionTransactionV1Descriptor;
```

Creates a descriptor for MosaicDefinitionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `id` | [`MosaicId`](../models/MosaicId.md) | Unique mosaic identifier obtained from the generator account's public key and the `nonce`. The SDK's can take care of generating this ID for you. |
| `duration` | [`BlockDuration`](../models/BlockDuration.md) | Mosaic duration expressed in blocks. If set to 0, the mosaic never expires. |
| `nonce` | [`MosaicNonce`](../models/MosaicNonce.md) | Random nonce used to generate the mosaic id. |
| `flags` | [`MosaicFlags`](../models/MosaicFlags.md) | Mosaic flags. |
| `divisibility` | `number` | Mosaic divisibility. |

#### Returns

`MosaicDefinitionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.divisibility` | `number` |
| `rawDescriptor.duration` | [`BlockDuration`](../models/BlockDuration.md) |
| `rawDescriptor.flags` | [`MosaicFlags`](../models/MosaicFlags.md) |
| `rawDescriptor.id` | [`MosaicId`](../models/MosaicId.md) |
| `rawDescriptor.nonce` | [`MosaicNonce`](../models/MosaicNonce.md) |
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
