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
#include "catapult/validators/ValidatorContext.h"
#include "catapult/types.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionDeadlineNotification;

	namespace {
		auto ValidateTransactionDeadline(Timestamp timestamp, Timestamp deadline, const utils::TimeSpan& maxTransactionLifetime) {
			if (timestamp > deadline)
				return Failure_Core_Past_Deadline;

			return deadline > timestamp + maxTransactionLifetime ? Failure_Core_Future_Deadline : ValidationResult::Success;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(Deadline, Notification)(const utils::TimeSpan& maxTransactionLifetime) {
		return MAKE_STATEFUL_VALIDATOR(Deadline, [maxTransactionLifetime](
				const Notification& notification,
				const ValidatorContext& context) {
			return ValidateTransactionDeadline(
					context.BlockTime,
					notification.Deadline,
					utils::TimeSpan() == notification.MaxLifetime ? maxTransactionLifetime : notification.MaxLifetime);
		});
	}
}}
