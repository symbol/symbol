# Class: Receipt

## Extended by

- [`HarvestFeeReceipt`](HarvestFeeReceipt.md)
- [`InflationReceipt`](InflationReceipt.md)
- [`LockHashCreatedFeeReceipt`](LockHashCreatedFeeReceipt.md)
- [`LockHashCompletedFeeReceipt`](LockHashCompletedFeeReceipt.md)
- [`LockHashExpiredFeeReceipt`](LockHashExpiredFeeReceipt.md)
- [`LockSecretCreatedFeeReceipt`](LockSecretCreatedFeeReceipt.md)
- [`LockSecretCompletedFeeReceipt`](LockSecretCompletedFeeReceipt.md)
- [`LockSecretExpiredFeeReceipt`](LockSecretExpiredFeeReceipt.md)
- [`MosaicExpiredReceipt`](MosaicExpiredReceipt.md)
- [`MosaicRentalFeeReceipt`](MosaicRentalFeeReceipt.md)
- [`NamespaceExpiredReceipt`](NamespaceExpiredReceipt.md)
- [`NamespaceDeletedReceipt`](NamespaceDeletedReceipt.md)
- [`NamespaceRentalFeeReceipt`](NamespaceRentalFeeReceipt.md)

## Constructors

### new Receipt()

```ts
new Receipt(): Receipt
```

#### Returns

[`Receipt`](Receipt.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_type"></a> `_type` | `public` | [`ReceiptType`](ReceiptType.md) |
| <a id="_version"></a> `_version` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.type` | `public` | `string` |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

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

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

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
