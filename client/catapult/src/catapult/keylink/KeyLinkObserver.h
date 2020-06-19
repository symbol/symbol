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
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace keylink {

	/// Creates a key link observer with \a name that links/unlinks target key.
	template<typename TNotification, typename TAccessor>
	observers::NotificationObserverPointerT<TNotification> CreateKeyLinkObserver(const std::string& name) {
		using ObserverType = observers::FunctionalNotificationObserverT<TNotification>;
		return std::make_unique<ObserverType>(name + "KeyLinkObserver", [](const auto& notification, auto& context) {
			auto& cache = context.Cache.template sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(notification.MainAccountPublicKey);
			auto& accountState = accountStateIter.get();

			auto& publicKeyAccessor = TAccessor::Get(accountState);
			if (observers::ShouldLink(notification.LinkAction, context.Mode))
				publicKeyAccessor.set(notification.LinkedPublicKey);
			else
				publicKeyAccessor.unset();
		});
	}
}}
