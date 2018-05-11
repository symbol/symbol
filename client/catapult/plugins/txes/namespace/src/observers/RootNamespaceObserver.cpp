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
#include <limits>

namespace catapult { namespace observers {

	namespace {
		bool IsRenewal(const state::RootNamespace& root, const model::RootNamespaceNotification& notification, Height height) {
			return root.lifetime().isActive(height) && root.owner() == notification.Signer;
		}
	}

	DEFINE_OBSERVER(RootNamespace, model::RootNamespaceNotification, [](const auto& notification, const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::NamespaceCache>();

		if (NotifyMode::Rollback == context.Mode) {
			cache.remove(notification.NamespaceId);
			return;
		}

		auto lifetimeEnd = Eternal_Artifact_Duration == notification.Duration
				? Height(std::numeric_limits<Height::ValueType>::max())
				: context.Height + Height(notification.Duration.unwrap());
		auto lifetime = state::NamespaceLifetime(context.Height, lifetimeEnd);
		if (cache.contains(notification.NamespaceId)) {
			// if a renewal, duration should add onto current expiry
			const auto& rootEntry = cache.get(notification.NamespaceId);
			if (IsRenewal(rootEntry.root(), notification, context.Height)) {
				lifetime = rootEntry.root().lifetime();
				lifetime.End = lifetime.End + Height(notification.Duration.unwrap());
			}
		}

		auto root = state::RootNamespace(notification.NamespaceId, notification.Signer, lifetime);
		cache.insert(root);
	});
}}
