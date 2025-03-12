# Class: AggregateBondedTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AggregateBondedTransactionV1Descriptor.

Propose an arrangement of transactions between different accounts (V1, deprecated).
Use this transaction when not all required signatures are available when the transaction is created.
Missing signatures must be provided using a Cosignature or DetachedCosignature.
To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be successfully announced and confirmed.

## Constructors

### new AggregateBondedTransactionV1Descriptor()

```ts
new AggregateBondedTransactionV1Descriptor(
   transactionsHash, 
   transactions?, 
   cosignatures?): AggregateBondedTransactionV1Descriptor
```

Creates a descriptor for AggregateBondedTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionsHash` | [`Hash256`](../../../../core/classes/Hash256.md) | Hash of the aggregate's transaction. |
| `transactions`? | [`EmbeddedTransaction`](../../models/classes/EmbeddedTransaction.md)[] | Embedded transaction data. Transactions are variable-sized and the total payload size is in bytes. Embedded transactions cannot be aggregates. |
| `cosignatures`? | [`Cosignature`](../../models/classes/Cosignature.md)[] | Cosignatures data. Fills up remaining body space after transactions. |

#### Returns

[`AggregateBondedTransactionV1Descriptor`](AggregateBondedTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.transactionsHash` | [`Hash256`](../../../../core/classes/Hash256.md) |
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
