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

namespace catapult { namespace validators {

	using Notification = model::AliasLinkNotification;

	DEFINE_STATEFUL_VALIDATOR(AliasAvailability, [](const Notification& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::NamespaceCache>();
		auto namespaceIter = cache.find(notification.NamespaceId);
		if (!namespaceIter.tryGet())
			return Failure_Namespace_Unknown;

		const auto& root = namespaceIter.get().root();
		auto aliasType = root.alias(notification.NamespaceId).type();
		if (model::AliasAction::Link == notification.AliasAction && state::AliasType::None != aliasType)
			return Failure_Namespace_Alias_Already_Exists;
		else if (model::AliasAction::Unlink == notification.AliasAction && state::AliasType::None == aliasType)
			return Failure_Namespace_Unknown_Alias;

		return ValidationResult::Success;
	})
}}
