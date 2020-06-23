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

#pragma once
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace keylink {

	/// Creates a stateful multi key link validator with \a name that validates:
	/// - no conflicting link exists when linking and no more than (\a maxLinks) links
	/// - matching link exists when unlinking
	template<typename TNotification, typename TAccessor>
	validators::stateful::NotificationValidatorPointerT<TNotification> CreateMultiKeyLinkValidator(
			const std::string& name,
			uint8_t maxLinks) {
		using ValidatorType = validators::stateful::FunctionalNotificationValidatorT<TNotification>;
		return std::make_unique<ValidatorType>(name + "MultiKeyLinkValidator", [maxLinks](
				const TNotification& notification,
				const validators::ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(notification.MainAccountPublicKey);
			const auto& accountState = accountStateIter.get();

			// TODO: need to check if StartPoint < CURRENT FinalizationPoint (this probably needs to be stored in context)

			const auto& publicKeysAccessor = TAccessor::Get(accountState);
			if (model::LinkAction::Link == notification.LinkAction) {
				if (maxLinks == publicKeysAccessor.size())
					return TAccessor::Failure_Too_Many_Links;

				if (publicKeysAccessor.upperBound() >= notification.LinkedPublicKey.StartPoint)
					return TAccessor::Failure_Link_Already_Exists;
			} else {
				if (!publicKeysAccessor.containsExact(notification.LinkedPublicKey))
					return TAccessor::Failure_Inconsistent_Unlink_Data;
			}

			return validators::ValidationResult::Success;
		});
	}
}}
