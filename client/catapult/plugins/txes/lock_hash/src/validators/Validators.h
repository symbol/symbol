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
#include "src/model/HashLockNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to hash lock notifications and validates that:
	/// - lock duration is at most \a maxHashLockDuration
	DECLARE_STATELESS_VALIDATOR(HashLockDuration, model::HashLockDurationNotification)(BlockDuration maxHashLockDuration);

	/// Validator that applies to hash lock mosaic notifications and validates that:
	/// - mosaic id is \a currencyMosaicId
	/// - mosaic amount is equal to \a lockedFundsPerAggregate
	DECLARE_STATEFUL_VALIDATOR(HashLockMosaic, model::HashLockMosaicNotification)(
			MosaicId currencyMosaicId,
			Amount lockedFundsPerAggregate);

	/// Validator that applies to hash lock notifications and validates that:
	/// - attached hash is not present in hash lock info cache
	DECLARE_STATEFUL_VALIDATOR(HashLockCacheUnique, model::HashLockNotification)();

	/// Validator that applies to transaction notifications and validates that:
	/// - incomplete aggregate transactions must have an active, unused hash lock info present in cache
	DECLARE_STATEFUL_VALIDATOR(AggregateHashPresent, model::TransactionNotification)();
}}
