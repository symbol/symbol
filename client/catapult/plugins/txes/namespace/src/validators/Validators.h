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
#include "src/model/MosaicNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/validators/ValidatorTypes.h"
#include <unordered_set>

namespace catapult { namespace validators {

	// region RegisterNamespaceTransaction

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

	// endregion

	// region MosaicChangeTransaction

	/// A validator implementation that applies to mosaic change notifications and validates that:
	/// - change transaction owner has permission to change mosaic
	DECLARE_STATEFUL_VALIDATOR(MosaicChangeAllowed, model::MosaicChangeNotification)();

	// endregion

	// region MosaicDefinitionTransaction

	/// A validator implementation that applies to mosaic name notifications and validates that:
	/// - mosaic name has a maximum size of \a maxNameSize
	/// - mosaic name consists only of allowed characters
	DECLARE_STATELESS_VALIDATOR(MosaicName, model::MosaicNameNotification)(uint8_t maxNameSize);

	/// A validator implementation that applies to mosaic properties notifications and validates that:
	/// - definition has valid mosaic flags
	/// - definition has divisibility no greater than \a maxDivisibility
	/// - mosaic duration has a value not larger than \a maxMosaicDuration
	/// - optional mosaic properties are sorted, known and not duplicative
	DECLARE_STATELESS_VALIDATOR(MosaicProperties, model::MosaicPropertiesNotification)(
			uint8_t maxDivisibility,
			BlockDuration maxMosaicDuration);

	/// A validator implementation that applies to mosaic definition notifications and validates that:
	/// - a mosaic is consistent with its purported namespace
	DECLARE_STATEFUL_VALIDATOR(NamespaceMosaicConsistency, model::MosaicDefinitionNotification)();

	/// A validator implementation that applies to mosaic definition notifications and validates that:
	/// - the mosaic is available and can be created or modified
	DECLARE_STATEFUL_VALIDATOR(MosaicAvailability, model::MosaicDefinitionNotification)();

	// endregion

	// region MosaicSupplyChangeTransaction

	/// A validator implementation that applies to mosaic supply change notifications and validates that:
	/// - direction has a valid value
	/// - delta amount is non-zero
	DECLARE_STATELESS_VALIDATOR(MosaicSupplyChange, model::MosaicSupplyChangeNotification)();

	/// A validator implementation that applies to all balance transfer notifications and validates that:
	/// - transferred mosaic is active and is transferable
	DECLARE_STATEFUL_VALIDATOR(MosaicTransfer, model::BalanceTransferNotification)();

	/// A validator implementation that applies to mosaic supply change notifications and validates that:
	/// - the affected mosaic has mutable supply
	/// - decrease does not cause owner amount to become negative
	/// - increase does not cause total divisible units to exceed \a maxDivisibleUnits
	/// \note This validator is dependent on MosaicChangeAllowedValidator.
	DECLARE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, model::MosaicSupplyChangeNotification)(Amount maxDivisibleUnits);

	/// A validator implementation that applies to mosaic supply change notifications and validates that:
	/// - the account changing the supply does not exceed the maximum number of mosaics (\a maxMosaics) an account is allowed to own
	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsSupplyChange, model::MosaicSupplyChangeNotification)(uint16_t maxMosaics);

	// endregion

	// region TransferTransaction

	/// A validator implementation that applies to all balance transfer notifications and validates that:
	/// - the recipient does not exceed the maximum number of mosaics (\a maxMosaics) an account is allowed to own
	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsBalanceTransfer, model::BalanceTransferNotification)(uint16_t maxMosaics);

	// endregion
}}
