import { Hash256 } from '../CryptoTypes.js';
import { deepCompare } from '../utils/arrayHelpers.js';
import { uint8ToHex } from '../utils/converter.js';
import { sha3_256 } from '@noble/hashes/sha3';

// region MerkleHashBuilder

/**
 * Builder for creating a merkle hash.
 */
export class MerkleHashBuilder {
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

				this.hashes[Math.floor(i / 2)] = hasher.digest();
				i += 2;
			}

			numRemainingHashes = Math.floor(numRemainingHashes / 2);
		}

		return new Hash256(this.hashes[0]);
	}
}

// endregion

// region proveMerkle

/**
 * Proves a merkle hash.
 * @param {Hash256} leafHash Leaf hash to prove.
 * @param {array<object>} merklePath Merkle *hash chain* path from leaf to root. Each part has shape {hash: Hash256, isLeft: boolean}.
 * @param {Hash256} rootHash Root hash of the merkle tree.
 * @returns {boolean} true if leaf hash is connected to root hash; false otherwise.
 */
export const proveMerkle = (leafHash, merklePath, rootHash) => {
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

// endregion

// region LeafNode / BranchNode

const getNibbleAt = (path, index) => {
	const byte = path.path[Math.floor(index / 2)];
	return 1 === index % 2 ? byte & 0xF : byte >>> 4;
};

const encodePath = (path, isLeaf) => {
	let i = 0;
	const buffer = new Uint8Array(1 + Math.floor(path.size / 2));
	buffer[0] = isLeaf ? 0x20 : 0;
	if (1 === path.size % 2) {
		buffer[0] |= 0x10 | getNibbleAt(path, 0);
		++i;
	}

	while (i < path.size) {
		buffer[1 + Math.floor(i / 2)] = (getNibbleAt(path, i) << 4) + (getNibbleAt(path, i + 1));
		i += 2;
	}

	return buffer;
};

/**
 *  Node in a compact patricia tree.
 */
class TreeNode {
	/**
	 * Creates a tree node.
	 * @param {PatriciaTreePath} path Node path.
	 */
	constructor(path) {
		this.path = path;
	}

	/**
	 * Gets hex representation of path.
	 * @returns {str} Hex representation of path.
	 */
	get hexPath() {
		return uint8ToHex(this.path.path).substring(0, this.path.size);
	}
}

/**
 *  Leaf node in a compact patricia tree.
 */
class LeafNode extends TreeNode {
	/**
	 * Creates a leaf node.
	 * @param {PatriciaTreePath} path Leaf path.
	 * @param {Hash256} value Leaf value.
	 */
	constructor(path, value) {
		super(path);
		this.value = value;
	}

	/**
	 * Calculates node hash.
	 * @returns {Hash256} Hash of the node.
	 */
	calculateHash() {
		const hasher = sha3_256.create();
		hasher.update(encodePath(this.path, true));
		hasher.update(this.value.bytes);
		return new Hash256(hasher.digest());
	}
}

/**
 *  Branch node in a compact patricia tree.
 */
class BranchNode extends TreeNode {
	/**
	 * Creates a branch node.
	 * @param {PatriciaTreePath} path Branch path.
	 * @param {array<Hash256>} links Branch links.
	 */
	constructor(path, links) {
		super(path);
		this.links = links;
	}

	/**
	 * Calculates node hash.
	 * @returns {Hash256} Hash of the node.
	 */
	calculateHash() {
		const hasher = sha3_256.create();
		hasher.update(encodePath(this.path, false));
		this.links.forEach(link => {
			hasher.update((undefined === link ? Hash256.zero() : link).bytes);
		});

		return new Hash256(hasher.digest());
	}
}

// endregion

// region deserializePatriciaTreeNodes

class BufferReader {
	constructor(buffer) {
		this.view = new DataView(buffer);
		this.offset = 0;
	}

	get eof() {
		return this.offset === this.view.byteLength;
	}

	readByte() {
		const result = this.view.getUint8(this.offset);
		++this.offset;
		return result;
	}

	readShort() {
		const result = this.view.getUint16(this.offset, true);
		this.offset += 2;
		return result;
	}

	readBytes(count) {
		const result = new Uint8Array(this.view.buffer, this.offset, count);
		this.offset += count;
		return result;
	}
}

const deserializePath = reader => {
	const numNibbles = reader.readByte();
	const numBytes = Math.floor((numNibbles + 1) / 2);
	return { path: reader.readBytes(numBytes), size: numNibbles };
};

const deserializeLeaf = reader => {
	const path = deserializePath(reader);
	const value = new Hash256(reader.readBytes(Hash256.SIZE));
	return new LeafNode(path, value);
};

const deserializeBranch = reader => {
	const path = deserializePath(reader);

	const linksMask = reader.readShort();
	const links = new Array(16);
	for (let i = 0; i < links.length; ++i)
		links[i] = linksMask & (2 ** i) ? new Hash256(reader.readBytes(Hash256.SIZE)) : undefined;

	return new BranchNode(path, links);
};

export const deserializePatriciaTreeNodes = buffer => {
	const reader = new BufferReader(buffer.buffer);
	const nodes = [];
	while (!reader.eof) {
		const nodeMarker = reader.readByte();

		switch (nodeMarker) {
		case 0xFF:
			nodes.push(deserializeLeaf(reader));
			break;

		case 0x00:
			nodes.push(deserializeBranch(reader));
			break;

		default:
			throw new Error(`invalid marker of a serialized node (${nodeMarker})`);
		}
	}

	return nodes;
};

// endregion

// region provePatriciaMerkle

/**
 * Possible results of a patricia merkle proof.
 */
export class PatriciaMerkleProofResult {
	/// Proof is valid (positive).
	static VALID_POSITIVE = 0x0001;

	/// Proof is valid (negative).
	static VALID_NEGATIVE = 0x0002;

	/// Negative proof is inconclusive.
	static INCONCLUSIVE = 0x4001;

	/// State hash cannot be derived from subcache merkle roots.
	static STATE_HASH_DOES_NOT_MATCH_ROOTS = 0x8001;

	/// Root of the path tree being proven is not a subcache merkle root.
	static UNANCHORED_PATH_TREE = 0x8002;

	/// Leaf value does not match expected value.
	static LEAF_VALUE_MISMATCH = 0x8003;

	/// Provided merkle hash contains an unlinked node.
	static UNLINKED_NODE = 0x8004;

	/// Actual merkle path does not match encoded key.
	static PATH_MISMATCH = 0x8005;
}

const checkStateHash = (stateHash, subcacheMerkleRoots) => {
	const hasher = sha3_256.create();
	subcacheMerkleRoots.forEach(root => {
		hasher.update(root.bytes);
	});

	return 0 === deepCompare(stateHash.bytes, hasher.digest());
};

const findLinkIndex = (branchNode, targetLinkHash) => (
	branchNode.links.findIndex(link => undefined !== link && 0 === deepCompare(targetLinkHash.bytes, link.bytes))
);

/**
 * Proves a patricia merkle hash.
 * @param {Hash256} encodedKey Encoded key of the state to prove.
 * @param {Hash256} valueToTest Expected hash of the state to prove.
 * @param {array<BranchNode|LeafNode>} merklePath Merkle *node* path from root to leaf. Each element is BranchNode or LeafNode.
 * @param {Hash256} stateHash State hash from a block header.
 * @param {Hash256} subcacheMerkleRoots Sub cache merkle roots corresponding to the state hash.
 * @returns {numeric} Proof result code.
 */
export const provePatriciaMerkle = (encodedKey, valueToTest, merklePath, stateHash, subcacheMerkleRoots) => {
	if (!checkStateHash(stateHash, subcacheMerkleRoots))
		return PatriciaMerkleProofResult.STATE_HASH_DOES_NOT_MATCH_ROOTS;

	const pathRootHash = merklePath[0].calculateHash();
	if (subcacheMerkleRoots.every(root => 0 !== deepCompare(pathRootHash.bytes, root.bytes)))
		return PatriciaMerkleProofResult.UNANCHORED_PATH_TREE;

	// positive proof must end with a leaf
	const isPositiveProof = undefined !== merklePath[merklePath.length - 1].value;
	if (isPositiveProof) {
		if (0 !== deepCompare(valueToTest.bytes, merklePath[merklePath.length - 1].value.bytes))
			return PatriciaMerkleProofResult.LEAF_VALUE_MISMATCH;
	}

	let childHash;
	let actualPath = '';
	for (let i = merklePath.length - 1; 0 <= i; --i) {
		const node = merklePath[i];
		const nodeHash = node.calculateHash();
		let formattedLinkIndex = '';
		if (childHash) {
			const linkIndex = findLinkIndex(node, childHash);
			if (-1 === linkIndex)
				return PatriciaMerkleProofResult.UNLINKED_NODE;

			formattedLinkIndex = uint8ToHex(new Uint8Array([linkIndex]))[1];
		}

		childHash = nodeHash;
		actualPath = `${formattedLinkIndex}${node.hexPath}${actualPath}`;
	}

	if (isPositiveProof) {
		// for positive proof, expected and calculated paths must match exactly
		return actualPath !== encodedKey.toString() ? PatriciaMerkleProofResult.PATH_MISMATCH : PatriciaMerkleProofResult.VALID_POSITIVE;
	}

	// for negative proof, expected path must start with calculated path and next nibble must be a dead end
	if (!encodedKey.toString().startsWith(actualPath))
		return PatriciaMerkleProofResult.PATH_MISMATCH;

	const nextNibble = getNibbleAt({ path: encodedKey.bytes, size: 2 * encodedKey.bytes.length }, actualPath.length);
	const nextNode = merklePath[merklePath.length - 1].links[nextNibble];
	return undefined !== nextNode ? PatriciaMerkleProofResult.INCONCLUSIVE : PatriciaMerkleProofResult.VALID_NEGATIVE;
};

// endregion
