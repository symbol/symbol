import hashlib
from binascii import hexlify
from collections import namedtuple
from enum import Enum
from functools import reduce

from ..BufferReader import BufferReader
from ..CryptoTypes import Hash256

MerklePart = namedtuple('MerklePart', ['hash', 'is_left'])
PatriciaTreePath = namedtuple('PatriciaTreePath', ['path', 'size'])

# region MerkleHashBuilder


class MerkleHashBuilder:
	"""Builder for creating a merkle hash."""

	def __init__(self):
		"""Creates a merkle hash builder."""
		self.hashes = []

	def update(self, component_hash):
		"""Adds a hash to the merkle hash."""
		self.hashes.append(component_hash.bytes)

	def final(self):
		"""Calculates the merkle hash."""
		if not self.hashes:
			return Hash256.zero()

		num_remaining_hashes = len(self.hashes)
		while num_remaining_hashes > 1:
			i = 0
			while i < num_remaining_hashes:
				hasher = hashlib.sha3_256()
				hasher.update(self.hashes[i])

				if i + 1 < num_remaining_hashes:
					hasher.update(self.hashes[i + 1])
				else:
					# if there is an odd number of hashes, duplicate the last one
					hasher.update(self.hashes[i])
					num_remaining_hashes += 1

				self.hashes[i // 2] = hasher.digest()
				i += 2

			num_remaining_hashes //= 2

		return Hash256(self.hashes[0])

# endregion


# region prove_merkle

def prove_merkle(leaf_hash, merkle_path, root_hash):
	"""
	Proves a merkle hash.
	Merkle *hash chain* path is ordered from leaf to root, where each element is MerklePart.
	"""

	def calculate_next_hash(working_hash, merkle_part):
		hasher = hashlib.sha3_256()
		if merkle_part.is_left:
			hasher.update(merkle_part.hash.bytes)
			hasher.update(working_hash.bytes)
		else:
			hasher.update(working_hash.bytes)
			hasher.update(merkle_part.hash.bytes)

		return Hash256(hasher.digest())

	computed_root_hash = reduce(calculate_next_hash, merkle_path, leaf_hash)
	return root_hash == computed_root_hash

# endregion


# region LeafNode / BranchNode

def _get_nibble_at(path, index):
	byte = path.path[index // 2]
	return byte & 0xF if 1 == index % 2 else byte >> 4


def _encode_path(path, is_leaf):
	i = 0
	buffer = [0x20 if is_leaf else 0]
	if 1 == path.size % 2:
		buffer[0] |= 0x10 | _get_nibble_at(path, 0)
		i += 1

	while i < path.size:
		buffer.append((_get_nibble_at(path, i) << 4) + _get_nibble_at(path, i + 1))
		i += 2

	return bytes(buffer)


class TreeNode:
	"""Node in a compact patricia tree."""

	def __init__(self, path):
		self.path = path

	@property
	def hex_path(self):
		"""Gets hex representation of path."""

		return hexlify(self.path.path).decode('utf8').upper()[:self.path.size]


class LeafNode(TreeNode):
	"""Leaf node in a compact patricia tree."""

	def __init__(self, path, value):
		super().__init__(path)
		self.value = value

	def calculate_hash(self):
		"""Calculates node hash."""

		hasher = hashlib.sha3_256()
		hasher.update(_encode_path(self.path, True))
		hasher.update(self.value.bytes)
		return Hash256(hasher.digest())


class BranchNode(TreeNode):
	"""Branch node in a compact patricia tree."""

	def __init__(self, path, links):
		super().__init__(path)
		self.links = links

	def calculate_hash(self):
		"""Calculates node hash."""

		hasher = hashlib.sha3_256()
		hasher.update(_encode_path(self.path, False))
		for link in self.links:
			hasher.update((link if link else Hash256.zero()).bytes)

		return Hash256(hasher.digest())

# endregion


# region deserialize_patricia_tree_nodes

def _deserialize_path(reader):
	num_nibbles = reader.read_int(1)
	num_bytes = (num_nibbles + 1) // 2
	return PatriciaTreePath(reader.read_bytes(num_bytes), num_nibbles)


def _deserialize_leaf(reader):
	path = _deserialize_path(reader)
	value = Hash256(reader.read_bytes(Hash256.SIZE))
	return LeafNode(path, value)


def _deserialize_branch(reader):
	path = _deserialize_path(reader)

	links_mask = reader.read_int(2)
	links = [None] * 16
	for index, _ in enumerate(links):
		if links_mask & (2 ** index):
			links[index] = Hash256(reader.read_bytes(Hash256.SIZE))

	return BranchNode(path, links)


def deserialize_patricia_tree_nodes(buffer):
	reader = BufferReader(buffer)
	nodes = []
	while not reader.eof:
		node_marker = reader.read_int(1)

		if 0xFF == node_marker:
			nodes.append(_deserialize_leaf(reader))
		elif 0x00 == node_marker:
			nodes.append(_deserialize_branch(reader))
		else:
			raise ValueError(f'invalid marker of a serialized node ({node_marker})')

	return nodes

# endregion


# region prove_patricia_merkle

class PatriciaMerkleProofResult(Enum):
	"""Possible results of a patricia merkle proof."""

	VALID_POSITIVE = 0x0001  # proof is valid (positive)
	VALID_NEGATIVE = 0x0002  # proof is valid (negative)

	INCONCLUSIVE = 0x4001  # negative proof is inconclusive

	STATE_HASH_DOES_NOT_MATCH_ROOTS = 0x8001  # state hash cannot be derived from subcache merkle roots
	UNANCHORED_PATH_TREE = 0x8002  # root of the path tree being proven is not a subcache merkle root
	LEAF_VALUE_MISMATCH = 0x8003  # leaf value does not match expected value
	UNLINKED_NODE = 0x8004  # provided merkle hash contains an unlinked node
	PATH_MISMATCH = 0x8005  # actual merkle path does not match encoded key


def _check_state_hash(state_hash, subcache_merkle_roots):
	hasher = hashlib.sha3_256()
	for root in subcache_merkle_roots:
		hasher.update(root.bytes)

	return state_hash == Hash256(hasher.digest())


def prove_patricia_merkle(encoded_key, value_to_test, merkle_path, state_hash, subcache_merkle_roots):
	"""
	Proves a patricia merkle hash.
	Merkle *node* path is ordered from root to leaf, where each element is either BranchNode or LeafNode.
	"""

	# pylint: disable=too-many-return-statements

	if not _check_state_hash(state_hash, subcache_merkle_roots):
		return PatriciaMerkleProofResult.STATE_HASH_DOES_NOT_MATCH_ROOTS

	if merkle_path[0].calculate_hash() not in subcache_merkle_roots:
		return PatriciaMerkleProofResult.UNANCHORED_PATH_TREE

	# positive proof must end with a leaf
	is_positive_proof = hasattr(merkle_path[-1], 'value')
	if is_positive_proof:
		if value_to_test != merkle_path[-1].value:
			return PatriciaMerkleProofResult.LEAF_VALUE_MISMATCH

	child_hash = None
	actual_path = ''
	for node in reversed(merkle_path):
		node_hash = node.calculate_hash()
		formatted_link_index = ''
		if child_hash:
			if child_hash not in node.links:
				return PatriciaMerkleProofResult.UNLINKED_NODE

			formatted_link_index = f'{node.links.index(child_hash):01X}'

		child_hash = node_hash
		actual_path = f'{formatted_link_index}{node.hex_path}{actual_path}'

	if is_positive_proof:
		# for positive proof, expected and calculated paths must match exactly
		return PatriciaMerkleProofResult.PATH_MISMATCH if actual_path != str(encoded_key) else PatriciaMerkleProofResult.VALID_POSITIVE

	# for negative proof, expected path must start with calculated path and next nibble must be a dead end
	if not str(encoded_key).startswith(actual_path):
		return PatriciaMerkleProofResult.PATH_MISMATCH

	next_nibble = _get_nibble_at(PatriciaTreePath(encoded_key.bytes, 2 * len(encoded_key.bytes)), len(actual_path))
	next_node = merkle_path[-1].links[next_nibble]
	return PatriciaMerkleProofResult.INCONCLUSIVE if next_node is not None else PatriciaMerkleProofResult.VALID_NEGATIVE

# endregion
