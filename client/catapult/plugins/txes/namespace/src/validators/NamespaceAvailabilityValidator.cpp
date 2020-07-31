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
#include "src/cache/NamespaceCache.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/constants.h"

namespace catapult { namespace validators {

	using Notification = model::RootNamespaceNotification;

	namespace {
		constexpr bool IsEternal(const state::NamespaceLifetime& lifetime) {
			return Height(std::numeric_limits<Height::ValueType>::max()) == lifetime.End;
		}
	}

	DEFINE_STATEFUL_VALIDATOR(RootNamespaceAvailability, [](const Notification& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::NamespaceCache>();
		auto height = context.Height;

		if (Height(1) != height && Eternal_Artifact_Duration == notification.Duration)
			return Failure_Namespace_Eternal_After_Nemesis_Block;

		auto namespaceIter = cache.find(notification.NamespaceId);
		if (!namespaceIter.tryGet())
			return ValidationResult::Success;

		const auto& root = namespaceIter.get().root();
		if (IsEternal(root.lifetime()) || Eternal_Artifact_Duration == notification.Duration)
			return Failure_Namespace_Invalid_Duration;

		// if grace period after expiration has passed, any signer can claim the namespace
		if (!root.lifetime().isActive(height))
			return ValidationResult::Success;

		return root.ownerAddress() == notification.Owner ? ValidationResult::Success : Failure_Namespace_Owner_Conflict;
	})
}}
