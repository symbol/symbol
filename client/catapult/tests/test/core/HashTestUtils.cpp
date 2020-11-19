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

#include "HashTestUtils.h"
#include "catapult/model/EntityRange.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		template<typename TData>
		model::HashRange CopyHashes(const TData* pData, size_t numHashes) {
			return model::HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(pData), numHashes);
		}
	}

	model::HashRange GenerateRandomHashes(size_t numHashes) {
		std::vector<Hash256> hashes(numHashes);
		for (auto i = 0u; i < numHashes; ++i)
			hashes[i] = GenerateRandomByteArray<Hash256>();

		return CopyHashes(hashes.data(), numHashes);
	}

	model::HashRange GenerateRandomHashesSubset(const model::HashRange& source, size_t numHashes) {
		return CopyHashes(source.data(), numHashes);
	}

	void InsertAll(std::vector<Hash256>& dest, const model::HashRange& source) {
		for (const auto& hash : source)
			dest.push_back(hash);
	}

	model::HashRange ConcatHashes(const model::HashRange& lhs, const model::HashRange& rhs) {
		auto numHashes = lhs.size() + rhs.size();

		std::vector<Hash256> hashes;
		InsertAll(hashes, lhs);
		InsertAll(hashes, rhs);

		return CopyHashes(hashes.data(), numHashes);
	}
}}
