# Class: MerkleHashBuilder

Builder for creating a merkle hash.

## Constructors

### new MerkleHashBuilder()

```ts
new MerkleHashBuilder(): MerkleHashBuilder
```

#### Returns

`MerkleHashBuilder`

## Methods

### final()

```ts
final(): Hash256
```

Calculates the merkle hash.

#### Returns

[`Hash256`](../../../index/classes/Hash256.md)

Merkle hash.

***

### update()

```ts
update(componentHash): void
```

Adds a hash to the merkle hash.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `componentHash` | [`Hash256`](../../../index/classes/Hash256.md) | Hash to add. |

#### Returns

`void`
