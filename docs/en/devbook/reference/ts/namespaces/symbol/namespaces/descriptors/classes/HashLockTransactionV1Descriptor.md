# Class: HashLockTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for HashLockTransactionV1Descriptor.

Lock a deposit needed to announce an AggregateBondedTransaction (V1, latest).
An AggregateBondedTransaction consumes network resources as it is stored in every node's partial cache while it waits to be fully signed. To avoid spam attacks a HashLockTransaction must be announced and confirmed before an AggregateBondedTransaction can be announced. The HashLockTransaction locks a certain amount of funds (**10** XYM by default) until the aggregate is signed.
Upon completion of the aggregate, the locked funds become available again to the account that signed the HashLockTransaction.
If the lock expires before the aggregate is signed by all cosignatories (**48h by default), the locked funds become a reward collected by the block harvester at the height where the lock expires.
\note It is not necessary to sign the aggregate and its HashLockTransaction with the same account. For example, if Bob wants to announce an aggregate and does not have enough funds to announce a HashLockTransaction, he can ask Alice to announce the lock transaction for him by sharing the signed AggregateTransaction hash.

## Constructors

### new HashLockTransactionV1Descriptor()

```ts
new HashLockTransactionV1Descriptor(
   mosaic, 
   duration, 
   hash): HashLockTransactionV1Descriptor
```

Creates a descriptor for HashLockTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `mosaic` | [`UnresolvedMosaicDescriptor`](UnresolvedMosaicDescriptor.md) | Locked mosaic. |
| `duration` | [`BlockDuration`](../../models/classes/BlockDuration.md) | Number of blocks for which a lock should be valid. The default maximum is 48h (See the `maxHashLockDuration` network property). |
| `hash` | [`Hash256`](../../../../core/classes/Hash256.md) | Hash of the AggregateBondedTransaction to be confirmed before unlocking the mosaics. |

#### Returns

[`HashLockTransactionV1Descriptor`](HashLockTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.duration` | [`BlockDuration`](../../models/classes/BlockDuration.md) |
| `rawDescriptor.hash` | [`Hash256`](../../../../core/classes/Hash256.md) |
| `rawDescriptor.mosaic` | `any` |
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
