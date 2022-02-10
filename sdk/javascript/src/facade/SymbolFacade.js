const {
	Hash256, PrivateKey, PublicKey, Signature
} = require('../CryptoTypes');
const { NetworkLocator } = require('../Network');
const { KeyPair, Verifier } = require('../symbol/KeyPair');
const { Address, Network } = require('../symbol/Network');
const { TransactionFactory } = require('../symbol/TransactionFactory');
const { TransactionType } = require('../symbol/models');
const { sha3_256 } = require('js-sha3');

const TRANSACTION_HEADER_SIZE = [
	4, // size
	4, // reserved1
	Signature.SIZE, // signature
	PublicKey.SIZE, // signer
	4 // reserved2
].reduce((x, y) => x + y);

const AGGREGATE_HASHED_SIZE = [
	4, // version, network, type
	8, // maxFee
	8, // deadline
	Hash256.SIZE // transactionsHash
].reduce((x, y) => x + y);

const isAggregateTransaction = transactionBuffer => {
	const transactionTypeOffset = TRANSACTION_HEADER_SIZE + 2; // skip version and network byte
	const transactionType = (transactionBuffer[transactionTypeOffset + 1] << 8) + transactionBuffer[transactionTypeOffset];
	const aggregateTypes = [TransactionType.AGGREGATE_BONDED.value, TransactionType.AGGREGATE_COMPLETE.value];
	return aggregateTypes.some(aggregateType => aggregateType === transactionType);
};

const transactionDataBuffer = transactionBuffer => {
	const dataBufferStart = TRANSACTION_HEADER_SIZE;
	const dataBufferEnd = isAggregateTransaction(transactionBuffer)
		? TRANSACTION_HEADER_SIZE + AGGREGATE_HASHED_SIZE
		: transactionBuffer.length;

	return transactionBuffer.subarray(dataBufferStart, dataBufferEnd);
};

/**
 * Facade used to interact with Symbol blockchain.
 */
class SymbolFacade {
	static BIP32_COIN_ID = 4343;

	static BIP32_CURVE_NAME = 'ed25519';

	static Address = Address;

	static KeyPair = KeyPair;

	static Verifier = Verifier;

	/**
	 * Creates a Symbol facade.
	 * @param {string} symbolNetworkName Symbol network name.
	 */
	constructor(symbolNetworkName) {
		this.network = NetworkLocator.findByName(Network.NETWORKS, symbolNetworkName);
		this.transactionFactory = new TransactionFactory(this.network);
	}

	/**
	 * Hashes a Symbol transaction.
	 * @param {object} transaction Transaction object.
	 * @returns {Hash256} Transaction hash.
	 */
	hashTransaction(transaction) {
		const hasher = sha3_256.create();
		hasher.update(transaction.signature.bytes);
		hasher.update(transaction.signerPublicKey.bytes);
		hasher.update(this.network.generationHashSeed.bytes);
		hasher.update(transactionDataBuffer(transaction.serialize()));
		return new Hash256(new Uint8Array(hasher.arrayBuffer()));
	}

	/**
	 * Signs a Symbol transaction.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {object} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(keyPair, transaction) {
		return keyPair.sign(new Uint8Array([
			...this.network.generationHashSeed.bytes,
			...transactionDataBuffer(transaction.serialize())
		]));
	}

	/**
	 * Verifies a Symbol transaction.
	 * @param {object} transaction Transaction object.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} true if transaction signature is verified.
	 */
	verifyTransaction(transaction, signature) {
		const verifyBuffer = new Uint8Array([
			...this.network.generationHashSeed.bytes,
			...transactionDataBuffer(transaction.serialize())
		]);
		return new Verifier(transaction.signerPublicKey).verify(verifyBuffer, signature);
	}

	/**
	 * Derives a Symbol KeyPair from a BIP32 node.
	 * @param {Bip32Node} bip32Node BIP32 node.
	 * @returns {KeyPair} Derived key pair.
	 */
	static bip32NodeToKeyPair(bip32Node) {
		return new KeyPair(new PrivateKey(bip32Node.privateKey.bytes));
	}
}

module.exports = { SymbolFacade };
