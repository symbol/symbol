import { keccak_256 } from '@noble/hashes/sha3';
import Ripemd160 from 'ripemd160';

/**
 * Hashes payload with keccak 256 and then hashes the result with ripemd160.
 * @param {Uint8Array} payload Input buffer to hash.
 * @returns {Uint8Array} Hash result.
 */
const ripemdKeccak256 = payload => {
	const partOneHash = keccak_256.create().update(payload).digest();
	return new Uint8Array(new Ripemd160().update(Buffer.from(partOneHash)).digest());
};

export { ripemdKeccak256 }; // eslint-disable-line import/prefer-default-export
