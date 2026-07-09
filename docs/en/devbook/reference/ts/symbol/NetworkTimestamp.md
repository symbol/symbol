# Class: NetworkTimestamp

Represents a Symbol network timestamp with millisecond resolution.

## Extends

- [`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md)

## Constructors

### Constructor

```ts
new NetworkTimestamp(timestamp): NetworkTimestamp;
```

Creates a timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `timestamp` | `number` \| `bigint` | Raw network timestamp. |

#### Returns

`NetworkTimestamp`

#### Inherited from

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`constructor`](../NetworkTimestamp/NetworkTimestamp.md#constructor)

## Properties

| Property | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ |
| <a id="timestamp"></a> `timestamp` | `bigint` | Underlying timestamp. | [`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`timestamp`](../NetworkTimestamp/NetworkTimestamp.md#timestamp) |

## Accessors

### isEpochal

#### Get Signature

```ts
get isEpochal(): boolean;
```

Determines if this is the epochal timestamp.

##### Returns

`boolean`

\c true if this is the epochal timestamp.

#### Inherited from

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`isEpochal`](../NetworkTimestamp/NetworkTimestamp.md#isepochal)

## Methods

### addHours()

```ts
addHours(count): NetworkTimestamp;
```

Adds a specified number of hours to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of hours to add. |

#### Returns

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md)

New timestamp that is the specified number of hours past this timestamp.

#### Inherited from

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`addHours`](../NetworkTimestamp/NetworkTimestamp.md#addhours)

***

### addMilliseconds()

```ts
addMilliseconds(count): NetworkTimestamp;
```

Adds a specified number of milliseconds to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of milliseconds to add. |

#### Returns

`NetworkTimestamp`

New timestamp that is the specified number of milliseconds past this timestamp.

***

### addMinutes()

```ts
addMinutes(count): NetworkTimestamp;
```

Adds a specified number of minutes to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of minutes to add. |

#### Returns

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md)

New timestamp that is the specified number of minutes past this timestamp.

#### Inherited from

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`addMinutes`](../NetworkTimestamp/NetworkTimestamp.md#addminutes)

***

### addSeconds()

```ts
addSeconds(count): NetworkTimestamp;
```

Adds a specified number of seconds to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of seconds to add. |

#### Returns

`NetworkTimestamp`

New timestamp that is the specified number of seconds past this timestamp.

#### Overrides

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`addSeconds`](../NetworkTimestamp/NetworkTimestamp.md#addseconds)

***

### toString()

```ts
toString(): string;
```

Returns string representation of this object.

#### Returns

`string`

String representation of this object

#### Inherited from

[`NetworkTimestamp`](../NetworkTimestamp/NetworkTimestamp.md).[`toString`](../NetworkTimestamp/NetworkTimestamp.md#tostring)
