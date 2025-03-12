# Class: CosignatureDescriptor

Type safe descriptor used to generate a descriptor map for CosignatureDescriptor.

Cosignature attached to an AggregateCompleteTransaction or AggregateBondedTransaction.

## Constructors

### new CosignatureDescriptor()

```ts
new CosignatureDescriptor(
   version, 
   signerPublicKey, 
   signature): CosignatureDescriptor
```

Creates a descriptor for Cosignature.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `version` | `bigint` | Version. |
| `signerPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) | Cosigner public key. |
| `signature` | [`Signature`](../../models/classes/Signature.md) | Transaction signature. |

#### Returns

[`CosignatureDescriptor`](CosignatureDescriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.signature` | [`Signature`](../../models/classes/Signature.md) |
| `rawDescriptor.signerPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) |
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
