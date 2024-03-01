import { KeyPair, Verifier } from './KeyPair.js';
import MessageEncoder from './MessageEncoder.js';
import { Address, Network, NetworkTimestamp } from './Network.js';
import SymbolTransactionFactory from './TransactionFactory.js';
import VotingKeysGenerator from './VotingKeysGenerator.js';
import {
	generateMosaicAliasId,
	generateMosaicId,
	generateNamespaceId,
	generateNamespacePath,
	isValidNamespaceName
} from './idGenerator.js';
import {
	deserializePatriciaTreeNodes,
	proveMerkle,
	provePatriciaMerkle
} from './merkle.js';
import { metadataUpdateValue } from './metadata.js';
import * as models from './models.js';
import SymbolFacade from '../facade/SymbolFacade.js';

export {
	/**
	 * Facade used to interact with Symbol blockchain.
	 * @type {typeof SymbolFacade}
	 */
	SymbolFacade,

	// region common

	/**
	 * Factory for creating Symbol transactions.
	 * @type {typeof SymbolTransactionFactory}
	 */
	SymbolTransactionFactory,

	/**
	 * Represents a Symbol network timestamp with second resolution.
	 * @type {typeof SymbolNetwork.NetworkTimestamp}
	 */
	NetworkTimestamp,

	/**
	 * Represents a Symbol address.
	 * @type {typeof SymbolNetwork.Address}
	 */
	Address,

	/**
	 * Represents a Symbol network.
	 * @type {typeof SymbolNetwork.Network}
	 */
	Network,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof SymbolMessageEncoder}
	 */
	MessageEncoder,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof SymbolKeyPair.KeyPair}
	 */
	KeyPair,

	/**
	 * Encrypts and encodes messages between two parties.
	 * @type {typeof SymbolKeyPair.Verifier}
	 */
	Verifier,

	// endregion

	// region Symbol extensions

	generateMosaicId,
	generateNamespaceId,
	isValidNamespaceName,
	generateNamespacePath,
	generateMosaicAliasId,

	proveMerkle,
	deserializePatriciaTreeNodes,
	provePatriciaMerkle,

	metadataUpdateValue,

	/**
	 * Generates symbol voting keys.
	 * @type {typeof VotingKeysGenerator}
	 */
	VotingKeysGenerator,

	/**
	 * Raw models generated from catbuffer schemas.
	 */
	models

	// endregion
};
