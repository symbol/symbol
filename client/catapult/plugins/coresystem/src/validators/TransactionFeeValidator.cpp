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
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionFeeNotification;

	DEFINE_STATELESS_VALIDATOR(TransactionFee, [](const Notification& notification) {
		if (notification.Fee > notification.MaxFee)
			return Failure_Core_Invalid_Transaction_Fee;

		constexpr auto Max_Raw_Block_Fee_Multiplier = static_cast<uint64_t>(std::numeric_limits<BlockFeeMultiplier::ValueType>::max());
		return notification.MaxFee.unwrap() > Max_Raw_Block_Fee_Multiplier * notification.TransactionSize
				? Failure_Core_Invalid_Transaction_Fee
				: ValidationResult::Success;
	})
}}
