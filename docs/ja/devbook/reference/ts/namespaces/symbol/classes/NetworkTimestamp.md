# Class: NetworkTimestamp

Represents a Symbol network timestamp with millisecond resolution.

## Extends

- `NetworkTimestamp`

## Constructors

### new NetworkTimestamp()

```ts
new NetworkTimestamp(timestamp): NetworkTimestamp
```

Creates a timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `timestamp` | `number` \| `bigint` | Raw network timestamp. |

#### Returns

[`NetworkTimestamp`](NetworkTimestamp.md)

#### Inherited from

```ts
BasicNetworkTimestamp.constructor
```

## Properties

| Property | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ |
| <a id="timestamp-1"></a> `timestamp` | `bigint` | Underlying timestamp. | `BasicNetworkTimestamp.timestamp` |

## Accessors

### isEpochal

#### Get Signature

```ts
get isEpochal(): boolean
```

Determines if this is the epochal timestamp.

##### Returns

`boolean`

\c true if this is the epochal timestamp.

#### Inherited from

```ts
BasicNetworkTimestamp.isEpochal
```

## Methods

### addHours()

```ts
addHours(count): NetworkTimestamp
```

Adds a specified number of hours to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of hours to add. |

#### Returns

`NetworkTimestamp`

New timestamp that is the specified number of hours past this timestamp.

#### Inherited from

```ts
BasicNetworkTimestamp.addHours
```

***

### addMilliseconds()

```ts
addMilliseconds(count): NetworkTimestamp
```

Adds a specified number of milliseconds to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of milliseconds to add. |

#### Returns

[`NetworkTimestamp`](NetworkTimestamp.md)

New timestamp that is the specified number of milliseconds past this timestamp.

***

### addMinutes()

```ts
addMinutes(count): NetworkTimestamp
```

Adds a specified number of minutes to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of minutes to add. |

#### Returns

`NetworkTimestamp`

New timestamp that is the specified number of minutes past this timestamp.

#### Inherited from

```ts
BasicNetworkTimestamp.addMinutes
```

***

### addSeconds()

```ts
addSeconds(count): NetworkTimestamp
```

Adds a specified number of seconds to this timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `count` | `number` \| `bigint` | Number of seconds to add. |

#### Returns

[`NetworkTimestamp`](NetworkTimestamp.md)

New timestamp that is the specified number of seconds past this timestamp.

#### Overrides

```ts
BasicNetworkTimestamp.addSeconds
```

***

### toString()

```ts
toString(): string
```

Returns string representation of this object.

#### Returns

`string`

String representation of this object

#### Inherited from

```ts
BasicNetworkTimestamp.toString
```
