# Class: NamespaceMetadataTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for NamespaceMetadataTransactionV1Descriptor.

Associate a key-value state ([metadata](/concepts/metadata.html)) to a **namespace** (V1, latest).
Compare to AccountMetadataTransaction and MosaicMetadataTransaction.

## Constructors

### new NamespaceMetadataTransactionV1Descriptor()

```ts
new NamespaceMetadataTransactionV1Descriptor(
   targetAddress, 
   scopedMetadataKey, 
   targetNamespaceId, 
   valueSizeDelta, 
   value?): NamespaceMetadataTransactionV1Descriptor
```

Creates a descriptor for NamespaceMetadataTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `targetAddress` | [`Address`](../../../classes/Address.md) | Account owning the namespace whose metadata should be modified. |
| `scopedMetadataKey` | `bigint` | Metadata key scoped to source, target and type. |
| `targetNamespaceId` | [`NamespaceId`](../../models/classes/NamespaceId.md) | Namespace whose metadata should be modified. |
| `valueSizeDelta` | `number` | Change in value size in bytes, compared to previous size. |
| `value`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array. |

#### Returns

[`NamespaceMetadataTransactionV1Descriptor`](NamespaceMetadataTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.scopedMetadataKey` | `bigint` |
| `rawDescriptor.targetAddress` | [`Address`](../../../classes/Address.md) |
| `rawDescriptor.targetNamespaceId` | [`NamespaceId`](../../models/classes/NamespaceId.md) |
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
