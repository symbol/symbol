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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult {
namespace keylink {

    /// Creates a stateful key link validator with \a name that validates:
    /// - no link exists when linking
    /// - matching link exists when unlinking
    template <typename TNotification, typename TAccessor>
    validators::stateful::NotificationValidatorPointerT<TNotification> CreateKeyLinkValidator(const std::string& name)
    {
        using ValidatorType = validators::stateful::FunctionalNotificationValidatorT<TNotification>;
        return std::make_unique<ValidatorType>(
            name + "KeyLinkValidator",
            [](const TNotification& notification, const validators::ValidatorContext& context) {
                const auto& cache = context.Cache.sub<cache::AccountStateCache>();
                auto accountStateIter = cache.find(notification.MainAccountPublicKey);
                const auto& accountState = accountStateIter.get();

                const auto& publicKeyAccessor = TAccessor::Get(accountState);
                if (model::LinkAction::Link == notification.LinkAction) {
                    if (publicKeyAccessor)
                        return TAccessor::Failure_Link_Already_Exists;
                } else {
                    if (!publicKeyAccessor || notification.LinkedPublicKey != publicKeyAccessor.get())
                        return TAccessor::Failure_Inconsistent_Unlink_Data;
                }

                return validators::ValidationResult::Success;
            });
    }
}
}
