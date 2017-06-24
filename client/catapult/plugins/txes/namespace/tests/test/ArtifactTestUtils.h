#pragma once
#include <vector>

namespace catapult { namespace test {

	/// Converts a vector of raw artifact ids (\a rawIds) to a vector of typed artifact ids.
	template<typename TArtifactId>
	std::vector<TArtifactId> ToArtifactIds(const std::vector<typename TArtifactId::ValueType>& rawIds) {
		std::vector<TArtifactId> ids;
		for (auto id : rawIds)
			ids.push_back(TArtifactId(id));

		return ids;
	}
}}
