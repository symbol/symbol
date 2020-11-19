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
#include "src/model/AccountLinkNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to remote account key link notifications and validates that:
	/// - link action is consistent with current state
	/// - only main account can unlink
	/// - unlink data matches current state
	DECLARE_STATEFUL_VALIDATOR(AccountKeyLink, model::RemoteAccountKeyLinkNotification)();

	/// Validator that applies to new remote account notifications and validates that:
	/// - remote account does not exist
	DECLARE_STATEFUL_VALIDATOR(NewRemoteAccountAvailability, model::NewRemoteAccountNotification)();

	/// Validator that applies to transaction notifications and validates that:
	/// - remote account is not the transaction signer
	DECLARE_STATEFUL_VALIDATOR(RemoteSender, model::TransactionNotification)();

	/// Validator that applies to address interaction notifications and validates that:
	/// - remote account is allowed to participate in the interaction
	DECLARE_STATEFUL_VALIDATOR(RemoteInteraction, model::AddressInteractionNotification)();
}}
