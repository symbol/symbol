# Class: MosaicMetadataTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicMetadataTransactionV1Descriptor.

Associate a key-value state ([metadata](/concepts/metadata.html)) to a **mosaic** (V1, latest).
Compare to AccountMetadataTransaction and NamespaceMetadataTransaction.

## Constructors

### new MosaicMetadataTransactionV1Descriptor()

```ts
new MosaicMetadataTransactionV1Descriptor(
   targetAddress, 
   scopedMetadataKey, 
   targetMosaicId, 
   valueSizeDelta, 
   value?): MosaicMetadataTransactionV1Descriptor
```

Creates a descriptor for MosaicMetadataTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `targetAddress` | [`Address`](../../../classes/Address.md) | Account owning the mosaic whose metadata should be modified. |
| `scopedMetadataKey` | `bigint` | Metadata key scoped to source, target and type. |
| `targetMosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) | Mosaic whose metadata should be modified. |
| `valueSizeDelta` | `number` | Change in value size in bytes, compared to previous size. |
| `value`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array. |

#### Returns

`MosaicMetadataTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.scopedMetadataKey` | `bigint` |
| `rawDescriptor.targetAddress` | [`Address`](../../../classes/Address.md) |
| `rawDescriptor.targetMosaicId` | [`UnresolvedMosaicId`](../../models/classes/UnresolvedMosaicId.md) |
| `rawDescriptor.type` | `string` |
| `rawDescriptor.valueSizeDelta` | `number` |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
