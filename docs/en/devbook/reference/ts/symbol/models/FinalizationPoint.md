# Class: FinalizationPoint

Represents a base integer.

## Extends

- [`BaseValue`](../../index/BaseValue.md)

## Constructors

### Constructor

```ts
new FinalizationPoint(finalizationPoint?): FinalizationPoint;
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `finalizationPoint?` | `number` |

#### Returns

`FinalizationPoint`

#### Overrides

[`BaseValue`](../../index/BaseValue.md).[`constructor`](../../index/BaseValue.md#constructor)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="issigned"></a> `isSigned` | `public` | `boolean` | \c true if the value should be treated as signed. | [`BaseValue`](../../index/BaseValue.md).[`isSigned`](../../index/BaseValue.md#issigned) |
| <a id="size"></a> `size` | `public` | `number` | Size of the integer. | [`BaseValue`](../../index/BaseValue.md).[`size`](../../index/BaseValue.md#size) |
| <a id="value"></a> `value` | `public` | `number` \| `bigint` | Value. | [`BaseValue`](../../index/BaseValue.md).[`value`](../../index/BaseValue.md#value) |
| <a id="size-1"></a> `SIZE` | `static` | `number` | - | - |

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>;
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### toJson()

```ts
toJson(): string | number;
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string` \| `number`

JSON-safe representation of this object.

#### Inherited from

[`BaseValue`](../../index/BaseValue.md).[`toJson`](../../index/BaseValue.md#tojson)

***

### toString()

```ts
toString(): string;
```

Converts base value to string.

#### Returns

`string`

String representation.

#### Inherited from

[`BaseValue`](../../index/BaseValue.md).[`toString`](../../index/BaseValue.md#tostring)

***

### deserialize()

```ts
static deserialize(payload): FinalizationPoint;
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`FinalizationPoint`

***

### deserializeAligned()

```ts
static deserializeAligned(payload): FinalizationPoint;
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`FinalizationPoint`
