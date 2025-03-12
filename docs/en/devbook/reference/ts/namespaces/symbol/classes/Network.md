# Class: Network

Represents a Symbol network.

## Extends

- `Network`&lt;`any`, `any`&gt;

## Constructors

### new Network()

```ts
new Network(
   name, 
   identifier, 
   epochTime, 
   generationHashSeed): Network
```

Creates a new network with the specified name, identifier byte and generation hash seed.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `name` | `string` | Network name. |
| `identifier` | `number` | Network identifier byte. |
| `epochTime` | `Date` | Network epoch time. |
| `generationHashSeed` | [`Hash256`](../../core/classes/Hash256.md) | Network generation hash seed. |

#### Returns

[`Network`](Network.md)

#### Overrides

```ts
BasicNetwork<any, any>.constructor
```

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="datetimeconverter"></a> `datetimeConverter` | `public` | `NetworkTimestampDatetimeConverter` | Network timestamp datetime converter associated with this network. | `BasicNetwork.datetimeConverter` |
| <a id="generationhashseed-1"></a> `generationHashSeed` | `public` | [`Hash256`](../../core/classes/Hash256.md) | Network generation hash seed. | - |
| <a id="identifier-1"></a> `identifier` | `public` | `number` | Network identifier byte. | `BasicNetwork.identifier` |
| <a id="name-1"></a> `name` | `public` | `string` | Network name. | `BasicNetwork.name` |
| <a id="mainnet"></a> `MAINNET` | `static` | [`Network`](Network.md) | Symbol main network. | - |
| <a id="networks"></a> `NETWORKS` | `static` | [`Network`](Network.md)[] | Symbol well known networks. | - |
| <a id="testnet"></a> `TESTNET` | `static` | [`Network`](Network.md) | Symbol test network. | - |

## Methods

### fromDatetime()

```ts
fromDatetime(referenceDatetime): any
```

Converts a datetime to a network timestamp.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `referenceDatetime` | `Date` | Reference datetime to convert. |

#### Returns

`any`

Network timestamp representation of the reference datetime.

#### Inherited from

```ts
BasicNetwork.fromDatetime
```

***

### isValidAddress()

```ts
isValidAddress(address): boolean
```

Checks if an address is valid and belongs to this network.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `address` | `any` | Address to check. |

#### Returns

`boolean`

\c true if address is valid and belongs to this network.

#### Inherited from

```ts
BasicNetwork.isValidAddress
```

***

### isValidAddressString()

```ts
isValidAddressString(addressString): boolean
```

Checks if an address string is valid and belongs to this network.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `addressString` | `string` | Address to check. |

#### Returns

`boolean`

\c true if address is valid and belongs to this network.

#### Inherited from

```ts
BasicNetwork.isValidAddressString
```

***

### publicKeyToAddress()

```ts
publicKeyToAddress(publicKey): any
```

Converts a public key to an address.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `publicKey` | [`PublicKey`](../../core/classes/PublicKey.md) | Public key to convert. |

#### Returns

`any`

Address corresponding to the public key input.

#### Inherited from

```ts
BasicNetwork.publicKeyToAddress
```

***

### toDatetime()

```ts
toDatetime(referenceNetworkTimestamp): Date
```

Converts a network timestamp to a datetime.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `referenceNetworkTimestamp` | `any` | Reference network timestamp to convert. |

#### Returns

`Date`

Datetime representation of the reference network timestamp.

#### Inherited from

```ts
BasicNetwork.toDatetime
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
BasicNetwork.toString
```
