# Class: SymbolAccount

Symbol account.

## Extends

- [`SymbolPublicAccount`](SymbolPublicAccount.md)

## Constructors

### new SymbolAccount()

```ts
new SymbolAccount(facade, keyPair): SymbolAccount
```

Creates a Symbol account.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `facade` | [`SymbolFacade`](SymbolFacade.md) | Symbol facade. |
| `keyPair` | [`KeyPair`](KeyPair.md) | Account key pair. |

#### Returns

`SymbolAccount`

#### Overrides

[`SymbolPublicAccount`](SymbolPublicAccount.md).[`constructor`](SymbolPublicAccount.md#constructor)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="_facade"></a> `_facade` | `protected` | [`SymbolFacade`](SymbolFacade.md) | - | [`SymbolPublicAccount`](SymbolPublicAccount.md).[`_facade`](SymbolPublicAccount.md#_facade) |
| <a id="address"></a> `address` | `public` | [`Address`](Address.md) | Account address. | [`SymbolPublicAccount`](SymbolPublicAccount.md).[`address`](SymbolPublicAccount.md#address) |
| <a id="keypair"></a> `keyPair` | `public` | [`KeyPair`](KeyPair.md) | Account key pair. | - |
| <a id="publickey"></a> `publicKey` | `public` | [`PublicKey`](../../index/classes/PublicKey.md) | Account public key. | [`SymbolPublicAccount`](SymbolPublicAccount.md).[`publicKey`](SymbolPublicAccount.md#publickey) |

## Methods

### cosignTransaction()

```ts
cosignTransaction(transaction, detached?): 
  | Cosignature
  | DetachedCosignature
```

Cosigns a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |
| `detached`? | `boolean` | \c true if resulting cosignature is appropriate for network propagation. \c false if resulting cosignature is appropriate for attaching to an aggregate. |

#### Returns

  \| [`Cosignature`](../namespaces/models/classes/Cosignature.md)
  \| [`DetachedCosignature`](../namespaces/models/classes/DetachedCosignature.md)

Signed cosignature.

***

### cosignTransactionHash()

```ts
cosignTransactionHash(transactionHash, detached?): 
  | Cosignature
  | DetachedCosignature
```

Cosigns a Symbol transaction hash.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transactionHash` | [`Hash256`](../../index/classes/Hash256.md) | Transaction hash. |
| `detached`? | `boolean` | \c true if resulting cosignature is appropriate for network propagation. \c false if resulting cosignature is appropriate for attaching to an aggregate. |

#### Returns

  \| [`Cosignature`](../namespaces/models/classes/Cosignature.md)
  \| [`DetachedCosignature`](../namespaces/models/classes/DetachedCosignature.md)

Signed cosignature.

***

### messageEncoder()

```ts
messageEncoder(): default
```

Creates a message encoder that can be used for encrypting and encoding messages between two parties.

#### Returns

[`default`](../MessageEncoder/classes/default.md)

Message encoder using this account as one party.

***

### signTransaction()

```ts
signTransaction(transaction): Signature
```

Signs a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |

#### Returns

[`Signature`](../../index/classes/Signature.md)

Transaction signature.
