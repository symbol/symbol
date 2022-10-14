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

#include "InvalidMerkleHashBuilder.h"
#include "Hashes.h"
#include "catapult/functions.h"

namespace catapult { namespace crypto {

	namespace {
		void InsertInWindow(std::vector<Hash256>& hashes, Hash256& hash, size_t index, size_t windowSize) {
			// shift all hashes after index within the window
			for (auto i = std::min(hashes.size(), windowSize) - 1; i > index ; --i)
				hashes[i] = hashes[i - 1];

			// update the new hash
			hashes[index] = hash;
		}

		Hash256 Final(std::vector<Hash256>& hashes) {
			if (hashes.empty()) {
				Hash256 hash{};
				return hash;
			}

			// build the merkle tree
			Hash256 newHash;
			auto numRemainingHashes = hashes.size();
			while (numRemainingHashes > 1) {
				auto i = 0u;
				for (; i < numRemainingHashes; i += 2) {
					if (i + 1 < numRemainingHashes) {
						Sha3_256({ hashes[i].data(), 2 * Hash256::Size }, newHash);
						InsertInWindow(hashes, newHash, i / 2, numRemainingHashes);
						continue;
					}

					// if there is an odd number of hashes, duplicate the last one
					Sha3_256_Builder builder;
					builder.update(hashes[i]);
					builder.update(hashes[i]);
					builder.final(newHash);
					InsertInWindow(hashes, newHash, i / 2, numRemainingHashes);
					++numRemainingHashes;
				}

				numRemainingHashes /= 2;
			}

			return hashes[0];
		}
	}

	InvalidMerkleHashBuilder::InvalidMerkleHashBuilder(size_t capacity) {
		m_hashes.reserve(capacity);
	}

	void InvalidMerkleHashBuilder::update(const Hash256& hash) {
		m_hashes.push_back(hash);
	}

	void InvalidMerkleHashBuilder::final(Hash256& hash) {
		// build the merkle root
		hash = Final(m_hashes);
	}
}}
