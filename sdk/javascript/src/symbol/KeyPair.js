const { PrivateKey, PublicKey, Signature } = require('../CryptoTypes');
const tweetnacl = require('tweetnacl');

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
		this._keyPair = tweetnacl.sign.keyPair.fromSeed(this._privateKey.bytes);
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
		return new Signature(tweetnacl.sign.detached(message, this._keyPair.secretKey));
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
		this.publicKey = publicKey;
	}

	/**
	 * Verifies a message signature.
	 * @param {Uint8Array} message Message to verify.
	 * @param {Signature} signature Signature to verify.
	 * @returns {boolean} true if the message signature verifies.
	 */
	verify(message, signature) {
		return tweetnacl.sign.detached.verify(message, signature.bytes, this.publicKey.bytes);
	}
}

module.exports = { KeyPair, Verifier };
