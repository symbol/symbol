# Class: SymbolFacade

Facade used to interact with Symbol blockchain.

## Constructors

### new SymbolFacade()

```ts
new SymbolFacade(network): SymbolFacade
```

Creates a Symbol facade.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `network` | `string` \| [`Network`](Network.md) | Symbol network or network name. |

#### Returns

`SymbolFacade`

## Properties

| Property | Modifier | Type | Description |
| ------ | ------ | ------ | ------ |
| <a id="network"></a> `network` | `public` | [`Network`](Network.md) | Underlying network. |
| <a id="transactionfactory"></a> `transactionFactory` | `public` | [`SymbolTransactionFactory`](SymbolTransactionFactory.md) | Underlying transaction factory. |
| <a id="address"></a> `Address` | `static` | *typeof* [`Address`](Address.md) | Network address class type. |
| <a id="bip32_curve_name"></a> `BIP32_CURVE_NAME` | `static` | `string` | BIP32 curve name. |
| <a id="derivesharedkey"></a> `deriveSharedKey` | `static` | (`keyPair`: [`KeyPair`](KeyPair.md), `otherPublicKey`: [`PublicKey`](../../index/classes/PublicKey.md)) => [`SharedKey256`](../../index/classes/SharedKey256.md) | Derives shared key from key pair and other party's public key. |
| <a id="keypair"></a> `KeyPair` | `static` | *typeof* [`KeyPair`](KeyPair.md) | Network key pair class type. |
| <a id="verifier"></a> `Verifier` | `static` | *typeof* [`Verifier`](Verifier.md) | Network verifier class type. |

## Accessors

### static

#### Get Signature

```ts
get static(): typeof SymbolFacade
```

Gets class type.

##### Returns

*typeof* `SymbolFacade`

Class type.

## Methods

### bip32Path()

```ts
bip32Path(accountId): number[]
```

Creates a network compatible BIP32 path for the specified account.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `accountId` | `number` | Id of the account for which to generate a BIP32 path. |

#### Returns

`number`[]

BIP32 path for the specified account.

***

### cosignTransaction()

```ts
cosignTransaction(
   keyPair, 
   transaction, 
   detached?): 
  | Cosignature
  | DetachedCosignature
```

Cosigns a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `keyPair` | [`KeyPair`](KeyPair.md) | Key pair of the cosignatory. |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |
| `detached`? | `boolean` | \c true if resulting cosignature is appropriate for network propagation. \c false if resulting cosignature is appropriate for attaching to an aggregate. |

#### Returns

  \| [`Cosignature`](../namespaces/models/classes/Cosignature.md)
  \| [`DetachedCosignature`](../namespaces/models/classes/DetachedCosignature.md)

Signed cosignature.

***

### createAccount()

```ts
createAccount(privateKey): SymbolAccount
```

Creates a Symbol account from a private key.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `privateKey` | [`PrivateKey`](../../index/classes/PrivateKey.md) | Account private key. |

#### Returns

[`SymbolAccount`](SymbolAccount.md)

Symbol account.

***

### createEmbeddedTransactionFromTypedDescriptor()

```ts
createEmbeddedTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey): EmbeddedTransaction
```

Creates an embedded transaction from a (typed) transaction descriptor.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `typedDescriptor` | `object` | Transaction (typed) descriptor. |
| `signerPublicKey` | [`PublicKey`](../../index/classes/PublicKey.md) | Signer public key. |

#### Returns

[`EmbeddedTransaction`](../namespaces/models/classes/EmbeddedTransaction.md)

Created embedded transaction.

***

### createPublicAccount()

```ts
createPublicAccount(publicKey): SymbolPublicAccount
```

Creates a Symbol public account from a public key.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `publicKey` | [`PublicKey`](../../index/classes/PublicKey.md) | Account public key. |

#### Returns

[`SymbolPublicAccount`](SymbolPublicAccount.md)

Symbol public account.

***

### createTransactionFromTypedDescriptor()

```ts
createTransactionFromTypedDescriptor(
   typedDescriptor, 
   signerPublicKey, 
   feeMultiplier, 
   deadlineSeconds, 
   cosignatureCount?): Transaction
```

Creates a transaction from a (typed) transaction descriptor.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `typedDescriptor` | `object` | Transaction (typed) descriptor. |
| `signerPublicKey` | [`PublicKey`](../../index/classes/PublicKey.md) | Signer public key. |
| `feeMultiplier` | `number` | Fee multiplier. |
| `deadlineSeconds` | `number` | Approximate seconds from now for deadline. |
| `cosignatureCount`? | `number` | Number of cosignature spaces to reserve. |

#### Returns

[`Transaction`](../namespaces/models/classes/Transaction.md)

Created transaction.

***

### extractSigningPayload()

```ts
extractSigningPayload(transaction): Uint8Array
```

Gets the payload to sign given a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |

#### Returns

`Uint8Array`

Verifiable data to sign.

***

### hashTransaction()

```ts
hashTransaction(transaction): Hash256
```

Hashes a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |

#### Returns

[`Hash256`](../../index/classes/Hash256.md)

Transaction hash.

***

### now()

```ts
now(): NetworkTimestamp
```

Creates a network timestamp representing the current time.

#### Returns

[`NetworkTimestamp`](NetworkTimestamp.md)

Network timestamp representing the current time.

***

### signTransaction()

```ts
signTransaction(keyPair, transaction): Signature
```

Signs a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `keyPair` | [`KeyPair`](KeyPair.md) | Key pair. |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |

#### Returns

[`Signature`](../../index/classes/Signature.md)

Transaction signature.

***

### verifyTransaction()

```ts
verifyTransaction(transaction, signature): boolean
```

Verifies a Symbol transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `transaction` | [`Transaction`](../namespaces/models/classes/Transaction.md) | Transaction object. |
| `signature` | [`Signature`](../../index/classes/Signature.md) | Signature to verify. |

#### Returns

`boolean`

\c true if transaction signature is verified.

***

### bip32NodeToKeyPair()

```ts
static bip32NodeToKeyPair(bip32Node): KeyPair
```

Derives a Symbol KeyPair from a BIP32 node.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `bip32Node` | [`Bip32Node`](../../Bip32/classes/Bip32Node.md) | BIP32 node. |

#### Returns

[`KeyPair`](KeyPair.md)

Derived key pair.

***

### cosignTransactionHash()

```ts
static cosignTransactionHash(
   keyPair, 
   transactionHash, 
   detached?): 
  | Cosignature
  | DetachedCosignature
```

Cosigns a Symbol transaction hash.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `keyPair` | [`KeyPair`](KeyPair.md) | Key pair of the cosignatory. |
| `transactionHash` | [`Hash256`](../../index/classes/Hash256.md) | Transaction hash. |
| `detached`? | `boolean` | \c true if resulting cosignature is appropriate for network propagation. \c false if resulting cosignature is appropriate for attaching to an aggregate. |

#### Returns

  \| [`Cosignature`](../namespaces/models/classes/Cosignature.md)
  \| [`DetachedCosignature`](../namespaces/models/classes/DetachedCosignature.md)

Signed cosignature.

***

### hashEmbeddedTransactions()

```ts
static hashEmbeddedTransactions(embeddedTransactions): Hash256
```

Hashes embedded transactions of an aggregate transaction.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `embeddedTransactions` | [`EmbeddedTransaction`](../namespaces/models/classes/EmbeddedTransaction.md)[] | Embedded transactions to hash. |

#### Returns

[`Hash256`](../../index/classes/Hash256.md)

Aggregate transactions hash.
