# Class: NemesisBlockV1

## Extends

- [`Block`](Block.md)

## Constructors

### new NemesisBlockV1()

```ts
new NemesisBlockV1(): NemesisBlockV1
```

#### Returns

[`NemesisBlockV1`](NemesisBlockV1.md)

#### Inherited from

[`Block`](Block.md).[`constructor`](Block.md#constructors)

## Properties

| Property | Modifier | Type | Overrides | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_beneficiaryaddress"></a> `_beneficiaryAddress` | `public` | [`Address`](Address.md) | - | [`Block`](Block.md).[`_beneficiaryAddress`](Block.md#_beneficiaryaddress) |
| <a id="_difficulty"></a> `_difficulty` | `public` | [`Difficulty`](Difficulty.md) | - | [`Block`](Block.md).[`_difficulty`](Block.md#_difficulty) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` | - | [`Block`](Block.md).[`_entityBodyReserved_1`](Block.md#_entitybodyreserved_1) |
| <a id="_feemultiplier"></a> `_feeMultiplier` | `public` | [`BlockFeeMultiplier`](BlockFeeMultiplier.md) | - | [`Block`](Block.md).[`_feeMultiplier`](Block.md#_feemultiplier) |
| <a id="_generationhashproof"></a> `_generationHashProof` | `public` | [`VrfProof`](VrfProof.md) | - | [`Block`](Block.md).[`_generationHashProof`](Block.md#_generationhashproof) |
| <a id="_harvestingeligibleaccountscount"></a> `_harvestingEligibleAccountsCount` | `public` | `bigint` | - | - |
| <a id="_height"></a> `_height` | `public` | [`Height`](Height.md) | - | [`Block`](Block.md).[`_height`](Block.md#_height) |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) | - | [`Block`](Block.md).[`_network`](Block.md#_network) |
| <a id="_previousblockhash"></a> `_previousBlockHash` | `public` | [`Hash256`](Hash256.md) | - | [`Block`](Block.md).[`_previousBlockHash`](Block.md#_previousblockhash) |
| <a id="_previousimportanceblockhash"></a> `_previousImportanceBlockHash` | `public` | [`Hash256`](Hash256.md) | - | - |
| <a id="_receiptshash"></a> `_receiptsHash` | `public` | [`Hash256`](Hash256.md) | - | [`Block`](Block.md).[`_receiptsHash`](Block.md#_receiptshash) |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) | - | [`Block`](Block.md).[`_signature`](Block.md#_signature) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) | - | [`Block`](Block.md).[`_signerPublicKey`](Block.md#_signerpublickey) |
| <a id="_statehash"></a> `_stateHash` | `public` | [`Hash256`](Hash256.md) | - | [`Block`](Block.md).[`_stateHash`](Block.md#_statehash) |
| <a id="_timestamp"></a> `_timestamp` | `public` | [`Timestamp`](Timestamp.md) | - | [`Block`](Block.md).[`_timestamp`](Block.md#_timestamp) |
| <a id="_totalvotingbalance"></a> `_totalVotingBalance` | `public` | [`Amount`](Amount.md) | - | - |
| <a id="_transactions"></a> `_transactions` | `public` | `any`[] | - | - |
| <a id="_transactionshash"></a> `_transactionsHash` | `public` | [`Hash256`](Hash256.md) | - | [`Block`](Block.md).[`_transactionsHash`](Block.md#_transactionshash) |
| <a id="_type"></a> `_type` | `public` | [`BlockType`](BlockType.md) | - | [`Block`](Block.md).[`_type`](Block.md#_type) |
| <a id="_verifiableentityheaderreserved_1"></a> `_verifiableEntityHeaderReserved_1` | `public` | `number` | - | [`Block`](Block.md).[`_verifiableEntityHeaderReserved_1`](Block.md#_verifiableentityheaderreserved_1) |
| <a id="_version"></a> `_version` | `public` | `number` | - | [`Block`](Block.md).[`_version`](Block.md#_version) |
| <a id="_votingeligibleaccountscount"></a> `_votingEligibleAccountsCount` | `public` | `number` | - | - |
| <a id="block_type"></a> `BLOCK_TYPE` | `static` | [`BlockType`](BlockType.md) | - | - |
| <a id="block_version"></a> `BLOCK_VERSION` | `static` | `number` | - | - |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` | [`Block`](Block.md).[`TYPE_HINTS`](Block.md#type_hints) | - |
| `TYPE_HINTS.beneficiaryAddress` | `public` | `string` | - | - |
| `TYPE_HINTS.difficulty` | `public` | `string` | - | - |
| `TYPE_HINTS.feeMultiplier` | `public` | `string` | - | - |
| `TYPE_HINTS.generationHashProof` | `public` | `string` | - | - |
| `TYPE_HINTS.height` | `public` | `string` | - | - |
| `TYPE_HINTS.network` | `public` | `string` | - | - |
| `TYPE_HINTS.previousBlockHash` | `public` | `string` | - | - |
| `TYPE_HINTS.previousImportanceBlockHash` | `public` | `string` | - | - |
| `TYPE_HINTS.receiptsHash` | `public` | `string` | - | - |
| `TYPE_HINTS.signature` | `public` | `string` | - | - |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` | - | - |
| `TYPE_HINTS.stateHash` | `public` | `string` | - | - |
| `TYPE_HINTS.timestamp` | `public` | `string` | - | - |
| `TYPE_HINTS.totalVotingBalance` | `public` | `string` | - | - |
| `TYPE_HINTS.transactions` | `public` | `string` | - | - |
| `TYPE_HINTS.transactionsHash` | `public` | `string` | - | - |
| `TYPE_HINTS.type` | `public` | `string` | - | - |

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

#### Inherited from

[`Block`](Block.md).[`beneficiaryAddress`](Block.md#beneficiaryaddress)

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

#### Inherited from

[`Block`](Block.md).[`difficulty`](Block.md#difficulty)

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

#### Inherited from

[`Block`](Block.md).[`feeMultiplier`](Block.md#feemultiplier)

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

#### Inherited from

[`Block`](Block.md).[`generationHashProof`](Block.md#generationhashproof)

***

### harvestingEligibleAccountsCount

#### Get Signature

```ts
get harvestingEligibleAccountsCount(): bigint
```

##### Returns

`bigint`

#### Set Signature

```ts
set harvestingEligibleAccountsCount(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `bigint` |

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

#### Inherited from

[`Block`](Block.md).[`height`](Block.md#height)

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

#### Inherited from

[`Block`](Block.md).[`network`](Block.md#network)

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

#### Inherited from

[`Block`](Block.md).[`previousBlockHash`](Block.md#previousblockhash)

***

### previousImportanceBlockHash

#### Get Signature

```ts
get previousImportanceBlockHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set previousImportanceBlockHash(value): void
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

#### Inherited from

[`Block`](Block.md).[`receiptsHash`](Block.md#receiptshash)

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

#### Inherited from

[`Block`](Block.md).[`signature`](Block.md#signature)

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

#### Inherited from

[`Block`](Block.md).[`signerPublicKey`](Block.md#signerpublickey)

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

#### Inherited from

[`Block`](Block.md).[`size`](Block.md#size)

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

#### Inherited from

[`Block`](Block.md).[`stateHash`](Block.md#statehash)

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

#### Inherited from

[`Block`](Block.md).[`timestamp`](Block.md#timestamp)

***

### totalVotingBalance

#### Get Signature

```ts
get totalVotingBalance(): Amount
```

##### Returns

[`Amount`](Amount.md)

#### Set Signature

```ts
set totalVotingBalance(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Amount`](Amount.md) |

##### Returns

`void`

***

### transactions

#### Get Signature

```ts
get transactions(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set transactions(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

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

#### Inherited from

[`Block`](Block.md).[`transactionsHash`](Block.md#transactionshash)

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

#### Inherited from

[`Block`](Block.md).[`type`](Block.md#type)

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

#### Inherited from

[`Block`](Block.md).[`version`](Block.md#version)

***

### votingEligibleAccountsCount

#### Get Signature

```ts
get votingEligibleAccountsCount(): number
```

##### Returns

`number`

#### Set Signature

```ts
set votingEligibleAccountsCount(value): void
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

#### Inherited from

[`Block`](Block.md).[`_serialize`](Block.md#_serialize)

***

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

#### Inherited from

[`Block`](Block.md).[`serialize`](Block.md#serialize)

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

#### Inherited from

[`Block`](Block.md).[`sort`](Block.md#sort)

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

#### Inherited from

[`Block`](Block.md).[`toJson`](Block.md#tojson)

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

#### Inherited from

[`Block`](Block.md).[`toString`](Block.md#tostring)

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

#### Inherited from

[`Block`](Block.md).[`_deserialize`](Block.md#_deserialize)

***

### deserialize()

```ts
static deserialize(payload): NemesisBlockV1
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`NemesisBlockV1`](NemesisBlockV1.md)
