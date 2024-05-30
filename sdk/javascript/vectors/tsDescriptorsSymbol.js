import { Hash256, PublicKey } from '../src/CryptoTypes.js';
import { Address } from '../src/symbol/Network.js';
import * as models from '../src/symbol/models.js';
import * as descriptors from '../src/symbol/models_ts.js';

// region flags parsers

/**
 * @param {string} flags Comma-delimited account restriction flag strings.
 * @returns {models.AccountRestrictionFlags} Account restriction flags value.
 */
const parseAccountRestrictionFlags = flags => new models.AccountRestrictionFlags(flags.split(' ').reduce(
	(bits, flagStr) => bits | models.AccountRestrictionFlags[flagStr.toUpperCase()].value,
	0
));

/**
 * @param {string} flags Comma-delimited mosaic flag strings.
 * @returns {models.MosaicFlags} Mosaic flags value.
 */
const parseMosaicFlags = flags => new models.MosaicFlags(flags.split(' ').reduce(
	(bits, flagStr) => bits | models.MosaicFlags[flagStr.toUpperCase()].value,
	0
));

// endregion

// region subobject parsers

/**
 * @param {object} mosaic Raw unresolved mosaic descriptor.
 * @returns {descriptors.UnresolvedMosaicDescriptor} Unresolved mosaic descriptor.
 */
const mapUnresolvedMosaic = mosaic => new descriptors.UnresolvedMosaicDescriptor(
	new models.UnresolvedMosaicId(mosaic.mosaicId),
	new models.Amount(mosaic.amount)
);

/**
 * @param {object} descriptor Raw cosignature descriptor.
 * @returns {models.Cosignature} Cosignature.
 */
const mapCosignature = descriptor => {
	const cosignature = new models.Cosignature();
	cosignature.signerPublicKey = new models.PublicKey(descriptor.signerPublicKey);
	cosignature.signature = new models.Signature(descriptor.signature);
	return cosignature;
};

/**
 * @param {string[]|undefined} addresses String addresses.
 * @returns {Address[]|undefined} Addresses.
 */
const mapAddressList = addresses => (addresses ? addresses.map(address => new Address(address)) : undefined);

/**
 * @param {bigint[]|undefined} mosaicIds String mosaic ids.
 * @returns {models.UnresolvedMosaicId[]|undefined} Mosaic ids.
 */
const mapMosaicIdList = mosaicIds => (mosaicIds ? mosaicIds.map(mosaicId => new models.UnresolvedMosaicId(mosaicId)) : undefined);

/**
 * @param {string[]|undefined} transactionTypes String transaction types.
 * @returns {models.TransactionType[]|undefined} Transaction types.
 */
const mapTransactionTypeList = transactionTypes => (transactionTypes
	? transactionTypes.map(transactionType => models.TransactionType[transactionType.toUpperCase()])
	: undefined);

// endregion

const createTypedTransactionDescriptor = descriptor => {
	// account link
	if ('account_key_link_transaction_v1' === descriptor.type) {
		return new descriptors.AccountKeyLinkTransactionV1Descriptor(
			new PublicKey(descriptor.linkedPublicKey),
			models.LinkAction[descriptor.linkAction.toUpperCase()]
		);
	}
	if ('node_key_link_transaction_v1' === descriptor.type) {
		return new descriptors.NodeKeyLinkTransactionV1Descriptor(
			new PublicKey(descriptor.linkedPublicKey),
			models.LinkAction[descriptor.linkAction.toUpperCase()]
		);
	}

	// coresystem
	if ('voting_key_link_transaction_v1' === descriptor.type) {
		return new descriptors.VotingKeyLinkTransactionV1Descriptor(
			new PublicKey(descriptor.linkedPublicKey),
			new models.FinalizationEpoch(descriptor.startEpoch),
			new models.FinalizationEpoch(descriptor.endEpoch),
			models.LinkAction[descriptor.linkAction.toUpperCase()]
		);
	}
	if ('vrf_key_link_transaction_v1' === descriptor.type) {
		return new descriptors.VrfKeyLinkTransactionV1Descriptor(
			new PublicKey(descriptor.linkedPublicKey),
			models.LinkAction[descriptor.linkAction.toUpperCase()]
		);
	}

	// lock hash
	if ('hash_lock_transaction_v1' === descriptor.type) {
		return new descriptors.HashLockTransactionV1Descriptor(
			mapUnresolvedMosaic(descriptor.mosaic),
			new models.BlockDuration(descriptor.duration),
			new Hash256(descriptor.hash)
		);
	}

	// lock secret
	if ('secret_lock_transaction_v1' === descriptor.type) {
		return new descriptors.SecretLockTransactionV1Descriptor(
			new Address(descriptor.recipientAddress),
			new Hash256(descriptor.secret),
			mapUnresolvedMosaic(descriptor.mosaic),
			new models.BlockDuration(descriptor.duration),
			models.LockHashAlgorithm[descriptor.hashAlgorithm.toUpperCase()]
		);
	}
	if ('secret_proof_transaction_v1' === descriptor.type) {
		return new descriptors.SecretProofTransactionV1Descriptor(
			new Address(descriptor.recipientAddress),
			new Hash256(descriptor.secret),
			models.LockHashAlgorithm[descriptor.hashAlgorithm.toUpperCase()],
			descriptor.proof
		);
	}

	// metadata
	if ('account_metadata_transaction_v1' === descriptor.type) {
		return new descriptors.AccountMetadataTransactionV1Descriptor(
			new Address(descriptor.targetAddress),
			descriptor.scopedMetadataKey,
			descriptor.valueSizeDelta,
			descriptor.value
		);
	}
	if ('mosaic_metadata_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicMetadataTransactionV1Descriptor(
			new Address(descriptor.targetAddress),
			descriptor.scopedMetadataKey,
			new models.UnresolvedMosaicId(descriptor.targetMosaicId),
			descriptor.valueSizeDelta,
			descriptor.value
		);
	}
	if ('namespace_metadata_transaction_v1' === descriptor.type) {
		return new descriptors.NamespaceMetadataTransactionV1Descriptor(
			new Address(descriptor.targetAddress),
			descriptor.scopedMetadataKey,
			new models.NamespaceId(descriptor.targetNamespaceId),
			descriptor.valueSizeDelta,
			descriptor.value
		);
	}

	// mosaic
	if ('mosaic_definition_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicDefinitionTransactionV1Descriptor(
			new models.MosaicId(0n), // placeholder, autogenerated
			new models.BlockDuration(descriptor.duration),
			new models.MosaicNonce(descriptor.nonce),
			parseMosaicFlags(descriptor.flags),
			descriptor.divisibility
		);
	}
	if ('mosaic_supply_change_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicSupplyChangeTransactionV1Descriptor(
			new models.UnresolvedMosaicId(descriptor.mosaicId),
			new models.Amount(descriptor.delta),
			models.MosaicSupplyChangeAction[descriptor.action.toUpperCase()]
		);
	}
	if ('mosaic_supply_revocation_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicSupplyRevocationTransactionV1Descriptor(
			new Address(descriptor.sourceAddress),
			mapUnresolvedMosaic(descriptor.mosaic)
		);
	}

	// multisig
	if ('multisig_account_modification_transaction_v1' === descriptor.type) {
		return new descriptors.MultisigAccountModificationTransactionV1Descriptor(
			descriptor.minRemovalDelta,
			descriptor.minApprovalDelta,
			mapAddressList(descriptor.addressAdditions),
			mapAddressList(descriptor.addressDeletions)
		);
	}

	// namespace
	if ('address_alias_transaction_v1' === descriptor.type) {
		return new descriptors.AddressAliasTransactionV1Descriptor(
			new models.NamespaceId(descriptor.namespaceId),
			new Address(descriptor.address),
			models.AliasAction[descriptor.aliasAction.toUpperCase()]
		);
	}
	if ('mosaic_alias_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicAliasTransactionV1Descriptor(
			new models.NamespaceId(descriptor.namespaceId),
			new models.MosaicId(descriptor.mosaicId),
			models.AliasAction[descriptor.aliasAction.toUpperCase()]
		);
	}
	if ('namespace_registration_transaction_v1' === descriptor.type) {
		return new descriptors.NamespaceRegistrationTransactionV1Descriptor(
			new models.NamespaceId(0n), // placeholder, autogenerated
			models.NamespaceRegistrationType[descriptor.registrationType.toUpperCase()],
			descriptor.duration ? new models.BlockDuration(descriptor.duration) : undefined, // only used for 'root'
			descriptor.parentId ? new models.NamespaceId(descriptor.parentId) : undefined, // only used for 'child'
			descriptor.name
		);
	}

	// restriction account
	if ('account_address_restriction_transaction_v1' === descriptor.type) {
		return new descriptors.AccountAddressRestrictionTransactionV1Descriptor(
			parseAccountRestrictionFlags(descriptor.restrictionFlags),
			mapAddressList(descriptor.restrictionAdditions),
			mapAddressList(descriptor.restrictionDeletions)
		);
	}
	if ('account_mosaic_restriction_transaction_v1' === descriptor.type) {
		return new descriptors.AccountMosaicRestrictionTransactionV1Descriptor(
			parseAccountRestrictionFlags(descriptor.restrictionFlags),
			mapMosaicIdList(descriptor.restrictionAdditions),
			mapMosaicIdList(descriptor.restrictionDeletions)
		);
	}
	if ('account_operation_restriction_transaction_v1' === descriptor.type) {
		return new descriptors.AccountOperationRestrictionTransactionV1Descriptor(
			parseAccountRestrictionFlags(descriptor.restrictionFlags),
			mapTransactionTypeList(descriptor.restrictionAdditions),
			mapTransactionTypeList(descriptor.restrictionDeletions)
		);
	}

	// restriction mosaic
	if ('mosaic_address_restriction_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicAddressRestrictionTransactionV1Descriptor(
			new models.UnresolvedMosaicId(descriptor.mosaicId),
			descriptor.restrictionKey,
			descriptor.previousRestrictionValue,
			descriptor.newRestrictionValue,
			new Address(descriptor.targetAddress)
		);
	}
	if ('mosaic_global_restriction_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicGlobalRestrictionTransactionV1Descriptor(
			new models.UnresolvedMosaicId(descriptor.mosaicId),
			new models.UnresolvedMosaicId(descriptor.referenceMosaicId),
			descriptor.restrictionKey,
			descriptor.previousRestrictionValue,
			descriptor.newRestrictionValue,
			models.MosaicRestrictionType[descriptor.previousRestrictionType.toUpperCase()],
			models.MosaicRestrictionType[descriptor.newRestrictionType.toUpperCase()]
		);
	}

	// transfer
	if ('transfer_transaction_v1' === descriptor.type) {
		return new descriptors.TransferTransactionV1Descriptor(
			new Address(descriptor.recipientAddress),
			descriptor.mosaics ? descriptor.mosaics.map(mapUnresolvedMosaic) : undefined,
			descriptor.message
		);
	}

	// aggregate

	/** @type {models.EmbeddedTransaction[]|undefined} */
	const transactions = descriptor.transactions
		? descriptor.transactions.map(createTypedTransactionDescriptor)
		: undefined;

	/** @type {models.Cosignature[]|undefined} */
	const cosignatures = descriptor.cosignatures ? descriptor.cosignatures.map(mapCosignature) : undefined;

	if ('aggregate_bonded_transaction_v1' === descriptor.type) {
		return new descriptors.AggregateBondedTransactionV1Descriptor(
			new Hash256(descriptor.transactionsHash),
			transactions,
			cosignatures
		);
	}
	if ('aggregate_bonded_transaction_v2' === descriptor.type) {
		return new descriptors.AggregateBondedTransactionV2Descriptor(
			new Hash256(descriptor.transactionsHash),
			transactions,
			cosignatures
		);
	}
	if ('aggregate_complete_transaction_v1' === descriptor.type) {
		return new descriptors.AggregateCompleteTransactionV1Descriptor(
			new Hash256(descriptor.transactionsHash),
			transactions,
			cosignatures
		);
	}
	if ('aggregate_complete_transaction_v2' === descriptor.type) {
		return new descriptors.AggregateCompleteTransactionV2Descriptor(
			new Hash256(descriptor.transactionsHash),
			transactions,
			cosignatures
		);
	}

	throw RangeError(`unknown transaction type ${descriptor.type}`);
};

export default descriptor => {
	const typedDescriptor = createTypedTransactionDescriptor(descriptor);
	return {
		...typedDescriptor.toMap(),

		// override base transaction properties to get vectors to pass
		signature: new models.Signature(descriptor.signature),
		signerPublicKey: descriptor.signerPublicKey,
		fee: descriptor.fee,
		deadline: descriptor.deadline
	};
};
