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
#include "src/model/NameChecker.h"
#include "src/model/NamespaceIdGenerator.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace validators {

	using Notification = model::NamespaceNameNotification;
	using NameSet = std::unordered_set<std::string>;

	DECLARE_STATELESS_VALIDATOR(NamespaceName, Notification)(uint8_t maxNameSize) {
		return MAKE_STATELESS_VALIDATOR(NamespaceName, ([maxNameSize](const Notification& notification) {
			if (maxNameSize < notification.NameSize || !model::IsValidName(notification.NamePtr, notification.NameSize))
				return Failure_Namespace_Invalid_Name;

			auto name = utils::RawString(reinterpret_cast<const char*>(notification.NamePtr), notification.NameSize);
			if (notification.NamespaceId != model::GenerateNamespaceId(notification.ParentId, name))
				return Failure_Namespace_Name_Id_Mismatch;

			return ValidationResult::Success;
		}));
	}

	DECLARE_STATEFUL_VALIDATOR(NamespaceReservedName, Notification)(const NameSet& reservedRootNamespaceNames) {
		std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>> reservedRootIds;
		for (const auto& name : reservedRootNamespaceNames)
			reservedRootIds.emplace(model::GenerateNamespaceId(Namespace_Base_Id, name));

		return MAKE_STATEFUL_VALIDATOR(NamespaceReservedName, ([reservedRootIds](
				const Notification& notification,
				const ValidatorContext& context) {
			if (Height(1) == context.Height)
				return ValidationResult::Success;

			auto namespaceId = Namespace_Base_Id == notification.ParentId ? notification.NamespaceId : notification.ParentId;
			return reservedRootIds.cend() == reservedRootIds.find(namespaceId)
					? ValidationResult::Success
					: Failure_Namespace_Root_Name_Reserved;
		}));
	}
}}
