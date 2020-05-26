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

#include "Validators.h"
#include "src/cache/MosaicRestrictionCache.h"
#include "src/cache/MosaicRestrictionCacheUtils.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		using AddressRulesEvaluator = predicate<
			const cache::MosaicRestrictionCacheTypes::CacheReadOnlyType&,
			const std::vector<cache::MosaicRestrictionResolvedRule>>;

		ValidationResult ProcessMosaicRules(
				UnresolvedMosaicId unresolvedMosaicId,
				const ValidatorContext& context,
				const AddressRulesEvaluator& addressRulesEvaluator) {
			const auto& cache = context.Cache.sub<cache::MosaicRestrictionCache>();

			auto mosaicId = context.Resolvers.resolve(unresolvedMosaicId);

			std::vector<cache::MosaicRestrictionResolvedRule> mosaicRules;
			auto result = cache::GetMosaicGlobalRestrictionResolvedRules(cache, mosaicId, mosaicRules);

			if (cache::MosaicGlobalRestrictionRuleResolutionResult::No_Rules == result)
				return ValidationResult::Success;

			if (cache::MosaicGlobalRestrictionRuleResolutionResult::Invalid_Rule == result)
				return Failure_RestrictionMosaic_Invalid_Global_Restriction;

			return addressRulesEvaluator(cache, mosaicRules)
					? ValidationResult::Success
					: Failure_RestrictionMosaic_Account_Unauthorized;
		}

		ValidationResult CheckTransferAuthorization(
				const model::BalanceTransferNotification& notification,
				const ValidatorContext& context) {
			return ProcessMosaicRules(notification.MosaicId, context, [&notification, &resolvers = context.Resolvers](
					const auto& cache,
					const auto& mosaicRules) {
				auto isSenderAuthorized = cache::EvaluateMosaicRestrictionResolvedRulesForAddress(cache, notification.Sender, mosaicRules);
				auto isRecipientAuthorized = cache::EvaluateMosaicRestrictionResolvedRulesForAddress(
						cache,
						resolvers.resolve(notification.Recipient),
						mosaicRules);
				return isSenderAuthorized && isRecipientAuthorized;
			});
		}

		ValidationResult CheckDebitAuthorization(const model::BalanceDebitNotification& notification, const ValidatorContext& context) {
			return ProcessMosaicRules(notification.MosaicId, context, [&notification](const auto& cache, const auto& mosaicRules) {
				return cache::EvaluateMosaicRestrictionResolvedRulesForAddress(cache, notification.Sender, mosaicRules);
			});
		}
	}

	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(MosaicRestrictionBalanceTransfer, model::BalanceTransferNotification, CheckTransferAuthorization)
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(MosaicRestrictionBalanceDebit, model::BalanceDebitNotification, CheckDebitAuthorization)
}}
