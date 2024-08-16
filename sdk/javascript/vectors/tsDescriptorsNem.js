import { Hash256, PublicKey } from '../src/CryptoTypes.js';
import { Address } from '../src/nem/Network.js';
import * as models from '../src/nem/models.js';
import * as descriptors from '../src/nem/models_ts.js';

// region subobject parsers

/**
 * @param {object} message Raw message descriptor.
 * @returns {descriptors.MessageDescriptor} Message descriptor.
 */
const mapMessage = message => new descriptors.MessageDescriptor(
	models.MessageType[message.messageType.toUpperCase()],
	message.message
);

/**
 * @param {object} mosaicId Raw mosaic id descriptor.
 * @returns {descriptors.MosaicIdDescriptor} Mosaic id descriptor.
 */
const mapMosaicIdDescriptor = mosaicId => new descriptors.MosaicIdDescriptor(
	new descriptors.NamespaceIdDescriptor(mosaicId.namespaceId.name),
	mosaicId.name
);

/**
 * @param {object} mosaic Raw mosaic descriptor.
 * @returns {descriptors.SizePrefixedMosaicDescriptor} Mosaic descriptor.
 */
const mapMosaicDescriptor = mosaic =>
	new descriptors.SizePrefixedMosaicDescriptor(new descriptors.MosaicDescriptor(
		mapMosaicIdDescriptor(mosaic.mosaic.mosaicId),
		new models.Amount(mosaic.mosaic.amount)
	));

/**
 * @param {object} modification Raw multisig account modification descriptor.
 * @returns {descriptors.SizePrefixedMultisigAccountModificationDescriptor} Multisig account modification descriptor.
 */
const mapMultisigAccountModification = modification =>
	new descriptors.SizePrefixedMultisigAccountModificationDescriptor(new descriptors.MultisigAccountModificationDescriptor(
		models.MultisigAccountModificationType[modification.modification.modificationType.toUpperCase()],
		new PublicKey(modification.modification.cosignatoryPublicKey)
	));

/**
 * @param {object} levy Raw mosaic levy descriptor.
 * @returns {descriptors.MosaicLevyDescriptor} Mosaic levy descriptor.
 */
const mapMosaicLevyDescriptor = levy => new descriptors.MosaicLevyDescriptor(
	models.MosaicTransferFeeType[levy.transferFeeType.toUpperCase()],
	new Address(levy.recipientAddress),
	mapMosaicIdDescriptor(levy.mosaicId),
	new models.Amount(levy.fee)
);

/**
 * @param {object} property Raw mosaic property descriptor.
 * @returns {descriptors.SizePrefixedMosaicPropertyDescriptor} Mosaic property descriptor.
 */
const mapMosaicPropertyDescriptor = property =>
	new descriptors.SizePrefixedMosaicPropertyDescriptor(new descriptors.MosaicPropertyDescriptor(
		property.property.name,
		property.property.value
	));

/**
 * @param {object} mosaicDefinition Raw mosaic definition descriptor.
 * @returns {descriptors.MosaicDefinitionDescriptor} Mosaic definition descriptor.
 */
const mapMosaicDefinition = mosaicDefinition => new descriptors.MosaicDefinitionDescriptor(
	new PublicKey(mosaicDefinition.ownerPublicKey),
	mapMosaicIdDescriptor(mosaicDefinition.id),
	mosaicDefinition.description,
	mosaicDefinition.properties ? mosaicDefinition.properties.map(mapMosaicPropertyDescriptor) : undefined,
	mosaicDefinition.levy ? mapMosaicLevyDescriptor(mosaicDefinition.levy) : undefined
);

// endregion

const createTypedTransactionDescriptor = descriptor => {
	// account key link
	if ('account_key_link_transaction_v1' === descriptor.type) {
		return new descriptors.AccountKeyLinkTransactionV1Descriptor(
			models.LinkAction[descriptor.linkAction.toUpperCase()],
			new PublicKey(descriptor.remotePublicKey)
		);
	}

	// mosaic
	if ('mosaic_definition_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicDefinitionTransactionV1Descriptor(
			mapMosaicDefinition(descriptor.mosaicDefinition),
			new Address(descriptor.rentalFeeSink),
			new models.Amount(descriptor.rentalFee)
		);
	}
	if ('mosaic_supply_change_transaction_v1' === descriptor.type) {
		return new descriptors.MosaicSupplyChangeTransactionV1Descriptor(
			mapMosaicIdDescriptor(descriptor.mosaicId),
			models.MosaicSupplyChangeAction[descriptor.action.toUpperCase()],
			new models.Amount(descriptor.delta)
		);
	}

	// multisig
	if ('multisig_account_modification_transaction_v1' === descriptor.type) {
		return new descriptors.MultisigAccountModificationTransactionV1Descriptor(descriptor.modifications
			.map(mapMultisigAccountModification));
	}
	if ('multisig_account_modification_transaction_v2' === descriptor.type) {
		return new descriptors.MultisigAccountModificationTransactionV2Descriptor(
			descriptor.minApprovalDelta,
			descriptor.modifications.map(mapMultisigAccountModification)
		);
	}

	// namespace
	if ('namespace_registration_transaction_v1' === descriptor.type) {
		return new descriptors.NamespaceRegistrationTransactionV1Descriptor(
			new Address(descriptor.rentalFeeSink),
			new models.Amount(descriptor.rentalFee),
			descriptor.name,
			descriptor.parentName
		);
	}

	// transfer
	if ('transfer_transaction_v1' === descriptor.type) {
		return new descriptors.TransferTransactionV1Descriptor(
			new Address(descriptor.recipientAddress),
			new models.Amount(descriptor.amount),
			descriptor.message ? mapMessage(descriptor.message) : undefined
		);
	}
	if ('transfer_transaction_v2' === descriptor.type) {
		return new descriptors.TransferTransactionV2Descriptor(
			new Address(descriptor.recipientAddress),
			new models.Amount(descriptor.amount),
			descriptor.message ? mapMessage(descriptor.message) : undefined,
			descriptor.mosaics ? descriptor.mosaics.map(mapMosaicDescriptor) : undefined
		);
	}

	// aggregate
	if ('cosignature_v1' === descriptor.type) {
		return new descriptors.CosignatureV1Descriptor(
			new Hash256(descriptor.otherTransactionHash),
			new Address(descriptor.multisigAccountAddress)
		);
	}

	if ('multisig_transaction_v1' === descriptor.type) {
		return new descriptors.MultisigTransactionV1Descriptor(
			createTypedTransactionDescriptor(descriptor.innerTransaction),
			descriptor.cosignatures
				? descriptor.cosignatures.map(cosignatureWrapper => {
					const { cosignature } = cosignatureWrapper;
					const cosignatureDescriptor = createTypedTransactionDescriptor({
						type: 'cosignature_v1',
						...cosignature
					});

					cosignatureDescriptor.rawDescriptor = {
						...cosignatureDescriptor.rawDescriptor,

						// override base transaction properties to get vectors to pass
						signature: new models.Signature(cosignature.signature),
						timestamp: cosignature.timestamp,
						signerPublicKey: cosignature.signerPublicKey,
						fee: cosignature.fee
					};
					return new descriptors.SizePrefixedCosignatureV1Descriptor(cosignatureDescriptor);
				})
				: undefined
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
		timestamp: descriptor.timestamp,
		signerPublicKey: descriptor.signerPublicKey,
		fee: descriptor.fee
	};
};
