#pragma once
#include <string>

namespace catapult {
	namespace cache {
		class CatapultCache;
		struct SupplementalData;
	}
}

namespace catapult { namespace local { namespace p2p {

	/// Save catapult \a cache state along with \a supplementalData into state directory inside \a dataDirectory.
	void SaveState(const std::string& dataDirectory, const cache::CatapultCache& cache, const cache::SupplementalData& supplementalData);

	/// Load catapult \a cache state and \a supplementalData from state directory inside \a dataDirectory.
	/// Returns \c true if data has been loaded, \c false if there was nothing to load.
	bool LoadState(const std::string& dataDirectory, cache::CatapultCache& cache, cache::SupplementalData& supplementalData);
}}}
