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

#include "Observers.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/constants.h"
#include <limits>

namespace catapult { namespace observers {

	namespace {
		bool IsRenewal(const state::RootNamespace& root, const model::RootNamespaceNotification& notification, Height height) {
			return root.lifetime().isActive(height) && root.ownerPublicKey() == notification.Owner;
		}

		state::NamespaceLifetime CalculateNewLifetime(
				const model::RootNamespaceNotification& notification,
				Height height,
				BlockDuration gracePeriodDuration) {
			auto lifetimeEnd = Eternal_Artifact_Duration == notification.Duration
					? Height(std::numeric_limits<Height::ValueType>::max())
					: height + Height(notification.Duration.unwrap());
			return state::NamespaceLifetime(height, lifetimeEnd, gracePeriodDuration);
		}

		state::NamespaceLifetime CalculateLifetime(
				const cache::NamespaceCacheDelta& cache,
				const model::RootNamespaceNotification& notification,
				Height height) {
			auto newLifetime = CalculateNewLifetime(notification, height, cache.gracePeriodDuration());
			auto namespaceIter = cache.find(notification.NamespaceId);
			if (!namespaceIter.tryGet())
				return newLifetime;

			const auto& rootEntry = namespaceIter.get();
			if (!IsRenewal(rootEntry.root(), notification, height))
				return newLifetime;

			// if a renewal, duration should add onto current expiry
			auto currentLifetime = rootEntry.root().lifetime();
			return state::NamespaceLifetime(currentLifetime.Start, currentLifetime.End + Height(notification.Duration.unwrap()));
		}
	}

	DEFINE_OBSERVER(RootNamespace, model::RootNamespaceNotification, [](
			const model::RootNamespaceNotification& notification,
			const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::NamespaceCache>();

		if (NotifyMode::Rollback == context.Mode) {
			cache.remove(notification.NamespaceId);
			return;
		}

		auto lifetime = CalculateLifetime(cache, notification, context.Height);
		auto root = state::RootNamespace(notification.NamespaceId, notification.Owner, lifetime);
		cache.insert(root);
	});
}}
