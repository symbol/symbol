import BaseValue from './BaseValue.js';
import { Bip32 } from './Bip32.js';
import ByteArray from './ByteArray.js';
import * as CryptoTypes from './CryptoTypes.js';
import { NetworkLocator } from './Network.js';
import NemFacade from './facade/NemFacade.js';
import SymbolFacade from './facade/SymbolFacade.js';
import * as NemKeyPair from './nem/KeyPair.js';
import NemMessageEncoder from './nem/MessageEncoder.js';
import * as NemNetwork from './nem/Network.js';
import NemTransactionFactory from './nem/TransactionFactory.js';
import * as NemModels from './nem/models.js';
import * as SymbolKeyPair from './symbol/KeyPair.js';
import SymbolMessageEncoder from './symbol/MessageEncoder.js';
import * as SymbolNetwork from './symbol/Network.js';
import SymbolTransactionFactory from './symbol/TransactionFactory.js';
import VotingKeysGenerator from './symbol/VotingKeysGenerator.js';
import * as SymbolIdGenerator from './symbol/idGenerator.js';
import * as SymbolMerkle from './symbol/merkle.js';
import * as SymbolMetadata from './symbol/metadata.js';
import * as SymbolModels from './symbol/models.js';
import { hexToUint8, uint8ToHex } from './utils/converter.js';

const sdk = {
	/**
	 * Represents a base integer.
	 * @type {typeof BaseValue}
	 */
	BaseValue,

	/**
	 * Factory of BIP32 root nodes.
	 * @type {typeof Bip32}
	 */
	Bip32,

	/**
	 * Represents a fixed size byte array.
	 * @type {typeof ByteArray}
	 */
	ByteArray,

	// region CryptoTypes

	/**
	 * Represents a 256-bit hash.
	 * @type {typeof CryptoTypes.Hash256}
	 */
	Hash256: CryptoTypes.Hash256,

	/**
	 * Represents a private key.
	 * @type {typeof CryptoTypes.PrivateKey}
	 */
	PrivateKey: CryptoTypes.PrivateKey,

	/**
	 * Represents a public key.
	 * @type {typeof CryptoTypes.PublicKey}
	 */
	PublicKey: CryptoTypes.PublicKey,

	/**
	 * Represents a 256-bit symmetric encryption key.
	 * @type {typeof CryptoTypes.SharedKey256}
	 */
	SharedKey256: CryptoTypes.SharedKey256,

	/**
	 * Represents a signature.
	 * @type {typeof CryptoTypes.Signature}
	 */
	Signature: CryptoTypes.Signature,

	// endregion

	/**
	 * Provides utility functions for finding a network.
	 * @type {typeof NetworkLocator}
	 */
	NetworkLocator,

	// region facade

	/**
	 * Network facades.
	 */
	facade: {
		/**
		 * Facade used to interact with NEM blockchain.
		 * @type {typeof NemFacade}
		 */
		NemFacade,

		/**
		 * Facade used to interact with Symbol blockchain.
		 * @type {typeof SymbolFacade}
		 */
		SymbolFacade
	},

	// endregion

	/**
	 * NEM blockchain accessors.
	 */
	nem: {
		...NemModels, // must be before Network to promote Address from Network

		// region common

		/**
		 * Factory for creating NEM transactions.
		 * @type {typeof NemTransactionFactory}
		 */
		NemTransactionFactory,

		/**
		 * Represents a NEM network timestamp with second resolution.
		 * @type {typeof NemNetwork.NetworkTimestamp}
		 */
		NetworkTimestamp: NemNetwork.NetworkTimestamp,

		/**
		 * Represents a NEM address.
		 * @type {typeof NemNetwork.Address}
		 */
		Address: NemNetwork.Address,

		/**
		 * Represents a NEM network.
		 * @type {typeof NemNetwork.Network}
		 */
		Network: NemNetwork.Network,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof NemMessageEncoder}
		 */
		MessageEncoder: NemMessageEncoder,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof NemKeyPair.KeyPair}
		 */
		KeyPair: NemKeyPair.KeyPair,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof NemKeyPair.Verifier}
		 */
		Verifier: NemKeyPair.Verifier

		// endregion
	},

	/**
	 * Symbol blockchain accessors.
	 */
	symbol: {
		...SymbolModels, // must be before Network to promote Address from Network

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
		NetworkTimestamp: SymbolNetwork.NetworkTimestamp,

		/**
		 * Represents a Symbol address.
		 * @type {typeof SymbolNetwork.Address}
		 */
		Address: SymbolNetwork.Address,

		/**
		 * Represents a Symbol network.
		 * @type {typeof SymbolNetwork.Network}
		 */
		Network: SymbolNetwork.Network,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof SymbolMessageEncoder}
		 */
		MessageEncoder: SymbolMessageEncoder,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof SymbolKeyPair.KeyPair}
		 */
		KeyPair: SymbolKeyPair.KeyPair,

		/**
		 * Encrypts and encodes messages between two parties.
		 * @type {typeof SymbolKeyPair.Verifier}
		 */
		Verifier: SymbolKeyPair.Verifier,

		// endregion

		// region Symbol extensions

		...SymbolIdGenerator,
		...SymbolMerkle,
		...SymbolMetadata,

		/**
		 * Generates symbol voting keys.
		 * @type {typeof VotingKeysGenerator}
		 */
		VotingKeysGenerator

		// endregion
	},

	/**
	 * Network independent utilities.
	 */
	utils: {
		hexToUint8,
		uint8ToHex
	}
};

export default sdk;
