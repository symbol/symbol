# Class: AggregateBondedTransactionV2Descriptor

Type safe descriptor used to generate a descriptor map for AggregateBondedTransactionV2Descriptor.

Propose an arrangement of transactions between different accounts (V2, latest).
Use this transaction when not all required signatures are available when the transaction is created.
Missing signatures must be provided using a Cosignature or DetachedCosignature.
To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be successfully announced and confirmed.

## Constructors

### new AggregateBondedTransactionV2Descriptor()

```ts
new AggregateBondedTransactionV2Descriptor(
   transactionsHash, 
   transactions?, 
   cosignatures?): AggregateBondedTransactionV2Descriptor
```

Creates a descriptor for AggregateBondedTransactionV2.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionsHash` | [`Hash256`](../../../../core/classes/Hash256.md) | Hash of the aggregate's transaction. |
| `transactions`? | [`EmbeddedTransaction`](../../models/classes/EmbeddedTransaction.md)[] | Embedded transaction data. Transactions are variable-sized and the total payload size is in bytes. Embedded transactions cannot be aggregates. |
| `cosignatures`? | [`Cosignature`](../../models/classes/Cosignature.md)[] | Cosignatures data. Fills up remaining body space after transactions. |

#### Returns

[`AggregateBondedTransactionV2Descriptor`](AggregateBondedTransactionV2Descriptor.md)

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
