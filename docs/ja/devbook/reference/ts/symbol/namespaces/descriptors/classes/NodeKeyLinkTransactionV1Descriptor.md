# Class: NodeKeyLinkTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for NodeKeyLinkTransactionV1Descriptor.

This transaction is required for all accounts willing to activate delegated harvesting (V1, latest).
Announce a NodeKeyLinkTransaction to link an account with a public key used by TLS to create sessions.

## Constructors

### new NodeKeyLinkTransactionV1Descriptor()

```ts
new NodeKeyLinkTransactionV1Descriptor(linkedPublicKey, linkAction): NodeKeyLinkTransactionV1Descriptor
```

Creates a descriptor for NodeKeyLinkTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `linkedPublicKey` | [`PublicKey`](../../../../index/classes/PublicKey.md) | Linked public key. |
| `linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) | Account link action. |

#### Returns

`NodeKeyLinkTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) |
| `rawDescriptor.linkedPublicKey` | [`PublicKey`](../../../../index/classes/PublicKey.md) |
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
