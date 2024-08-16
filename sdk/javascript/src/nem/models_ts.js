/* eslint-disable max-len */

import { Address } from './Network.js';
import * as models from './models.js';
import { Hash256, PublicKey } from '../CryptoTypes.js';

/**
 * Type safe descriptor used to generate a descriptor map for AccountKeyLinkTransactionV1Descriptor.
 *
 * binary layout for an account key link transaction (V1, latest)
 */
export class AccountKeyLinkTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountKeyLinkTransactionV1.
	 * @param {models.LinkAction} linkAction link action
	 * @param {PublicKey} remotePublicKey public key of remote account to which importance should be transferred
	 */
	constructor(linkAction, remotePublicKey) {
		this.rawDescriptor = {
			type: 'account_key_link_transaction_v1',
			linkAction,
			remotePublicKey
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for NamespaceIdDescriptor.
 *
 * binary layout for a namespace id
 */
export class NamespaceIdDescriptor {
	/**
	 * Creates a descriptor for NamespaceId.
	 * @param {Uint8Array|string|undefined} name name
	 */
	constructor(name = undefined) {
		this.rawDescriptor = {
		};

		if (name)
			this.rawDescriptor.name = name;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicIdDescriptor.
 *
 * binary layout for a mosaic id
 */
export class MosaicIdDescriptor {
	/**
	 * Creates a descriptor for MosaicId.
	 * @param {NamespaceIdDescriptor} namespaceId namespace id
	 * @param {Uint8Array|string|undefined} name name
	 */
	constructor(namespaceId, name = undefined) {
		this.rawDescriptor = {
			namespaceId: namespaceId.toMap()
		};

		if (name)
			this.rawDescriptor.name = name;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicDescriptor.
 *
 * binary layout for a mosaic
 */
export class MosaicDescriptor {
	/**
	 * Creates a descriptor for Mosaic.
	 * @param {MosaicIdDescriptor} mosaicId mosaic id
	 * @param {models.Amount} amount quantity
	 */
	constructor(mosaicId, amount) {
		this.rawDescriptor = {
			mosaicId: mosaicId.toMap(),
			amount
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SizePrefixedMosaicDescriptor.
 *
 * binary layout for a mosaic with a size prefixed size
 */
export class SizePrefixedMosaicDescriptor {
	/**
	 * Creates a descriptor for SizePrefixedMosaic.
	 * @param {MosaicDescriptor} mosaic mosaic
	 */
	constructor(mosaic) {
		this.rawDescriptor = {
			mosaic: mosaic.toMap()
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicLevyDescriptor.
 *
 * binary layout for a mosaic levy
 */
export class MosaicLevyDescriptor {
	/**
	 * Creates a descriptor for MosaicLevy.
	 * @param {models.MosaicTransferFeeType} transferFeeType mosaic fee type
	 * @param {Address} recipientAddress recipient address
	 * @param {MosaicIdDescriptor} mosaicId levy mosaic id
	 * @param {models.Amount} fee amount of levy mosaic to transfer
	 */
	constructor(transferFeeType, recipientAddress, mosaicId, fee) {
		this.rawDescriptor = {
			transferFeeType,
			recipientAddress,
			mosaicId: mosaicId.toMap(),
			fee
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicPropertyDescriptor.
 *
 * binary layout for a mosaic property supported property names are: divisibility, initialSupply, supplyMutable, transferable
 */
export class MosaicPropertyDescriptor {
	/**
	 * Creates a descriptor for MosaicProperty.
	 * @param {Uint8Array|string|undefined} name property name
	 * @param {Uint8Array|string|undefined} value property value
	 */
	constructor(name = undefined, value = undefined) {
		this.rawDescriptor = {
		};

		if (name)
			this.rawDescriptor.name = name;

		if (value)
			this.rawDescriptor.value = value;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SizePrefixedMosaicPropertyDescriptor.
 *
 * binary layout for a size prefixed mosaic property
 */
export class SizePrefixedMosaicPropertyDescriptor {
	/**
	 * Creates a descriptor for SizePrefixedMosaicProperty.
	 * @param {MosaicPropertyDescriptor} property property value
	 */
	constructor(property) {
		this.rawDescriptor = {
			property: property.toMap()
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicDefinitionDescriptor.
 *
 * binary layout for a mosaic definition
 */
export class MosaicDefinitionDescriptor {
	/**
	 * Creates a descriptor for MosaicDefinition.
	 * @param {PublicKey} ownerPublicKey owner public key
	 * @param {MosaicIdDescriptor} id mosaic id referenced by this definition
	 * @param {Uint8Array|string|undefined} description description
	 * @param {SizePrefixedMosaicPropertyDescriptor[]|undefined} properties properties
	 * @param {MosaicLevyDescriptor|undefined} levy optional levy that is applied to transfers of this mosaic
	 */
	constructor(ownerPublicKey, id, description = undefined, properties = undefined, levy = undefined) {
		this.rawDescriptor = {
			ownerPublicKey,
			id: id.toMap()
		};

		if (description)
			this.rawDescriptor.description = description;

		if (properties)
			this.rawDescriptor.properties = properties.map(descriptor => descriptor.toMap());

		if (levy)
			this.rawDescriptor.levy = levy.toMap();
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicDefinitionTransactionV1Descriptor.
 *
 * binary layout for a mosaic definition transaction (V1, latest)
 */
export class MosaicDefinitionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicDefinitionTransactionV1.
	 * @param {MosaicDefinitionDescriptor} mosaicDefinition mosaic definition
	 * @param {Address} rentalFeeSink mosaic rental fee sink public key
	 * @param {models.Amount} rentalFee mosaic rental fee
	 */
	constructor(mosaicDefinition, rentalFeeSink, rentalFee) {
		this.rawDescriptor = {
			type: 'mosaic_definition_transaction_v1',
			mosaicDefinition: mosaicDefinition.toMap(),
			rentalFeeSink,
			rentalFee
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicSupplyChangeTransactionV1Descriptor.
 *
 * binary layout for a mosaic supply change transaction (V1, latest)
 */
export class MosaicSupplyChangeTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicSupplyChangeTransactionV1.
	 * @param {MosaicIdDescriptor} mosaicId mosaic id
	 * @param {models.MosaicSupplyChangeAction} action supply change action
	 * @param {models.Amount} delta change amount
	 */
	constructor(mosaicId, action, delta) {
		this.rawDescriptor = {
			type: 'mosaic_supply_change_transaction_v1',
			mosaicId: mosaicId.toMap(),
			action,
			delta
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MultisigAccountModificationDescriptor.
 *
 * binary layout for a multisig account modification
 */
export class MultisigAccountModificationDescriptor {
	/**
	 * Creates a descriptor for MultisigAccountModification.
	 * @param {models.MultisigAccountModificationType} modificationType modification type
	 * @param {PublicKey} cosignatoryPublicKey cosignatory public key
	 */
	constructor(modificationType, cosignatoryPublicKey) {
		this.rawDescriptor = {
			modificationType,
			cosignatoryPublicKey
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SizePrefixedMultisigAccountModificationDescriptor.
 *
 * binary layout for a multisig account modification prefixed with size
 */
export class SizePrefixedMultisigAccountModificationDescriptor {
	/**
	 * Creates a descriptor for SizePrefixedMultisigAccountModification.
	 * @param {MultisigAccountModificationDescriptor} modification modification
	 */
	constructor(modification) {
		this.rawDescriptor = {
			modification: modification.toMap()
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MultisigAccountModificationTransactionV1Descriptor.
 *
 * binary layout for a multisig account modification transaction (V1)
 */
export class MultisigAccountModificationTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MultisigAccountModificationTransactionV1.
	 * @param {SizePrefixedMultisigAccountModificationDescriptor[]|undefined} modifications multisig account modifications
	 */
	constructor(modifications = undefined) {
		this.rawDescriptor = {
			type: 'multisig_account_modification_transaction_v1'
		};

		if (modifications)
			this.rawDescriptor.modifications = modifications.map(descriptor => descriptor.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MultisigAccountModificationTransactionV2Descriptor.
 *
 * binary layout for a multisig account modification transaction (V2, latest)
 */
export class MultisigAccountModificationTransactionV2Descriptor {
	/**
	 * Creates a descriptor for MultisigAccountModificationTransactionV2.
	 * @param {number} minApprovalDelta relative change of the minimal number of cosignatories required when approving a transaction
	 * @param {SizePrefixedMultisigAccountModificationDescriptor[]|undefined} modifications multisig account modifications
	 */
	constructor(minApprovalDelta, modifications = undefined) {
		this.rawDescriptor = {
			type: 'multisig_account_modification_transaction_v2',
			minApprovalDelta
		};

		if (modifications)
			this.rawDescriptor.modifications = modifications.map(descriptor => descriptor.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for CosignatureV1BodyDescriptor.
 *
 * shared content between V1 verifiable and non-verifiable cosignature transactions
 */
export class CosignatureV1BodyDescriptor {
	/**
	 * Creates a descriptor for CosignatureV1Body.
	 * @param {Hash256} otherTransactionHash other transaction hash
	 * @param {Address} multisigAccountAddress multisig account address
	 */
	constructor(otherTransactionHash, multisigAccountAddress) {
		this.rawDescriptor = {
			otherTransactionHash,
			multisigAccountAddress
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for CosignatureV1Descriptor.
 *
 * binary layout for a cosignature transaction (V1, latest)
 */
export class CosignatureV1Descriptor {
	/**
	 * Creates a descriptor for CosignatureV1.
	 * @param {Hash256} otherTransactionHash other transaction hash
	 * @param {Address} multisigAccountAddress multisig account address
	 */
	constructor(otherTransactionHash, multisigAccountAddress) {
		this.rawDescriptor = {
			type: 'cosignature_v1',
			otherTransactionHash,
			multisigAccountAddress
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SizePrefixedCosignatureV1Descriptor.
 *
 * cosignature attached to a multisig transaction with prefixed size
 */
export class SizePrefixedCosignatureV1Descriptor {
	/**
	 * Creates a descriptor for SizePrefixedCosignatureV1.
	 * @param {CosignatureV1Descriptor} cosignature cosignature
	 */
	constructor(cosignature) {
		this.rawDescriptor = {
			cosignature: cosignature.toMap()
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MultisigTransactionV1Descriptor.
 *
 * binary layout for a multisig transaction (V1, latest)
 */
export class MultisigTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MultisigTransactionV1.
	 * @param {models.NonVerifiableTransaction} innerTransaction inner transaction
	 * @param {SizePrefixedCosignatureV1Descriptor[]|undefined} cosignatures cosignatures
	 */
	constructor(innerTransaction, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'multisig_transaction_v1',
			innerTransaction
		};

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures.map(descriptor => descriptor.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for NamespaceRegistrationTransactionV1Descriptor.
 *
 * binary layout for a namespace registration transaction (V1, latest)
 */
export class NamespaceRegistrationTransactionV1Descriptor {
	/**
	 * Creates a descriptor for NamespaceRegistrationTransactionV1.
	 * @param {Address} rentalFeeSink mosaic rental fee sink public key
	 * @param {models.Amount} rentalFee mosaic rental fee
	 * @param {Uint8Array|string|undefined} name new namespace name
	 * @param {Uint8Array|string|undefined} parentName parent namespace name
	 */
	constructor(rentalFeeSink, rentalFee, name = undefined, parentName = undefined) {
		this.rawDescriptor = {
			type: 'namespace_registration_transaction_v1',
			rentalFeeSink,
			rentalFee
		};

		if (name)
			this.rawDescriptor.name = name;

		if (parentName)
			this.rawDescriptor.parentName = parentName;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MessageDescriptor.
 *
 * binary layout for a message
 */
export class MessageDescriptor {
	/**
	 * Creates a descriptor for Message.
	 * @param {models.MessageType} messageType message type
	 * @param {Uint8Array|string|undefined} message message payload
	 */
	constructor(messageType, message = undefined) {
		this.rawDescriptor = {
			messageType
		};

		if (message)
			this.rawDescriptor.message = message;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for TransferTransactionV1Descriptor.
 *
 * binary layout for a transfer transaction (V1)
 */
export class TransferTransactionV1Descriptor {
	/**
	 * Creates a descriptor for TransferTransactionV1.
	 * @param {Address} recipientAddress recipient address
	 * @param {models.Amount} amount XEM amount
	 * @param {MessageDescriptor|undefined} message optional message
	 */
	constructor(recipientAddress, amount, message = undefined) {
		this.rawDescriptor = {
			type: 'transfer_transaction_v1',
			recipientAddress,
			amount
		};

		if (message)
			this.rawDescriptor.message = message.toMap();
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for TransferTransactionV2Descriptor.
 *
 * binary layout for a transfer transaction (V2, latest)
 */
export class TransferTransactionV2Descriptor {
	/**
	 * Creates a descriptor for TransferTransactionV2.
	 * @param {Address} recipientAddress recipient address
	 * @param {models.Amount} amount XEM amount
	 * @param {MessageDescriptor|undefined} message optional message
	 * @param {SizePrefixedMosaicDescriptor[]|undefined} mosaics attached mosaics notice that mosaic amount is multipled by transfer amount to get effective amount
	 */
	constructor(recipientAddress, amount, message = undefined, mosaics = undefined) {
		this.rawDescriptor = {
			type: 'transfer_transaction_v2',
			recipientAddress,
			amount
		};

		if (message)
			this.rawDescriptor.message = message.toMap();

		if (mosaics)
			this.rawDescriptor.mosaics = mosaics.map(descriptor => descriptor.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}
