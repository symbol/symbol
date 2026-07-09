# Class: TreeNode

Node in a compact Patricia tree.

## Extended by

- [`LeafNode`](LeafNode.md)
- [`BranchNode`](BranchNode.md)

## Constructors

### Constructor

```ts
new TreeNode(path): TreeNode;
```

Creates a tree node.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `path` | [`PatriciaTreePath`](PatriciaTreePath.md) | Node path. |

#### Returns

`TreeNode`

## Properties

| Property | Type | Description |
| ------ | ------ | ------ |
| <a id="path"></a> `path` | [`PatriciaTreePath`](PatriciaTreePath.md) | Node path. |

## Accessors

### hexPath

#### Get Signature

```ts
get hexPath(): string;
```

Gets hex representation of path.

##### Returns

`string`

Hex representation of path.

## Methods

### calculateHash()

```ts
calculateHash(): Hash256;
```

Calculates node hash.

#### Returns

[`Hash256`](../../index/Hash256.md)

Hash of the node.
