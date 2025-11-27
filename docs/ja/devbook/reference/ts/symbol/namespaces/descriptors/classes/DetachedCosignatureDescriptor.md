# Class: DetachedCosignatureDescriptor

Type safe descriptor used to generate a descriptor map for DetachedCosignatureDescriptor.

Cosignature detached from an AggregateCompleteTransaction or AggregateBondedTransaction.

## Constructors

### new DetachedCosignatureDescriptor()

```ts
new DetachedCosignatureDescriptor(
   version, 
   signerPublicKey, 
   signature, 
   parentHash): DetachedCosignatureDescriptor
```

Creates a descriptor for DetachedCosignature.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `version` | `bigint` | Version. |
| `signerPublicKey` | [`PublicKey`](../../../../index/classes/PublicKey.md) | Cosigner public key. |
| `signature` | [`Signature`](../../models/classes/Signature.md) | Transaction signature. |
| `parentHash` | [`Hash256`](../../../../index/classes/Hash256.md) | Hash of the AggregateBondedTransaction that is signed by this cosignature. |

#### Returns

`DetachedCosignatureDescriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.parentHash` | [`Hash256`](../../../../index/classes/Hash256.md) |
| `rawDescriptor.signature` | [`Signature`](../../models/classes/Signature.md) |
| `rawDescriptor.signerPublicKey` | [`PublicKey`](../../../../index/classes/PublicKey.md) |
| `rawDescriptor.version` | `bigint` |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
