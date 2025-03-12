# Function: provePatriciaMerkle()

```ts
function provePatriciaMerkle(
   encodedKey, 
   valueToTest, 
   merklePath, 
   stateHash, 
   subcacheMerkleRoots): number
```

Proves a patricia merkle hash.

## Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `encodedKey` | [`Hash256`](../../core/classes/Hash256.md) | Encoded key of the state to prove. |
| `valueToTest` | [`Hash256`](../../core/classes/Hash256.md) | Expected hash of the state to prove. |
| `merklePath` | `TreeNode`[] | Merkle *node* path from root to leaf. |
| `stateHash` | [`Hash256`](../../core/classes/Hash256.md) | State hash from a block header. |
| `subcacheMerkleRoots` | [`Hash256`](../../core/classes/Hash256.md)[] | Sub cache merkle roots corresponding to the state hash. |

## Returns

`number`

Proof result code.
