/* eslint-disable no-unused-vars */
import { Bip32Node } from '../Bip32.js';
/* eslint-enable no-unused-vars */
import {
	Hash256,
	PrivateKey,
	PublicKey,
	/* eslint-disable no-unused-vars */
	SharedKey256,
	/* eslint-enable no-unused-vars */
	Signature
} from '../CryptoTypes.js';
import { NetworkLocator } from '../Network.js';
import { KeyPair, Verifier } from '../symbol/KeyPair.js';
import MessageEncoder from '../symbol/MessageEncoder.js';
import {
	Address,
	Network,
	/* eslint-disable no-unused-vars */
	NetworkTimestamp
	/* eslint-enable no-unused-vars */
} from '../symbol/Network.js';
import { deriveSharedKey } from '../symbol/SharedKey.js';
import TransactionFactory from '../symbol/TransactionFactory.js';
import { MerkleHashBuilder } from '../symbol/merkle.js';
import * as sc from '../symbol/models.js';
import { sha3_256 } from '@noble/hashes/sha3.js';

const TRANSACTION_HEADER_SIZE = [
	4, // size
	4, // reserved1
	Signature.SIZE, // signature
	PublicKey.SIZE, // signer
	4 // reserved2
].reduce((x, y) => x + y);

const PRE_V3_AGGREGATE_HASHED_SIZE = [
	4, // version, network, type
	8, // maxFee
	8, // deadline
	Hash256.SIZE // transactionsHash
].reduce((x, y) => x + y);

const AGGREGATE_HASHED_SIZE = PRE_V3_AGGREGATE_HASHED_SIZE + [
	4 // payloadSize
].reduce((x, y) => x + y);

const isAggregateTransaction = transactionBuffer => {
	const transactionTypeOffset = TRANSACTION_HEADER_SIZE + 2; // skip version and network byte
	const transactionType = (transactionBuffer[transactionTypeOffset + 1] << 8) + transactionBuffer[transactionTypeOffset];
	const aggregateTypes = [sc.TransactionType.AGGREGATE_BONDED.value, sc.TransactionType.AGGREGATE_COMPLETE.value];
	return aggregateTypes.some(aggregateType => aggregateType === transactionType);
};

const transactionDataBuffer = transactionBuffer => {
	const dataBufferStart = TRANSACTION_HEADER_SIZE;
	let dataBufferEnd = transactionBuffer.length;
	if (isAggregateTransaction(transactionBuffer)) {
		const version = transactionBuffer[TRANSACTION_HEADER_SIZE];
		dataBufferEnd = TRANSACTION_HEADER_SIZE + (3 <= version ? AGGREGATE_HASHED_SIZE : PRE_V3_AGGREGATE_HASHED_SIZE);
	}

	return transactionBuffer.subarray(dataBufferStart, dataBufferEnd);
};

// region SymbolPublicAccount / SymbolAccount

/**
 * Symbol public account.
 */
export class SymbolPublicAccount {
	/**
	 * Creates a Symbol public account.
	 * @param {SymbolFacade} facade Symbol facade.
	 * @param {PublicKey} publicKey Account public key.
	 */
	constructor(facade, publicKey) {
		/**
		 * @protected
		 */
		this._facade = facade;

		/**
		 * Account public key.
		 * @type {PublicKey}
		 */
		this.publicKey = publicKey;

		/**
		 * Account address.
		 * @type {Address}
		 */
		this.address = this._facade.network.publicKeyToAddress(this.publicKey);
	}
}

/**
 * Symbol account.
 */
export class SymbolAccount extends SymbolPublicAccount {
	/**
	 * Creates a Symbol account.
	 * @param {SymbolFacade} facade Symbol facade.
	 * @param {KeyPair} keyPair Account key pair.
	 */
	constructor(facade, keyPair) {
		super(facade, keyPair.publicKey);

		/**
		 * Account key pair.
		 * @type {KeyPair}
		 */
		this.keyPair = keyPair;
	}

	/**
	 * Creates a message encoder that can be used for encrypting and encoding messages between two parties.
	 * @returns {MessageEncoder} Message encoder using this account as one party.
	 */
	messageEncoder() {
		return new MessageEncoder(this.keyPair);
	}

	/**
	 * Signs a Symbol transaction.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(transaction) {
		return this._facade.signTransaction(this.keyPair, transaction);
	}

	/**
	 * Cosigns a Symbol transaction.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @param {boolean} detached \c true if resulting cosignature is appropriate for network propagation.
	 *                           \c false if resulting cosignature is appropriate for attaching to an aggregate.
	 * @returns {sc.Cosignature|sc.DetachedCosignature} Signed cosignature.
	 */
	cosignTransaction(transaction, detached = false) {
		return this._facade.cosignTransaction(this.keyPair, transaction, detached);
	}

	/**
	 * Cosigns a Symbol transaction hash.
	 * @param {Hash256} transactionHash Transaction hash.
	 * @param {boolean} detached \c true if resulting cosignature is appropriate for network propagation.
	 *                           \c false if resulting cosignature is appropriate for attaching to an aggregate.
	 * @returns {sc.Cosignature|sc.DetachedCosignature} Signed cosignature.
	 */
	cosignTransactionHash(transactionHash, detached = false) {
		return this._facade.static.cosignTransactionHash(this.keyPair, transactionHash, detached);
	}
}

// endregion

/**
 * Facade used to interact with Symbol blockchain.
 */
export class SymbolFacade {
	/**
	 * BIP32 curve name.
	 * @type {string}
	 */
	static BIP32_CURVE_NAME = 'ed25519';

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
	 * Creates a Symbol facade.
	 * @param {string|Network} network Symbol network or network name.
	 */
	constructor(network) {
		/**
		 * Underlying network.
		 * @type {Network}
		 */
		this.network = 'string' === typeof network ? NetworkLocator.findByName(Network.NETWORKS, network) : network;

		/**
		 * Underlying transaction factory.
		 * @type {TransactionFactory}
		 */
		this.transactionFactory = new TransactionFactory(this.network);
	}

	/**
	 * Gets class type.
	 * @returns {typeof SymbolFacade} Class type.
	 */
	get static() { // eslint-disable-line class-methods-use-this
		return SymbolFacade;
	}

	/**
	 * Creates a network timestamp representing the current time.
	 * @returns {NetworkTimestamp} Network timestamp representing the current time.
	 */
	now() {
		return this.network.fromDatetime(new Date());
	}

	/**
	 * Creates a Symbol public account from a public key.
	 * @param {PublicKey} publicKey Account public key.
	 * @returns {SymbolPublicAccount} Symbol public account.
	 */
	createPublicAccount(publicKey) {
		return new SymbolPublicAccount(this, publicKey);
	}

	/**
	 * Creates a Symbol account from a private key.
	 * @param {PrivateKey} privateKey Account private key.
	 * @returns {SymbolAccount} Symbol account.
	 */
	createAccount(privateKey) {
		return new SymbolAccount(this, new KeyPair(privateKey));
	}

	/**
	 * Creates a transaction from a (typed) transaction descriptor.
	 * @param {object} typedDescriptor Transaction (typed) descriptor.
	 * @param {PublicKey} signerPublicKey Signer public key.
	 * @param {number} feeMultiplier Fee multiplier.
	 * @param {number} deadlineSeconds Approximate seconds from now for deadline.
	 * @param {number} cosignatureCount Number of cosignature spaces to reserve.
	 * @returns {sc.Transaction} Created transaction.
	 */
	createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, feeMultiplier, deadlineSeconds, cosignatureCount = 0) {
		const rawDescriptor = typedDescriptor.toMap();
		const transaction = this.transactionFactory.create({
			...rawDescriptor,

			signerPublicKey,
			deadline: this.now().addSeconds(deadlineSeconds).timestamp
		});

		// if cosignatures are specified in the descriptor, use the max of them and cosignatureCount
		let cosignatureCountAdjustment = cosignatureCount;
		if (rawDescriptor.cosignatures) {
			cosignatureCountAdjustment = rawDescriptor.cosignatures.length > cosignatureCount
				? 0
				: cosignatureCount - rawDescriptor.cosignatures.length;
		}

		const transactionWithCosignaturesSize = transaction.size + (cosignatureCountAdjustment * new sc.Cosignature().size);
		transaction.fee = new sc.Amount(BigInt(transactionWithCosignaturesSize) * BigInt(feeMultiplier));
		return transaction;
	}

	/**
	 * Creates an embedded transaction from a (typed) transaction descriptor.
	 * @param {object} typedDescriptor Transaction (typed) descriptor.
	 * @param {PublicKey} signerPublicKey Signer public key.
	 * @returns {sc.EmbeddedTransaction} Created embedded transaction.
	 */
	createEmbeddedTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey) {
		const transaction = this.transactionFactory.createEmbedded({
			...typedDescriptor.toMap(),

			signerPublicKey
		});
		return transaction;
	}

	/**
	 * Hashes a Symbol transaction.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @returns {Hash256} Transaction hash.
	 */
	hashTransaction(transaction) {
		const hasher = sha3_256.create();
		hasher.update(transaction.signature.bytes);
		hasher.update(transaction.signerPublicKey.bytes);
		hasher.update(this.network.generationHashSeed.bytes);
		hasher.update(transactionDataBuffer(transaction.serialize()));
		return new Hash256(hasher.digest());
	}

	/**
	 * Gets the payload to sign given a Symbol transaction.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @returns {Uint8Array} Verifiable data to sign.
	 */
	extractSigningPayload(transaction) {
		return new Uint8Array([
			...this.network.generationHashSeed.bytes,
			...transactionDataBuffer(transaction.serialize())
		]);
	}

	/**
	 * Signs a Symbol transaction.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(keyPair, transaction) {
		return keyPair.sign(this.extractSigningPayload(transaction));
	}

	/**
	 * Verifies a Symbol transaction.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} \c true if transaction signature is verified.
	 */
	verifyTransaction(transaction, signature) {
		const verifyBuffer = new Uint8Array(this.extractSigningPayload(transaction));
		return new Verifier(transaction.signerPublicKey).verify(verifyBuffer, signature);
	}

	/**
	 * Cosigns a Symbol transaction hash.
	 * @param {KeyPair} keyPair Key pair of the cosignatory.
	 * @param {Hash256} transactionHash Transaction hash.
	 * @param {boolean} detached \c true if resulting cosignature is appropriate for network propagation.
	 *                           \c false if resulting cosignature is appropriate for attaching to an aggregate.
	 * @returns {sc.Cosignature|sc.DetachedCosignature} Signed cosignature.
	 */
	static cosignTransactionHash(keyPair, transactionHash, detached = false) {
		const initializeCosignature = cosignature => {
			cosignature.version = 0n;
			cosignature.signerPublicKey = new sc.PublicKey(keyPair.publicKey.bytes);
			cosignature.signature = new sc.Signature(keyPair.sign(transactionHash.bytes).bytes);
		};

		if (detached) {
			const cosignature = new sc.DetachedCosignature();
			cosignature.parentHash = new sc.Hash256(transactionHash.bytes);
			initializeCosignature(cosignature);
			return cosignature;
		}

		const cosignature = new sc.Cosignature();
		initializeCosignature(cosignature);
		return cosignature;
	}

	/**
	 * Cosigns a Symbol transaction.
	 * @param {KeyPair} keyPair Key pair of the cosignatory.
	 * @param {sc.Transaction} transaction Transaction object.
	 * @param {boolean} detached \c true if resulting cosignature is appropriate for network propagation.
	 *                           \c false if resulting cosignature is appropriate for attaching to an aggregate.
	 * @returns {sc.Cosignature|sc.DetachedCosignature} Signed cosignature.
	 */
	cosignTransaction(keyPair, transaction, detached = false) {
		const transactionHash = this.hashTransaction(transaction);

		return SymbolFacade.cosignTransactionHash(keyPair, transactionHash, detached);
	}

	/**
	 * Hashes embedded transactions of an aggregate transaction.
	 * @param {Array<sc.EmbeddedTransaction>} embeddedTransactions Embedded transactions to hash.
	 * @returns {Hash256} Aggregate transactions hash.
	 */
	static hashEmbeddedTransactions(embeddedTransactions) {
		const hashBuilder = new MerkleHashBuilder();
		embeddedTransactions.forEach(embeddedTransaction => {
			hashBuilder.update(new Hash256(sha3_256(embeddedTransaction.serialize())));
		});

		return hashBuilder.final();
	}

	/**
	 * Creates a network compatible BIP32 path for the specified account.
	 * @param {number} accountId Id of the account for which to generate a BIP32 path.
	 * @returns {Array<number>} BIP32 path for the specified account.
	 */
	bip32Path(accountId) {
		return [44, 'mainnet' === this.network.name ? 4343 : 1, accountId, 0, 0];
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
