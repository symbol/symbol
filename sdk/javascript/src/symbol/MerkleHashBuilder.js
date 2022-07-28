const { Hash256 } = require('../CryptoTypes');
const { deepCompare } = require('../utils/arrayHelpers');
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

/**
 * Proves a merkle hash.
 * @param {Hash256} leafHash Leaf hash to prove.
 * @param {array<object>} merklePath Merkle hash chain from leaf to root. Each part has shape {hash: Hash256, isLeft: boolean}.
 * @param {Hash256} rootHash Root hash of the merkle tree.
 * @returns {boolean} true if leaf hash is connected to root hash; false otherwise.
 */
const proveMerkle = (leafHash, merklePath, rootHash) => {
	const computedRootHash = merklePath.reduce((workingHash, merklePart) => {
		const hasher = sha3_256.create();
		if (merklePart.isLeft) {
			hasher.update(merklePart.hash.bytes);
			hasher.update(workingHash.bytes);
		} else {
			hasher.update(workingHash.bytes);
			hasher.update(merklePart.hash.bytes);
		}

		return new Hash256(hasher.digest());
	}, leafHash);

	return 0 === deepCompare(rootHash.bytes, computedRootHash.bytes);
};

module.exports = { MerkleHashBuilder, proveMerkle };
