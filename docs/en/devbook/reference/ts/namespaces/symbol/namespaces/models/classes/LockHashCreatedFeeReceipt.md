# Class: LockHashCreatedFeeReceipt

## Extends

- [`Receipt`](Receipt.md)

## Constructors

### new LockHashCreatedFeeReceipt()

```ts
new LockHashCreatedFeeReceipt(): LockHashCreatedFeeReceipt
```

#### Returns

[`LockHashCreatedFeeReceipt`](LockHashCreatedFeeReceipt.md)

#### Inherited from

[`Receipt`](Receipt.md).[`constructor`](Receipt.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_mosaic"></a> `_mosaic` | `public` | [`Mosaic`](Mosaic.md) | - | - |
| <a id="_targetaddress"></a> `_targetAddress` | `public` | [`Address`](Address.md) | - | - |
| <a id="_type"></a> `_type` | `public` | [`ReceiptType`](ReceiptType.md) | - | [`Receipt`](Receipt.md).[`_type`](Receipt.md#_type) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`Receipt`](Receipt.md).[`_version`](Receipt.md#_version) |
| <a id="receipt_type"></a> `RECEIPT_TYPE` | `static` | [`ReceiptType`](ReceiptType.md) | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`Receipt`](Receipt.md).[`TYPE_HINTS`](Receipt.md#type_hints) | - |
| `TYPE_HINTS.mosaic` | `public` | `string` | - | - |
| `TYPE_HINTS.targetAddress` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

## Accessors

### mosaic

#### Get Signature

```ts
get mosaic(): Mosaic
```

##### Returns

[`Mosaic`](Mosaic.md)

#### Set Signature

```ts
set mosaic(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Mosaic`](Mosaic.md) |

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

### targetAddress

#### Get Signature

```ts
get targetAddress(): Address
```

##### Returns

[`Address`](Address.md)

#### Set Signature

```ts
set targetAddress(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Address`](Address.md) |

##### Returns

`void`

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
static deserialize(payload): LockHashCreatedFeeReceipt
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`LockHashCreatedFeeReceipt`](LockHashCreatedFeeReceipt.md)

***

### deserializeAligned()

```ts
static deserializeAligned(payload): LockHashCreatedFeeReceipt
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`LockHashCreatedFeeReceipt`](LockHashCreatedFeeReceipt.md)
