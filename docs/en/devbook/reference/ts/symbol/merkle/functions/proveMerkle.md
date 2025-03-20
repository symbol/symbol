# Function: proveMerkle()

```ts
function proveMerkle(
   leafHash, 
   merklePath, 
   rootHash): boolean
```

Proves a merkle hash.

## Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `leafHash` | [`Hash256`](../../../index/classes/Hash256.md) | Leaf hash to prove. |
| `merklePath` | [`MerklePart`](../type-aliases/MerklePart.md)[] | Merkle *hash chain* path from leaf to root. |
| `rootHash` | [`Hash256`](../../../index/classes/Hash256.md) | Root hash of the merkle tree. |

## Returns

`boolean`

\c true if leaf hash is connected to root hash; false otherwise.
