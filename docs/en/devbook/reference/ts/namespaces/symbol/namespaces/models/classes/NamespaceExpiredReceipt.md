# Class: NamespaceExpiredReceipt

## Extends

- [`Receipt`](Receipt.md)

## Constructors

### new NamespaceExpiredReceipt()

```ts
new NamespaceExpiredReceipt(): NamespaceExpiredReceipt
```

#### Returns

[`NamespaceExpiredReceipt`](NamespaceExpiredReceipt.md)

#### Inherited from

[`Receipt`](Receipt.md).[`constructor`](Receipt.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_artifactid"></a> `_artifactId` | `public` | [`NamespaceId`](NamespaceId.md) | - | - |
| <a id="_type"></a> `_type` | `public` | [`ReceiptType`](ReceiptType.md) | - | [`Receipt`](Receipt.md).[`_type`](Receipt.md#_type) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`Receipt`](Receipt.md).[`_version`](Receipt.md#_version) |
| <a id="receipt_type"></a> `RECEIPT_TYPE` | `static` | [`ReceiptType`](ReceiptType.md) | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`Receipt`](Receipt.md).[`TYPE_HINTS`](Receipt.md#type_hints) | - |
| `TYPE_HINTS.artifactId` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

## Accessors

### artifactId

#### Get Signature

```ts
get artifactId(): NamespaceId
```

##### Returns

[`NamespaceId`](NamespaceId.md)

#### Set Signature

```ts
set artifactId(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`NamespaceId`](NamespaceId.md) |

##### Returns

`void`

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

#### Inherited from

[`Receipt`](Receipt.md).[`size`](Receipt.md#size)

***

### type

#### Get Signature

```ts
get type(): ReceiptType
```

##### Returns

[`ReceiptType`](ReceiptType.md)

#### Set Signature

```ts
set type(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`ReceiptType`](ReceiptType.md) |

##### Returns

`void`

#### Inherited from

[`Receipt`](Receipt.md).[`type`](Receipt.md#type)

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

[`Receipt`](Receipt.md).[`version`](Receipt.md#version)

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

[`Receipt`](Receipt.md).[`_serialize`](Receipt.md#_serialize)

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

#### Inherited from

[`Receipt`](Receipt.md).[`serialize`](Receipt.md#serialize)

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

#### Inherited from

[`Receipt`](Receipt.md).[`sort`](Receipt.md#sort)

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

#### Inherited from

[`Receipt`](Receipt.md).[`toJson`](Receipt.md#tojson)

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

#### Inherited from

[`Receipt`](Receipt.md).[`toString`](Receipt.md#tostring)

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

[`Receipt`](Receipt.md).[`_deserialize`](Receipt.md#_deserialize)

***

### \_deserializeAligned()

```ts
static _deserializeAligned(view, instance): void
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `view` | `any` |
| `instance` | `any` |

#### Returns

`void`

#### Inherited from

[`Receipt`](Receipt.md).[`_deserializeAligned`](Receipt.md#_deserializealigned)

***

### deserialize()

```ts
static deserialize(payload): NamespaceExpiredReceipt
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`NamespaceExpiredReceipt`](NamespaceExpiredReceipt.md)

***

### deserializeAligned()

```ts
static deserializeAligned(payload): NamespaceExpiredReceipt
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`NamespaceExpiredReceipt`](NamespaceExpiredReceipt.md)
