import { sha3_256 } from '@noble/hashes/sha3.js';
import { utf8ToBytes } from '@noble/hashes/utils.js';

/**
 * Generates a mosaic restriction key from a string.
 * @param {string} seed Mosaic restriction key seed.
 * @returns {bigint} Mosaic restriction key.
 */
const mosaicRestrictionGenerateKey = seed => {
	const hashResult = sha3_256(utf8ToBytes(seed));

	const keyBytes = hashResult.subarray(0, 8);

	const keys = new BigUint64Array(keyBytes.buffer);
	return keys[0];
};

export { mosaicRestrictionGenerateKey }; // eslint-disable-line import/prefer-default-export
