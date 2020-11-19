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

#include "Validators.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/constants.h"

namespace catapult { namespace validators {

	using Notification = model::RootNamespaceNotification;

	namespace {
		constexpr Height ToHeight(BlockDuration duration) {
			return Height(duration.unwrap());
		}

		constexpr bool AddOverflows(Height height, BlockDuration duration) {
			return height + ToHeight(duration) < height;
		}

		constexpr Height CalculateMaxLifetimeEnd(Height height, BlockDuration duration) {
			return AddOverflows(height, duration) ? Height(std::numeric_limits<uint64_t>::max()) : height + ToHeight(duration);
		}
	}

	DECLARE_STATEFUL_VALIDATOR(NamespaceDurationOverflow, Notification)(BlockDuration maxNamespaceDuration) {
		return MAKE_STATEFUL_VALIDATOR(NamespaceDurationOverflow, [maxNamespaceDuration](
				const Notification& notification,
				const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::NamespaceCache>();
			auto height = context.Height;

			if (AddOverflows(height, notification.Duration))
				return Failure_Namespace_Invalid_Duration;

			auto namespaceIter = cache.find(notification.NamespaceId);
			if (!namespaceIter.tryGet())
				return ValidationResult::Success;

			// if grace period after expiration has passed, overflow check above is sufficient
			const auto& root = namespaceIter.get().root();
			if (!root.lifetime().isActive(height))
				return ValidationResult::Success;

			if (AddOverflows(root.lifetime().End, notification.Duration))
				return Failure_Namespace_Invalid_Duration;

			auto newLifetimeEnd = root.lifetime().End + ToHeight(notification.Duration);
			auto maxLifetimeEnd = CalculateMaxLifetimeEnd(height, maxNamespaceDuration);
			if (newLifetimeEnd > maxLifetimeEnd)
				return Failure_Namespace_Invalid_Duration;

			return ValidationResult::Success;
		});
	}
}}
