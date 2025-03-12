# Class: SymbolTransactionFactory

Factory for creating Symbol transactions.

## Constructors

### new SymbolTransactionFactory()

```ts
new SymbolTransactionFactory(network, typeRuleOverrides?): SymbolTransactionFactory
```

Creates a factory for the specified network.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `network` | [`Network`](Network.md) | Symbol network. |
| `typeRuleOverrides`? | `Map`&lt;`string`, `Function`&gt; | Type rule overrides. |

#### Returns

[`SymbolTransactionFactory`](SymbolTransactionFactory.md)

## Accessors

### ruleNames

#### Get Signature

```ts
get ruleNames(): string[]
```

Gets rule names with registered hints.

##### Returns

`string`[]

Rule names with registered hints.

***

### static

#### Get Signature

```ts
get static(): typeof SymbolTransactionFactory
```

Gets class type.

##### Returns

*typeof* [`SymbolTransactionFactory`](SymbolTransactionFactory.md)

Class type.

## Methods

### create()

```ts
create(transactionDescriptor, autosort?): Transaction
```

Creates a transaction from a transaction descriptor.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionDescriptor` | `object` | Transaction descriptor. |
| `autosort`? | `boolean` | When set (default), descriptor arrays requiring ordering will be automatically sorted. When unset, descriptor arrays will be presumed to be already sorted. |

#### Returns

[`Transaction`](../namespaces/models/classes/Transaction.md)

Newly created transaction.

***

### createEmbedded()

```ts
createEmbedded(transactionDescriptor, autosort?): EmbeddedTransaction
```

Creates an embedded transaction from a transaction descriptor.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionDescriptor` | `object` | Transaction descriptor. |
| `autosort`? | `boolean` | When set (default), descriptor arrays requiring ordering will be automatically sorted. When unset, descriptor arrays will be presumed to be already sorted. |

#### Returns

[`EmbeddedTransaction`](../namespaces/models/classes/EmbeddedTransaction.md)

Newly created transaction.

***

### attachSignature()

```ts
static attachSignature(transaction, signature): string
```

Attaches a signature to a transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |
| `signature` | [`Signature`](../../core/classes/Signature.md) | Signature to attach. |

#### Returns

`string`

JSON transaction payload.

***

### deserialize()

```ts
static deserialize(payload): Transaction
```

Deserializes a transaction from a binary payload.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `payload` | `Uint8Array` | Binary payload. |

#### Returns

[`Transaction`](../namespaces/models/classes/Transaction.md)

Deserialized transaction.

***

### deserializeEmbedded()

```ts
static deserializeEmbedded(payload): EmbeddedTransaction
```

Deserializes an embedded transaction from a binary payload.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `payload` | `Uint8Array` | Binary payload. |

#### Returns

[`EmbeddedTransaction`](../namespaces/models/classes/EmbeddedTransaction.md)

Deserialized embedded transaction.

***

### lookupTransactionName()

```ts
static lookupTransactionName(transactionType, transactionVersion): string
```

Looks up the friendly name for the specified transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionType` | [`TransactionType`](../namespaces/models/classes/TransactionType.md) | Transaction type. |
| `transactionVersion` | `number` | Transaction version. |

#### Returns

`string`

Transaction friendly name.
