# Class: AggregateCompleteTransactionV2Descriptor

Type safe descriptor used to generate a descriptor map for AggregateCompleteTransactionV2Descriptor.

Send transactions in batches to different accounts (V2, latest).
Use this transaction when all required signatures are available when the transaction is created.

## Constructors

### new AggregateCompleteTransactionV2Descriptor()

```ts
new AggregateCompleteTransactionV2Descriptor(
   transactionsHash, 
   transactions?, 
   cosignatures?): AggregateCompleteTransactionV2Descriptor
```

Creates a descriptor for AggregateCompleteTransactionV2.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionsHash` | [`Hash256`](../../../../core/classes/Hash256.md) | Hash of the aggregate's transaction. |
| `transactions`? | [`EmbeddedTransaction`](../../models/classes/EmbeddedTransaction.md)[] | Embedded transaction data. Transactions are variable-sized and the total payload size is in bytes. Embedded transactions cannot be aggregates. |
| `cosignatures`? | [`Cosignature`](../../models/classes/Cosignature.md)[] | Cosignatures data. Fills up remaining body space after transactions. |

#### Returns

[`AggregateCompleteTransactionV2Descriptor`](AggregateCompleteTransactionV2Descriptor.md)

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
