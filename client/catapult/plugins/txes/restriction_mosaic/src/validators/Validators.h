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
#include "src/model/MosaicRestrictionNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to mosaic restriction type notifications and validates that:
	/// - restriction type is known
	DECLARE_STATELESS_VALIDATOR(MosaicRestrictionType, model::MosaicRestrictionTypeNotification)();

	/// Validator that applies to mosaic restriction required notifications and validates that:
	/// - corresponding global restriction exists
	DECLARE_STATEFUL_VALIDATOR(MosaicRestrictionRequired, model::MosaicRestrictionRequiredNotification)();

	/// Validator that applies to new value notifications of mosaic global restriction modifications and validates that:
	/// - the requested modification will not cause the number of restrictions to exceed \a maxMosaicRestrictionValues
	/// - delete only applies to existing values
	DECLARE_STATEFUL_VALIDATOR(
			MosaicGlobalRestrictionMaxValues,
			model::MosaicGlobalRestrictionModificationNewValueNotification)(uint8_t maxMosaicRestrictionValues);

	/// Validator that applies to new value notifications of mosaic address restriction modifications and validates that:
	/// - the requested modification will not cause the number of restrictions to exceed \a maxMosaicRestrictionValues
	/// - delete only applies to existing values
	DECLARE_STATEFUL_VALIDATOR(
			MosaicAddressRestrictionMaxValues,
			model::MosaicAddressRestrictionModificationNewValueNotification)(uint8_t maxMosaicRestrictionValues);

	/// Validator that applies to previous value notifications of mosaic global restriction modifications and validates that:
	/// - the specified previous value(s) match the current value(s)
	DECLARE_STATEFUL_VALIDATOR(MosaicGlobalRestrictionModification, model::MosaicGlobalRestrictionModificationPreviousValueNotification)();

	/// Validator that applies to previous value notifications of mosaic address restriction modifications and validates that:
	/// - the specified previous value(s) match the current value(s)
	DECLARE_STATEFUL_VALIDATOR(
			MosaicAddressRestrictionModification,
			model::MosaicAddressRestrictionModificationPreviousValueNotification)();

	/// Validator that applies to balance transfer notifications and validates that:
	/// - sender is authorized to send mosaic
	/// - recipient is authorized to receive mosaic
	DECLARE_STATEFUL_VALIDATOR(MosaicRestrictionBalanceTransfer, model::BalanceTransferNotification)();

	/// Validator that applies to balance debit notifications and validates that:
	/// - recipient is authorized to receive mosaic
	DECLARE_STATEFUL_VALIDATOR(MosaicRestrictionBalanceDebit, model::BalanceDebitNotification)();
}}
