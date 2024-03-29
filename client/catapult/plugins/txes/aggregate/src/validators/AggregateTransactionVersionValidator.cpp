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
#include "src/model/AggregateEntityType.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::EntityNotification;

	namespace {
		constexpr bool IsAggregate(model::EntityType entityType) {
			return model::Entity_Type_Aggregate_Bonded == entityType || model::Entity_Type_Aggregate_Complete == entityType;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(AggregateTransactionVersion, Notification)(Height v2ForkHeight) {
		return MAKE_STATEFUL_VALIDATOR(AggregateTransactionVersion, ([v2ForkHeight](
				const Notification& notification,
				const ValidatorContext& context) {
			if (!IsAggregate(notification.EntityType))
				return ValidationResult::Success;

			if (context.Height < v2ForkHeight)
				return 1 == notification.EntityVersion ? ValidationResult::Success : Failure_Aggregate_V2_Prohibited;
			else
				return 2 <= notification.EntityVersion ? ValidationResult::Success : Failure_Aggregate_V1_Prohibited;
		}));
	}
}}
