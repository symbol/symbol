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
#include "AccountPropertyView.h"
#include "src/cache/PropertyCache.h"
#include "src/model/PropertyTransaction.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyTransactionTypePropertyValueNotification;

	namespace {
		constexpr auto Relevant_Entity_Type = model::TransactionTypePropertyTransaction::Entity_Type;

		bool Validate(const Notification& notification, const ValidatorContext& context) {
			AccountPropertyView view(context.Cache);
			auto isRelevantEntityType = Relevant_Entity_Type == notification.Modification.Value;
			auto isAllow = state::OperationType::Allow == notification.PropertyDescriptor.operationType();

			// cannot delete relevant entity type for operation type Allow
			if (model::PropertyModificationType::Del == notification.Modification.ModificationType)
				return !(isAllow && isRelevantEntityType);

			size_t numTypedProperties = 0;
			if (view.initialize(model::PublicKeyToAddress(notification.Key, context.Network.Identifier))) {
				auto typedProperty = view.get<model::EntityType>(model::PropertyType::TransactionType);
				numTypedProperties = typedProperty.size();
			}

			// Adding a value to an account properties should only be allowed for
			// - operation type Allow: if it is the relevant entity type or the relevant entity type is already contained
			// - operation type Block: if it is not the relevant entity type
			auto isAllowAndForbidden = isAllow && !isRelevantEntityType && 0 == numTypedProperties;
			auto isBlockAndForbidden = !isAllow && isRelevantEntityType;
			return !(isAllowAndForbidden || isBlockAndForbidden);
		}
	}

	DEFINE_STATEFUL_VALIDATOR(TransactionTypeNoSelfBlocking, [](const auto& notification, const ValidatorContext& context) {
		return Validate(notification, context) ? ValidationResult::Success : Failure_Property_Modification_Not_Allowed;
	});
}}
