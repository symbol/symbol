# Class: BlockType

## Constructors

### new BlockType()

```ts
new BlockType(value): BlockType
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`BlockType`](BlockType.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="importance"></a> `IMPORTANCE` | `static` | [`BlockType`](BlockType.md) |
| <a id="nemesis"></a> `NEMESIS` | `static` | [`BlockType`](BlockType.md) |
| <a id="normal"></a> `NORMAL` | `static` | [`BlockType`](BlockType.md) |

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
