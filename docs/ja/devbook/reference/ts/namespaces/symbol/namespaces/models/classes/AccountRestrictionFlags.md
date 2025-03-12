# Class: AccountRestrictionFlags

## Constructors

### new AccountRestrictionFlags()

```ts
new AccountRestrictionFlags(value): AccountRestrictionFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`AccountRestrictionFlags`](AccountRestrictionFlags.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="address"></a> `ADDRESS` | `static` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |
| <a id="block"></a> `BLOCK` | `static` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |
| <a id="mosaic_id"></a> `MOSAIC_ID` | `static` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |
| <a id="outgoing"></a> `OUTGOING` | `static` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |
| <a id="transaction_type"></a> `TRANSACTION_TYPE` | `static` | [`AccountRestrictionFlags`](AccountRestrictionFlags.md) |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

## Methods

### has()

```ts
has(flag): boolean
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `flag` | `any` |

#### Returns

`boolean`

***

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
static deserialize(payload): AccountRestrictionFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`AccountRestrictionFlags`](AccountRestrictionFlags.md)

***

### deserializeAligned()

```ts
static deserializeAligned(payload): AccountRestrictionFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`AccountRestrictionFlags`](AccountRestrictionFlags.md)
