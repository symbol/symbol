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

#include "Observers.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	namespace {
		template<typename TNotification>
		void AliasedObserverHandler(const ObserverContext& context, const TNotification& notification) {
			auto& cache = context.Cache.sub<cache::NamespaceCache>();

			if (ShouldLink(notification.AliasAction, context.Mode))
				cache.setAlias(notification.NamespaceId, state::NamespaceAlias(notification.AliasedData));
			else
				cache.setAlias(notification.NamespaceId, state::NamespaceAlias());
		}
	}

#define DEFINE_ALIASED_DATA_OBSERVER(OBSERVER_NAME, NOTIFICATION) \
	DEFINE_OBSERVER(OBSERVER_NAME, NOTIFICATION, [](const auto& notification, const ObserverContext& context) { \
		AliasedObserverHandler(context, notification); \
	})

	DEFINE_ALIASED_DATA_OBSERVER(AliasedAddress, model::AliasedAddressNotification)
	DEFINE_ALIASED_DATA_OBSERVER(AliasedMosaicId, model::AliasedMosaicIdNotification)
}}
