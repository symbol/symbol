/** @module crypto/sha3Hasher */
import { sha3_256, sha3_512 } from 'js-sha3';
import array from '../utils/array';
import convert from '../utils/convert';

function getHasher(length = 64) {
	return { 32: sha3_256, 64: sha3_512 }[length];
}

export default {
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
