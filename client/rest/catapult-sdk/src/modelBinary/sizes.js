/** @module modelBinary/sizes */

module.exports = {
	/**
	 * @property {numeric} The size of a signature.
	 */
	signature: 64,

	/**
	 * @property {numeric} The size of a signer (public key).
	 */
	signer: 32,

	/**
	 * @property {numeric} The size of a decoded address.
	 */
	addressDecoded: 25,

	/**
	 * @property {numeric} The size of a hash.
	 */
	hash: 32,

	/**
	 * @property {numeric} The size of a transaction header.
	 */
	transactionHeader: 4 + 64 + 32
};
