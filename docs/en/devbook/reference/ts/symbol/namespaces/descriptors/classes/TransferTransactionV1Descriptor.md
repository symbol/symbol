# Class: TransferTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for TransferTransactionV1Descriptor.

Send mosaics and messages between two accounts (V1, latest).

## Constructors

### new TransferTransactionV1Descriptor()

```ts
new TransferTransactionV1Descriptor(
   recipientAddress, 
   mosaics?, 
   message?): TransferTransactionV1Descriptor
```

Creates a descriptor for TransferTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientAddress` | [`Address`](../../../classes/Address.md) | recipient address |
| `mosaics`? | [`UnresolvedMosaicDescriptor`](UnresolvedMosaicDescriptor.md)[] | attached mosaics |
| `message`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | attached message |

#### Returns

`TransferTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.recipientAddress` | [`Address`](../../../classes/Address.md) |
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
