# Class: SecretLockTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for SecretLockTransactionV1Descriptor.

Start a token swap between different chains (V1, latest).
Use a SecretLockTransaction to transfer mosaics between two accounts. The mosaics sent remain locked until a valid SecretProofTransaction unlocks them.
The default expiration date is **365 days** after announcement (See the `maxSecretLockDuration` network property). If the lock expires before a valid SecretProofTransaction is announced the locked amount goes back to the initiator of the SecretLockTransaction.

## Constructors

### new SecretLockTransactionV1Descriptor()

```ts
new SecretLockTransactionV1Descriptor(
   recipientAddress, 
   secret, 
   mosaic, 
   duration, 
   hashAlgorithm): SecretLockTransactionV1Descriptor
```

Creates a descriptor for SecretLockTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientAddress` | [`Address`](../../../classes/Address.md) | Address that receives the funds once successfully unlocked by a SecretProofTransaction. |
| `secret` | [`Hash256`](../../../../core/classes/Hash256.md) | Hashed proof. |
| `mosaic` | [`UnresolvedMosaicDescriptor`](UnresolvedMosaicDescriptor.md) | Locked mosaics. |
| `duration` | [`BlockDuration`](../../models/classes/BlockDuration.md) | Number of blocks to wait for the SecretProofTransaction. |
| `hashAlgorithm` | [`LockHashAlgorithm`](../../models/classes/LockHashAlgorithm.md) | Algorithm used to hash the proof. |

#### Returns

[`SecretLockTransactionV1Descriptor`](SecretLockTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.duration` | [`BlockDuration`](../../models/classes/BlockDuration.md) |
| `rawDescriptor.hashAlgorithm` | [`LockHashAlgorithm`](../../models/classes/LockHashAlgorithm.md) |
| `rawDescriptor.mosaic` | `any` |
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
