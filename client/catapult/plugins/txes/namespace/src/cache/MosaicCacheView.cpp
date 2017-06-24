#include "MosaicCacheView.h"
#include <numeric>

namespace catapult { namespace cache {

	namespace {
		using IdBasedHistoryBaseSet = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetType;

		const state::MosaicHistory& GetMosaicHistory(const IdBasedHistoryBaseSet& historyById, MosaicId id) {
			const auto* pHistory = historyById.find(id);
			if (!pHistory)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown mosaic", id);

			return *pHistory;
		}
	}

	size_t BasicMosaicCacheView::size() const {
		return m_historyById.size();
	}

	size_t BasicMosaicCacheView::deepSize() const {
		size_t sum = 0;
		return std::accumulate(m_historyById.cbegin(), m_historyById.cend(), sum, [](auto value, const auto& pair) {
			return value + pair.second.historyDepth();
		});
	}

	bool BasicMosaicCacheView::contains(MosaicId id) const {
		return m_historyById.contains(id);
	}

	bool BasicMosaicCacheView::isActive(MosaicId id, Height height) const {
		return contains(id) && get(id).definition().isActive(height);
	}

	const state::MosaicEntry& BasicMosaicCacheView::get(MosaicId id) const {
		return GetMosaicHistory(m_historyById, id).back();
	}
}}
