# Class: default

Encrypts and encodes messages between two parties.

## Constructors

### new default()

```ts
new default(keyPair): MessageEncoder
```

Creates message encoder around key pair.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `keyPair` | [`KeyPair`](../../classes/KeyPair.md) | Key pair. |

#### Returns

`MessageEncoder`

## Accessors

### publicKey

#### Get Signature

```ts
get publicKey(): PublicKey
```

Public key used for message encoding.

##### Returns

[`PublicKey`](../../../index/classes/PublicKey.md)

Public key used for message encoding.

## Methods

### encode()

```ts
encode(recipientPublicKey, message): Uint8Array
```

Encodes message to recipient using recommended format.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientPublicKey` | [`PublicKey`](../../../index/classes/PublicKey.md) | Recipient public key. |
| `message` | `Uint8Array` | Message to encode. |

#### Returns

`Uint8Array`

Encrypted and encoded message.

***

### ~~encodeDeprecated()~~

```ts
encodeDeprecated(recipientPublicKey, message): Uint8Array
```

Encodes message to recipient using (deprecated) wallet format.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientPublicKey` | [`PublicKey`](../../../index/classes/PublicKey.md) | Recipient public key. |
| `message` | `Uint8Array` | Message to encode. |

#### Returns

`Uint8Array`

Encrypted and encoded message.

#### Deprecated

This function is only provided for compatability with the original Symbol wallets.
            Please use `encode` in any new code.

***

### encodePersistentHarvestingDelegation()

```ts
encodePersistentHarvestingDelegation(
   nodePublicKey, 
   remoteKeyPair, 
   vrfKeyPair): Uint8Array
```

Encodes persistent harvesting delegation to node.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `nodePublicKey` | [`PublicKey`](../../../index/classes/PublicKey.md) | Node public key. |
| `remoteKeyPair` | [`KeyPair`](../../classes/KeyPair.md) | Remote key pair. |
| `vrfKeyPair` | [`KeyPair`](../../classes/KeyPair.md) | Vrf key pair. |

#### Returns

`Uint8Array`

Encrypted and encoded harvesting delegation request.

***

### tryDecode()

```ts
tryDecode(recipientPublicKey, encodedMessage): TryDecodeResult
```

Tries to decode encoded message.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientPublicKey` | [`PublicKey`](../../../index/classes/PublicKey.md) | Recipient's public key. |
| `encodedMessage` | `Uint8Array` | Encoded message. |

#### Returns

[`TryDecodeResult`](../type-aliases/TryDecodeResult.md)

Tuple containing decoded status and message.

***

### ~~tryDecodeDeprecated()~~

```ts
tryDecodeDeprecated(recipientPublicKey, encodedMessage): TryDecodeResult
```

Tries to decode encoded message.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `recipientPublicKey` | [`PublicKey`](../../../index/classes/PublicKey.md) | Recipient's public key. |
| `encodedMessage` | `Uint8Array` | Encoded message |

#### Returns

[`TryDecodeResult`](../type-aliases/TryDecodeResult.md)

Tuple containing decoded status and message.

#### Deprecated

This function is only provided for compatability with the original Symbol wallets.
            Please use `tryDecode` in any new code.
