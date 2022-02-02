const { Hash256, PrivateKey } = require('../CryptoTypes');
const { NetworkLocator } = require('../Network');
const { KeyPair, Verifier } = require('../nem/KeyPair');
const { Address, Network } = require('../nem/Network');
const { TransactionFactory } = require('../nem/TransactionFactory');
const { keccak_256 } = require('js-sha3');

/**
 * Facade used to interact with NEM blockchain.
 */
class NemFacade {
	static BIP32_COIN_ID = 43;

	static BIP32_CURVE_NAME = 'ed25519-keccak';

	static Address = Address;

	static KeyPair = KeyPair;

	static Verifier = Verifier;

	/**
	 * Creates a NEM facade.
	 * @param {string} nemNetworkName NEM network name.
	 */
	constructor(nemNetworkName) {
		this.network = NetworkLocator.findByName(Network.NETWORKS, nemNetworkName);
		this.transactionFactory = new TransactionFactory(this.network);
	}

	// the following three functions are NOT static in order for NemFacade and SymbolFacade to conform to the same interface

	/**
	 * Hashes a NEM transaction.
	 * @param {object} transaction Transaction object.
	 * @returns {Hash256} Transaction hash.
	 */
	hashTransaction(transaction) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return new Hash256(new Uint8Array(keccak_256.create().update(nonVerifiableTransaction.serialize()).arrayBuffer()));
	}

	/**
	 * Signs a NEM transaction.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {object} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(keyPair, transaction) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return keyPair.sign(nonVerifiableTransaction.serialize());
	}

	/**
	 * Verifies a NEM transaction.
	 * @param {object} transaction Transaction object.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} true if transaction signature is verified.
	 */
	verifyTransaction(transaction, signature) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return new Verifier(transaction.signerPublicKey).verify(nonVerifiableTransaction.serialize(), signature);
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

module.exports = { NemFacade };
