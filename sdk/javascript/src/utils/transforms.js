const { keccak_256 } = require('@noble/hashes/sha3');
const Ripemd160 = require('ripemd160');

/**
 * Hashes payload with keccak 256 and then hashes the result with ripemd160.
 * @param {Uint8Array} payload Input buffer to hash.
 * @returns {Uint8Array} Hash result.
 */
const ripemdKeccak256 = payload => {
	const partOneHash = keccak_256.create().update(payload).digest();
	return new Ripemd160().update(Buffer.from(partOneHash)).digest();
};

module.exports = {
	ripemdKeccak256
};
