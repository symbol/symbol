package org.symbol.sdk.symbol;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Transforms;

/**
 * Merkle tree utilities for Symbol: merkle root building, merkle hash-chain proofs, and compact Patricia tree node types with
 * deserialization and proof checking.
 */
public final class Merkle {
	private Merkle() {
	}

	// region MerkleHashBuilder

	/** Builder for creating a merkle hash. */
	public static final class MerkleHashBuilder {
		private final List<byte[]> hashes = new ArrayList<>();

		/**
		 * Adds a hash to the merkle hash.
		 *
		 * @param componentHash Hash to add.
		 */
		public void update(final CryptoTypes.Hash256 componentHash) {
			hashes.add(componentHash.bytes().clone());
		}

		/**
		 * Calculates the merkle hash.
		 *
		 * @return Merkle hash.
		 */
		public CryptoTypes.Hash256 finalHash() {
			if (hashes.isEmpty())
				return CryptoTypes.Hash256.zero();

			// Work on a copy/reference that shrinks level by level
			List<byte[]> currentLevel = hashes;
			while (currentLevel.size() > 1) {
				final List<byte[]> nextLevel = new ArrayList<>((currentLevel.size() + 1) / 2);
				for (int i = 0; i < currentLevel.size(); i += 2) {
					final byte[] left = currentLevel.get(i);
					// If there's no right sibling, duplicate the left one
					final byte[] right = (i + 1 < currentLevel.size()) ? currentLevel.get(i + 1) : left;
					nextLevel.add(Transforms.sha3_256(left, right));
				}

				currentLevel = nextLevel;
			}

			return new CryptoTypes.Hash256(currentLevel.get(0));
		}
	}

	// endregion

	// region proveMerkle

	/** Represents part of a merkle tree proof. */
	public record MerklePart(CryptoTypes.Hash256 hash, boolean isLeft) {
	}

	/**
	 * Proves a merkle hash.
	 *
	 * @param leafHash Leaf hash to prove.
	 * @param merklePath Merkle <em>hash chain</em> from leaf to root.
	 * @param rootHash Root hash of the merkle tree.
	 * @return {@code true} if leaf hash is connected to root hash.
	 */
	public static boolean proveMerkle(final CryptoTypes.Hash256 leafHash, final List<MerklePart> merklePath,
			final CryptoTypes.Hash256 rootHash) {
		CryptoTypes.Hash256 working = leafHash;
		for (MerklePart part : merklePath) {
			final byte[] left = part.isLeft ? part.hash.bytes() : working.bytes();
			final byte[] right = part.isLeft ? working.bytes() : part.hash.bytes();
			working = new CryptoTypes.Hash256(Transforms.sha3_256(left, right));
		}

		return Arrays.equals(rootHash.bytes(), working.bytes());
	}

	// endregion

	// region TreeNode / LeafNode / BranchNode

	/** Path in a Patricia merkle tree. */
	public record PatriciaTreePath(byte[] path, int size) {
		@Override
		public boolean equals(final Object other) {
			if (this == other)
				return true;

			// the default record equals compares the byte[] by identity; compare by content so structurally equal paths match
			return other instanceof PatriciaTreePath otherPath && size == otherPath.size && Arrays.equals(path, otherPath.path);
		}

		@Override
		public int hashCode() {
			return 31 * size + Arrays.hashCode(path);
		}
	}

	private static int getNibbleAt(final PatriciaTreePath path, final int index) {
		final int b = path.path[index / 2] & 0xFF;
		return 1 == (index % 2) ? b & 0xF : b >>> 4;
	}

	private static byte[] encodePath(final PatriciaTreePath path, final boolean isLeaf) {
		int i = 0;
		final byte[] buffer = new byte[1 + path.size / 2];
		buffer[0] = isLeaf ? (byte) 0x20 : 0;
		if (1 == path.size % 2) {
			buffer[0] = (byte) ((buffer[0] & 0xFF) | 0x10 | getNibbleAt(path, 0));
			++i;
		}

		while (i < path.size) {
			buffer[1 + i / 2] = (byte) ((getNibbleAt(path, i) << 4) + getNibbleAt(path, i + 1));
			i += 2;
		}

		return buffer;
	}

	/** Node in a compact Patricia tree. */
	public abstract static class TreeNode {
		/** Node path. */
		public final PatriciaTreePath path;

		/**
		 * Creates a tree node.
		 *
		 * @param path Node path.
		 */
		protected TreeNode(final PatriciaTreePath path) {
			this.path = path;
		}

		/**
		 * Returns the hex representation of the path, truncated to {@code path.size} nibbles.
		 *
		 * @return Hex representation of path.
		 */
		public String hexPath() {
			return Converter.uint8ToHex(path.path).substring(0, path.size);
		}

		/**
		 * Calculates the hash of this node.
		 *
		 * @return Hash of the node.
		 */
		public abstract CryptoTypes.Hash256 calculateHash();
	}

	/** Leaf node in a compact Patricia tree. */
	public static final class LeafNode extends TreeNode {
		/** Leaf value. */
		public final CryptoTypes.Hash256 value;

		/**
		 * Creates a leaf node.
		 *
		 * @param path Leaf path.
		 * @param value Leaf value.
		 */
		private LeafNode(final PatriciaTreePath path, final CryptoTypes.Hash256 value) {
			super(path);
			this.value = value;
		}

		@Override
		public CryptoTypes.Hash256 calculateHash() {
			return new CryptoTypes.Hash256(Transforms.sha3_256(encodePath(path, true), value.bytes()));
		}
	}

	/** Branch node in a compact Patricia tree. */
	public static final class BranchNode extends TreeNode {
		/** Branch links — a 16-element array, each element either a hash or {@code null}. */
		public final CryptoTypes.Hash256[] links;

		/**
		 * Creates a branch node.
		 *
		 * @param path Branch path.
		 * @param links Branch links.
		 */
		private BranchNode(final PatriciaTreePath path, final CryptoTypes.Hash256[] links) {
			super(path);
			this.links = links;
		}

		@Override
		public CryptoTypes.Hash256 calculateHash() {
			final byte[][] parts = new byte[1 + links.length][];
			parts[0] = encodePath(path, false);
			for (int i = 0; i < links.length; ++i)
				parts[i + 1] = (null == links[i] ? CryptoTypes.Hash256.zero() : links[i]).bytes();

			return new CryptoTypes.Hash256(Transforms.sha3_256(parts));
		}
	}

	// endregion

	// region deserializePatriciaTreeNodes

	private static final class BufferReader {
		private final ByteBuffer view;

		BufferReader(final byte[] data) {
			this.view = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN);
		}

		boolean eof() {
			return !view.hasRemaining();
		}

		int readByte() {
			return view.get() & 0xFF;
		}

		int readShort() {
			return view.getShort() & 0xFFFF;
		}

		byte[] readBytes(final int count) {
			final byte[] out = new byte[count];
			view.get(out);
			return out;
		}
	}

	private static PatriciaTreePath deserializePath(final BufferReader reader) {
		final int numNibbles = reader.readByte();
		final int numBytes = (numNibbles + 1) / 2;
		return new PatriciaTreePath(reader.readBytes(numBytes), numNibbles);
	}

	private static LeafNode deserializeLeaf(final BufferReader reader) {
		final PatriciaTreePath path = deserializePath(reader);
		final CryptoTypes.Hash256 value = new CryptoTypes.Hash256(reader.readBytes(CryptoTypes.Hash256.SIZE));
		return new LeafNode(path, value);
	}

	private static BranchNode deserializeBranch(final BufferReader reader) {
		final PatriciaTreePath path = deserializePath(reader);
		final int linksMask = reader.readShort();
		final CryptoTypes.Hash256[] links = new CryptoTypes.Hash256[16];
		for (int i = 0; i < links.length; ++i) {
			if (0 != (linksMask & (1 << i)))
				links[i] = new CryptoTypes.Hash256(reader.readBytes(CryptoTypes.Hash256.SIZE));
		}
		return new BranchNode(path, links);
	}

	/**
	 * Deserializes a buffer containing patricia tree nodes.
	 *
	 * @param buffer Buffer containing serialized patricia tree nodes.
	 * @return Deserialized patricia tree nodes.
	 */
	public static List<TreeNode> deserializePatriciaTreeNodes(final byte[] buffer) {
		final BufferReader reader = new BufferReader(buffer);
		final List<TreeNode> nodes = new ArrayList<>();
		while (!reader.eof()) {
			final int marker = reader.readByte();
			switch (marker) {
				case 0xFF -> nodes.add(deserializeLeaf(reader));
				case 0x00 -> nodes.add(deserializeBranch(reader));
				default -> throw new IllegalArgumentException("invalid marker of a serialized node (" + marker + ")");
			}
		}
		return nodes;
	}

	// endregion

	// region provePatriciaMerkle

	/** Possible results of a patricia merkle proof. */
	public static final class PatriciaMerkleProofResult {
		/** Proof is valid (positive). */
		public static final int VALID_POSITIVE = 0x0001;
		/** Proof is valid (negative). */
		public static final int VALID_NEGATIVE = 0x0002;
		/** Negative proof is inconclusive. */
		public static final int INCONCLUSIVE = 0x4001;
		/** State hash cannot be derived from sub-cache merkle roots. */
		public static final int STATE_HASH_DOES_NOT_MATCH_ROOTS = 0x8001;
		/** Root of the path tree being proven is not a sub-cache merkle root. */
		public static final int UNANCHORED_PATH_TREE = 0x8002;
		/** Leaf value does not match expected value. */
		public static final int LEAF_VALUE_MISMATCH = 0x8003;
		/** Provided merkle hash contains an unlinked node. */
		public static final int UNLINKED_NODE = 0x8004;
		/** Actual merkle path does not match encoded key. */
		public static final int PATH_MISMATCH = 0x8005;

		private PatriciaMerkleProofResult() {
		}
	}

	private static boolean checkStateHash(final CryptoTypes.Hash256 stateHash, final List<CryptoTypes.Hash256> subcacheMerkleRoots) {
		final byte[][] parts = subcacheMerkleRoots.stream().map(CryptoTypes.Hash256::bytes).toArray(byte[][]::new);
		final byte[] digest = Transforms.sha3_256(parts);
		return Arrays.equals(stateHash.bytes(), digest);
	}

	private static int findLinkIndex(final BranchNode branch, final CryptoTypes.Hash256 targetLinkHash) {
		for (int i = 0; i < branch.links.length; ++i) {
			if (null != branch.links[i] && Arrays.equals(branch.links[i].bytes(), targetLinkHash.bytes()))
				return i;
		}
		return -1;
	}

	/**
	 * Proves a patricia merkle hash.
	 *
	 * @param encodedKey Encoded key of the state to prove.
	 * @param valueToTest Expected hash of the state to prove.
	 * @param merklePath Merkle <em>node</em> path from root to leaf.
	 * @param stateHash State hash from a block header.
	 * @param subcacheMerkleRoots Sub-cache merkle roots corresponding to the state hash.
	 * @return Proof result code.
	 */
	public static int provePatriciaMerkle(final CryptoTypes.Hash256 encodedKey, final CryptoTypes.Hash256 valueToTest,
			final List<TreeNode> merklePath, final CryptoTypes.Hash256 stateHash, final List<CryptoTypes.Hash256> subcacheMerkleRoots) {
		if (!checkStateHash(stateHash, subcacheMerkleRoots))
			return PatriciaMerkleProofResult.STATE_HASH_DOES_NOT_MATCH_ROOTS;

		// an empty path anchors to no root; guard here so an adversarial/malformed proof returns a result code instead of
		// crashing on merklePath.get(0) below
		if (merklePath.isEmpty())
			return PatriciaMerkleProofResult.UNANCHORED_PATH_TREE;

		final CryptoTypes.Hash256 pathRootHash = merklePath.get(0).calculateHash();
		boolean anchored = false;
		for (CryptoTypes.Hash256 root : subcacheMerkleRoots) {
			if (Arrays.equals(pathRootHash.bytes(), root.bytes())) {
				anchored = true;
				break;
			}
		}
		if (!anchored)
			return PatriciaMerkleProofResult.UNANCHORED_PATH_TREE;

		// positive proof must end with a leaf
		final boolean isPositiveProof = merklePath.get(merklePath.size() - 1) instanceof LeafNode;
		if (isPositiveProof) {
			final LeafNode leaf = (LeafNode) merklePath.get(merklePath.size() - 1);
			if (!Arrays.equals(valueToTest.bytes(), leaf.value.bytes()))
				return PatriciaMerkleProofResult.LEAF_VALUE_MISMATCH;
		}

		CryptoTypes.Hash256 childHash = null;
		final StringBuilder actualPath = new StringBuilder();
		for (int i = merklePath.size() - 1; 0 <= i; --i) {
			final TreeNode node = merklePath.get(i);
			final CryptoTypes.Hash256 nodeHash = node.calculateHash();
			String formattedLinkIndex = "";
			if (null != childHash) {
				// only a branch has links to a child; a leaf in a non-terminal position is a malformed proof, not a link match
				if (!(node instanceof BranchNode branch))
					return PatriciaMerkleProofResult.UNLINKED_NODE;

				final int linkIndex = findLinkIndex(branch, childHash);
				if (-1 == linkIndex)
					return PatriciaMerkleProofResult.UNLINKED_NODE;

				formattedLinkIndex = String.valueOf("0123456789ABCDEF".charAt(linkIndex));
			}

			childHash = nodeHash;
			actualPath.insert(0, formattedLinkIndex + node.hexPath());
		}

		final String encodedKeyHex = encodedKey.toString();
		if (isPositiveProof) {
			return actualPath.toString().equals(encodedKeyHex)
					? PatriciaMerkleProofResult.VALID_POSITIVE
					: PatriciaMerkleProofResult.PATH_MISMATCH;
		}

		if (!encodedKeyHex.startsWith(actualPath.toString()))
			return PatriciaMerkleProofResult.PATH_MISMATCH;

		final PatriciaTreePath fullKey = new PatriciaTreePath(encodedKey.bytes(), 2 * encodedKey.bytes().length);
		// when a malformed proof consumes the full key (actualPath spans all 2*len nibbles), the next nibble is out of range;
		final int nextNibble = actualPath.length() < 2 * encodedKey.bytes().length ? getNibbleAt(fullKey, actualPath.length()) : 0;
		final BranchNode last = (BranchNode) merklePath.get(merklePath.size() - 1);
		return null != last.links[nextNibble] ? PatriciaMerkleProofResult.INCONCLUSIVE : PatriciaMerkleProofResult.VALID_NEGATIVE;
	}

	// endregion
}
