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
#include "src/model/MetadataNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to metadata sizes notifications and validates that:
	/// - values have a minimum value size of one
	/// - magnitude of value size delta is less than value size
	/// - values have a maximum value size of \a maxValueSize
	DECLARE_STATELESS_VALIDATOR(MetadataSizes, model::MetadataSizesNotification)(uint16_t maxValueSize);

	/// Validator that applies to metadata value notifications and validates that:
	/// - previous value size matches current state
	/// - value trunction is reversible
	DECLARE_STATEFUL_VALIDATOR(MetadataValue, model::MetadataValueNotification)();
}}
