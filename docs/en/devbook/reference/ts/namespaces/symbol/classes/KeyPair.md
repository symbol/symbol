# Class: KeyPair

Represents an ED25519 private and public key.

## Constructors

### new KeyPair()

```ts
new KeyPair(privateKey): KeyPair
```

Creates a key pair from a private key.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `privateKey` | [`PrivateKey`](../../core/classes/PrivateKey.md) | Private key. |

#### Returns

[`KeyPair`](KeyPair.md)

## Accessors

### privateKey

#### Get Signature

```ts
get privateKey(): PrivateKey
```

Gets the private key.

##### Returns

[`PrivateKey`](../../core/classes/PrivateKey.md)

Private key.

***

### publicKey

#### Get Signature

```ts
get publicKey(): PublicKey
```

Gets the public key.

##### Returns

[`PublicKey`](../../core/classes/PublicKey.md)

Public key.

## Methods

### sign()

```ts
sign(message): Signature
```

Signs a message with the private key.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `message` | `Uint8Array` | Message to sign. |

#### Returns

[`Signature`](../../core/classes/Signature.md)

Message signature.
