const { PrivateKey, PublicKey, Signature } = require('../CryptoTypes');
const ed25519 = require('../impl/ed25519');
const { deepCompare } = require('../utils/arrayHelpers');

const HASH_MODE = 'Keccak';

/**
 * Represents an ED25519 private and public key.
 */
class KeyPair {
	/**
	 * Creates a key pair from a private key.
	 * @param {PrivateKey} privateKey Private key.
	 */
	constructor(privateKey) {
		this._privateKey = privateKey;

		const reversedPrivateKeyBytes = new Uint8Array([...privateKey.bytes]);
		reversedPrivateKeyBytes.reverse();
		this._keyPair = ed25519.keyPairFromSeed(HASH_MODE, reversedPrivateKeyBytes);
	}

	/**
	 * Gets the public key.
	 * @returns {PublicKey} Public key.
	 */
	get publicKey() {
		return new PublicKey(this._keyPair.publicKey);
	}

	/**
	 * Gets the private key.
	 * @returns {PrivateKey} Private key.
	 */
	get privateKey() {
		return new PrivateKey(this._privateKey.bytes);
	}

	/**
	 * Signs a message with the private key.
	 * @param {Uint8Array} message Message to sign.
	 * @returns {Signature} Message signature.
	 */
	sign(message) {
		return new Signature(ed25519.sign(HASH_MODE, message, this._keyPair.privateKey));
	}
}

/**
 * Verifies signatures signed by a single key pair.
 */
class Verifier {
	/**
	 * Creates a verifier from a public key.
	 * @param {PublicKey} publicKey Public key.
	 */
	constructor(publicKey) {
		if (0 === deepCompare(new Uint8Array(PublicKey.SIZE), publicKey.bytes))
			throw new Error('public key cannot be zero');

		this.publicKey = publicKey;
	}

	/**
	 * Verifies a message signature.
	 * @param {Uint8Array} message Message to verify.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} true if the message signature verifies.
	 */
	verify(message, signature) {
		return ed25519.verify(HASH_MODE, message, signature.bytes, this.publicKey.bytes);
	}
}

module.exports = { KeyPair, Verifier };
