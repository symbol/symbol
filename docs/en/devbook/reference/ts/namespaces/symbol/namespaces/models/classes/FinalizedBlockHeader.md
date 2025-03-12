# Class: FinalizedBlockHeader

## Constructors

### new FinalizedBlockHeader()

```ts
new FinalizedBlockHeader(): FinalizedBlockHeader
```

#### Returns

[`FinalizedBlockHeader`](FinalizedBlockHeader.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_hash"></a> `_hash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_height"></a> `_height` | `public` | [`Height`](Height.md) |
| <a id="_round"></a> `_round` | `public` | [`FinalizationRound`](FinalizationRound.md) |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.hash` | `public` | `string` |
| `TYPE_HINTS.height` | `public` | `string` |
| `TYPE_HINTS.round` | `public` | `string` |

## Accessors

### hash

#### Get Signature

```ts
get hash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set hash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

### height

#### Get Signature

```ts
get height(): Height
```

##### Returns

[`Height`](Height.md)

#### Set Signature

```ts
set height(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Height`](Height.md) |

##### Returns

`void`

***

### round

#### Get Signature

```ts
get round(): FinalizationRound
```

##### Returns

[`FinalizationRound`](FinalizationRound.md)

#### Set Signature

```ts
set round(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`FinalizationRound`](FinalizationRound.md) |

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

## Methods

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

### deserialize()

```ts
static deserialize(payload): FinalizedBlockHeader
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`FinalizedBlockHeader`](FinalizedBlockHeader.md)
