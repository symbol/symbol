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
import MessageEncoder from '../nem/MessageEncoder.js';
import {
	Address,
	Network,
	/* eslint-disable no-unused-vars */
	NetworkTimestamp
	/* eslint-enable no-unused-vars */
} from '../nem/Network.js';
import { deriveSharedKey } from '../nem/SharedKey.js';
import TransactionFactory from '../nem/TransactionFactory.js';
/* eslint-disable no-unused-vars */
import * as nc from '../nem/models.js';
/* eslint-enable no-unused-vars */
import { keccak_256 } from '@noble/hashes/sha3.js';

// region NemPublicAccount / NemAccount

/**
 * NEM public account.
 */
export class NemPublicAccount {
	/**
	 * Creates a NEM public account.
	 * @param {NemFacade} facade NEM facade.
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
 * NEM account.
 */
export class NemAccount extends NemPublicAccount {
	/**
	 * Creates a NEM account.
	 * @param {NemFacade} facade NEM facade.
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
	 * Signs a NEM transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(transaction) {
		return this._facade.signTransaction(this.keyPair, transaction);
	}
}

// endregion

/**
 * Facade used to interact with NEM blockchain.
 */
export class NemFacade {
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
	 * @param {string|Network} network NEM network or network name.
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
	 * @returns {typeof NemFacade} Class type.
	 */
	get static() { // eslint-disable-line class-methods-use-this
		return NemFacade;
	}

	/**
	 * Creates a network timestamp representing the current time.
	 * @returns {NetworkTimestamp} Network timestamp representing the current time.
	 */
	now() {
		return this.network.fromDatetime(new Date());
	}

	/**
	 * Creates a NEM public account from a public key.
	 * @param {PublicKey} publicKey Account public key.
	 * @returns {NemPublicAccount} NEM public account.
	 */
	createPublicAccount(publicKey) {
		return new NemPublicAccount(this, publicKey);
	}

	/**
	 * Creates a NEM account from a private key.
	 * @param {PrivateKey} privateKey Account private key.
	 * @returns {NemAccount} NEM account.
	 */
	createAccount(privateKey) {
		return new NemAccount(this, new KeyPair(privateKey));
	}

	/**
	 * Creates a transaction from a (typed) transaction descriptor.
	 * @param {object} typedDescriptor Transaction (typed) descriptor.
	 * @param {PublicKey} signerPublicKey Signer public key.
	 * @param {bigint} fee Transaction fee.
	 * @param {number} deadlineSeconds Approximate seconds from now for deadline.
	 * @returns {nc.Transaction} Created transaction.
	 */
	createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, fee, deadlineSeconds) {
		const now = this.now();
		const transaction = this.transactionFactory.create({
			...typedDescriptor.toMap(),

			signerPublicKey,
			fee,
			timestamp: now.timestamp,
			deadline: now.addSeconds(deadlineSeconds).timestamp
		});
		return transaction;
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
	 * Gets the payload to sign given a NEM transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @returns {Uint8Array} Verifiable data to sign.
	 */
	extractSigningPayload(transaction) { // eslint-disable-line class-methods-use-this
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);
		return nonVerifiableTransaction.serialize();
	}

	/**
	 * Signs a NEM transaction.
	 * @param {KeyPair} keyPair Key pair.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @returns {Signature} Transaction signature.
	 */
	signTransaction(keyPair, transaction) {
		return keyPair.sign(this.extractSigningPayload(transaction));
	}

	/**
	 * Verifies a NEM transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} \c true if transaction signature is verified.
	 */
	verifyTransaction(transaction, signature) {
		return new Verifier(transaction.signerPublicKey).verify(this.extractSigningPayload(transaction), signature);
	}

	/**
	 * Creates a network compatible BIP32 path for the specified account.
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
