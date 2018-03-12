const serialize = require('./serialize');
const sizes = require('./sizes');
const crypto = require('../crypto/keyPair');
const sha3Hasher = require('../crypto/sha3Hasher');

// serialize.toBuffer returns a nodejs buffer that does not copy any data when slicing
const serializeToBuffer = (codec, transaction) => serialize.toBuffer(codec, transaction).slice(sizes.transactionHeader);

const transactionExtensions = {
	/**
	 * Calculates the hash of a transaction.
	 * @param {module:modelBinary/ModelCodec} codec The transaction codec.
	 * @param {object} transaction The transaction to hash.
	 * @returns {Uint8Array} The transaction hash.
	 */
	hash: (codec, transaction) => {
		const transactionHash = new Uint8Array(32);
		const hasher = sha3Hasher.createHasher(32);
		hasher.reset();

		// "R"
		hasher.update(transaction.signature.slice(0, 32));

		// pubkey
		hasher.update(transaction.signer);

		// data
		const transactionBuffer = serializeToBuffer(codec, transaction);
		hasher.update(transactionBuffer);
		hasher.finalize(transactionHash);

		return transactionHash;
	},

	/**
	 * Signs a transaction using a signer's private key.
	 * @param {module:modelBinary/ModelCodec} codec The transaction codec.
	 * @param {object} keyPair The signer's key pair.
	 * @param {object} transaction The transaction to sign.
	 */
	sign: (codec, keyPair, transaction) => {
		const transactionBuffer = serializeToBuffer(codec, transaction);
		transaction.signature = crypto.sign(keyPair, transactionBuffer);
	},

	/**
	 * Verifies the signature of a transaction.
	 * @param {module:modelBinary/ModelCodec} codec The transaction codec.
	 * @param {object} transaction The transaction to verify.
	 * @returns {boolean} true if the transaction signature is valid.
	 */
	verify: (codec, transaction) => {
		const transactionBuffer = serializeToBuffer(codec, transaction);
		return crypto.verify(transaction.signer, transactionBuffer, transaction.signature);
	}
};

module.exports = transactionExtensions;
