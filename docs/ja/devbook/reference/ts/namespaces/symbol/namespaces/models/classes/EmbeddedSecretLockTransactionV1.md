# Class: EmbeddedSecretLockTransactionV1

## Extends

- [`EmbeddedTransaction`](EmbeddedTransaction.md)

## Constructors

### new EmbeddedSecretLockTransactionV1()

```ts
new EmbeddedSecretLockTransactionV1(): EmbeddedSecretLockTransactionV1
```

#### Returns

[`EmbeddedSecretLockTransactionV1`](EmbeddedSecretLockTransactionV1.md)

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`constructor`](EmbeddedTransaction.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_duration"></a> `_duration` | `public` | [`BlockDuration`](BlockDuration.md) | - | - |
| <a id="_embeddedtransactionheaderreserved_1"></a> `_embeddedTransactionHeaderReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_embeddedTransactionHeaderReserved_1`](EmbeddedTransaction.md#_embeddedtransactionheaderreserved_1) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_entityBodyReserved_1`](EmbeddedTransaction.md#_entitybodyreserved_1) |
| <a id="_hashalgorithm"></a> `_hashAlgorithm` | `public` | [`LockHashAlgorithm`](LockHashAlgorithm.md) | - | - |
| <a id="_mosaic"></a> `_mosaic` | `public` | [`UnresolvedMosaic`](UnresolvedMosaic.md) | - | - |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_network`](EmbeddedTransaction.md#_network) |
| <a id="_recipientaddress"></a> `_recipientAddress` | `public` | [`UnresolvedAddress`](UnresolvedAddress.md) | - | - |
| <a id="_secret"></a> `_secret` | `public` | [`Hash256`](Hash256.md) | - | - |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_signerPublicKey`](EmbeddedTransaction.md#_signerpublickey) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_type`](EmbeddedTransaction.md#_type) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_version`](EmbeddedTransaction.md#_version) |
| <a id="transaction_type"></a> `TRANSACTION_TYPE` | `static` | [`TransactionType`](TransactionType.md) | - | - |
| <a id="transaction_version"></a> `TRANSACTION_VERSION` | `static` | `number` | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`TYPE_HINTS`](EmbeddedTransaction.md#type_hints) | - |
| `TYPE_HINTS.duration` | `public` | `string` | - | - |
| `TYPE_HINTS.hashAlgorithm` | `public` | `string` | - | - |
| `TYPE_HINTS.mosaic` | `public` | `string` | - | - |
| `TYPE_HINTS.network` | `public` | `string` | - | - |
| `TYPE_HINTS.recipientAddress` | `public` | `string` | - | - |
| `TYPE_HINTS.secret` | `public` | `string` | - | - |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

## Accessors

### duration

#### Get Signature

```ts
get duration(): BlockDuration
```

##### Returns

[`BlockDuration`](BlockDuration.md)

#### Set Signature

```ts
set duration(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`BlockDuration`](BlockDuration.md) |

##### Returns

`void`

***

### hashAlgorithm

#### Get Signature

```ts
get hashAlgorithm(): LockHashAlgorithm
```

##### Returns

[`LockHashAlgorithm`](LockHashAlgorithm.md)

#### Set Signature

```ts
set hashAlgorithm(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`LockHashAlgorithm`](LockHashAlgorithm.md) |

##### Returns

`void`

***

### mosaic

#### Get Signature

```ts
get mosaic(): UnresolvedMosaic
```

##### Returns

[`UnresolvedMosaic`](UnresolvedMosaic.md)

#### Set Signature

```ts
set mosaic(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`UnresolvedMosaic`](UnresolvedMosaic.md) |

##### Returns

`void`

***

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

### recipientAddress

#### Get Signature

```ts
get recipientAddress(): UnresolvedAddress
```

##### Returns

[`UnresolvedAddress`](UnresolvedAddress.md)

#### Set Signature

```ts
set recipientAddress(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`UnresolvedAddress`](UnresolvedAddress.md) |

##### Returns

`void`

***

### secret

#### Get Signature

```ts
get secret(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set secret(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

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
static deserialize(payload): EmbeddedSecretLockTransactionV1
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`EmbeddedSecretLockTransactionV1`](EmbeddedSecretLockTransactionV1.md)
