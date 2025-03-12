# Class: AccountKeyLinkTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountKeyLinkTransactionV1Descriptor.

This transaction is required for all accounts wanting to activate remote or delegated harvesting (V1, latest).
Announce an AccountKeyLinkTransaction to delegate the account importance score to a proxy account.

## Constructors

### new AccountKeyLinkTransactionV1Descriptor()

```ts
new AccountKeyLinkTransactionV1Descriptor(linkedPublicKey, linkAction): AccountKeyLinkTransactionV1Descriptor
```

Creates a descriptor for AccountKeyLinkTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `linkedPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) | Linked public key. |
| `linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) | Account link action. |

#### Returns

[`AccountKeyLinkTransactionV1Descriptor`](AccountKeyLinkTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) |
| `rawDescriptor.linkedPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) |
| `rawDescriptor.type` | `string` |

## Methods

### toMap()

```ts
toMap(): object
```

Builds a representation of this descriptor that can be passed to a factory function.

#### Returns

`object`

Descriptor that can be passed to a factory function.
