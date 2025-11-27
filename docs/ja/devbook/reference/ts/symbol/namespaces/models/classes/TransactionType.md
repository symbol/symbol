# Class: TransactionType

## Constructors

### new TransactionType()

```ts
new TransactionType(value): TransactionType
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`TransactionType`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value"></a> `value` | `public` | `any` |
| <a id="account_address_restriction"></a> `ACCOUNT_ADDRESS_RESTRICTION` | `static` | `TransactionType` |
| <a id="account_key_link"></a> `ACCOUNT_KEY_LINK` | `static` | `TransactionType` |
| <a id="account_metadata"></a> `ACCOUNT_METADATA` | `static` | `TransactionType` |
| <a id="account_mosaic_restriction"></a> `ACCOUNT_MOSAIC_RESTRICTION` | `static` | `TransactionType` |
| <a id="account_operation_restriction"></a> `ACCOUNT_OPERATION_RESTRICTION` | `static` | `TransactionType` |
| <a id="address_alias"></a> `ADDRESS_ALIAS` | `static` | `TransactionType` |
| <a id="aggregate_bonded"></a> `AGGREGATE_BONDED` | `static` | `TransactionType` |
| <a id="aggregate_complete"></a> `AGGREGATE_COMPLETE` | `static` | `TransactionType` |
| <a id="hash_lock"></a> `HASH_LOCK` | `static` | `TransactionType` |
| <a id="mosaic_address_restriction"></a> `MOSAIC_ADDRESS_RESTRICTION` | `static` | `TransactionType` |
| <a id="mosaic_alias"></a> `MOSAIC_ALIAS` | `static` | `TransactionType` |
| <a id="mosaic_definition"></a> `MOSAIC_DEFINITION` | `static` | `TransactionType` |
| <a id="mosaic_global_restriction"></a> `MOSAIC_GLOBAL_RESTRICTION` | `static` | `TransactionType` |
| <a id="mosaic_metadata"></a> `MOSAIC_METADATA` | `static` | `TransactionType` |
| <a id="mosaic_supply_change"></a> `MOSAIC_SUPPLY_CHANGE` | `static` | `TransactionType` |
| <a id="mosaic_supply_revocation"></a> `MOSAIC_SUPPLY_REVOCATION` | `static` | `TransactionType` |
| <a id="multisig_account_modification"></a> `MULTISIG_ACCOUNT_MODIFICATION` | `static` | `TransactionType` |
| <a id="namespace_metadata"></a> `NAMESPACE_METADATA` | `static` | `TransactionType` |
| <a id="namespace_registration"></a> `NAMESPACE_REGISTRATION` | `static` | `TransactionType` |
| <a id="node_key_link"></a> `NODE_KEY_LINK` | `static` | `TransactionType` |
| <a id="secret_lock"></a> `SECRET_LOCK` | `static` | `TransactionType` |
| <a id="secret_proof"></a> `SECRET_PROOF` | `static` | `TransactionType` |
| <a id="transfer"></a> `TRANSFER` | `static` | `TransactionType` |
| <a id="voting_key_link"></a> `VOTING_KEY_LINK` | `static` | `TransactionType` |
| <a id="vrf_key_link"></a> `VRF_KEY_LINK` | `static` | `TransactionType` |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### toJson()

```ts
toJson(): any
```

#### Returns

`any`

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

***

### deserialize()

```ts
static deserialize(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### deserializeAligned()

```ts
static deserializeAligned(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### fromValue()

```ts
static fromValue(value): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`any`

***

### valueToKey()

```ts
static valueToKey(value): string
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`string`
