# Class: FinalizationRound

## Constructors

### new FinalizationRound()

```ts
new FinalizationRound(): FinalizationRound
```

#### Returns

[`FinalizationRound`](FinalizationRound.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_epoch"></a> `_epoch` | `public` | [`FinalizationEpoch`](FinalizationEpoch.md) |
| <a id="_point"></a> `_point` | `public` | [`FinalizationPoint`](FinalizationPoint.md) |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.epoch` | `public` | `string` |
| `TYPE_HINTS.point` | `public` | `string` |

## Accessors

### epoch

#### Get Signature

```ts
get epoch(): FinalizationEpoch
```

##### Returns

[`FinalizationEpoch`](FinalizationEpoch.md)

#### Set Signature

```ts
set epoch(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`FinalizationEpoch`](FinalizationEpoch.md) |

##### Returns

`void`

***

### point

#### Get Signature

```ts
get point(): FinalizationPoint
```

##### Returns

[`FinalizationPoint`](FinalizationPoint.md)

#### Set Signature

```ts
set point(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`FinalizationPoint`](FinalizationPoint.md) |

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
static deserialize(payload): FinalizationRound
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`FinalizationRound`](FinalizationRound.md)
