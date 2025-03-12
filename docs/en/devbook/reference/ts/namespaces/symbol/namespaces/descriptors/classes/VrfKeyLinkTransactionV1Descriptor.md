# Class: VrfKeyLinkTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for VrfKeyLinkTransactionV1Descriptor.

Link an account with a VRF public key required for harvesting (V1, latest).
Announce a VrfKeyLinkTransaction to link an account with a VRF public key. The linked key is used to randomize block production and leader/participant selection.
This transaction is required for all accounts wishing to [harvest](/concepts/harvesting.html).

## Constructors

### new VrfKeyLinkTransactionV1Descriptor()

```ts
new VrfKeyLinkTransactionV1Descriptor(linkedPublicKey, linkAction): VrfKeyLinkTransactionV1Descriptor
```

Creates a descriptor for VrfKeyLinkTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `linkedPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) | Linked VRF public key. |
| `linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) | Account link action. |

#### Returns

[`VrfKeyLinkTransactionV1Descriptor`](VrfKeyLinkTransactionV1Descriptor.md)

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
