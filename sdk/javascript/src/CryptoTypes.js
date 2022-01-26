const { ByteArray } = require('./ByteArray');
const crypto = require('crypto');

/**
 *  Represents a 256-bit hash.
 */
class Hash256 extends ByteArray {
	static SIZE = 32;

	/**
	 * Creates a hash from bytes or a hex string.
	 * @param {Uint8Array|string} hash256 Input string or byte array.
	 */
	constructor(hash256) {
		super(Hash256.SIZE, hash256);
	}

	/**
	 * Creates a zeroed hash.
	 * @returns {Hash256} Zeroed hash.
	 */
	static zero() {
		return new Hash256(new Uint8Array(Hash256.SIZE));
	}
}

/**
 *  Represents a private key.
 */
class PrivateKey extends ByteArray {
	static SIZE = 32;

	/**
	 * Creates a private key from bytes or a hex string.
	 * @param {Uint8Array|string} privateKey Input string or byte array.
	 */
	constructor(privateKey) {
		super(PrivateKey.SIZE, privateKey);
	}

	/**
	 * Creates a random private key.
	 * @returns {PrivateKey} Random private key.
	 */
	static random() {
		return new PrivateKey(crypto.randomBytes(PrivateKey.SIZE));
	}
}

/**
 *  Represents a public key.
 */
class PublicKey extends ByteArray {
	static SIZE = 32;

	/**
	 * Creates a public key from bytes or a hex string.
	 * @param {Uint8Array|string} publicKey Input string, byte array or public key.
	 */
	constructor(publicKey) {
		super(PublicKey.SIZE, publicKey instanceof PublicKey ? publicKey.bytes : publicKey);
	}
}

/**
 *  Represents a signature.
 */
class Signature extends ByteArray {
	static SIZE = 64;

	/**
	 * Creates a signature from bytes or a hex string.
	 * @param {Uint8Array|string} signature Input string or byte array.
	 */
	constructor(signature) {
		super(Signature.SIZE, signature);
	}

	/**
	 * Creates a zeroed signature.
	 * @returns {Signature} Zeroed signature.
	 */
	static zero() {
		return new Signature(new Uint8Array(Signature.SIZE));
	}
}

module.exports = {
	Hash256, PrivateKey, PublicKey, Signature
};
