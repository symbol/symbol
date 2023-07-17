/* eslint-disable no-unused-vars */
import { Bip32Node } from '../Bip32.js';
/* eslint-enable no-unused-vars */
import {
	Hash256,
	PrivateKey,
	/* eslint-disable no-unused-vars */
	PublicKey,
	SharedKey256,
	Signature
	/* eslint-enable no-unused-vars */
} from '../CryptoTypes.js';
import { NetworkLocator } from '../Network.js';
import { KeyPair, Verifier } from '../nem/KeyPair.js';
import { Address, Network as NemNetwork } from '../nem/Network.js';
import { deriveSharedKey } from '../nem/SharedKey.js';
import TransactionFactory from '../nem/TransactionFactory.js';
/* eslint-disable no-unused-vars */
import * as nc from '../nem/models.js';
/* eslint-enable no-unused-vars */
import { keccak_256 } from '@noble/hashes/sha3';

/**
 * Facade used to interact with NEM blockchain.
 */
export default class NemFacade {
	/**
	 * BIP32 curve name.
	 * @type {string}
	 */
	static BIP32_CURVE_NAME = 'ed25519-keccak';

	/**
	 * Network address class type.
	 * @type {typeof Address}
	 */
	static Address = Address;

	/**
	 * Network key pair class type.
	 * @type {typeof KeyPair}
	 */
	static KeyPair = KeyPair;

	/**
	 * Network verifier class type.
	 * @type {typeof Verifier}
	 */
	static Verifier = Verifier;

	/**
	 * Derives shared key from key pair and other party's public key.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {PublicKey} otherPublicKey Other party's public key.
	 * @returns {SharedKey256} Shared encryption key.
	 */
	static deriveSharedKey = deriveSharedKey;

	/**
	 * Creates a NEM facade.
	 * @param {string|NemNetwork} network NEM network or network name.
	 */
	constructor(network) {
		/**
		 * Underlying network.
		 * @type NemNetwork
		 */
		this.network = 'string' === typeof network ? NetworkLocator.findByName(NemNetwork.NETWORKS, network) : network;

		/**
		 * Underlying transaction factory.
		 * @type TransactionFactory
		 */
		this.transactionFactory = new TransactionFactory(this.network);
	}

	// the following three functions are NOT static in order for NemFacade and SymbolFacade to conform to the same interface

	/**
	 * Hashes a NEM transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @returns {Hash256} Transaction hash.
	 */
	hashTransaction(transaction) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return new Hash256(keccak_256(nonVerifiableTransaction.serialize()));
	}

	/**
	 * Signs a NEM transaction.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(keyPair, transaction) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return keyPair.sign(nonVerifiableTransaction.serialize());
	}

	/**
	 * Verifies a NEM transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} \c true if transaction signature is verified.
	 */
	verifyTransaction(transaction, signature) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return new Verifier(transaction.signerPublicKey).verify(nonVerifiableTransaction.serialize(), signature);
	}

	/**
	 * Creates a network compatible BIP32 path for the specified account.
	 *
	 * @param {number} accountId Id of the account for which to generate a BIP32 path.
	 * @returns {Array<number>} BIP32 path for the specified account.
	 */
	bip32Path(accountId) {
		return [44, 'mainnet' === this.network.name ? 43 : 1, accountId, 0, 0];
	}

	/**
	 * Derives a NEM KeyPair from a BIP32 node.
	 * @param {Bip32Node} bip32Node BIP32 node.
	 * @returns {KeyPair} Derived key pair.
	 */
	static bip32NodeToKeyPair(bip32Node) {
		// BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
		const reversedPrivateKeyBytes = new Uint8Array([...bip32Node.privateKey.bytes]);
		reversedPrivateKeyBytes.reverse();

		return new KeyPair(new PrivateKey(reversedPrivateKeyBytes));
	}
}
