# Function: calculateTransactionFee()

```ts
function calculateTransactionFee(
   transaction, 
   feeMultiplier, 
   cosignatureCount?): bigint;
```

Calculates the minimum required transaction fee for a transaction.

## Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](models/Transaction.md) | Transaction. |
| `feeMultiplier` | `number` | Fee multiplier to use. |
| `cosignatureCount?` | `number` | Number of expected cosignatures to be attached. |

## Returns

`bigint`

Transaction fee.
