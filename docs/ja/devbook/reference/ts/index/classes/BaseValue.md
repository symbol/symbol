# Class: BaseValue

Represents a base integer.

## Extended by

- [`Amount`](../../symbol/namespaces/models/classes/Amount.md)
- [`BlockDuration`](../../symbol/namespaces/models/classes/BlockDuration.md)
- [`BlockFeeMultiplier`](../../symbol/namespaces/models/classes/BlockFeeMultiplier.md)
- [`Difficulty`](../../symbol/namespaces/models/classes/Difficulty.md)
- [`FinalizationEpoch`](../../symbol/namespaces/models/classes/FinalizationEpoch.md)
- [`FinalizationPoint`](../../symbol/namespaces/models/classes/FinalizationPoint.md)
- [`Height`](../../symbol/namespaces/models/classes/Height.md)
- [`Importance`](../../symbol/namespaces/models/classes/Importance.md)
- [`ImportanceHeight`](../../symbol/namespaces/models/classes/ImportanceHeight.md)
- [`UnresolvedMosaicId`](../../symbol/namespaces/models/classes/UnresolvedMosaicId.md)
- [`MosaicId`](../../symbol/namespaces/models/classes/MosaicId.md)
- [`Timestamp`](../../symbol/namespaces/models/classes/Timestamp.md)
- [`NamespaceId`](../../symbol/namespaces/models/classes/NamespaceId.md)
- [`MosaicNonce`](../../symbol/namespaces/models/classes/MosaicNonce.md)
- [`MosaicRestrictionKey`](../../symbol/namespaces/models/classes/MosaicRestrictionKey.md)

## Constructors

### new BaseValue()

```ts
new BaseValue(
   size, 
   value, 
   isSigned?): BaseValue
```

Creates a base value.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `size` | `number` | Size of the integer. |
| `value` | `number` \| `bigint` | Value. |
| `isSigned`? | `boolean` | \c true if the value should be treated as signed. |

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
toJson(): string | number
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string` \| `number`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string
```

Converts base value to string.

#### Returns

`string`

String representation.
