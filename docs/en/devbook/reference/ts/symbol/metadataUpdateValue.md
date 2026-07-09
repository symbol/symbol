# Function: metadataUpdateValue()

```ts
function metadataUpdateValue(oldValue, newValue): Uint8Array;
```

Creates a metadata payload for updating old value to new value.

## Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `oldValue` | `Uint8Array`&lt;`ArrayBufferLike`&gt; \| `undefined` | Old metadata value. |
| `newValue` | `Uint8Array` | New metadata value. |

## Returns

`Uint8Array`

Metadata payload for updating old value to new value.
