# Class: ReceiptType

## Constructors

### new ReceiptType()

```ts
new ReceiptType(value): ReceiptType
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`ReceiptType`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value"></a> `value` | `public` | `any` |
| <a id="address_alias_resolution"></a> `ADDRESS_ALIAS_RESOLUTION` | `static` | `ReceiptType` |
| <a id="harvest_fee"></a> `HARVEST_FEE` | `static` | `ReceiptType` |
| <a id="inflation"></a> `INFLATION` | `static` | `ReceiptType` |
| <a id="lock_hash_completed"></a> `LOCK_HASH_COMPLETED` | `static` | `ReceiptType` |
| <a id="lock_hash_created"></a> `LOCK_HASH_CREATED` | `static` | `ReceiptType` |
| <a id="lock_hash_expired"></a> `LOCK_HASH_EXPIRED` | `static` | `ReceiptType` |
| <a id="lock_secret_completed"></a> `LOCK_SECRET_COMPLETED` | `static` | `ReceiptType` |
| <a id="lock_secret_created"></a> `LOCK_SECRET_CREATED` | `static` | `ReceiptType` |
| <a id="lock_secret_expired"></a> `LOCK_SECRET_EXPIRED` | `static` | `ReceiptType` |
| <a id="mosaic_alias_resolution"></a> `MOSAIC_ALIAS_RESOLUTION` | `static` | `ReceiptType` |
| <a id="mosaic_expired"></a> `MOSAIC_EXPIRED` | `static` | `ReceiptType` |
| <a id="mosaic_rental_fee"></a> `MOSAIC_RENTAL_FEE` | `static` | `ReceiptType` |
| <a id="namespace_deleted"></a> `NAMESPACE_DELETED` | `static` | `ReceiptType` |
| <a id="namespace_expired"></a> `NAMESPACE_EXPIRED` | `static` | `ReceiptType` |
| <a id="namespace_rental_fee"></a> `NAMESPACE_RENTAL_FEE` | `static` | `ReceiptType` |
| <a id="transaction_group"></a> `TRANSACTION_GROUP` | `static` | `ReceiptType` |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### toJson()

```ts
toJson(): any
```

#### Returns

`any`

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

***

### deserialize()

```ts
static deserialize(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### deserializeAligned()

```ts
static deserializeAligned(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### fromValue()

```ts
static fromValue(value): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`any`

***

### valueToKey()

```ts
static valueToKey(value): string
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`string`
