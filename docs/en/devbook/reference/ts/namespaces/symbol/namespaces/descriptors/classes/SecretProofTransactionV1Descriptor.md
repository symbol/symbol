# Class: SecretProofTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for SecretProofTransactionV1Descriptor.

Conclude a token swap between different chains (V1, latest).
Use a SecretProofTransaction to unlock the funds locked by a SecretLockTransaction.
The transaction must prove knowing the *proof* that unlocks the mosaics.

## Constructors

### new SecretProofTransactionV1Descriptor()

```ts
new SecretProofTransactionV1Descriptor(
   recipientAddress, 
   secret, 
   hashAlgorithm, 
   proof?): SecretProofTransactionV1Descriptor
```

Creates a descriptor for SecretProofTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientAddress` | [`Address`](../../../classes/Address.md) | Address that receives the funds once unlocked. |
| `secret` | [`Hash256`](../../../../core/classes/Hash256.md) | Hashed proof. |
| `hashAlgorithm` | [`LockHashAlgorithm`](../../models/classes/LockHashAlgorithm.md) | Algorithm used to hash the proof. |
| `proof`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Original random set of bytes that were hashed. |

#### Returns

[`SecretProofTransactionV1Descriptor`](SecretProofTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.hashAlgorithm` | [`LockHashAlgorithm`](../../models/classes/LockHashAlgorithm.md) |
| `rawDescriptor.recipientAddress` | [`Address`](../../../classes/Address.md) |
| `rawDescriptor.secret` | [`Hash256`](../../../../core/classes/Hash256.md) |
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
