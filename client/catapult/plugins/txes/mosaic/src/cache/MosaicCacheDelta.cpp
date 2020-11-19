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

#include "MosaicCacheDelta.h"
#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "catapult/utils/Casting.h"
#include <unordered_set>

namespace catapult { namespace cache {

	namespace {
		using MosaicByIdMap = MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType;
		using HeightBasedMosaicIdsMap = MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaType;

		Height GetExpiryHeight(const state::MosaicDefinition& definition) {
			return Height(definition.startHeight().unwrap() + definition.properties().duration().unwrap());
		}

		void UpdateExpiryMap(HeightBasedMosaicIdsMap& mosaicIdsByExpiryHeight, const state::MosaicEntry& entry) {
			// in case the mosaic is not eternal, update the expiry height based mosaic ids map
			const auto& definition = entry.definition();
			if (definition.isEternal())
				return;

			AddIdentifierWithGroup(mosaicIdsByExpiryHeight, GetExpiryHeight(definition), entry.mosaicId());
		}
	}

	BasicMosaicCacheDelta::BasicMosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets)
			: MosaicCacheDeltaMixins::Size(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::Contains(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::ConstAccessor(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::MutableAccessor(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::PatriciaTreeDelta(*mosaicSets.pPrimary, mosaicSets.pPatriciaTree)
			, MosaicCacheDeltaMixins::ActivePredicate(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::BasicInsertRemove(*mosaicSets.pPrimary)
			, MosaicCacheDeltaMixins::Touch(*mosaicSets.pPrimary, *mosaicSets.pHeightGrouping)
			, MosaicCacheDeltaMixins::DeltaElements(*mosaicSets.pPrimary)
			, m_pEntryById(mosaicSets.pPrimary)
			, m_pMosaicIdsByExpiryHeight(mosaicSets.pHeightGrouping)
	{}

	void BasicMosaicCacheDelta::insert(const state::MosaicEntry& entry) {
		MosaicCacheDeltaMixins::BasicInsertRemove::insert(entry);
		UpdateExpiryMap(*m_pMosaicIdsByExpiryHeight, entry);
	}

	void BasicMosaicCacheDelta::remove(MosaicId mosaicId) {
		auto iter = m_pEntryById->find(mosaicId);
		const auto* pEntry = iter.get();
		if (!!pEntry) {
			const auto& definition = pEntry->definition();
			if (!definition.isEternal())
				RemoveIdentifierWithGroup(*m_pMosaicIdsByExpiryHeight, GetExpiryHeight(definition), mosaicId);
		}

		MosaicCacheDeltaMixins::BasicInsertRemove::remove(mosaicId);
	}
}}
