# Class: MosaicSupplyRevocationTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MosaicSupplyRevocationTransactionV1Descriptor.

Revoke mosaic (V1, latest).

## Constructors

### new MosaicSupplyRevocationTransactionV1Descriptor()

```ts
new MosaicSupplyRevocationTransactionV1Descriptor(sourceAddress, mosaic): MosaicSupplyRevocationTransactionV1Descriptor
```

Creates a descriptor for MosaicSupplyRevocationTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `sourceAddress` | [`Address`](../../../classes/Address.md) | Address from which tokens should be revoked. |
| `mosaic` | [`UnresolvedMosaicDescriptor`](UnresolvedMosaicDescriptor.md) | Revoked mosaic and amount. |

#### Returns

[`MosaicSupplyRevocationTransactionV1Descriptor`](MosaicSupplyRevocationTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.mosaic` | `any` |
| `rawDescriptor.sourceAddress` | [`Address`](../../../classes/Address.md) |
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
