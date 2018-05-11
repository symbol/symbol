/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "MosaicCacheDelta.h"
#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "catapult/utils/Casting.h"
#include <unordered_set>

namespace catapult { namespace cache {

	namespace {
		using MosaicByIdMap = MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType;
		using MosaicIdsByNamespaceIdMap = MosaicCacheTypes::NamespaceGroupingTypes::BaseSetDeltaType;
		using HeightBasedMosaicIdsMap = MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaType;

		void UpdateExpiryMap(HeightBasedMosaicIdsMap& mosaicIdsByExpiryHeight, const state::MosaicEntry& entry) {
			// in case the mosaic is not eternal, update the expiry height based mosaic ids map
			const auto& definition = entry.definition();
			if (definition.isEternal())
				return;

			Height expiryHeight(definition.height().unwrap() + definition.properties().duration().unwrap());
			AddIdentifierWithGroup(mosaicIdsByExpiryHeight, expiryHeight, entry.mosaicId());
		}
	}

	BasicMosaicCacheDelta::BasicMosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets, size_t deepSize)
			: MosaicCacheDeltaMixins::Size(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::Contains(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::ConstAccessor(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::MutableAccessor(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::ActivePredicate(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::DeltaElements(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::MosaicDeepSize(deepSize)
			, m_pHistoryById(mosaicSets.pPrimary)
			, m_pMosaicIdsByNamespaceId(mosaicSets.pNamespaceGrouping)
			, m_pMosaicIdsByExpiryHeight(mosaicSets.pHeightGrouping)
	{}

	void BasicMosaicCacheDelta::insert(const state::MosaicEntry& entry) {
		// if a history entry exists, append the data to the back
		auto* pHistory = m_pHistoryById->find(entry.mosaicId());
		if (pHistory) {
			if (pHistory->back().namespaceId() != entry.namespaceId())
				CATAPULT_THROW_RUNTIME_ERROR_1("owning namespace of mosaic does not match", entry.mosaicId());

			pHistory->push_back(entry.definition(), entry.supply());
			incrementDeepSize();

			UpdateExpiryMap(*m_pMosaicIdsByExpiryHeight, entry);
			return;
		}

		// otherwise, create a history entry
		state::MosaicHistory history(entry.namespaceId(), entry.mosaicId());
		history.push_back(entry.definition(), entry.supply());
		m_pHistoryById->insert(std::move(history));
		incrementDeepSize();

		// update secondary maps
		AddIdentifierWithGroup(*m_pMosaicIdsByNamespaceId, entry.namespaceId(), entry.mosaicId());
		UpdateExpiryMap(*m_pMosaicIdsByExpiryHeight, entry);
	}

	void BasicMosaicCacheDelta::remove(MosaicId id) {
		auto* pHistory = m_pHistoryById->find(id);
		if (!pHistory)
			CATAPULT_THROW_INVALID_ARGUMENT_1("no mosaic exists", id);

		pHistory->pop_back();
		decrementDeepSize();
		removeIfEmpty(*pHistory);
	}

	void BasicMosaicCacheDelta::remove(NamespaceId id) {
		// need to calculate the total history depth removed because it is not derivable from number of identifiers removed
		size_t numRemoved = 0;
		ForEachIdentifierWithGroup(*m_pHistoryById, *m_pMosaicIdsByNamespaceId, id, [&numRemoved](const auto& history) {
			numRemoved += history.historyDepth();
		});

		RemoveAllIdentifiersWithGroup(*m_pHistoryById, *m_pMosaicIdsByNamespaceId, id);
		decrementDeepSize(numRemoved);
	}

	void BasicMosaicCacheDelta::prune(Height height) {
		// note that the complete history might already have been pruned since the call to prune() below can prune several/all entries
		// This can happen in the following scenario:
		// 1) owner creates mosaic definition that expires at height x
		//    ==> height based mosaic ids map contains mapping x -> mosaic id
		// 2) owner changes definition to expire at height y < x
		//    ==> height based mosaic ids map contains mappings x -> mosaic id and y -> mosaic id
		// 3) at height y prune() is called on the mosaic history object. *Both* entries are pruned and
		//    the history is removed from the history map
		// 4) at height x m_pHistoryById->find(mosaic Id) is called but cannot find the history
		ForEachIdentifierWithGroup(*m_pHistoryById, *m_pMosaicIdsByExpiryHeight, height, [this, height](auto& history) {
			// prune and conditionally remove history
			auto numErasedHistories = history.prune(height);
			decrementDeepSize(numErasedHistories);
			this->removeIfEmpty(history);
		});

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
}}
