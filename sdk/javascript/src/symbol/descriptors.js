/* eslint-disable no-unused-vars */
import { Address } from './Network.js';
import { Amount, Timestamp, UnresolvedMosaicId } from './models.js';
import { PublicKey } from '../CryptoTypes.js';
/* eslint-enable no-unused-vars */

/**
 * Type safe descriptor for building unresolved mosaic.
 */
export class UnresolvedMosaicDescriptor {
	/**
	 * Creates an unresolved mosaic descriptor.
	 * @param {UnresolvedMosaicId} mosaicId Mosaic id.
	 * @param {Amount} amount Mosaic amount.
	 */
	constructor(mosaicId, amount) {
		this.rawDescriptor = { mosaicId, amount };
	}

	/**
	 * Builds a representation of this descriptor that can be passed to TransactionFactory::create.
	 * @returns {object} Unresolved mosaic descriptor.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor for building transfer transaction v1.
 */
export class TransferTransactionV1Descriptor {
	/**
	 * Creates a transfer transaction descriptor.
	 * @param {PublicKey} signerPublicKey Public key of the signer of the entity.
	 * @param {Amount} fee Transaction fee.
	 * @param {Timestamp} deadline Transaction deadline.
	 * @param {Address} recipientAddress Recipient address.
	 * @param {Uint8Array|string|undefined} message Attached message.
	 * @param {UnresolvedMosaicDescriptor[]|undefined} mosaics Attached mosaics.
	 */
	constructor(signerPublicKey, fee, deadline, recipientAddress, message = undefined, mosaics = undefined) {
		this.rawDescriptor = {
			type: 'transfer_transaction_v1',
			signerPublicKey,
			fee,
			deadline,
			recipientAddress
		};

		if (message)
			this.rawDescriptor.message = message;

		if (mosaics)
			this.rawDescriptor.mosaics = mosaics.map(mosaic => mosaic.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to TransactionFactory::create.
	 * @returns {object} Transaction descriptor.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor for building unresolved mosaic.
 */
export class UnresolvedMosaicDescriptor2 {
	/**
	 * Creates an unresolved mosaic descriptor.
	 * @param {bigint} mosaicId Mosaic id.
	 * @param {bigint} amount Mosaic amount.
	 */
	constructor(mosaicId, amount) {
		this.rawDescriptor = { mosaicId, amount };
	}

	/**
	 * Builds a representation of this descriptor that can be passed to TransactionFactory::create.
	 * @returns {object} Unresolved mosaic descriptor.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor for building transfer transaction v1.
 */
export class TransferTransactionV1Descriptor2 {
	/**
	 * Creates a transfer transaction descriptor.
	 * @param {PublicKey} signerPublicKey Public key of the signer of the entity.
	 * @param {bigint} fee Transaction fee.
	 * @param {bigint} deadline Transaction deadline.
	 * @param {Address} recipientAddress Recipient address.
	 * @param {Uint8Array|string|undefined} message Attached message.
	 * @param {UnresolvedMosaicDescriptor2[]|undefined} mosaics Attached mosaics.
	 */
	constructor(signerPublicKey, fee, deadline, recipientAddress, message = undefined, mosaics = undefined) {
		this.rawDescriptor = {
			type: 'transfer_transaction_v1',
			signerPublicKey,
			fee,
			deadline,
			recipientAddress
		};

		if (message)
			this.rawDescriptor.message = message;

		if (mosaics)
			this.rawDescriptor.mosaics = mosaics.map(mosaic => mosaic.toMap());
	}

	/**
	 * Builds a representation of this descriptor that can be passed to TransactionFactory::create.
	 * @returns {object} Transaction descriptor.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}
