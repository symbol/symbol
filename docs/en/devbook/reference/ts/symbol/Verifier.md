# Class: Verifier

Verifies signatures signed by a single key pair.

## Constructors

### Constructor

```ts
new Verifier(publicKey): Verifier;
```

Creates a verifier from a public key.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `publicKey` | [`PublicKey`](../index/PublicKey.md) | Public key. |

#### Returns

`Verifier`

## Properties

| Property | Type | Description |
| ------ | ------ | ------ |
| <a id="publickey"></a> `publicKey` | [`PublicKey`](../index/PublicKey.md) | Public key used for signature verification. |

## Methods

### verify()

```ts
verify(message, signature): boolean;
```

Verifies a message signature.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `message` | `Uint8Array` | Message to verify. |
| `signature` | [`Signature`](../index/Signature.md) | Signature to verify. |

#### Returns

`boolean`

true if the message signature verifies.
