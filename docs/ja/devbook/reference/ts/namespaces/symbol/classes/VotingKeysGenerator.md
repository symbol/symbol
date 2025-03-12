# Class: VotingKeysGenerator

Generates symbol voting keys.

## Constructors

### new VotingKeysGenerator()

```ts
new VotingKeysGenerator(rootKeyPair, privateKeyGenerator?): VotingKeysGenerator
```

Creates a generator around a voting root key pair.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `rootKeyPair` | [`KeyPair`](KeyPair.md) | Voting root key pair. |
| `privateKeyGenerator`? | `Function` | Private key generator. |

#### Returns

[`VotingKeysGenerator`](VotingKeysGenerator.md)

## Methods

### generate()

```ts
generate(startEpoch, endEpoch): Uint8Array
```

Generates voting keys for specified epochs.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `startEpoch` | `bigint` | Start epoch. |
| `endEpoch` | `bigint` | End epoch. |

#### Returns

`Uint8Array`

Serialized voting keys.
