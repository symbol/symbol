# Class: MosaicRestrictionType

## Constructors

### new MosaicRestrictionType()

```ts
new MosaicRestrictionType(value): MosaicRestrictionType
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`MosaicRestrictionType`](MosaicRestrictionType.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="eq"></a> `EQ` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="ge"></a> `GE` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="gt"></a> `GT` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="le"></a> `LE` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="lt"></a> `LT` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="ne"></a> `NE` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |
| <a id="none"></a> `NONE` | `static` | [`MosaicRestrictionType`](MosaicRestrictionType.md) |

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
