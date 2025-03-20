# Class: AccountAddressRestrictionTransactionV1Descriptor

Type safe descriptor used to generate a descriptor map for AccountAddressRestrictionTransactionV1Descriptor.

Allow or block incoming and outgoing transactions for a given a set of addresses (V1, latest).

## Constructors

### new AccountAddressRestrictionTransactionV1Descriptor()

```ts
new AccountAddressRestrictionTransactionV1Descriptor(
   restrictionFlags, 
   restrictionAdditions?, 
   restrictionDeletions?): AccountAddressRestrictionTransactionV1Descriptor
```

Creates a descriptor for AccountAddressRestrictionTransactionV1.

#### Parameters

| Parameter | Type | Description |
| ------ | ------ | ------ |
| `restrictionFlags` | [`AccountRestrictionFlags`](../../models/classes/AccountRestrictionFlags.md) | Type of restriction being applied to the listed addresses. |
| `restrictionAdditions`? | [`Address`](../../../classes/Address.md)[] | Array of account addresses being added to the restricted list. |
| `restrictionDeletions`? | [`Address`](../../../classes/Address.md)[] | Array of account addresses being removed from the restricted list. |

#### Returns

`AccountAddressRestrictionTransactionV1Descriptor`

## Properties

| Property | Type |
| ------ | ------ |
| <a id="rawdescriptor"></a> `rawDescriptor` | `object` |
| `rawDescriptor.restrictionFlags` | [`AccountRestrictionFlags`](../../models/classes/AccountRestrictionFlags.md) |
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
