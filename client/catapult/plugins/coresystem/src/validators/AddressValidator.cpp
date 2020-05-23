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
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::AccountAddressNotification;

	DEFINE_STATEFUL_VALIDATOR(Address, [](const Notification& notification, const ValidatorContext& context) {
		auto networkIdentifier = context.Network.Identifier;
		auto address = notification.Address.resolved(context.Resolvers);
		if (utils::to_underlying_type(networkIdentifier) != (address[0] & 0xFE))
			return Failure_Core_Invalid_Address;

		auto isValidAddress = model::IsValidAddress(address, networkIdentifier);
		return isValidAddress ? ValidationResult::Success : Failure_Core_Invalid_Address;
	});
}}
