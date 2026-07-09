# Class: BaseValue

Represents a base integer.

## Extended by

- [`Amount`](../symbol/models/Amount.md)
- [`BlockDuration`](../symbol/models/BlockDuration.md)
- [`BlockFeeMultiplier`](../symbol/models/BlockFeeMultiplier.md)
- [`Difficulty`](../symbol/models/Difficulty.md)
- [`FinalizationEpoch`](../symbol/models/FinalizationEpoch.md)
- [`FinalizationPoint`](../symbol/models/FinalizationPoint.md)
- [`Height`](../symbol/models/Height.md)
- [`Importance`](../symbol/models/Importance.md)
- [`ImportanceHeight`](../symbol/models/ImportanceHeight.md)
- [`UnresolvedMosaicId`](../symbol/models/UnresolvedMosaicId.md)
- [`MosaicId`](../symbol/models/MosaicId.md)
- [`Timestamp`](../symbol/models/Timestamp.md)
- [`NamespaceId`](../symbol/models/NamespaceId.md)
- [`MosaicNonce`](../symbol/models/MosaicNonce.md)
- [`MosaicRestrictionKey`](../symbol/models/MosaicRestrictionKey.md)

## Constructors

### Constructor

```ts
new BaseValue(
   size, 
   value, 
   isSigned?): BaseValue;
```

Creates a base value.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `size` | `number` | Size of the integer. |
| `value` | `number` \| `bigint` | Value. |
| `isSigned?` | `boolean` | \c true if the value should be treated as signed. |

#### Returns

`BaseValue`

## Properties

| Property | Type | Description |
| ------ | ------ | ------ |
| <a id="issigned"></a> `isSigned` | `boolean` | \c true if the value should be treated as signed. |
| <a id="size"></a> `size` | `number` | Size of the integer. |
| <a id="value"></a> `value` | `number` \| `bigint` | Value. |

## Methods

### toJson()

```ts
toJson(): string | number;
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string` \| `number`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string;
```

Converts base value to string.

#### Returns

`string`

String representation.
