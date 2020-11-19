/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MerkleHashBuilder.h"
#include "Hashes.h"
#include "catapult/functions.h"

namespace catapult { namespace crypto {

	namespace {
		Hash256 Final(std::vector<Hash256>& hashes, const consumer<const Hash256*, size_t>& hashConsumer) {
			if (hashes.empty()) {
				Hash256 hash{};
				hashConsumer(&hash, 1);
				return hash;
			}

			// build the merkle tree
			auto numRemainingHashes = hashes.size();
			hashConsumer(hashes.data(), hashes.size());
			while (numRemainingHashes > 1) {
				// merkle tree needs padding in case of an odd number of hashes, need to do before the next round of hashes is
				// pushed into the vector because nodes with same depth should be consecutive entries in the vector
				if (1 == numRemainingHashes % 2)
					hashConsumer(&hashes[numRemainingHashes - 1], 1);

				auto i = 0u;
				for (; i < numRemainingHashes; i += 2) {
					if (i + 1 < numRemainingHashes) {
						Sha3_256({ hashes[i].data(), 2 * Hash256::Size }, hashes[i / 2]);
						hashConsumer(&hashes[i / 2], 1);
						continue;
					}

					// if there is an odd number of hashes, duplicate the last one
					Sha3_256_Builder builder;
					builder.update(hashes[i]);
					builder.update(hashes[i]);
					builder.final(hashes[i / 2]);
					hashConsumer(&hashes[i / 2], 1);
					++numRemainingHashes;
				}

				numRemainingHashes /= 2;
			}

			return hashes[0];
		}
	}

	MerkleHashBuilder::MerkleHashBuilder(size_t capacity) {
		m_hashes.reserve(capacity);
	}

	void MerkleHashBuilder::update(const Hash256& hash) {
		m_hashes.push_back(hash);
	}

	void MerkleHashBuilder::final(Hash256& hash) {
		// build the merkle root
		hash = Final(m_hashes, [](const auto*, auto) {});
	}

	void MerkleHashBuilder::final(std::vector<Hash256>& tree) {
		// build the complete merkle tree
		tree.reserve(TreeSize(m_hashes.size()));
		Final(m_hashes, [&tree](const Hash256* pHash, size_t count) {
			for (auto i = 0u; i < count; ++i)
				tree.push_back(*pHash++);
		});
	}

	size_t MerkleHashBuilder::TreeSize(size_t leafCount) {
		// the tree is not required to be fully balanced, padding is only done in case of an odd number of nodes to make it even
		size_t size = 0;
		auto adjustedLeafCount = 1 == leafCount
				? 1
				: 0 == leafCount % 2 ? leafCount : leafCount + 1;
		for (auto i = adjustedLeafCount; i > 1; i = (i + 1) >> 1)
			size += i;

		return size >= 2 ? ++size : 1;
	}
}}
