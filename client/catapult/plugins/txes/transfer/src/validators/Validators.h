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
#include "catapult/validators/ValidatorTypes.h"
#include "src/model/TransferNotifications.h"

namespace catapult {
namespace validators {

	/// Validator that applies to transfer message notifications and validates that:
	/// - messages have a maximum message size of \a maxMessageSize
	DECLARE_STATELESS_VALIDATOR(TransferMessage, model::TransferMessageNotification)
	(uint16_t maxMessageSize);

	/// Validator that applies to transfer mosaics notifications and validates that:
	/// - mosaics are ordered
	DECLARE_STATELESS_VALIDATOR(TransferMosaics, model::TransferMosaicsNotification)
	();
}
}
