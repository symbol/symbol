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

<a id="linkaction"></a>

--8<-- 'devbook/reference/serialization/LinkAction.html'

<a id="networktype"></a>

--8<-- 'devbook/reference/serialization/NetworkType.html'

<a id="transactiontype"></a>

--8<-- 'devbook/reference/serialization/TransactionType.html'

<a id="blocktype"></a>

--8<-- 'devbook/reference/serialization/BlockType.html'

<a id="receipttype"></a>

--8<-- 'devbook/reference/serialization/ReceiptType.html'

<a id="namespaceregistrationtype"></a>

--8<-- 'devbook/reference/serialization/NamespaceRegistrationType.html'

<a id="aliasaction"></a>

--8<-- 'devbook/reference/serialization/AliasAction.html'

<a id="lockhashalgorithm"></a>

--8<-- 'devbook/reference/serialization/LockHashAlgorithm.html'

<a id="mosaicflags"></a>

--8<-- 'devbook/reference/serialization/MosaicFlags.html'

<a id="mosaicsupplychangeaction"></a>

--8<-- 'devbook/reference/serialization/MosaicSupplyChangeAction.html'

<a id="accountrestrictionflags"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictionFlags.html'

<a id="mosaicrestrictiontype"></a>

--8<-- 'devbook/reference/serialization/MosaicRestrictionType.html'

<a id="accounttype"></a>

--8<-- 'devbook/reference/serialization/AccountType.html'

<a id="accountkeytypeflags"></a>

--8<-- 'devbook/reference/serialization/AccountKeyTypeFlags.html'

<a id="accountstateformat"></a>

--8<-- 'devbook/reference/serialization/AccountStateFormat.html'

<a id="lockstatus"></a>

--8<-- 'devbook/reference/serialization/LockStatus.html'

<a id="metadatatype"></a>

--8<-- 'devbook/reference/serialization/MetadataType.html'

<a id="namespacealiastype"></a>

--8<-- 'devbook/reference/serialization/NamespaceAliasType.html'

<a id="mosaicrestrictionentrytype"></a>

--8<-- 'devbook/reference/serialization/MosaicRestrictionEntryType.html'

## Structures

<a id="mosaic"></a>

--8<-- 'devbook/reference/serialization/Mosaic.html'

<a id="unresolvedmosaic"></a>

--8<-- 'devbook/reference/serialization/UnresolvedMosaic.html'

<a id="transaction"></a>

--8<-- 'devbook/reference/serialization/Transaction.html'

<a id="embeddedtransaction"></a>

--8<-- 'devbook/reference/serialization/EmbeddedTransaction.html'

<a id="vrfproof"></a>

--8<-- 'devbook/reference/serialization/VrfProof.html'

<a id="block"></a>

--8<-- 'devbook/reference/serialization/Block.html'

<a id="nemesisblockv1"></a>

--8<-- 'devbook/reference/serialization/NemesisBlockV1.html'

<a id="normalblockv1"></a>

--8<-- 'devbook/reference/serialization/NormalBlockV1.html'

<a id="importanceblockv1"></a>

--8<-- 'devbook/reference/serialization/ImportanceBlockV1.html'

<a id="finalizationround"></a>

--8<-- 'devbook/reference/serialization/FinalizationRound.html'

<a id="finalizedblockheader"></a>

--8<-- 'devbook/reference/serialization/FinalizedBlockHeader.html'

<a id="receipt"></a>

--8<-- 'devbook/reference/serialization/Receipt.html'

<a id="harvestfeereceipt"></a>

--8<-- 'devbook/reference/serialization/HarvestFeeReceipt.html'

<a id="inflationreceipt"></a>

--8<-- 'devbook/reference/serialization/InflationReceipt.html'

<a id="lockhashcreatedfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockHashCreatedFeeReceipt.html'

<a id="lockhashcompletedfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockHashCompletedFeeReceipt.html'

<a id="lockhashexpiredfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockHashExpiredFeeReceipt.html'

<a id="locksecretcreatedfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockSecretCreatedFeeReceipt.html'

<a id="locksecretcompletedfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockSecretCompletedFeeReceipt.html'

<a id="locksecretexpiredfeereceipt"></a>

--8<-- 'devbook/reference/serialization/LockSecretExpiredFeeReceipt.html'

<a id="mosaicexpiredreceipt"></a>

--8<-- 'devbook/reference/serialization/MosaicExpiredReceipt.html'

<a id="mosaicrentalfeereceipt"></a>

--8<-- 'devbook/reference/serialization/MosaicRentalFeeReceipt.html'

<a id="namespaceexpiredreceipt"></a>

--8<-- 'devbook/reference/serialization/NamespaceExpiredReceipt.html'

<a id="namespacedeletedreceipt"></a>

--8<-- 'devbook/reference/serialization/NamespaceDeletedReceipt.html'

<a id="namespacerentalfeereceipt"></a>

--8<-- 'devbook/reference/serialization/NamespaceRentalFeeReceipt.html'

<a id="receiptsource"></a>

--8<-- 'devbook/reference/serialization/ReceiptSource.html'

<a id="addressresolutionentry"></a>

--8<-- 'devbook/reference/serialization/AddressResolutionEntry.html'

<a id="addressresolutionstatement"></a>

--8<-- 'devbook/reference/serialization/AddressResolutionStatement.html'

<a id="mosaicresolutionentry"></a>

--8<-- 'devbook/reference/serialization/MosaicResolutionEntry.html'

<a id="mosaicresolutionstatement"></a>

--8<-- 'devbook/reference/serialization/MosaicResolutionStatement.html'

<a id="transactionstatement"></a>

--8<-- 'devbook/reference/serialization/TransactionStatement.html'

<a id="blockstatement"></a>

--8<-- 'devbook/reference/serialization/BlockStatement.html'

<a id="accountkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/AccountKeyLinkTransactionV1.html'

<a id="embeddedaccountkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAccountKeyLinkTransactionV1.html'

<a id="nodekeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/NodeKeyLinkTransactionV1.html'

<a id="embeddednodekeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedNodeKeyLinkTransactionV1.html'

<a id="cosignature"></a>

--8<-- 'devbook/reference/serialization/Cosignature.html'

<a id="detachedcosignature"></a>

--8<-- 'devbook/reference/serialization/DetachedCosignature.html'

<a id="aggregatecompletetransactionv1"></a>

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV1.html'

<a id="aggregatecompletetransactionv2"></a>

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV2.html'

<a id="aggregatecompletetransactionv3"></a>

--8<-- 'devbook/reference/serialization/AggregateCompleteTransactionV3.html'

<a id="aggregatebondedtransactionv1"></a>

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV1.html'

<a id="aggregatebondedtransactionv2"></a>

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV2.html'

<a id="aggregatebondedtransactionv3"></a>

--8<-- 'devbook/reference/serialization/AggregateBondedTransactionV3.html'

<a id="votingkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/VotingKeyLinkTransactionV1.html'

<a id="embeddedvotingkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedVotingKeyLinkTransactionV1.html'

<a id="vrfkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/VrfKeyLinkTransactionV1.html'

<a id="embeddedvrfkeylinktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedVrfKeyLinkTransactionV1.html'

<a id="hashlocktransactionv1"></a>

--8<-- 'devbook/reference/serialization/HashLockTransactionV1.html'

<a id="embeddedhashlocktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedHashLockTransactionV1.html'

<a id="secretlocktransactionv1"></a>

--8<-- 'devbook/reference/serialization/SecretLockTransactionV1.html'

<a id="embeddedsecretlocktransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedSecretLockTransactionV1.html'

<a id="secretprooftransactionv1"></a>

--8<-- 'devbook/reference/serialization/SecretProofTransactionV1.html'

<a id="embeddedsecretprooftransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedSecretProofTransactionV1.html'

<a id="accountmetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/AccountMetadataTransactionV1.html'

<a id="embeddedaccountmetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAccountMetadataTransactionV1.html'

<a id="mosaicmetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicMetadataTransactionV1.html'

<a id="embeddedmosaicmetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicMetadataTransactionV1.html'

<a id="namespacemetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/NamespaceMetadataTransactionV1.html'

<a id="embeddednamespacemetadatatransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedNamespaceMetadataTransactionV1.html'

<a id="mosaicdefinitiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicDefinitionTransactionV1.html'

<a id="embeddedmosaicdefinitiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicDefinitionTransactionV1.html'

<a id="mosaicsupplychangetransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicSupplyChangeTransactionV1.html'

<a id="embeddedmosaicsupplychangetransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicSupplyChangeTransactionV1.html'

<a id="mosaicsupplyrevocationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicSupplyRevocationTransactionV1.html'

<a id="embeddedmosaicsupplyrevocationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicSupplyRevocationTransactionV1.html'

<a id="multisigaccountmodificationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/MultisigAccountModificationTransactionV1.html'

<a id="embeddedmultisigaccountmodificationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMultisigAccountModificationTransactionV1.html'

<a id="addressaliastransactionv1"></a>

--8<-- 'devbook/reference/serialization/AddressAliasTransactionV1.html'

<a id="embeddedaddressaliastransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAddressAliasTransactionV1.html'

<a id="mosaicaliastransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicAliasTransactionV1.html'

<a id="embeddedmosaicaliastransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicAliasTransactionV1.html'

<a id="namespaceregistrationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/NamespaceRegistrationTransactionV1.html'

<a id="embeddednamespaceregistrationtransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedNamespaceRegistrationTransactionV1.html'

<a id="accountaddressrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/AccountAddressRestrictionTransactionV1.html'

<a id="embeddedaccountaddressrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAccountAddressRestrictionTransactionV1.html'

<a id="accountmosaicrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/AccountMosaicRestrictionTransactionV1.html'

<a id="embeddedaccountmosaicrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAccountMosaicRestrictionTransactionV1.html'

<a id="accountoperationrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/AccountOperationRestrictionTransactionV1.html'

<a id="embeddedaccountoperationrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedAccountOperationRestrictionTransactionV1.html'

<a id="mosaicaddressrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicAddressRestrictionTransactionV1.html'

<a id="embeddedmosaicaddressrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicAddressRestrictionTransactionV1.html'

<a id="mosaicglobalrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/MosaicGlobalRestrictionTransactionV1.html'

<a id="embeddedmosaicglobalrestrictiontransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedMosaicGlobalRestrictionTransactionV1.html'

<a id="transfertransactionv1"></a>

--8<-- 'devbook/reference/serialization/TransferTransactionV1.html'

<a id="embeddedtransfertransactionv1"></a>

--8<-- 'devbook/reference/serialization/EmbeddedTransferTransactionV1.html'

<a id="pinnedvotingkey"></a>

--8<-- 'devbook/reference/serialization/PinnedVotingKey.html'

<a id="importancesnapshot"></a>

--8<-- 'devbook/reference/serialization/ImportanceSnapshot.html'

<a id="heightactivitybucket"></a>

--8<-- 'devbook/reference/serialization/HeightActivityBucket.html'

<a id="heightactivitybuckets"></a>

--8<-- 'devbook/reference/serialization/HeightActivityBuckets.html'

<a id="accountstate"></a>

--8<-- 'devbook/reference/serialization/AccountState.html'

<a id="hashlockinfo"></a>

--8<-- 'devbook/reference/serialization/HashLockInfo.html'

<a id="metadatavalue"></a>

--8<-- 'devbook/reference/serialization/MetadataValue.html'

<a id="metadataentry"></a>

--8<-- 'devbook/reference/serialization/MetadataEntry.html'

<a id="mosaicproperties"></a>

--8<-- 'devbook/reference/serialization/MosaicProperties.html'

<a id="mosaicdefinition"></a>

--8<-- 'devbook/reference/serialization/MosaicDefinition.html'

<a id="mosaicentry"></a>

--8<-- 'devbook/reference/serialization/MosaicEntry.html'

<a id="multisigentry"></a>

--8<-- 'devbook/reference/serialization/MultisigEntry.html'

<a id="namespacelifetime"></a>

--8<-- 'devbook/reference/serialization/NamespaceLifetime.html'

<a id="namespacealias"></a>

--8<-- 'devbook/reference/serialization/NamespaceAlias.html'

<a id="namespacepath"></a>

--8<-- 'devbook/reference/serialization/NamespacePath.html'

<a id="rootnamespacehistory"></a>

--8<-- 'devbook/reference/serialization/RootNamespaceHistory.html'

<a id="accountrestrictionaddressvalue"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictionAddressValue.html'

<a id="accountrestrictionmosaicvalue"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictionMosaicValue.html'

<a id="accountrestrictiontransactiontypevalue"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictionTransactionTypeValue.html'

<a id="accountrestrictionsinfo"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictionsInfo.html'

<a id="accountrestrictions"></a>

--8<-- 'devbook/reference/serialization/AccountRestrictions.html'

<a id="addresskeyvalue"></a>

--8<-- 'devbook/reference/serialization/AddressKeyValue.html'

<a id="addresskeyvalueset"></a>

--8<-- 'devbook/reference/serialization/AddressKeyValueSet.html'

<a id="restrictionrule"></a>

--8<-- 'devbook/reference/serialization/RestrictionRule.html'

<a id="globalkeyvalue"></a>

--8<-- 'devbook/reference/serialization/GlobalKeyValue.html'

<a id="globalkeyvalueset"></a>

--8<-- 'devbook/reference/serialization/GlobalKeyValueSet.html'

<a id="mosaicaddressrestrictionentry"></a>

--8<-- 'devbook/reference/serialization/MosaicAddressRestrictionEntry.html'

<a id="mosaicglobalrestrictionentry"></a>

--8<-- 'devbook/reference/serialization/MosaicGlobalRestrictionEntry.html'

<a id="mosaicrestrictionentry"></a>

--8<-- 'devbook/reference/serialization/MosaicRestrictionEntry.html'

<a id="secretlockinfo"></a>

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
.md-typeset table:not([class]) td:has(dl) {
    padding-left: 0;
}
</style>
