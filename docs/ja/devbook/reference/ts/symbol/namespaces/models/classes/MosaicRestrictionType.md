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

`MosaicRestrictionType`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value"></a> `value` | `public` | `any` |
| <a id="eq"></a> `EQ` | `static` | `MosaicRestrictionType` |
| <a id="ge"></a> `GE` | `static` | `MosaicRestrictionType` |
| <a id="gt"></a> `GT` | `static` | `MosaicRestrictionType` |
| <a id="le"></a> `LE` | `static` | `MosaicRestrictionType` |
| <a id="lt"></a> `LT` | `static` | `MosaicRestrictionType` |
| <a id="ne"></a> `NE` | `static` | `MosaicRestrictionType` |
| <a id="none"></a> `NONE` | `static` | `MosaicRestrictionType` |

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
