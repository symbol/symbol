---
hide:
  - toc
---

# Serialization

## Basic Types

<div class="big-table3">
   <div id="amount"><b>Amount</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>A quantity of mosaics in absolute units. <br/>It can only be positive or zero. Negative quantities must be indicated by other means (See for example <a href="#mosaicsupplychangetransactionv1" title="Change the total supply of a mosaic (V1, latest).">MosaicSupplyChangeTransactionV1</a> and <a href="#mosaicsupplychangeaction" title="Enumeration of mosaic supply change actions.">MosaicSupplyChangeAction</a>). </p></div>
   <div id="blockduration"><b>BlockDuration</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>A time lapse, expressed in number of blocks. </p></div>
   <div id="blockfeemultiplier"><b>BlockFeeMultiplier</b></div>
   <div>4&nbsp;ubytes</div>
   <div class="description"><p>Multiplier applied to the size of a transaction to obtain its fee, in absolute units. <br/>See the fees documentation. </p></div>
   <div id="difficulty"><b>Difficulty</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>How hard it was to harvest this block. <br/>The initial value is 1e14 and it will remain like this as long as blocks are generated every <code class="docutils literal">blockGenerationTargetTime</code> seconds (network property). <br/>If blocks start taking more or less time than the configured value, the difficulty will be adjusted (in the range of 1e13 to 1e15) to try to hit the target time. <br/>See the Technical Reference section 8.1. </p></div>
   <div id="finalizationepoch"><b>FinalizationEpoch</b></div>
   <div>4&nbsp;ubytes</div>
   <div class="description"><p>Index of a finalization epoch. <br/>The first epoch is number 1 and contains only the first block (the Nemesis block). Epoch duration (in blocks) is defined by the <code class="docutils literal">votingSetGrouping</code> network property. </p></div>
   <div id="finalizationpoint"><b>FinalizationPoint</b></div>
   <div>4&nbsp;ubytes</div>
   <div class="description"><p>A particular point in time inside a finalization epoch. <br/>See the Technical Reference section 15.2. </p></div>
   <div id="height"><b>Height</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>Index of a block in the blockchain. <br/>The first block (the Nemesis block) has height 1 and each subsequent block increases height by 1. </p></div>
   <div id="importance"><b>Importance</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>Importance score for an account. <br/>See also <a href="#importanceheight" title="Block height at which an Importance was calculated.">ImportanceHeight</a> and <a href="#importancesnapshot" title="temporal importance information">ImportanceSnapshot</a>. </p></div>
   <div id="importanceheight"><b>ImportanceHeight</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p><a href="#block" title="binary layout for a block">Block</a> height at which an <a href="#importance" title="Importance score for an account.">Importance</a> was calculated. </p></div>
   <div id="unresolvedmosaicid"><b>UnresolvedMosaicId</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>Either a <a href="#mosaicid" title="A Mosaic identifier.">MosaicId</a> or a <a href="#namespaceid" title="">NamespaceId</a>. <br/>The <strong>most</strong>-significant bit of the first byte is 0 for <a href="#mosaicid" title="A Mosaic identifier.">MosaicId</a>'s and 1 for <a href="#namespaceid" title="">NamespaceId</a>'s. </p></div>
   <div id="mosaicid"><b>MosaicId</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>A Mosaic identifier. </p></div>
   <div id="timestamp"><b>Timestamp</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"><p>Number of milliseconds elapsed since the creation of the Nemesis block. <br/>The Nemesis block creation time can be found in the <code class="docutils literal">epochAdjustment</code> field returned by the /network/properties REST endpoint. This is the number of seconds elapsed since the <a href="https://en.wikipedia.org/wiki/Unix_time">UNIX epoch</a> and it is always 1615853185 for Symbol's MAINNET. </p></div>
   <div id="unresolvedaddress"><b>UnresolvedAddress</b></div>
   <div>24&nbsp;ubytes</div>
   <div class="description"><p>Either an <a href="#address" title="An address identifies an account and is derived from its PublicKey.">Address</a> or a <a href="#namespaceid" title="">NamespaceId</a>. <br/>The <strong>least</strong>-significant bit of the first byte is 0 for Addresses and 1 for <a href="#namespaceid" title="">NamespaceId</a>'s. </p></div>
   <div id="address"><b>Address</b></div>
   <div>24&nbsp;ubytes</div>
   <div class="description"><p>An address identifies an account and is derived from its <a href="#publickey" title="A 32-byte (256 bit) integer derived from a private key.">PublicKey</a>. </p></div>
   <div id="hash256"><b>Hash256</b></div>
   <div>32&nbsp;ubytes</div>
   <div class="description"><p>A 32-byte (256 bit) hash. <br/>The exact algorithm is unspecified as it can change depending on where it is used. </p></div>
   <div id="hash512"><b>Hash512</b></div>
   <div>64&nbsp;ubytes</div>
   <div class="description"><p>A 64-byte (512 bit) hash. <br/>The exact algorithm is unspecified as it can change depending on where it is used. </p></div>
   <div id="publickey"><b>PublicKey</b></div>
   <div>32&nbsp;ubytes</div>
   <div class="description"><p>A 32-byte (256 bit) integer derived from a private key. <br/>It serves as the public identifier of the key pair and can be disseminated widely. It is used to prove that an entity was signed with the paired private key. </p></div>
   <div id="votingpublickey"><b>VotingPublicKey</b></div>
   <div>32&nbsp;ubytes</div>
   <div class="description"><p>A <a href="#publickey" title="A 32-byte (256 bit) integer derived from a private key.">PublicKey</a> used for voting during the finalization process. </p></div>
   <div id="signature"><b>Signature</b></div>
   <div>64&nbsp;ubytes</div>
   <div class="description"><p>A 64-byte (512 bit) array certifying that the signed data has not been modified. <br/>Symbol currently uses <a href="https://ed25519.cr.yp.to/">Ed25519</a> signatures. </p></div>
   <div id="proofgamma"><b>ProofGamma</b></div>
   <div>32&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="proofverificationhash"><b>ProofVerificationHash</b></div>
   <div>16&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="proofscalar"><b>ProofScalar</b></div>
   <div>32&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="namespaceid"><b>NamespaceId</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="mosaicnonce"><b>MosaicNonce</b></div>
   <div>4&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="mosaicrestrictionkey"><b>MosaicRestrictionKey</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"></div>
   <div id="scopedmetadatakey"><b>ScopedMetadataKey</b></div>
   <div>8&nbsp;ubytes</div>
   <div class="description"></div>
</div>

## Enumerations

### LinkAction

--8<-- 'devbook/reference/serialization/LinkAction.html'

### NetworkType

--8<-- 'devbook/reference/serialization/NetworkType.html'

### TransactionType

--8<-- 'devbook/reference/serialization/TransactionType.html'

### BlockType

--8<-- 'devbook/reference/serialization/BlockType.html'

### ReceiptType

--8<-- 'devbook/reference/serialization/ReceiptType.html'

### NamespaceRegistrationType

--8<-- 'devbook/reference/serialization/NamespaceRegistrationType.html'

### AliasAction

--8<-- 'devbook/reference/serialization/AliasAction.html'

### LockHashAlgorithm

--8<-- 'devbook/reference/serialization/LockHashAlgorithm.html'

### MosaicFlags

--8<-- 'devbook/reference/serialization/MosaicFlags.html'

### MosaicSupplyChangeAction

--8<-- 'devbook/reference/serialization/MosaicSupplyChangeAction.html'

### AccountRestrictionFlags

--8<-- 'devbook/reference/serialization/AccountRestrictionFlags.html'

### MosaicRestrictionType

--8<-- 'devbook/reference/serialization/MosaicRestrictionType.html'

### AccountType

--8<-- 'devbook/reference/serialization/AccountType.html'

### AccountKeyTypeFlags

--8<-- 'devbook/reference/serialization/AccountKeyTypeFlags.html'

### AccountStateFormat

--8<-- 'devbook/reference/serialization/AccountStateFormat.html'

### LockStatus

--8<-- 'devbook/reference/serialization/LockStatus.html'

### MetadataType

--8<-- 'devbook/reference/serialization/MetadataType.html'

### NamespaceAliasType

--8<-- 'devbook/reference/serialization/NamespaceAliasType.html'

### MosaicRestrictionEntryType

--8<-- 'devbook/reference/serialization/MosaicRestrictionEntryType.html'

## Structures

### Mosaic

--8<-- 'devbook/reference/serialization/Mosaic.html'

### UnresolvedMosaic

--8<-- 'devbook/reference/serialization/UnresolvedMosaic.html'

### Transaction

--8<-- 'devbook/reference/serialization/Transaction.html'

### EmbeddedTransaction

--8<-- 'devbook/reference/serialization/EmbeddedTransaction.html'

### VrfProof

--8<-- 'devbook/reference/serialization/VrfProof.html'

### Block

--8<-- 'devbook/reference/serialization/Block.html'

### NemesisBlockV1

--8<-- 'devbook/reference/serialization/NemesisBlockV1.html'

### NormalBlockV1

--8<-- 'devbook/reference/serialization/NormalBlockV1.html'

### ImportanceBlockV1

--8<-- 'devbook/reference/serialization/ImportanceBlockV1.html'

### FinalizationRound

--8<-- 'devbook/reference/serialization/FinalizationRound.html'

### FinalizedBlockHeader

--8<-- 'devbook/reference/serialization/FinalizedBlockHeader.html'

### Receipt

--8<-- 'devbook/reference/serialization/Receipt.html'

### HarvestFeeReceipt

--8<-- 'devbook/reference/serialization/HarvestFeeReceipt.html'

### InflationReceipt

--8<-- 'devbook/reference/serialization/InflationReceipt.html'

### LockHashCreatedFeeReceipt

--8<-- 'devbook/reference/serialization/LockHashCreatedFeeReceipt.html'

### LockHashCompletedFeeReceipt

--8<-- 'devbook/reference/serialization/LockHashCompletedFeeReceipt.html'

### LockHashExpiredFeeReceipt

--8<-- 'devbook/reference/serialization/LockHashExpiredFeeReceipt.html'

### LockSecretCreatedFeeReceipt

--8<-- 'devbook/reference/serialization/LockSecretCreatedFeeReceipt.html'

### LockSecretCompletedFeeReceipt

--8<-- 'devbook/reference/serialization/LockSecretCompletedFeeReceipt.html'

### LockSecretExpiredFeeReceipt

--8<-- 'devbook/reference/serialization/LockSecretExpiredFeeReceipt.html'

### MosaicExpiredReceipt

--8<-- 'devbook/reference/serialization/MosaicExpiredReceipt.html'

### MosaicRentalFeeReceipt

--8<-- 'devbook/reference/serialization/MosaicRentalFeeReceipt.html'

### NamespaceExpiredReceipt

--8<-- 'devbook/reference/serialization/NamespaceExpiredReceipt.html'

### NamespaceDeletedReceipt

--8<-- 'devbook/reference/serialization/NamespaceDeletedReceipt.html'

### NamespaceRentalFeeReceipt

--8<-- 'devbook/reference/serialization/NamespaceRentalFeeReceipt.html'

### ReceiptSource

--8<-- 'devbook/reference/serialization/ReceiptSource.html'

### AddressResolutionEntry

--8<-- 'devbook/reference/serialization/AddressResolutionEntry.html'

### AddressResolutionStatement

--8<-- 'devbook/reference/serialization/AddressResolutionStatement.html'

### MosaicResolutionEntry

--8<-- 'devbook/reference/serialization/MosaicResolutionEntry.html'

### MosaicResolutionStatement

--8<-- 'devbook/reference/serialization/MosaicResolutionStatement.html'

### TransactionStatement

--8<-- 'devbook/reference/serialization/TransactionStatement.html'

### BlockStatement

--8<-- 'devbook/reference/serialization/BlockStatement.html'

### AccountKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/AccountKeyLinkTransactionV1.html'

### EmbeddedAccountKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAccountKeyLinkTransactionV1.html'

### NodeKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/NodeKeyLinkTransactionV1.html'

### EmbeddedNodeKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedNodeKeyLinkTransactionV1.html'

### Cosignature

--8<-- 'devbook/reference/serialization/Cosignature.html'

### DetachedCosignature

--8<-- 'devbook/reference/serialization/DetachedCosignature.html'

### AggregateCompleteTransactionV1

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV1.html'

### AggregateCompleteTransactionV2

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV2.html'

### AggregateCompleteTransactionV3

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV3.html'

### AggregateBondedTransactionV1

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV1.html'

### AggregateBondedTransactionV2

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV2.html'

### AggregateBondedTransactionV3

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV3.html'

### VotingKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/VotingKeyLinkTransactionV1.html'

### EmbeddedVotingKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedVotingKeyLinkTransactionV1.html'

### VrfKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/VrfKeyLinkTransactionV1.html'

### EmbeddedVrfKeyLinkTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedVrfKeyLinkTransactionV1.html'

### HashLockTransactionV1

--8<-- 'devbook/reference/serialization/HashLockTransactionV1.html'

### EmbeddedHashLockTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedHashLockTransactionV1.html'

### SecretLockTransactionV1

--8<-- 'devbook/reference/serialization/SecretLockTransactionV1.html'

### EmbeddedSecretLockTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedSecretLockTransactionV1.html'

### SecretProofTransactionV1

--8<-- 'devbook/reference/serialization/SecretProofTransactionV1.html'

### EmbeddedSecretProofTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedSecretProofTransactionV1.html'

### AccountMetadataTransactionV1

--8<-- 'devbook/reference/serialization/AccountMetadataTransactionV1.html'

### EmbeddedAccountMetadataTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAccountMetadataTransactionV1.html'

### MosaicMetadataTransactionV1

--8<-- 'devbook/reference/serialization/MosaicMetadataTransactionV1.html'

### EmbeddedMosaicMetadataTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicMetadataTransactionV1.html'

### NamespaceMetadataTransactionV1

--8<-- 'devbook/reference/serialization/NamespaceMetadataTransactionV1.html'

### EmbeddedNamespaceMetadataTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedNamespaceMetadataTransactionV1.html'

### MosaicDefinitionTransactionV1

--8<-- 'devbook/reference/serialization/MosaicDefinitionTransactionV1.html'

### EmbeddedMosaicDefinitionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicDefinitionTransactionV1.html'

### MosaicSupplyChangeTransactionV1

--8<-- 'devbook/reference/serialization/MosaicSupplyChangeTransactionV1.html'

### EmbeddedMosaicSupplyChangeTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicSupplyChangeTransactionV1.html'

### MosaicSupplyRevocationTransactionV1

--8<-- 'devbook/reference/serialization/MosaicSupplyRevocationTransactionV1.html'

### EmbeddedMosaicSupplyRevocationTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicSupplyRevocationTransactionV1.html'

### MultisigAccountModificationTransactionV1

--8<-- 'devbook/reference/serialization/MultisigAccountModificationTransactionV1.html'

### EmbeddedMultisigAccountModificationTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMultisigAccountModificationTransactionV1.html'

### AddressAliasTransactionV1

--8<-- 'devbook/reference/serialization/AddressAliasTransactionV1.html'

### EmbeddedAddressAliasTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAddressAliasTransactionV1.html'

### MosaicAliasTransactionV1

--8<-- 'devbook/reference/serialization/MosaicAliasTransactionV1.html'

### EmbeddedMosaicAliasTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicAliasTransactionV1.html'

### NamespaceRegistrationTransactionV1

--8<-- 'devbook/reference/serialization/NamespaceRegistrationTransactionV1.html'

### EmbeddedNamespaceRegistrationTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedNamespaceRegistrationTransactionV1.html'

### AccountAddressRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/AccountAddressRestrictionTransactionV1.html'

### EmbeddedAccountAddressRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAccountAddressRestrictionTransactionV1.html'

### AccountMosaicRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/AccountMosaicRestrictionTransactionV1.html'

### EmbeddedAccountMosaicRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAccountMosaicRestrictionTransactionV1.html'

### AccountOperationRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/AccountOperationRestrictionTransactionV1.html'

### EmbeddedAccountOperationRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedAccountOperationRestrictionTransactionV1.html'

### MosaicAddressRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/MosaicAddressRestrictionTransactionV1.html'

### EmbeddedMosaicAddressRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicAddressRestrictionTransactionV1.html'

### MosaicGlobalRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/MosaicGlobalRestrictionTransactionV1.html'

### EmbeddedMosaicGlobalRestrictionTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedMosaicGlobalRestrictionTransactionV1.html'

### TransferTransactionV1

--8<-- 'devbook/reference/serialization/TransferTransactionV1.html'

### EmbeddedTransferTransactionV1

--8<-- 'devbook/reference/serialization/EmbeddedTransferTransactionV1.html'

### PinnedVotingKey

--8<-- 'devbook/reference/serialization/PinnedVotingKey.html'

### ImportanceSnapshot

--8<-- 'devbook/reference/serialization/ImportanceSnapshot.html'

### HeightActivityBucket

--8<-- 'devbook/reference/serialization/HeightActivityBucket.html'

### HeightActivityBuckets

--8<-- 'devbook/reference/serialization/HeightActivityBuckets.html'

### AccountState

--8<-- 'devbook/reference/serialization/AccountState.html'

### HashLockInfo

--8<-- 'devbook/reference/serialization/HashLockInfo.html'

### MetadataValue

--8<-- 'devbook/reference/serialization/MetadataValue.html'

### MetadataEntry

--8<-- 'devbook/reference/serialization/MetadataEntry.html'

### MosaicProperties

--8<-- 'devbook/reference/serialization/MosaicProperties.html'

### MosaicDefinition

--8<-- 'devbook/reference/serialization/MosaicDefinition.html'

### MosaicEntry

--8<-- 'devbook/reference/serialization/MosaicEntry.html'

### MultisigEntry

--8<-- 'devbook/reference/serialization/MultisigEntry.html'

### NamespaceLifetime

--8<-- 'devbook/reference/serialization/NamespaceLifetime.html'

### NamespaceAlias

--8<-- 'devbook/reference/serialization/NamespaceAlias.html'

### NamespacePath

--8<-- 'devbook/reference/serialization/NamespacePath.html'

### RootNamespaceHistory

--8<-- 'devbook/reference/serialization/RootNamespaceHistory.html'

### AccountRestrictionAddressValue

--8<-- 'devbook/reference/serialization/AccountRestrictionAddressValue.html'

### AccountRestrictionMosaicValue

--8<-- 'devbook/reference/serialization/AccountRestrictionMosaicValue.html'

### AccountRestrictionTransactionTypeValue

--8<-- 'devbook/reference/serialization/AccountRestrictionTransactionTypeValue.html'

### AccountRestrictionsInfo

--8<-- 'devbook/reference/serialization/AccountRestrictionsInfo.html'

### AccountRestrictions

--8<-- 'devbook/reference/serialization/AccountRestrictions.html'

### AddressKeyValue

--8<-- 'devbook/reference/serialization/AddressKeyValue.html'

### AddressKeyValueSet

--8<-- 'devbook/reference/serialization/AddressKeyValueSet.html'

### RestrictionRule

--8<-- 'devbook/reference/serialization/RestrictionRule.html'

### GlobalKeyValue

--8<-- 'devbook/reference/serialization/GlobalKeyValue.html'

### GlobalKeyValueSet

--8<-- 'devbook/reference/serialization/GlobalKeyValueSet.html'

### MosaicAddressRestrictionEntry

--8<-- 'devbook/reference/serialization/MosaicAddressRestrictionEntry.html'

### MosaicGlobalRestrictionEntry

--8<-- 'devbook/reference/serialization/MosaicGlobalRestrictionEntry.html'

### MosaicRestrictionEntry

--8<-- 'devbook/reference/serialization/MosaicRestrictionEntry.html'

### SecretLockInfo

--8<-- 'devbook/reference/serialization/SecretLockInfo.html'

## Inner Structures

These are structures only meant to be included inside other structures.
Their description is already present in the containing structures above and is only repeated here for completeness.

<style>
.md-typeset h3 {
    background: var(--md-accent-fg-color--light);
    padding: 10px;
}

.md-typeset .big-table3 {
    /* Tables with lots of content in 3 columns */
    font-size: medium;
    word-break: normal;
    display: grid;
    grid-template-columns: max-content max-content auto;
    margin-top: 20px;
}

.md-typeset .big-table3 p,
.md-typeset .big-table6 p {
    margin-top: 0;
}

.md-typeset .big-table6 {
    /* Tables with lots of content in 6 columns*/
    font-size: medium;
    word-break: normal;
    display: grid;
    grid-template-columns: 10px 10px 10px minmax(min-content, 25%) minmax(min-content,25%) auto;
    margin-top: 20px;
}

/* divs inside big-table are actually cells */
.md-typeset .big-table3 div,
.md-typeset .big-table6 div {
    padding-left: 10px;
    vertical-align: top;
    border-top: 1px solid var(--md-accent-fg-color--light);
}

.md-typeset__table,
.md-typeset__table tbody {
    display: table;
    width: 100%;
    margin: 0;
}

.side-info {
    float: right;
}

.side-info td {
    background-color: var(--md-accent-fg-color--light);
}

.md-typeset table:not([class]) {
    border: none;
}
.md-typeset table:not([class]) td {
    border: none;
    padding-top: 0.25rem;
    padding-bottom: 0.25rem;
}
</style>
