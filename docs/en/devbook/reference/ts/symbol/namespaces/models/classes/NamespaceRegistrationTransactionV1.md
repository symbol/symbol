# Class: NamespaceRegistrationTransactionV1

## Extends

- [`Transaction`](Transaction.md)

## Constructors

### new NamespaceRegistrationTransactionV1()

```ts
new NamespaceRegistrationTransactionV1(): NamespaceRegistrationTransactionV1
```

#### Returns

`NamespaceRegistrationTransactionV1`

#### Inherited from

[`Transaction`](Transaction.md).[`constructor`](Transaction.md#constructor)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_deadline"></a> `_deadline` | `public` | [`Timestamp`](Timestamp.md) | - | [`Transaction`](Transaction.md).[`_deadline`](Transaction.md#_deadline) |
| <a id="_duration"></a> `_duration` | `public` | [`BlockDuration`](BlockDuration.md) | - | - |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` | - | [`Transaction`](Transaction.md).[`_entityBodyReserved_1`](Transaction.md#_entitybodyreserved_1) |
| <a id="_fee"></a> `_fee` | `public` | [`Amount`](Amount.md) | - | [`Transaction`](Transaction.md).[`_fee`](Transaction.md#_fee) |
| <a id="_id"></a> `_id` | `public` | [`NamespaceId`](NamespaceId.md) | - | - |
| <a id="_name"></a> `_name` | `public` | `Uint8Array`&lt;`ArrayBuffer`&gt; | - | - |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) | - | [`Transaction`](Transaction.md).[`_network`](Transaction.md#_network) |
| <a id="_parentid"></a> `_parentId` | `public` | `any` | - | - |
| <a id="_registrationtype"></a> `_registrationType` | `public` | [`NamespaceRegistrationType`](NamespaceRegistrationType.md) | - | - |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) | - | [`Transaction`](Transaction.md).[`_signature`](Transaction.md#_signature) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) | - | [`Transaction`](Transaction.md).[`_signerPublicKey`](Transaction.md#_signerpublickey) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) | - | [`Transaction`](Transaction.md).[`_type`](Transaction.md#_type) |
| <a id="_verifiableentityheaderreserved_1"></a> `_verifiableEntityHeaderReserved_1` | `public` | `number` | - | [`Transaction`](Transaction.md).[`_verifiableEntityHeaderReserved_1`](Transaction.md#_verifiableentityheaderreserved_1) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`Transaction`](Transaction.md).[`_version`](Transaction.md#_version) |
| <a id="transaction_type"></a> `TRANSACTION_TYPE` | `static` | [`TransactionType`](TransactionType.md) | - | - |
| <a id="transaction_version"></a> `TRANSACTION_VERSION` | `static` | `number` | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`Transaction`](Transaction.md).[`TYPE_HINTS`](Transaction.md#type_hints) | - |
| `TYPE_HINTS.deadline` | `public` | `string` | - | - |
| `TYPE_HINTS.duration` | `public` | `string` | - | - |
| `TYPE_HINTS.fee` | `public` | `string` | - | - |
| `TYPE_HINTS.id` | `public` | `string` | - | - |
| `TYPE_HINTS.name` | `public` | `string` | - | - |
| `TYPE_HINTS.network` | `public` | `string` | - | - |
| `TYPE_HINTS.parentId` | `public` | `string` | - | - |
| `TYPE_HINTS.registrationType` | `public` | `string` | - | - |
| `TYPE_HINTS.signature` | `public` | `string` | - | - |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

## Accessors

### deadline

#### Get Signature

```ts
get deadline(): Timestamp
```

##### Returns

[`Timestamp`](Timestamp.md)

#### Set Signature

```ts
set deadline(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Timestamp`](Timestamp.md) |

##### Returns

`void`

#### Inherited from

[`Transaction`](Transaction.md).[`deadline`](Transaction.md#deadline)

***

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

### fee

#### Get Signature

```ts
get fee(): Amount
```

##### Returns

[`Amount`](Amount.md)

#### Set Signature

```ts
set fee(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Amount`](Amount.md) |

##### Returns

`void`

#### Inherited from

[`Transaction`](Transaction.md).[`fee`](Transaction.md#fee)

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

[`Transaction`](Transaction.md).[`network`](Transaction.md#network)

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

### signature

#### Get Signature

```ts
get signature(): Signature
```

##### Returns

[`Signature`](Signature.md)

#### Set Signature

```ts
set signature(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Signature`](Signature.md) |

##### Returns

`void`

#### Inherited from

[`Transaction`](Transaction.md).[`signature`](Transaction.md#signature)

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

[`Transaction`](Transaction.md).[`signerPublicKey`](Transaction.md#signerpublickey)

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

#### Inherited from

[`Transaction`](Transaction.md).[`size`](Transaction.md#size)

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

[`Transaction`](Transaction.md).[`type`](Transaction.md#type)

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

[`Transaction`](Transaction.md).[`version`](Transaction.md#version)

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

[`Transaction`](Transaction.md).[`_serialize`](Transaction.md#_serialize)

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

#### Inherited from

[`Transaction`](Transaction.md).[`serialize`](Transaction.md#serialize)

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

#### Inherited from

[`Transaction`](Transaction.md).[`sort`](Transaction.md#sort)

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

#### Inherited from

[`Transaction`](Transaction.md).[`toJson`](Transaction.md#tojson)

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

#### Inherited from

[`Transaction`](Transaction.md).[`toString`](Transaction.md#tostring)

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

[`Transaction`](Transaction.md).[`_deserialize`](Transaction.md#_deserialize)

***

### deserialize()

```ts
static deserialize(payload): NamespaceRegistrationTransactionV1
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`NamespaceRegistrationTransactionV1`
