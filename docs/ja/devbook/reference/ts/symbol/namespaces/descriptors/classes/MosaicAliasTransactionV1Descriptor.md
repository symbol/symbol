# Class: MosaicAliasTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicAliasTransactionV1Descriptor.

Attach or detach a [namespace](/concepts/namespace.html) to a Mosaic.(V1, latest)
Setting an alias to a mosaic is only possible if the account announcing this transaction has also created the namespace and the mosaic involved.

## Constructors

### new MosaicAliasTransactionV1Descriptor()

```ts
new MosaicAliasTransactionV1Descriptor(
   namespaceId, 
   mosaicId, 
   aliasAction): MosaicAliasTransactionV1Descriptor
```

Creates a descriptor for MosaicAliasTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `namespaceId` | [`NamespaceId`](../../models/classes/NamespaceId.md) | Identifier of the namespace that will become (or stop being) an alias for the Mosaic. |
| `mosaicId` | [`MosaicId`](../../models/classes/MosaicId.md) | Aliased mosaic identifier. |
| `aliasAction` | [`AliasAction`](../../models/classes/AliasAction.md) | Alias action. |

#### Returns

`MosaicAliasTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.aliasAction` | [`AliasAction`](../../models/classes/AliasAction.md) |
| `rawDescriptor.mosaicId` | [`MosaicId`](../../models/classes/MosaicId.md) |
| `rawDescriptor.namespaceId` | [`NamespaceId`](../../models/classes/NamespaceId.md) |
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
