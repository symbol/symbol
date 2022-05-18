const { Hash256 } = require('../CryptoTypes');
const { sha3_256 } = require('@noble/hashes/sha3');

/**
 * Builder for creating a merkle hash.
 */
class MerkleHashBuilder {
	/**
	 * Creates a merkle hash builder.
	 */
	constructor() {
		this.hashes = [];
	}

	/**
	 * Adds a hash to the merkle hash.""
	 * @param {Hash256} componentHash Hash to add.
	 */
	update(componentHash) {
		this.hashes.push(componentHash.bytes);
	}

	/**
	 * Calculates the merkle hash.
	 * @returns {Hash256} Merkle hash.
	 */
	final() {
		if (0 === this.hashes.length)
			return Hash256.zero();

		let numRemainingHashes = this.hashes.length;
		while (1 < numRemainingHashes) {
			let i = 0;
			while (i < numRemainingHashes) {
				const hasher = sha3_256.create();
				hasher.update(this.hashes[i]);

				if (i + 1 < numRemainingHashes) {
					hasher.update(this.hashes[i + 1]);
				} else {
					// if there is an odd number of hashes, duplicate the last one
					hasher.update(this.hashes[i]);
					numRemainingHashes += 1;
				}

				this.hashes[Math.trunc(i / 2)] = hasher.digest();
				i += 2;
			}

			numRemainingHashes = Math.trunc(numRemainingHashes / 2);
		}

		return new Hash256(this.hashes[0]);
	}
}

module.exports = { MerkleHashBuilder };
