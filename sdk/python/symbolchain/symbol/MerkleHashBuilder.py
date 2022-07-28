from collections import namedtuple
from functools import reduce

import sha3

from ..CryptoTypes import Hash256

MerklePart = namedtuple('MerklePart', ['hash', 'is_left'])


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
				hasher = sha3.sha3_256()
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


def prove_merkle(leaf_hash, merkle_path, root_hash):
	"""Proves a merkle hash."""

	def calculate_next_hash(working_hash, merkle_part):
		hasher = sha3.sha3_256()
		if merkle_part.is_left:
			hasher.update(merkle_part.hash.bytes)
			hasher.update(working_hash.bytes)
		else:
			hasher.update(working_hash.bytes)
			hasher.update(merkle_part.hash.bytes)

		return Hash256(hasher.digest())

	computed_root_hash = reduce(calculate_next_hash, merkle_path, leaf_hash)
	return root_hash == computed_root_hash
