# Type Alias: MerklePart

```ts
type MerklePart = object;
```

Represents part of a merkle tree proof.

## Properties

| Property | Type | Description |
| ------ | ------ | ------ |
| <a id="hash"></a> `hash` | [`Hash256`](../../../index/classes/Hash256.md) | Hash at this node. |
| <a id="isleft"></a> `isLeft` | `boolean` | \c true if this is a left node; right otherwise. |
