import { KeyPair, Verifier } from './KeyPair.js';
import MessageEncoder from './MessageEncoder.js';
import { Address, Network, NetworkTimestamp } from './Network.js';
import TransactionFactory from './TransactionFactory.js';
import * as models from './models.js';
import NemFacade from '../facade/NemFacade.js';

export {
	/**
	 * Facade used to interact with NEM blockchain.
	 * @type {typeof NemFacade}
	 */
	NemFacade,

	// region common

	/**
	 * Factory for creating NEM transactions.
	 * @type {typeof NemTransactionFactory}
	 */
	TransactionFactory,

	/**
	 * Represents a NEM network timestamp with second resolution.
	 * @type {typeof NemNetwork.NetworkTimestamp}
	 */
	NetworkTimestamp,

	/**
	 * Represents a NEM address.
	 * @type {typeof NemNetwork.Address}
	 */
	Address,

	/**
	 * Represents a NEM network.
	 * @type {typeof NemNetwork.Network}
	 */
	Network,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof NemMessageEncoder}
	 */
	MessageEncoder,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof NemKeyPair.KeyPair}
	 */
	KeyPair,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof NemKeyPair.Verifier}
	 */
	Verifier,

	// region NEM extensions

	/**
	 * Raw models generated from catbuffer schemas.
	 */
	models

	// endregion
};
