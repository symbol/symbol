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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountBalances.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		template<typename TAccountIdentifier>
		ValidationResult CheckAccount(
				uint16_t maxMosaics,
				MosaicId mosaicId,
				const TAccountIdentifier& accountIdentifier,
				const ValidatorContext& context) {
			const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
			auto accountStateIter = accountStateCache.find(accountIdentifier);
			const auto& balances = accountStateIter.get().Balances;
			if (balances.get(mosaicId) != Amount())
				return ValidationResult::Success;

			return maxMosaics <= balances.size() ? Failure_Mosaic_Max_Mosaics_Exceeded : ValidationResult::Success;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsBalanceTransfer, model::BalanceTransferNotification)(uint16_t maxMosaics) {
		using ValidatorType = stateful::FunctionalNotificationValidatorT<model::BalanceTransferNotification>;
		auto name = "MaxMosaicsBalanceTransferValidator";
		return std::make_unique<ValidatorType>(name, [maxMosaics](
				const model::BalanceTransferNotification& notification,
				const ValidatorContext& context) {
			if (Amount() == notification.Amount)
				return ValidationResult::Success;

			return CheckAccount(
					maxMosaics,
					context.Resolvers.resolve(notification.MosaicId),
					context.Resolvers.resolve(notification.Recipient),
					context);
		});
	}

	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsSupplyChange, model::MosaicSupplyChangeNotification)(uint16_t maxMosaics) {
		using ValidatorType = stateful::FunctionalNotificationValidatorT<model::MosaicSupplyChangeNotification>;
		auto name = "MaxMosaicsSupplyChangeValidator";
		return std::make_unique<ValidatorType>(name, [maxMosaics](
				const model::MosaicSupplyChangeNotification& notification,
				const ValidatorContext& context) {
			if (model::MosaicSupplyChangeAction::Decrease == notification.Action)
				return ValidationResult::Success;

			return CheckAccount(maxMosaics, context.Resolvers.resolve(notification.MosaicId), notification.Owner, context);
		});
	}
}}
