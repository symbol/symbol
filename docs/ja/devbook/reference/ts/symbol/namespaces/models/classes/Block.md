# Class: Block

## Extended by

- [`NemesisBlockV1`](NemesisBlockV1.md)
- [`NormalBlockV1`](NormalBlockV1.md)
- [`ImportanceBlockV1`](ImportanceBlockV1.md)

## Constructors

### new Block()

```ts
new Block(): Block
```

#### Returns

`Block`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_beneficiaryaddress"></a> `_beneficiaryAddress` | `public` | [`Address`](Address.md) |
| <a id="_difficulty"></a> `_difficulty` | `public` | [`Difficulty`](Difficulty.md) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` |
| <a id="_feemultiplier"></a> `_feeMultiplier` | `public` | [`BlockFeeMultiplier`](BlockFeeMultiplier.md) |
| <a id="_generationhashproof"></a> `_generationHashProof` | `public` | [`VrfProof`](VrfProof.md) |
| <a id="_height"></a> `_height` | `public` | [`Height`](Height.md) |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) |
| <a id="_previousblockhash"></a> `_previousBlockHash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_receiptshash"></a> `_receiptsHash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) |
| <a id="_statehash"></a> `_stateHash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_timestamp"></a> `_timestamp` | `public` | [`Timestamp`](Timestamp.md) |
| <a id="_transactionshash"></a> `_transactionsHash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_type"></a> `_type` | `public` | [`BlockType`](BlockType.md) |
| <a id="_verifiableentityheaderreserved_1"></a> `_verifiableEntityHeaderReserved_1` | `public` | `number` |
| <a id="_version"></a> `_version` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.beneficiaryAddress` | `public` | `string` |
| `TYPE_HINTS.difficulty` | `public` | `string` |
| `TYPE_HINTS.feeMultiplier` | `public` | `string` |
| `TYPE_HINTS.generationHashProof` | `public` | `string` |
| `TYPE_HINTS.height` | `public` | `string` |
| `TYPE_HINTS.network` | `public` | `string` |
| `TYPE_HINTS.previousBlockHash` | `public` | `string` |
| `TYPE_HINTS.receiptsHash` | `public` | `string` |
| `TYPE_HINTS.signature` | `public` | `string` |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` |
| `TYPE_HINTS.stateHash` | `public` | `string` |
| `TYPE_HINTS.timestamp` | `public` | `string` |
| `TYPE_HINTS.transactionsHash` | `public` | `string` |
| `TYPE_HINTS.type` | `public` | `string` |

## Accessors

### beneficiaryAddress

#### Get Signature

```ts
get beneficiaryAddress(): Address
```

##### Returns

[`Address`](Address.md)

#### Set Signature

```ts
set beneficiaryAddress(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Address`](Address.md) |

##### Returns

`void`

***

### difficulty

#### Get Signature

```ts
get difficulty(): Difficulty
```

##### Returns

[`Difficulty`](Difficulty.md)

#### Set Signature

```ts
set difficulty(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Difficulty`](Difficulty.md) |

##### Returns

`void`

***

### feeMultiplier

#### Get Signature

```ts
get feeMultiplier(): BlockFeeMultiplier
```

##### Returns

[`BlockFeeMultiplier`](BlockFeeMultiplier.md)

#### Set Signature

```ts
set feeMultiplier(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`BlockFeeMultiplier`](BlockFeeMultiplier.md) |

##### Returns

`void`

***

### generationHashProof

#### Get Signature

```ts
get generationHashProof(): VrfProof
```

##### Returns

[`VrfProof`](VrfProof.md)

#### Set Signature

```ts
set generationHashProof(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`VrfProof`](VrfProof.md) |

##### Returns

`void`

***

### height

#### Get Signature

```ts
get height(): Height
```

##### Returns

[`Height`](Height.md)

#### Set Signature

```ts
set height(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Height`](Height.md) |

##### Returns

`void`

***

### network

#### Get Signature

```ts
get network(): NetworkType
```

##### Returns

[`NetworkType`](NetworkType.md)

#### Set Signature

```ts
set network(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`NetworkType`](NetworkType.md) |

##### Returns

`void`

***

### previousBlockHash

#### Get Signature

```ts
get previousBlockHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set previousBlockHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

### receiptsHash

#### Get Signature

```ts
get receiptsHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set receiptsHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

### signature

#### Get Signature

```ts
get signature(): Signature
```

##### Returns

[`Signature`](Signature.md)

#### Set Signature

```ts
set signature(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Signature`](Signature.md) |

##### Returns

`void`

***

### signerPublicKey

#### Get Signature

```ts
get signerPublicKey(): PublicKey
```

##### Returns

[`PublicKey`](PublicKey.md)

#### Set Signature

```ts
set signerPublicKey(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`PublicKey`](PublicKey.md) |

##### Returns

`void`

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

***

### stateHash

#### Get Signature

```ts
get stateHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set stateHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

### timestamp

#### Get Signature

```ts
get timestamp(): Timestamp
```

##### Returns

[`Timestamp`](Timestamp.md)

#### Set Signature

```ts
set timestamp(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Timestamp`](Timestamp.md) |

##### Returns

`void`

***

### transactionsHash

#### Get Signature

```ts
get transactionsHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set transactionsHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

### type

#### Get Signature

```ts
get type(): BlockType
```

##### Returns

[`BlockType`](BlockType.md)

#### Set Signature

```ts
set type(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`BlockType`](BlockType.md) |

##### Returns

`void`

***

### version

#### Get Signature

```ts
get version(): number
```

##### Returns

`number`

#### Set Signature

```ts
set version(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `number` |

##### Returns

`void`

## Methods

### \_serialize()

```ts
_serialize(buffer): void
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `buffer` | `any` |

#### Returns

`void`

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

***

### \_deserialize()

```ts
static _deserialize(view, instance): void
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `view` | `any` |
| `instance` | `any` |

#### Returns

`void`
