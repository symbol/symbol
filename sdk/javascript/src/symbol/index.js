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
import { metadataGenerateKey, metadataUpdateValue } from './metadata.js';
import * as models from './models.js';
import * as descriptors from './models_ts.js';
import { SymbolAccount, SymbolFacade, SymbolPublicAccount } from '../facade/SymbolFacade.js';

export {
	// region facade

	/**
	 * Facade used to interact with Symbol blockchain.
	 * @type {typeof SymbolFacade}
	 */
	SymbolFacade,

	/**
	 * Symbol public account.
	 * @type {typeof SymbolPublicAccount}
	 */
	SymbolPublicAccount,

	/**
	 * Symbol account.
	 * @type {typeof SymbolAccount}
	 */
	SymbolAccount,

	// endregion

	// region common

	/**
	 * Factory for creating Symbol transactions.
	 * @type {typeof SymbolTransactionFactory}
	 */
	SymbolTransactionFactory,

	/**
	 * Represents a Symbol network timestamp with second resolution.
	 * @type {typeof NetworkTimestamp}
	 */
	NetworkTimestamp,

	/**
	 * Represents a Symbol address.
	 * @type {typeof Address}
	 */
	Address,

	/**
	 * Represents a Symbol network.
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

	metadataGenerateKey,
	metadataUpdateValue,

	/**
	 * Generates symbol voting keys.
	 * @type {typeof VotingKeysGenerator}
	 */
	VotingKeysGenerator,

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
