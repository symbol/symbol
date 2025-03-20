# Class: MultisigAccountModificationTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for MultisigAccountModificationTransactionV1Descriptor.

Create or modify a [multi-signature](/concepts/multisig-account.html) account (V1, latest).
This transaction allows you to: - Transform a regular account into a multisig account. - Change the configurable properties of a multisig account. - Add or delete cosignatories from a multisig account (removing all cosignatories turns a multisig account into a regular account again).

## Constructors

### new MultisigAccountModificationTransactionV1Descriptor()

```ts
new MultisigAccountModificationTransactionV1Descriptor(
   minRemovalDelta, 
   minApprovalDelta, 
   addressAdditions?, 
   addressDeletions?): MultisigAccountModificationTransactionV1Descriptor
```

Creates a descriptor for MultisigAccountModificationTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `minRemovalDelta` | `number` | Relative change to the **minimum** number of cosignatures required when **removing a cosignatory**. E.g., when moving from 0 to 2 cosignatures this number would be **2**. When moving from 4 to 3 cosignatures, the number would be **-1**. |
| `minApprovalDelta` | `number` | Relative change to the **minimum** number of cosignatures required when **approving a transaction**. E.g., when moving from 0 to 2 cosignatures this number would be **2**. When moving from 4 to 3 cosignatures, the number would be **-1**. |
| `addressAdditions`? | [`Address`](../../../classes/Address.md)[] | Cosignatory address additions. All accounts in this list will be able to cosign transactions on behalf of the multisig account. The number of required cosignatures depends on the configured minimum approval and minimum removal values. |
| `addressDeletions`? | [`Address`](../../../classes/Address.md)[] | Cosignatory address deletions. All accounts in this list will stop being able to cosign transactions on behalf of the multisig account. A transaction containing **any** address in this array requires a number of cosignatures at least equal to the minimum removal value. |

#### Returns

`MultisigAccountModificationTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.minApprovalDelta` | `number` |
| `rawDescriptor.minRemovalDelta` | `number` |
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
