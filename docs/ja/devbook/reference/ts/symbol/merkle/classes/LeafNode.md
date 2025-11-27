# Class: LeafNode

Leaf node in a compact Patricia tree.

## Extends

- [`TreeNode`](TreeNode.md)

## Constructors

### new LeafNode()

```ts
new LeafNode(path, value): LeafNode
```

Creates a leaf node.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `path` | [`PatriciaTreePath`](../type-aliases/PatriciaTreePath.md) | Leaf path. |
| `value` | [`Hash256`](../../../index/classes/Hash256.md) | Leaf value. |

#### Returns

`LeafNode`

#### Overrides

[`TreeNode`](TreeNode.md).[`constructor`](TreeNode.md#constructor)

## Properties

| Property | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ |
| <a id="path"></a> `path` | [`PatriciaTreePath`](../type-aliases/PatriciaTreePath.md) | Node path. | [`TreeNode`](TreeNode.md).[`path`](TreeNode.md#path) |
| <a id="value"></a> `value` | [`Hash256`](../../../index/classes/Hash256.md) | Leaf value. | - |

## Accessors

### hexPath

#### Get Signature

```ts
get hexPath(): string
```

Gets hex representation of path.

##### Returns

`string`

Hex representation of path.

#### Inherited from

[`TreeNode`](TreeNode.md).[`hexPath`](TreeNode.md#hexpath)

## Methods

### calculateHash()

```ts
calculateHash(): Hash256
```

Calculates node hash.

#### Returns

[`Hash256`](../../../index/classes/Hash256.md)

Hash of the node.

#### Inherited from

[`TreeNode`](TreeNode.md).[`calculateHash`](TreeNode.md#calculatehash)
