# Class: EmbeddedNamespaceRegistrationTransactionV1

## Extends

- [`EmbeddedTransaction`](EmbeddedTransaction.md)

## Constructors

### new EmbeddedNamespaceRegistrationTransactionV1()

```ts
new EmbeddedNamespaceRegistrationTransactionV1(): EmbeddedNamespaceRegistrationTransactionV1
```

#### Returns

[`EmbeddedNamespaceRegistrationTransactionV1`](EmbeddedNamespaceRegistrationTransactionV1.md)

#### Inherited from

[`EmbeddedTransaction`](EmbeddedTransaction.md).[`constructor`](EmbeddedTransaction.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_duration"></a> `_duration` | `public` | [`BlockDuration`](BlockDuration.md) | - | - |
| <a id="_embeddedtransactionheaderreserved_1"></a> `_embeddedTransactionHeaderReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_embeddedTransactionHeaderReserved_1`](EmbeddedTransaction.md#_embeddedtransactionheaderreserved_1) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_entityBodyReserved_1`](EmbeddedTransaction.md#_entitybodyreserved_1) |
| <a id="_id"></a> `_id` | `public` | [`NamespaceId`](NamespaceId.md) | - | - |
| <a id="_name"></a> `_name` | `public` | `Uint8Array`&lt;`ArrayBuffer`&gt; | - | - |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_network`](EmbeddedTransaction.md#_network) |
| <a id="_parentid"></a> `_parentId` | `public` | `any` | - | - |
| <a id="_registrationtype"></a> `_registrationType` | `public` | [`NamespaceRegistrationType`](NamespaceRegistrationType.md) | - | - |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_signerPublicKey`](EmbeddedTransaction.md#_signerpublickey) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_type`](EmbeddedTransaction.md#_type) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`_version`](EmbeddedTransaction.md#_version) |
| <a id="transaction_type"></a> `TRANSACTION_TYPE` | `static` | [`TransactionType`](TransactionType.md) | - | - |
| <a id="transaction_version"></a> `TRANSACTION_VERSION` | `static` | `number` | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`EmbeddedTransaction`](EmbeddedTransaction.md).[`TYPE_HINTS`](EmbeddedTransaction.md#type_hints) | - |
| `TYPE_HINTS.duration` | `public` | `string` | - | - |
| `TYPE_HINTS.id` | `public` | `string` | - | - |
| `TYPE_HINTS.name` | `public` | `string` | - | - |
| `TYPE_HINTS.network` | `public` | `string` | - | - |
| `TYPE_HINTS.parentId` | `public` | `string` | - | - |
| `TYPE_HINTS.registrationType` | `public` | `string` | - | - |
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

### id

#### Get Signature

```ts
get id(): NamespaceId
```

##### Returns

[`NamespaceId`](NamespaceId.md)

#### Set Signature

```ts
set id(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`NamespaceId`](NamespaceId.md) |

##### Returns

`void`

***

### name

#### Get Signature

```ts
get name(): Uint8Array<ArrayBuffer>
```

##### Returns

`Uint8Array`&lt;`ArrayBuffer`&gt;

#### Set Signature

```ts
set name(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `Uint8Array`&lt;`ArrayBuffer`&gt; |

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

### parentId

#### Get Signature

```ts
get parentId(): any
```

##### Returns

`any`

#### Set Signature

```ts
set parentId(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

##### Returns

`void`

***

### registrationType

#### Get Signature

```ts
get registrationType(): NamespaceRegistrationType
```

##### Returns

[`NamespaceRegistrationType`](NamespaceRegistrationType.md)

#### Set Signature

```ts
set registrationType(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`NamespaceRegistrationType`](NamespaceRegistrationType.md) |

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
static deserialize(payload): EmbeddedNamespaceRegistrationTransactionV1
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`EmbeddedNamespaceRegistrationTransactionV1`](EmbeddedNamespaceRegistrationTransactionV1.md)
