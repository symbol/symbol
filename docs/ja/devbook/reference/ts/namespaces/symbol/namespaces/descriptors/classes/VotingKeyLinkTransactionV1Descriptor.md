# Class: VotingKeyLinkTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for VotingKeyLinkTransactionV1Descriptor.

Link an account with a public key required for finalization voting (V1, latest).
This transaction is required for node operators wanting to vote for [finalization](/concepts/block.html#finalization).
Announce a VotingKeyLinkTransaction to associate a voting key with an account during a fixed period. An account can be linked to up to **3** different voting keys at the same time.
The recommended production setting is to always have at least **2** linked keys with different ``endPoint`` values to ensure a key is registered after the first one expires.
See more details in [the manual node setup guide](/guides/network/running-a-symbol-node-manually.html#manual-voting-key-renewal).

## Constructors

### new VotingKeyLinkTransactionV1Descriptor()

```ts
new VotingKeyLinkTransactionV1Descriptor(
   linkedPublicKey, 
   startEpoch, 
   endEpoch, 
   linkAction): VotingKeyLinkTransactionV1Descriptor
```

Creates a descriptor for VotingKeyLinkTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `linkedPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) | Linked voting public key. |
| `startEpoch` | [`FinalizationEpoch`](../../models/classes/FinalizationEpoch.md) | Starting finalization epoch. |
| `endEpoch` | [`FinalizationEpoch`](../../models/classes/FinalizationEpoch.md) | Ending finalization epoch. |
| `linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) | Account link action. |

#### Returns

[`VotingKeyLinkTransactionV1Descriptor`](VotingKeyLinkTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.endEpoch` | [`FinalizationEpoch`](../../models/classes/FinalizationEpoch.md) |
| `rawDescriptor.linkAction` | [`LinkAction`](../../models/classes/LinkAction.md) |
| `rawDescriptor.linkedPublicKey` | [`PublicKey`](../../../../core/classes/PublicKey.md) |
| `rawDescriptor.startEpoch` | [`FinalizationEpoch`](../../models/classes/FinalizationEpoch.md) |
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
