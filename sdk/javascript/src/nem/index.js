import { KeyPair, Verifier } from './KeyPair.js';
import MessageEncoder from './MessageEncoder.js';
import { Address, Network, NetworkTimestamp } from './Network.js';
import TransactionFactory from './TransactionFactory.js';
import * as models from './models.js';
import * as descriptors from './models_ts.js';
import { NemAccount, NemFacade, NemPublicAccount } from '../facade/NemFacade.js';

export {
	// region facade

	/**
	 * Facade used to interact with NEM blockchain.
	 * @type {typeof NemFacade}
	 */
	NemFacade,

	/**
	 * NEM public account.
	 * @type {typeof NemPublicAccount}
	 */
	NemPublicAccount,

	/**
	 * NEM account.
	 * @type {typeof NemAccount}
	 */
	NemAccount,

	// endregion

	// region common

	/**
	 * Factory for creating NEM transactions.
	 * @type {typeof TransactionFactory}
	 */
	TransactionFactory,

	/**
	 * Represents a NEM network timestamp with second resolution.
	 * @type {typeof NetworkTimestamp}
	 */
	NetworkTimestamp,

	/**
	 * Represents a NEM address.
	 * @type {typeof Address}
	 */
	Address,

	/**
	 * Represents a NEM network.
	 * @type {typeof Network}
	 */
	Network,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof MessageEncoder}
	 */
	MessageEncoder,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof KeyPair}
	 */
	KeyPair,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof Verifier}
	 */
	Verifier,

	// region NEM extensions

	/**
	 * Raw models generated from catbuffer schemas.
	 */
	models,

	/**
	 * Descriptors generated from catbuffer schemas for improved TypeScript support.
	 */
	descriptors

	// endregion
};
