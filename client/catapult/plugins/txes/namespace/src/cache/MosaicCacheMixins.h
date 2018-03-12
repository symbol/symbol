#pragma once
#include "catapult/deltaset/BaseSetDelta.h"
#include <numeric>

namespace catapult { namespace cache {

	/// A mixin for calculating the deep size of mosaics.
	template<typename TSet>
	class MosaicDeepSizeMixin {
	public:
		/// Creates a mixin around \a set.
		explicit MosaicDeepSizeMixin(const TSet& set) : m_set(set)
		{}

	public:
		/// Gets the total number of mosaics in the cache (including versions).
		size_t deepSize() const {
			size_t sum = 0;
			return std::accumulate(m_set.begin(), m_set.end(), sum, [](auto value, const auto& pair) {
				return value + pair.second.historyDepth();
			});
		}

	private:
		const TSet& m_set;
	};
}}
