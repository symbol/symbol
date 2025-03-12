# Class: EmbeddedAccountAddressRestrictionTransactionV1

## Extends

- [`EmbeddedTransaction`](EmbeddedTransaction.md)

## Constructors

### new EmbeddedAccountAddressRestrictionTransactionV1()

```ts
new EmbeddedAccountAddressRestrictionTransactionV1(): EmbeddedAccountAddressRestrictionTransactionV1
```

#### Returns

[`EmbeddedAccountAddressRestrictionTransactionV1`](EmbeddedAccountAddressRestrictionTransactionV1.md)

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`constructor`](EmbeddedTransaction.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_accountrestrictiontransactionbodyreserved_1"></a> `_accountRestrictionTransactionBodyReserved_1` | `public` | `number` | - | - |
| <a id="_embeddedtransactionheaderreserved_1"></a> `_embeddedTransactionHeaderReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_embeddedTransactionHeaderReserved_1`](EmbeddedTransaction.md#_embeddedtransactionheaderreserved_1) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_entityBodyReserved_1`](EmbeddedTransaction.md#_entitybodyreserved_1) |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_network`](EmbeddedTransaction.md#_network) |
| <a id="_restrictionadditions"></a> `_restrictionAdditions` | `public` | `any`[] | - | - |
| <a id="_restrictiondeletions"></a> `_restrictionDeletions` | `public` | `any`[] | - | - |
| <a id="_restrictionflags"></a> `_restrictionFlags` | `public` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) | - | - |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_signerPublicKey`](EmbeddedTransaction.md#_signerpublickey) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_type`](EmbeddedTransaction.md#_type) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_version`](EmbeddedTransaction.md#_version) |
| <a id="transaction_type"></a> `TRANSACTION_TYPE` | `static` | [`TransactionType`](TransactionType.md) | - | - |
| <a id="transaction_version"></a> `TRANSACTION_VERSION` | `static` | `number` | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`TYPE_HINTS`](EmbeddedTransaction.md#type_hints) | - |
| `TYPE_HINTS.network` | `public` | `string` | - | - |
| `TYPE_HINTS.restrictionAdditions` | `public` | `string` | - | - |
| `TYPE_HINTS.restrictionDeletions` | `public` | `string` | - | - |
| `TYPE_HINTS.restrictionFlags` | `public` | `string` | - | - |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

## Accessors

### network

#### Get Signature

```ts
get network(): NetworkType
```

##### Returns

[`NetworkType`](NetworkType.md)

#### Set Signature

```ts
set network(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`NetworkType`](NetworkType.md) |

##### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`network`](EmbeddedTransaction.md#network)

***

### restrictionAdditions

#### Get Signature

```ts
get restrictionAdditions(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set restrictionAdditions(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

##### Returns

`void`

***

### restrictionDeletions

#### Get Signature

```ts
get restrictionDeletions(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set restrictionDeletions(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

##### Returns

`void`

***

### restrictionFlags

#### Get Signature

```ts
get restrictionFlags(): AccountRestrictionFlags
```

##### Returns

[`AccountRestrictionFlags`](AccountRestrictionFlags.md)

#### Set Signature

```ts
set restrictionFlags(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |

##### Returns

`void`

***

### signerPublicKey

#### Get Signature

```ts
get signerPublicKey(): PublicKey
```

##### Returns

[`PublicKey`](PublicKey.md)

#### Set Signature

```ts
set signerPublicKey(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`PublicKey`](PublicKey.md) |

##### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`signerPublicKey`](EmbeddedTransaction.md#signerpublickey)

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`size`](EmbeddedTransaction.md#size)

***

### type

#### Get Signature

```ts
get type(): TransactionType
```

##### Returns

[`TransactionType`](TransactionType.md)

#### Set Signature

```ts
set type(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`TransactionType`](TransactionType.md) |

##### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`type`](EmbeddedTransaction.md#type)

***

### version

#### Get Signature

```ts
get version(): number
```

##### Returns

`number`

#### Set Signature

```ts
set version(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `number` |

##### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`version`](EmbeddedTransaction.md#version)

## Methods

### \_serialize()

```ts
_serialize(buffer): void
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `buffer` | `any` |

#### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`_serialize`](EmbeddedTransaction.md#_serialize)

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`serialize`](EmbeddedTransaction.md#serialize)

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`sort`](EmbeddedTransaction.md#sort)

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`toJson`](EmbeddedTransaction.md#tojson)

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`toString`](EmbeddedTransaction.md#tostring)

***

### \_deserialize()

```ts
static _deserialize(view, instance): void
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `view` | `any` |
| `instance` | `any` |

#### Returns

`void`

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`_deserialize`](EmbeddedTransaction.md#_deserialize)

***

### deserialize()

```ts
static deserialize(payload): EmbeddedAccountAddressRestrictionTransactionV1
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`EmbeddedAccountAddressRestrictionTransactionV1`](EmbeddedAccountAddressRestrictionTransactionV1.md)
