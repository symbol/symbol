# Class: FinalizationPoint

Represents a base integer.

## Extends

- [`BaseValue`](../../../../core/classes/BaseValue.md)

## Constructors

### new FinalizationPoint()

```ts
new FinalizationPoint(finalizationPoint?): FinalizationPoint
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `finalizationPoint`? | `number` |

#### Returns

[`FinalizationPoint`](FinalizationPoint.md)

#### Overrides

[`BaseValue`](../../../../core/classes/BaseValue.md).[`constructor`](../../../../core/classes/BaseValue.md#constructors)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="issigned"></a> `isSigned` | `public` | `boolean` | \c true if the value should be treated as signed. | [`BaseValue`](../../../../core/classes/BaseValue.md).[`isSigned`](../../../../core/classes/BaseValue.md#issigned-1) |
| <a id="size"></a> `size` | `public` | `number` | Size of the integer. | [`BaseValue`](../../../../core/classes/BaseValue.md).[`size`](../../../../core/classes/BaseValue.md#size-1) |
| <a id="value"></a> `value` | `public` | `number` \| `bigint` | Value. | [`BaseValue`](../../../../core/classes/BaseValue.md).[`value`](../../../../core/classes/BaseValue.md#value-1) |
| <a id="size-1"></a> `SIZE` | `static` | `number` | - | - |

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
toJson(): string | number
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string` \| `number`

JSON-safe representation of this object.

#### Inherited from

[`BaseValue`](../../../../core/classes/BaseValue.md).[`toJson`](../../../../core/classes/BaseValue.md#tojson)

***

### toString()

```ts
toString(): string
```

Converts base value to string.

#### Returns

`string`

String representation.

#### Inherited from

[`BaseValue`](../../../../core/classes/BaseValue.md).[`toString`](../../../../core/classes/BaseValue.md#tostring)

***

### deserialize()

```ts
static deserialize(payload): FinalizationPoint
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`FinalizationPoint`](FinalizationPoint.md)

***

### deserializeAligned()

```ts
static deserializeAligned(payload): FinalizationPoint
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`FinalizationPoint`](FinalizationPoint.md)
