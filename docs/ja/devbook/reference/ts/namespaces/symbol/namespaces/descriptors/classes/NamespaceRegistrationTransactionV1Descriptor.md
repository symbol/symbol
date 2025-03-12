# Class: NamespaceRegistrationTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for NamespaceRegistrationTransactionV1Descriptor.

Register (or renew a registration for) a [namespace](/concepts/namespace.html) (V1, latest).
Namespaces help keep assets organized.

## Constructors

### new NamespaceRegistrationTransactionV1Descriptor()

```ts
new NamespaceRegistrationTransactionV1Descriptor(
   id, 
   registrationType, 
   duration?, 
   parentId?, 
   name?): NamespaceRegistrationTransactionV1Descriptor
```

Creates a descriptor for NamespaceRegistrationTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `id` | [`NamespaceId`](../../models/classes/NamespaceId.md) | Namespace identifier. |
| `registrationType` | [`NamespaceRegistrationType`](../../models/classes/NamespaceRegistrationType.md) | Namespace registration type. |
| `duration`? | [`BlockDuration`](../../models/classes/BlockDuration.md) | Number of confirmed blocks you would like to rent the namespace for. Required for root namespaces. |
| `parentId`? | [`NamespaceId`](../../models/classes/NamespaceId.md) | Parent namespace identifier. Required for sub-namespaces. |
| `name`? | `string` \| `Uint8Array`&lt;`ArrayBufferLike`&gt; | Namespace name. |

#### Returns

[`NamespaceRegistrationTransactionV1Descriptor`](NamespaceRegistrationTransactionV1Descriptor.md)

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.id` | [`NamespaceId`](../../models/classes/NamespaceId.md) |
| `rawDescriptor.registrationType` | [`NamespaceRegistrationType`](../../models/classes/NamespaceRegistrationType.md) |
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
