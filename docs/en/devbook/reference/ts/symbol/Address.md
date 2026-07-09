# Class: Address

Represents a Symbol address.

## Extends

- [`ByteArray`](../index/ByteArray.md)

## Constructors

### Constructor

```ts
new Address(addressInput): Address;
```

Creates a Symbol address.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `addressInput` | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; \| `Address` | Input string, byte array or address. |

#### Returns

`Address`

#### Overrides

[`ByteArray`](../index/ByteArray.md).[`constructor`](../index/ByteArray.md#constructor)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="bytes"></a> `bytes` | `public` | `Uint8Array`&lt;`ArrayBuffer`&gt; | Underlying bytes. | [`ByteArray`](../index/ByteArray.md).[`bytes`](../index/ByteArray.md#bytes) |
| <a id="encoded_size"></a> `ENCODED_SIZE` | `static` | `number` | Length of encoded address string. | - |
| <a id="name"></a> `NAME` | `static` | `string` | Byte array name (required because `constructor.name` is dropped during minification). | [`ByteArray`](../index/ByteArray.md).[`NAME`](../index/ByteArray.md#name) |
| <a id="size"></a> `SIZE` | `static` | `number` | Byte size of raw address. | - |

## Methods

### isAlias()

```ts
isAlias(): boolean;
```

Determines if this address is an alias.

#### Returns

`boolean`

true if this address is an alias.

***

### toJson()

```ts
toJson(): string;
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string`

JSON-safe representation of this object.

#### Inherited from

[`ByteArray`](../index/ByteArray.md).[`toJson`](../index/ByteArray.md#tojson)

***

### toNamespaceId()

```ts
toNamespaceId(): NamespaceId | undefined;
```

Attempts to convert this address into a namespace id.

#### Returns

[`NamespaceId`](models/NamespaceId.md) \| `undefined`

Namespace id if this adresss is an alias, undefined otherwise.

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

[`ByteArray`](../index/ByteArray.md).[`toString`](../index/ByteArray.md#tostring)

***

### fromDecodedAddressHexString()

```ts
static fromDecodedAddressHexString(hexString): Address;
```

Creates an address from a decoded address hex string (typically from REST).

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `hexString` | `string` | Decoded address hex string. |

#### Returns

`Address`

Equivalent address.

***

### fromNamespaceId()

```ts
static fromNamespaceId(namespaceId, networkIdentifier): Address;
```

Creates an address from a namespace id.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `namespaceId` | [`NamespaceId`](models/NamespaceId.md) | Namespace id. |
| `networkIdentifier` | `number` | Network identifier byte. |

#### Returns

`Address`

Address referencing namespace id.
