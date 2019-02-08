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
#include "Results.h"
#include "src/model/AliasNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/validators/ValidatorTypes.h"
#include <unordered_set>

namespace catapult { namespace validators {

	/// A validator implementation that applies to namespace notifications and validates that:
	/// - namespace type is valid
	DECLARE_STATELESS_VALIDATOR(NamespaceType, model::NamespaceNotification)();

	/// A validator implementation that applies to namespace name notifications and validates that:
	/// - namespace name has a maximum size of \a maxNameSize
	/// - namespace name consists only of allowed characters
	/// - for root namespaces, name is not in \a reservedRootNamespaceNames
	/// - for child namespaces, the parent id is not an id that can be generated from \a reservedRootNamespaceNames
	DECLARE_STATELESS_VALIDATOR(NamespaceName, model::NamespaceNameNotification)(
			uint8_t maxNameSize,
			const std::unordered_set<std::string>& reservedRootNamespaceNames);

	/// A validator implementation that applies to root namespace notifications and validates that:
	/// - namespace duration is less than or equal to \a maxDuration for root namespace
	/// - namespace duration is zero for child namespace
	DECLARE_STATELESS_VALIDATOR(RootNamespace, model::RootNamespaceNotification)(BlockDuration maxDuration);

	/// A validator implementation that applies to root register namespace transactions and validates that:
	/// - the namespace is available and can be created or renewed given \a maxNamespaceDuration
	DECLARE_STATEFUL_VALIDATOR(RootNamespaceAvailability, model::RootNamespaceNotification)(BlockDuration maxNamespaceDuration);

	/// A validator implementation that applies to child register namespace transactions and validates that:
	/// - the namespace is available and can be created
	DECLARE_STATEFUL_VALIDATOR(ChildNamespaceAvailability, model::ChildNamespaceNotification)();

	/// A validator implementation that applies to child register namespace transactions and validates that:
	/// - the maximum number of children (\a maxChildren) for a root namespace is not exceeded
	DECLARE_STATEFUL_VALIDATOR(RootNamespaceMaxChildren, model::ChildNamespaceNotification)(uint16_t maxChildren);

	/// A validator implementation that applies to alias owner notifications and validates that:
	/// - alias action is valid
	DECLARE_STATELESS_VALIDATOR(AliasAction, model::AliasOwnerNotification)();

	/// A validator implementation that applies to alias owner notifications and validates that:
	/// - namespace exists
	/// - link does not overwrite existing link
	/// - unlinked alias exists
	/// - owner of namespace matches alias owner
	DECLARE_STATEFUL_VALIDATOR(AliasAvailability, model::AliasOwnerNotification)();

	/// A validator implementation that applies to aliased address notifications and validates that:
	/// - unlink operation matches existing link
	DECLARE_STATEFUL_VALIDATOR(UnlinkAliasedAddressConsistency, model::AliasedAddressNotification)();

	/// A validator implementation that applies to aliased mosaic id notifications and validates that:
	/// - unlink operation matches existing link
	DECLARE_STATEFUL_VALIDATOR(UnlinkAliasedMosaicIdConsistency, model::AliasedMosaicIdNotification)();

	/// A validator implementation that applies to aliased address notifications and validates that:
	/// - account is known
	DECLARE_STATEFUL_VALIDATOR(AddressAlias, model::AliasedAddressNotification)();
}}
