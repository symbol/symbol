# Class: ByteArray

Represents a fixed size byte array.

## Extended by

- [`Hash256`](Hash256.md)
- [`PrivateKey`](PrivateKey.md)
- [`PublicKey`](PublicKey.md)
- [`SharedKey256`](SharedKey256.md)
- [`Signature`](Signature.md)
- [`Address`](../../symbol/classes/Address.md)
- [`UnresolvedAddress`](../../symbol/namespaces/models/classes/UnresolvedAddress.md)
- [`Address`](../../symbol/namespaces/models/classes/Address.md)
- [`Hash256`](../../symbol/namespaces/models/classes/Hash256.md)
- [`Hash512`](../../symbol/namespaces/models/classes/Hash512.md)
- [`PublicKey`](../../symbol/namespaces/models/classes/PublicKey.md)
- [`VotingPublicKey`](../../symbol/namespaces/models/classes/VotingPublicKey.md)
- [`Signature`](../../symbol/namespaces/models/classes/Signature.md)
- [`ProofGamma`](../../symbol/namespaces/models/classes/ProofGamma.md)
- [`ProofVerificationHash`](../../symbol/namespaces/models/classes/ProofVerificationHash.md)
- [`ProofScalar`](../../symbol/namespaces/models/classes/ProofScalar.md)

## Constructors

### new ByteArray()

```ts
new ByteArray(fixedSize, arrayInput): ByteArray
```

Creates a byte array.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `fixedSize` | `number` | Size of the array. |
| `arrayInput` | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Byte array or hex string. |

#### Returns

[`ByteArray`](ByteArray.md)

## Properties

| Property | Modifier | Type | Description |
| ------ | ------ | ------ | ------ |
| <a id="bytes"></a> `bytes` | `public` | `Uint8Array` | Underlying bytes. |
| <a id="name"></a> `NAME` | `static` | `string` | Byte array name (required because `constructor.name` is dropped during minification). |

## Methods

### toJson()

```ts
toJson(): string
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string
```

Returns string representation of this object.

#### Returns

`string`

String representation of this object
