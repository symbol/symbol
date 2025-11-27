# Class: PatriciaMerkleProofResult

Possible results of a patricia merkle proof.

## Constructors

### new PatriciaMerkleProofResult()

```ts
new PatriciaMerkleProofResult(): PatriciaMerkleProofResult
```

#### Returns

`PatriciaMerkleProofResult`

## Properties

| Property | Modifier | Type | Description |
| ------ | ------ | ------ | ------ |
| <a id="inconclusive"></a> `INCONCLUSIVE` | `static` | `number` | Negative proof is inconclusive. |
| <a id="leaf_value_mismatch"></a> `LEAF_VALUE_MISMATCH` | `static` | `number` | Leaf value does not match expected value. |
| <a id="path_mismatch"></a> `PATH_MISMATCH` | `static` | `number` | Actual merkle path does not match encoded key. |
| <a id="state_hash_does_not_match_roots"></a> `STATE_HASH_DOES_NOT_MATCH_ROOTS` | `static` | `number` | State hash cannot be derived from subcache merkle roots. |
| <a id="unanchored_path_tree"></a> `UNANCHORED_PATH_TREE` | `static` | `number` | Root of the path tree being proven is not a subcache merkle root. |
| <a id="unlinked_node"></a> `UNLINKED_NODE` | `static` | `number` | Provided merkle hash contains an unlinked node. |
| <a id="valid_negative"></a> `VALID_NEGATIVE` | `static` | `number` | Proof is valid (negative). |
| <a id="valid_positive"></a> `VALID_POSITIVE` | `static` | `number` | Proof is valid (positive). |
