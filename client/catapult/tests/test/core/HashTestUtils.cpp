#include "HashTestUtils.h"
#include "catapult/model/EntityRange.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		template<typename TData>
		model::HashRange CopyHashes(TData pData, size_t numHashes) {
			return model::HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(pData), numHashes);
		}
	}

	model::HashRange GenerateRandomHashes(size_t numHashes) {
		std::vector<Hash256> hashes(numHashes);
		for (auto i = 0u; i < numHashes; ++i)
			hashes[i] = GenerateRandomData<Hash256_Size>();

		return CopyHashes(hashes.data(), numHashes);
	}

	model::HashRange GenerateRandomHashesSubset(const model::HashRange& source, size_t numHashes) {
		return CopyHashes(&*source.begin(), numHashes);
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
