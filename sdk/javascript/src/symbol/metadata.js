import { sha3_256 } from '@noble/hashes/sha3.js';
import { utf8ToBytes } from '@noble/hashes/utils.js';

/**
 * Generates a metadata key from a string.
 * @param {string} seed Metadata key seed.
 * @returns {bigint} Metadata key.
 */
const metadataGenerateKey = seed => {
	const hashResult = sha3_256(utf8ToBytes(seed));

	const keyBytes = hashResult.subarray(0, 8);
	keyBytes[7] |= 0x80; // set high bit to match SDK V2 implementation

	const keys = new BigUint64Array(keyBytes.buffer);
	return keys[0];
};

/**
 * Creates a metadata payload for updating old value to new value.
 * @param {Uint8Array|undefined} oldValue Old metadata value.
 * @param {Uint8Array} newValue New metadata value.
 * @returns {Uint8Array} Metadata payload for updating old value to new value.
 */
const metadataUpdateValue = (oldValue, newValue) => {
	if (!oldValue)
		return newValue;

	const shorterLength = Math.min(oldValue.length, newValue.length);
	const longerLength = Math.max(oldValue.length, newValue.length);
	const isNewValueShorter = oldValue.length > newValue.length;

	const result = new Uint8Array(longerLength);

	let i = 0;
	for (i = 0; i < shorterLength; ++i)
		result[i] = oldValue[i] ^ newValue[i];

	for (; i < longerLength; ++i)
		result[i] = (isNewValueShorter ? oldValue : newValue)[i];

	return result;
};

export { metadataGenerateKey, metadataUpdateValue };
