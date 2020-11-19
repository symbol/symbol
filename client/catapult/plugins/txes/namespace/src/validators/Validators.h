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
#include "Results.h"
#include "src/model/AliasNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/validators/ValidatorTypes.h"
#include <unordered_set>

namespace catapult { namespace validators {

	// region namespace validators

	/// Validator that applies to namespace notifications and validates that:
	/// - namespace registration type is valid
	DECLARE_STATELESS_VALIDATOR(NamespaceRegistrationType, model::NamespaceRegistrationNotification)();

	/// Validator that applies to namespace name notifications and validates that:
	/// - namespace name has a maximum size of \a maxNameSize
	/// - namespace name consists only of allowed characters
	DECLARE_STATELESS_VALIDATOR(NamespaceName, model::NamespaceNameNotification)(uint8_t maxNameSize);

	/// Validator that applies to namespace name notifications and validates that:
	/// - for root namespaces, name is not in \a reservedRootNamespaceNames
	/// - for child namespaces, the parent id is not an id that can be generated from \a reservedRootNamespaceNames
	DECLARE_STATEFUL_VALIDATOR(NamespaceReservedName, model::NamespaceNameNotification)(
			const std::unordered_set<std::string>& reservedRootNamespaceNames);

	/// Validator that applies to root namespace notifications and validates that:
	/// - namespace duration is between \a minDuration and \a maxDuration, inclusive, for root namespace
	/// - namespace duration is zero for child namespace
	DECLARE_STATELESS_VALIDATOR(RootNamespace, model::RootNamespaceNotification)(BlockDuration minDuration, BlockDuration maxDuration);

	/// Validator that applies to root namespace notifications and validates that:
	/// - namespace is available and can be created or renewed
	DECLARE_STATEFUL_VALIDATOR(RootNamespaceAvailability, model::RootNamespaceNotification)();

	/// Validator that applies to root namespace notifications and validates that:
	/// - namespace duration does not overflow given \a maxNamespaceDuration
	DECLARE_STATEFUL_VALIDATOR(NamespaceDurationOverflow, model::RootNamespaceNotification)(BlockDuration maxNamespaceDuration);

	/// Validator that applies to child namespace notifications and validates that:
	/// - namespace is available and can be created
	/// - child namespace depth does not exceed \a maxNamespaceDepth
	DECLARE_STATEFUL_VALIDATOR(ChildNamespaceAvailability, model::ChildNamespaceNotification)(uint8_t maxNamespaceDepth);

	/// Validator that applies to child namespace notifications and validates that:
	/// - maximum number of children (\a maxChildren) for a root namespace is not exceeded
	DECLARE_STATEFUL_VALIDATOR(RootNamespaceMaxChildren, model::ChildNamespaceNotification)(uint16_t maxChildren);

	/// Validator that applies to namespace required notifications and validates that:
	/// - namespace exists and is active (not in grace period)
	/// - namespace owner matches requesting signer
	DECLARE_STATEFUL_VALIDATOR(RequiredNamespace, model::NamespaceRequiredNotification)();

	// endregion

	// region alias validators

	/// Validator that applies to alias owner notifications and validates that:
	/// - alias action is valid
	DECLARE_STATELESS_VALIDATOR(AliasAction, model::AliasLinkNotification)();

	/// Validator that applies to alias owner notifications and validates that:
	/// - link does not overwrite existing link
	/// - unlinked alias exists
	DECLARE_STATEFUL_VALIDATOR(AliasAvailability, model::AliasLinkNotification)();

	/// Validator that applies to aliased address notifications and validates that:
	/// - unlink operation matches existing link
	DECLARE_STATEFUL_VALIDATOR(UnlinkAliasedAddressConsistency, model::AliasedAddressNotification)();

	/// Validator that applies to aliased mosaic id notifications and validates that:
	/// - unlink operation matches existing link
	DECLARE_STATEFUL_VALIDATOR(UnlinkAliasedMosaicIdConsistency, model::AliasedMosaicIdNotification)();

	/// Validator that applies to aliased address notifications and validates that:
	/// - account is known
	DECLARE_STATEFUL_VALIDATOR(AddressAlias, model::AliasedAddressNotification)();

	// endregion
}}
