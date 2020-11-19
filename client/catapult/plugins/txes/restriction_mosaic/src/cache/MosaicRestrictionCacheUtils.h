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

#pragma once
#include "MosaicRestrictionCacheTypes.h"

namespace catapult { namespace cache {

	// region GetMosaicGlobalRestrictionResolvedRules

	/// Result of mosaic global restriction rule resolution.
	enum class MosaicGlobalRestrictionRuleResolutionResult {
		/// All rules are valid and have been resolved.
		Success,

		/// No rules are set.
		No_Rules,

		/// At least one rule is invalid.
		Invalid_Rule
	};

	/// Resolved mosaic restriction rule.
	struct MosaicRestrictionResolvedRule {
		/// Identifier of the mosaic providing the restriction key.
		catapult::MosaicId MosaicId;

		/// Restriction key.
		uint64_t RestrictionKey;

		/// Restriction value.
		uint64_t RestrictionValue;

		/// Restriction type.
		model::MosaicRestrictionType RestrictionType;
	};

	/// Gets the (resolved) global restriction rules for the mosaic identified by \a mosaicId using \a restrictionCache
	/// and inserts resolved rules into \a resolvedRules.
	MosaicGlobalRestrictionRuleResolutionResult GetMosaicGlobalRestrictionResolvedRules(
			const MosaicRestrictionCacheTypes::CacheReadOnlyType& restrictionCache,
			MosaicId mosaicId,
			std::vector<MosaicRestrictionResolvedRule>& resolvedRules);

	// endregion

	// region EvaluateMosaicRestrictionResolvedRulesForAddress

	/// Evaluates all resolved rules (\a resolvedRules) for the account identified by \a address using \a restrictionCache.
	bool EvaluateMosaicRestrictionResolvedRulesForAddress(
			const MosaicRestrictionCacheTypes::CacheReadOnlyType& restrictionCache,
			const Address& address,
			const std::vector<MosaicRestrictionResolvedRule>& resolvedRules);

	// endregion
}}
