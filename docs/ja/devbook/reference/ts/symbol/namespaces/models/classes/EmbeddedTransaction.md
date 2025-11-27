# Class: EmbeddedTransaction

## Extended by

- [`EmbeddedAccountKeyLinkTransactionV1`](EmbeddedAccountKeyLinkTransactionV1.md)
- [`EmbeddedNodeKeyLinkTransactionV1`](EmbeddedNodeKeyLinkTransactionV1.md)
- [`EmbeddedVotingKeyLinkTransactionV1`](EmbeddedVotingKeyLinkTransactionV1.md)
- [`EmbeddedVrfKeyLinkTransactionV1`](EmbeddedVrfKeyLinkTransactionV1.md)
- [`EmbeddedHashLockTransactionV1`](EmbeddedHashLockTransactionV1.md)
- [`EmbeddedSecretLockTransactionV1`](EmbeddedSecretLockTransactionV1.md)
- [`EmbeddedSecretProofTransactionV1`](EmbeddedSecretProofTransactionV1.md)
- [`EmbeddedAccountMetadataTransactionV1`](EmbeddedAccountMetadataTransactionV1.md)
- [`EmbeddedMosaicMetadataTransactionV1`](EmbeddedMosaicMetadataTransactionV1.md)
- [`EmbeddedNamespaceMetadataTransactionV1`](EmbeddedNamespaceMetadataTransactionV1.md)
- [`EmbeddedMosaicDefinitionTransactionV1`](EmbeddedMosaicDefinitionTransactionV1.md)
- [`EmbeddedMosaicSupplyChangeTransactionV1`](EmbeddedMosaicSupplyChangeTransactionV1.md)
- [`EmbeddedMosaicSupplyRevocationTransactionV1`](EmbeddedMosaicSupplyRevocationTransactionV1.md)
- [`EmbeddedMultisigAccountModificationTransactionV1`](EmbeddedMultisigAccountModificationTransactionV1.md)
- [`EmbeddedAddressAliasTransactionV1`](EmbeddedAddressAliasTransactionV1.md)
- [`EmbeddedMosaicAliasTransactionV1`](EmbeddedMosaicAliasTransactionV1.md)
- [`EmbeddedNamespaceRegistrationTransactionV1`](EmbeddedNamespaceRegistrationTransactionV1.md)
- [`EmbeddedAccountAddressRestrictionTransactionV1`](EmbeddedAccountAddressRestrictionTransactionV1.md)
- [`EmbeddedAccountMosaicRestrictionTransactionV1`](EmbeddedAccountMosaicRestrictionTransactionV1.md)
- [`EmbeddedAccountOperationRestrictionTransactionV1`](EmbeddedAccountOperationRestrictionTransactionV1.md)
- [`EmbeddedMosaicAddressRestrictionTransactionV1`](EmbeddedMosaicAddressRestrictionTransactionV1.md)
- [`EmbeddedMosaicGlobalRestrictionTransactionV1`](EmbeddedMosaicGlobalRestrictionTransactionV1.md)
- [`EmbeddedTransferTransactionV1`](EmbeddedTransferTransactionV1.md)

## Constructors

### new EmbeddedTransaction()

```ts
new EmbeddedTransaction(): EmbeddedTransaction
```

#### Returns

`EmbeddedTransaction`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_embeddedtransactionheaderreserved_1"></a> `_embeddedTransactionHeaderReserved_1` | `public` | `number` |
| <a id="_entitybodyreserved_1"></a> `_entityBodyReserved_1` | `public` | `number` |
| <a id="_network"></a> `_network` | `public` | [`NetworkType`](NetworkType.md) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) |
| <a id="_type"></a> `_type` | `public` | [`TransactionType`](TransactionType.md) |
| <a id="_version"></a> `_version` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.network` | `public` | `string` |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` |
| `TYPE_HINTS.type` | `public` | `string` |

## Accessors

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
