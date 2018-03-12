/** @module crypto/sha3Hasher */
const { sha3_256, sha3_512 } = require('js-sha3');
const array = require('../utils/array');
const convert = require('../utils/convert');

const getHasher = (length = 64) => ({ 32: sha3_256, 64: sha3_512 }[length]);

module.exports = {
	/**
	 * Calculates the hash of data.
	 * @param {Uint8Array} dest The computed hash destination.
	 * @param {Uint8Array} data The data to hash.
	 * @param {numeric} length The hash length in bytes.
	 */
	func: (dest, data, length) => {
		const hasher = getHasher(length);
		const hash = hasher.arrayBuffer(data);
		array.copy(dest, array.uint8View(hash));
	},

	/**
	 * Creates a hasher object.
	 * @param {numeric} length The hash length in bytes.
	 * @returns {object} The hasher.
	 */
	createHasher: length => {
		let hash;
		return {
			reset: () => {
				hash = getHasher(length).create();
			},
			update: data => {
				if (data instanceof Uint8Array)
					hash.update(data);
				else if ('string' === typeof data)
					hash.update(convert.hexToUint8(data));
				else
					throw Error('unsupported data type');
			},
			finalize: result => {
				array.copy(result, array.uint8View(hash.arrayBuffer()));
			}
		};
	}
};
