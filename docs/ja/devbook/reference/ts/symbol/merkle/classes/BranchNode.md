# Class: BranchNode

Branch node in a compact Patricia tree.

## Extends

- [`TreeNode`](TreeNode.md)

## Constructors

### new BranchNode()

```ts
new BranchNode(path, links): BranchNode
```

Creates a branch node.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `path` | [`PatriciaTreePath`](../type-aliases/PatriciaTreePath.md) | Branch path. |
| `links` | [`Hash256`](../../../index/classes/Hash256.md)[] | Branch links. |

#### Returns

`BranchNode`

#### Overrides

[`TreeNode`](TreeNode.md).[`constructor`](TreeNode.md#constructor)

## Properties

| Property | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ |
| <a id="links"></a> `links` | [`Hash256`](../../../index/classes/Hash256.md)[] | Branch links. | - |
| <a id="path"></a> `path` | [`PatriciaTreePath`](../type-aliases/PatriciaTreePath.md) | Node path. | [`TreeNode`](TreeNode.md).[`path`](TreeNode.md#path) |

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
