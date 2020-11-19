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
#include "src/model/MosaicNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/validators/ValidatorTypes.h"
#include <unordered_set>

namespace catapult { namespace validators {

	// region MosaicChangeTransaction

	/// Validator that applies to mosaic required notifications and validates that:
	/// - mosaic exists and is active
	/// - mosaic owner matches requesting signer
	DECLARE_STATEFUL_VALIDATOR(RequiredMosaic, model::MosaicRequiredNotification)();

	// endregion

	// region MosaicDefinitionTransaction

	/// Validator that applies to mosaic properties notifications and validates that:
	/// - definition has valid mosaic flags
	DECLARE_STATELESS_VALIDATOR(MosaicFlags, model::MosaicPropertiesNotification)();

	/// Validator that applies to mosaic nonce notifications and validates that:
	/// - mosaic id is the expected id generated from signer and nonce
	DECLARE_STATELESS_VALIDATOR(MosaicId, model::MosaicNonceNotification)();

	/// Validator that applies to mosaic definition notifications and validates that:
	/// - the mosaic is available and can be created or modified
	DECLARE_STATEFUL_VALIDATOR(MosaicAvailability, model::MosaicDefinitionNotification)();

	/// Validator that applies to mosaic definition notifications and validates that:
	/// - the resulting mosaic duration is no greater than \a maxMosaicDuration and there was no overflow
	DECLARE_STATEFUL_VALIDATOR(MosaicDuration, model::MosaicDefinitionNotification)(BlockDuration maxMosaicDuration);

	/// Validator that applies to mosaic definition notifications and validates that:
	/// - the resulting mosaic divisibility is no greater than \a maxDivisibility
	DECLARE_STATEFUL_VALIDATOR(MosaicDivisibility, model::MosaicDefinitionNotification)(uint8_t maxDivisibility);

	// endregion

	// region MosaicSupplyChangeTransaction

	/// Validator that applies to mosaic supply change notifications and validates that:
	/// - action has a valid value
	/// - delta amount is non-zero
	DECLARE_STATELESS_VALIDATOR(MosaicSupplyChange, model::MosaicSupplyChangeNotification)();

	/// Validator that applies to all balance transfer notifications and validates that:
	/// - transferred mosaic is active and is transferable
	/// - as an optimization, special currency mosaic (\a currencyMosaicId) transfers are always allowed
	DECLARE_STATEFUL_VALIDATOR(MosaicTransfer, model::BalanceTransferNotification)(UnresolvedMosaicId currencyMosaicId);

	/// Validator that applies to mosaic supply change notifications and validates that:
	/// - the affected mosaic has mutable supply
	/// - decrease does not cause owner amount to become negative
	/// - increase does not cause total atomic units to exceed \a maxAtomicUnits
	/// \note This validator is dependent on RequiredMosaicValidator.
	DECLARE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, model::MosaicSupplyChangeNotification)(Amount maxAtomicUnits);

	/// Validator that applies to mosaic supply change notifications and validates that:
	/// - the account changing the supply does not exceed the maximum number of mosaics (\a maxMosaics) an account is allowed to own
	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsSupplyChange, model::MosaicSupplyChangeNotification)(uint16_t maxMosaics);

	// endregion

	// region TransferTransaction

	/// Validator that applies to all balance transfer notifications and validates that:
	/// - the recipient does not exceed the maximum number of mosaics (\a maxMosaics) an account is allowed to own
	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsBalanceTransfer, model::BalanceTransferNotification)(uint16_t maxMosaics);

	// endregion
}}
