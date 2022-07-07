const { PrivateKey } = require('./CryptoTypes');
const Mnemonic = require('bitcore-mnemonic');
const crypto = require('crypto');

/**
 * Representation of a BIP32 node.
 */
class Bip32Node {
	/**
	 * Creates a BIP32 node around a key and data.
	 * @param {string} hmacKey BIP32 HMAC key.
	 * @param {Uint8Array} data BIP32 seed.
	 */
	constructor(hmacKey, data) {
		const hmac = crypto.createHmac('sha512', hmacKey);
		hmac.update(data);
		const hmacResult = hmac.digest();

		this.privateKey = new PrivateKey(hmacResult.subarray(0, PrivateKey.SIZE));
		this.chainCode = hmacResult.subarray(PrivateKey.SIZE);
	}

	/**
	 * Derives a direct child node with specified identifier.
	 * @param {number} identifier Child identifier.
	 * @returns {Bip32Node} BIP32 child node.
	 */
	deriveOne(identifier) {
		const childData = new Uint8Array(1 + PrivateKey.SIZE + 4);
		childData[0] = 0;
		childData[childData.length - 4] = 0x80;

		for (let i = 0; 4 > i; ++i)
			childData[childData.length - 1 - i] |= (identifier >> (8 * i)) & 0xFF;

		for (let i = 0; i < PrivateKey.SIZE; ++i)
			childData[1 + i] = this.privateKey.bytes[i];

		return new Bip32Node(this.chainCode, childData);
	}

	/**
	 * Derives a descendent node with specified path.
	 * @param {array<number>} path BIP32 path.
	 * @returns {Bip32Node} BIP32 node at the end of the path.
	 */
	derivePath(path) {
		let nextNode = this;
		path.forEach(identifier => {
			nextNode = nextNode.deriveOne(identifier);
		});

		return nextNode;
	}
}

/**
 * Factory of BIP32 root nodes.
 */
class Bip32 {
	/**
	 * Creates a BIP32 root node factory.
	 * @param {string} curveName Elliptic curve to use.
	 * @param {string} mnemonicLanguage Language of constructed mnemonics.
	 */
	constructor(curveName = 'ed25519', mnemonicLanguage = 'english') {
		this.rootHmacKey = Buffer.from(`${curveName} seed`);
		this.mnemonicLanguage = mnemonicLanguage;
	}

	/**
	 * Creates a BIP32 root node from a seed.
	 * @param {Uint8Array} seed BIP32 seed.
	 * @returns {Bip32Node} BIP32 root node.
	 */
	fromSeed(seed) {
		return new Bip32Node(this.rootHmacKey, seed);
	}

	/**
	 * Creates a BIP32 root node from a BIP39 mnemonic and password.
	 * @param {string} mnemonic BIP32 mnemonic.
	 * @param {string} password BIP32 mnemonic password.
	 * @returns {Bip32Node} BIP32 root node.
	 */
	fromMnemonic(mnemonic, password) {
		const wordlist = Mnemonic.Words[this.mnemonicLanguage.toUpperCase()];
		return this.fromSeed(new Mnemonic(mnemonic, wordlist).toSeed(password));
	}

	/**
	 * Creates a random BIP32 mnemonic.
	 * @param {number} seedLength Length of random seed to use when generating mnemonic.
	 * @returns {string} Random mnemonic created with the specified entropy.
	 */
	random(seedLength = 32) {
		const wordlist = Mnemonic.Words[this.mnemonicLanguage.toUpperCase()];
		return new Mnemonic(seedLength * 8, wordlist).phrase;
	}
}

module.exports = { Bip32 };
