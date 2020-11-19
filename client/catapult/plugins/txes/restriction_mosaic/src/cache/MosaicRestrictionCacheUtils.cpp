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

#include "MosaicRestrictionCacheUtils.h"
#include "MosaicRestrictionCacheDelta.h"
#include "MosaicRestrictionCacheView.h"
#include "src/state/MosaicRestrictionEvaluator.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace cache {

	// region GetMosaicGlobalRestrictionResolvedRules

	MosaicGlobalRestrictionRuleResolutionResult GetMosaicGlobalRestrictionResolvedRules(
			const MosaicRestrictionCacheTypes::CacheReadOnlyType& restrictionCache,
			MosaicId mosaicId,
			std::vector<MosaicRestrictionResolvedRule>& resolvedRules) {
		auto entryIter = restrictionCache.find(state::CreateMosaicRestrictionEntryKey(mosaicId));
		if (!entryIter.tryGet())
			return MosaicGlobalRestrictionRuleResolutionResult::No_Rules;

		const auto& restriction = entryIter.get().asGlobalRestriction();
		for (auto key : restriction.keys()) {
			state::MosaicGlobalRestriction::RestrictionRule rule;
			restriction.tryGet(key, rule);

			if (mosaicId == rule.ReferenceMosaicId)
				return MosaicGlobalRestrictionRuleResolutionResult::Invalid_Rule;

			resolvedRules.push_back(MosaicRestrictionResolvedRule{
				MosaicId() == rule.ReferenceMosaicId ? mosaicId : rule.ReferenceMosaicId,
				key,
				rule.RestrictionValue,
				rule.RestrictionType
			});
		}

		return MosaicGlobalRestrictionRuleResolutionResult::Success;
	}

	// endregion

	// region EvaluateMosaicRestrictionResolvedRulesForAddress

	bool EvaluateMosaicRestrictionResolvedRulesForAddress(
			const MosaicRestrictionCacheTypes::CacheReadOnlyType& restrictionCache,
			const Address& address,
			const std::vector<MosaicRestrictionResolvedRule>& resolvedRules) {
		std::unordered_map<MosaicId, state::MosaicAddressRestriction, utils::BaseValueHasher<MosaicId>> restrictionCacheSet;
		for (const auto& resolvedRule : resolvedRules) {
			auto mosaicId = resolvedRule.MosaicId;
			auto restrictionIter = restrictionCacheSet.find(mosaicId);
			if (restrictionCacheSet.cend() == restrictionIter) {
				auto entryIter = restrictionCache.find(state::CreateMosaicRestrictionEntryKey(mosaicId, address));
				auto restriction = entryIter.tryGet()
						? entryIter.get().asAddressRestriction()
						: state::MosaicAddressRestriction(mosaicId, address);
				restrictionIter = restrictionCacheSet.emplace(mosaicId, restriction).first;
			}

			auto evaluateResult = state::EvaluateMosaicRestriction(
					{ MosaicId(), resolvedRule.RestrictionValue, resolvedRule.RestrictionType },
					restrictionIter->second.get(resolvedRule.RestrictionKey));
			if (!evaluateResult)
				return false;
		}

		return true;
	}

	// endregion
}}
