#include "MosaicCacheDelta.h"
#include "src/state/GroupedMosaicIds.h"
#include "catapult/utils/Casting.h"
#include <numeric>
#include <unordered_set>

namespace catapult { namespace cache {

	namespace {
		using MosaicByIdMap = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetDeltaPointerType::element_type;
		using MosaicIdsByNamespaceIdMap = mosaic_cache_types::namespace_id_mosaic_ids_map::BaseSetDeltaPointerType::element_type;
		using HeightBasedMosaicIdsMap = mosaic_cache_types::height_mosaic_ids_map::BaseSetDeltaPointerType::element_type;

		template<typename TMosaicByIdMap>
		auto& GetMosaicHistory(TMosaicByIdMap& historyById, MosaicId id) {
			auto* pHistory = historyById.find(id);
			if (!pHistory)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown mosaic", id);

			return *pHistory;
		}

		template<typename TMap, typename TValue, typename TKey>
		void UpdateMap(TMap& map, TKey key, MosaicId mosaicId) {
			if (!map.contains(key))
				map.insert(TValue(key));

			auto* pValue = map.find(key);
			pValue->add(mosaicId);
		}

		void UpdateExpiryMap(HeightBasedMosaicIdsMap& mosaicIdsByExpiryHeight, const state::MosaicEntry& entry) {
			// in case the mosaic is not eternal, update the expiry height based mosaic ids map
			const auto& definition = entry.definition();
			if (definition.isEternal())
				return;

			Height expiryHeight(definition.height().unwrap() + definition.properties().duration().unwrap());
			UpdateMap<HeightBasedMosaicIdsMap, state::HeightMosaics>(mosaicIdsByExpiryHeight, expiryHeight, entry.mosaicId());
		}
	}

	size_t BasicMosaicCacheDelta::size() const {
		return m_pHistoryById->size();
	}

	size_t BasicMosaicCacheDelta::deepSize() const {
		size_t sum = 0;
		return std::accumulate(m_pHistoryById->cbegin(), m_pHistoryById->cend(), sum, [](auto value, const auto& pair) {
			return value + pair.second.historyDepth();
		});
	}

	bool BasicMosaicCacheDelta::contains(MosaicId id) const {
		return m_pHistoryById->contains(id);
	}

	bool BasicMosaicCacheDelta::isActive(MosaicId id, Height height) const {
		return contains(id) && get(id).definition().isActive(height);
	}

	const state::MosaicEntry& BasicMosaicCacheDelta::get(MosaicId id) const {
		return GetMosaicHistory(utils::as_const(*m_pHistoryById), id).back();
	}

	state::MosaicEntry& BasicMosaicCacheDelta::get(MosaicId id) {
		return GetMosaicHistory(*m_pHistoryById, id).back();
	}

	void BasicMosaicCacheDelta::insert(const state::MosaicEntry& entry) {
		// if a history entry exists, append the data to the back
		auto* pHistory = m_pHistoryById->find(entry.mosaicId());
		if (pHistory) {
			if (pHistory->back().namespaceId() != entry.namespaceId())
				CATAPULT_THROW_RUNTIME_ERROR_1("owning namespace of mosaic does not match", entry.mosaicId());

			pHistory->push_back(entry.definition(), entry.supply());

			UpdateExpiryMap(*m_pMosaicIdsByExpiryHeight, entry);
			return;
		}

		// otherwise, create a history entry
		state::MosaicHistory history(entry.namespaceId(), entry.mosaicId());
		history.push_back(entry.definition(), entry.supply());
		m_pHistoryById->insert(std::move(history));

		// update secondary maps
		UpdateMap<MosaicIdsByNamespaceIdMap, state::NamespaceMosaics>(*m_pMosaicIdsByNamespaceId, entry.namespaceId(), entry.mosaicId());
		UpdateExpiryMap(*m_pMosaicIdsByExpiryHeight, entry);
	}

	void BasicMosaicCacheDelta::remove(MosaicId id) {
		auto* pHistory = m_pHistoryById->find(id);
		if (!pHistory)
			CATAPULT_THROW_INVALID_ARGUMENT_1("no mosaic exists", id);

		pHistory->pop_back();
		removeIfEmpty(*pHistory);
	}

	void BasicMosaicCacheDelta::remove(NamespaceId id) {
		const auto* pNamespaceMosaics = m_pMosaicIdsByNamespaceId->find(id);
		if (!pNamespaceMosaics)
			return;

		const auto& mosaicIds = pNamespaceMosaics->mosaicIds();
		for (auto mosaicId : mosaicIds)
			m_pHistoryById->remove(mosaicId);

		m_pMosaicIdsByNamespaceId->remove(id);
	}

	void BasicMosaicCacheDelta::prune(Height height) {
		const auto* pExpiredMosaics = m_pMosaicIdsByExpiryHeight->find(height);
		if (!pExpiredMosaics)
			return;

		const auto& mosaicIds = pExpiredMosaics->mosaicIds();
		for (auto mosaicId : mosaicIds) {
			// note that the complete history might already have been pruned since the call to prune() below can prune several/all entries
			// This can happen in the following scenario:
			// 1) owner creates mosaic definition that expires at height x
			//    ==> height based mosaic ids map contains mapping x -> mosaic id
			// 2) owner changes definition to expire at height y < x
			//    ==> height based mosaic ids map contains mappings x -> mosaic id and y -> mosaic id
			// 3) at height y prune() is called on the mosaic history object. *Both* entries are pruned and
			//    the history is removed from the history map
			// 4) at height x m_pHistoryById->find(mosaic Id) is called but cannot find the history
			auto* pHistory = m_pHistoryById->find(mosaicId);
			if (!pHistory)
				continue;

			// prune and conditionally remove history
			pHistory->prune(height);
			removeIfEmpty(*pHistory);
		}

		m_pMosaicIdsByExpiryHeight->remove(height);
	}

	void BasicMosaicCacheDelta::removeIfEmpty(const state::MosaicHistory& history) {
		if (!history.empty())
			return;

		auto* pNamespaceMosaics = m_pMosaicIdsByNamespaceId->find(history.namespaceId());
		pNamespaceMosaics->remove(history.id());
		if (pNamespaceMosaics->empty())
			m_pMosaicIdsByNamespaceId->remove(pNamespaceMosaics->key());

		m_pHistoryById->remove(history.id());
	}

	namespace {
		template<typename TDestination, typename TSource>
		void CollectAll(TDestination& dest, const TSource& source) {
			for (const auto& pair : source)
				dest.push_back(&pair.second);
		}
	}

	std::vector<const state::MosaicHistory*> BasicMosaicCacheDelta::addedMosaicHistories() const {
		std::vector<const state::MosaicHistory*> addedHistories;
		auto deltas = m_pHistoryById->deltas();
		CollectAll(addedHistories, deltas.Added);
		return addedHistories;
	}

	std::vector<const state::MosaicHistory*> BasicMosaicCacheDelta::modifiedMosaicHistories() const {
		std::vector<const state::MosaicHistory*> modifiedHistories;
		auto deltas = m_pHistoryById->deltas();
		CollectAll(modifiedHistories, deltas.Copied);
		return modifiedHistories;
	}

	std::vector<MosaicId> BasicMosaicCacheDelta::removedMosaicHistories() const {
		std::vector<MosaicId> removedHistories;
		auto deltas = m_pHistoryById->deltas();
		for (const auto& pair : deltas.Removed)
			removedHistories.push_back(pair.first);

		return removedHistories;
	}
}}
