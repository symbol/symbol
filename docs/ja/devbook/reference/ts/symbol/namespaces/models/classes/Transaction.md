# Class: Transaction

## Extended by

- [`AccountKeyLinkTransactionV1`](AccountKeyLinkTransactionV1.md)
- [`NodeKeyLinkTransactionV1`](NodeKeyLinkTransactionV1.md)
- [`AggregateCompleteTransactionV1`](AggregateCompleteTransactionV1.md)
- [`AggregateCompleteTransactionV2`](AggregateCompleteTransactionV2.md)
- [`AggregateBondedTransactionV1`](AggregateBondedTransactionV1.md)
- [`AggregateBondedTransactionV2`](AggregateBondedTransactionV2.md)
- [`VotingKeyLinkTransactionV1`](VotingKeyLinkTransactionV1.md)
- [`VrfKeyLinkTransactionV1`](VrfKeyLinkTransactionV1.md)
- [`HashLockTransactionV1`](HashLockTransactionV1.md)
- [`SecretLockTransactionV1`](SecretLockTransactionV1.md)
- [`SecretProofTransactionV1`](SecretProofTransactionV1.md)
- [`AccountMetadataTransactionV1`](AccountMetadataTransactionV1.md)
- [`MosaicMetadataTransactionV1`](MosaicMetadataTransactionV1.md)
- [`NamespaceMetadataTransactionV1`](NamespaceMetadataTransactionV1.md)
- [`MosaicDefinitionTransactionV1`](MosaicDefinitionTransactionV1.md)
- [`MosaicSupplyChangeTransactionV1`](MosaicSupplyChangeTransactionV1.md)
- [`MosaicSupplyRevocationTransactionV1`](MosaicSupplyRevocationTransactionV1.md)
- [`MultisigAccountModificationTransactionV1`](MultisigAccountModificationTransactionV1.md)
- [`AddressAliasTransactionV1`](AddressAliasTransactionV1.md)
- [`MosaicAliasTransactionV1`](MosaicAliasTransactionV1.md)
- [`NamespaceRegistrationTransactionV1`](NamespaceRegistrationTransactionV1.md)
- [`AccountAddressRestrictionTransactionV1`](AccountAddressRestrictionTransactionV1.md)
- [`AccountMosaicRestrictionTransactionV1`](AccountMosaicRestrictionTransactionV1.md)
- [`AccountOperationRestrictionTransactionV1`](AccountOperationRestrictionTransactionV1.md)
- [`MosaicAddressRestrictionTransactionV1`](MosaicAddressRestrictionTransactionV1.md)
- [`MosaicGlobalRestrictionTransactionV1`](MosaicGlobalRestrictionTransactionV1.md)
- [`TransferTransactionV1`](TransferTransactionV1.md)

## Constructors

### new Transaction()

```ts
new Transaction(): Transaction
```

#### Returns

`Transaction`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_deadline"></a> `_deadline` | `public` | [`Timestamp`](Timestamp.md) |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` |
| <a id="_fee"></a> `_fee` | `public` | [`Amount`](Amount.md) |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) |
| <a id="_verifiableentityheaderreserved_1"></a> `_verifiableEntityHeaderReserved_1` | `public` | `number` |
| <a id="_version"></a> `_version` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.deadline` | `public` | `string` |
| `TYPE_HINTS.fee` | `public` | `string` |
| `TYPE_HINTS.network` | `public` | `string` |
| `TYPE_HINTS.signature` | `public` | `string` |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` |
| `TYPE_HINTS.type` | `public` | `string` |

## Accessors

### deadline

#### Get Signature

```ts
get deadline(): Timestamp
```

##### Returns

[`Timestamp`](Timestamp.md)

#### Set Signature

```ts
set deadline(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Timestamp`](Timestamp.md) |

##### Returns

`void`

***

### fee

#### Get Signature

```ts
get fee(): Amount
```

##### Returns

[`Amount`](Amount.md)

#### Set Signature

```ts
set fee(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Amount`](Amount.md) |

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

### type

#### Get Signature

```ts
get type(): TransactionType
```

##### Returns

[`TransactionType`](TransactionType.md)

#### Set Signature

```ts
set type(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`TransactionType`](TransactionType.md) |

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
