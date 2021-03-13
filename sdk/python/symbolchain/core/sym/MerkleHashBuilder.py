import sha3


class MerkleHashBuilder:
    """Builder for creating a merkle hash."""

    def __init__(self):
        """Creates a merkle hash builder."""
        self.hashes = []

    def update(self, component_hash):
        """Adds a hash to the merkle hash."""
        self.hashes.append(component_hash)

    def final(self):
        """Calculates the merkle hash."""
        if not self.hashes:
            return bytes([0] * 32)

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

        return self.hashes[0]
