# Class: AccountMetadataTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountMetadataTransactionV1Descriptor.

Associate a key-value state ([metadata](/concepts/metadata.html)) to an **account** (V1, latest).
\note This transaction must **always** be wrapped in an AggregateTransaction so that a cosignature from `target_address` can be provided. Without this cosignature the transaction is invalid.
Compare to MosaicMetadataTransaction and NamespaceMetadataTransaction.

## Constructors

### new AccountMetadataTransactionV1Descriptor()

```ts
new AccountMetadataTransactionV1Descriptor(
   targetAddress, 
   scopedMetadataKey, 
   valueSizeDelta, 
   value?): AccountMetadataTransactionV1Descriptor
```

Creates a descriptor for AccountMetadataTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `targetAddress` | [`Address`](../../../classes/Address.md) | Account whose metadata should be modified. |
| `scopedMetadataKey` | `bigint` | Metadata key scoped to source, target and type. |
| `valueSizeDelta` | `number` | Change in value size in bytes, compared to previous size. |
| `value`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array. |

#### Returns

[`AccountMetadataTransactionV1Descriptor`](AccountMetadataTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.scopedMetadataKey` | `bigint` |
| `rawDescriptor.targetAddress` | [`Address`](../../../classes/Address.md) |
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
