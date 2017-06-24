#include "MerkleHashBuilder.h"
#include "Hashes.h"

namespace catapult { namespace crypto {

	MerkleHashBuilder::MerkleHashBuilder(size_t capacity) {
		m_hashes.reserve(capacity);
	}

	void MerkleHashBuilder::update(const Hash256& hash) {
		m_hashes.push_back(hash);
	}

	void MerkleHashBuilder::final(Hash256& hash) {
		// build the merkle tree in place
		auto numRemainingHashes = m_hashes.size();
		while (numRemainingHashes > 1) {
			auto i = 0u;
			for (; i < numRemainingHashes; i += 2) {
				if (i + 1 < numRemainingHashes) {
					Sha3_256({ m_hashes[i].data(), 2 * Hash256_Size }, m_hashes[i / 2]);
					continue;
				}

				// if there is an odd number of hashes, duplicate the last one
				Sha3_256_Builder builder;
				builder.update(m_hashes[i]);
				builder.update(m_hashes[i]);
				builder.final(m_hashes[i / 2]);
				++numRemainingHashes;
			}

			numRemainingHashes /= 2;
		}

		m_hashes.resize(numRemainingHashes);
		hash = 0 == numRemainingHashes ? Hash256() : m_hashes[0];
	}
}}
