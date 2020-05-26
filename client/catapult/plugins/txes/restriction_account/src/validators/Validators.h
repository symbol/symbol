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

#pragma once
#include "Results.h"
#include "src/model/AccountRestrictionNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

#define DECLARE_SHARED_VALIDATORS(VALUE_NAME) \
	/* Validator that applies to account restriction notifications and validates that: */ \
	/* - there is no redundant restriction modification */ \
	DECLARE_STATEFUL_VALIDATOR( \
			Account##VALUE_NAME##RestrictionRedundantModification, \
			model::ModifyAccount##VALUE_NAME##RestrictionsNotification)(); \
	\
	/* Validator that applies to account restriction value notifications and validates that: */ \
	/* - add modification does not add a known value */ \
	/* - delete modification does not delete an unknown value */ \
	DECLARE_STATEFUL_VALIDATOR( \
			Account##VALUE_NAME##RestrictionValueModification, \
			model::ModifyAccount##VALUE_NAME##RestrictionValueNotification)(); \
	\
	/* Validator that applies to account restriction notifications and validates that: */ \
	/* - the maximum number of modifications (\a maxAccountRestrictionValues) is not exceeded */ \
	/* - the maximum number of account restriction values (\a maxAccountRestrictionValues) is not exeeded */ \
	DECLARE_STATEFUL_VALIDATOR(MaxAccount##VALUE_NAME##RestrictionValues, model::ModifyAccount##VALUE_NAME##RestrictionsNotification)( \
			uint16_t maxAccountRestrictionValues);

	DECLARE_SHARED_VALIDATORS(Address)
	DECLARE_SHARED_VALIDATORS(Mosaic)
	DECLARE_SHARED_VALIDATORS(Operation)

	/// Validator that applies to account restriction modification notifications and validates that:
	/// - restriction flags are known
	DECLARE_STATELESS_VALIDATOR(AccountRestrictionFlags, model::AccountRestrictionModificationNotification)();

	/// Validator that applies to account restriction modification notifications and validates that:
	/// - there is at least one restriction modification
	DECLARE_STATELESS_VALIDATOR(AccountRestrictionModificationPresent, model::AccountRestrictionModificationNotification)();

	/// Validator that applies to account address restriction value notifications and validates that:
	/// - modification value is valid
	DECLARE_STATEFUL_VALIDATOR(AccountAddressRestrictionNoSelfModification, model::ModifyAccountAddressRestrictionValueNotification)();

	/// Validator that applies to address interaction notifications and validates that:
	/// - the source address is allowed to interact with all participant addresses
	DECLARE_STATEFUL_VALIDATOR(AddressInteraction, model::AddressInteractionNotification)();

	/// Validator that applies to balance transfer notifications and validates that:
	/// - the mosaic is allowed to be transferred to the recipient
	DECLARE_STATEFUL_VALIDATOR(MosaicRecipient, model::BalanceTransferNotification)();

	/// Validator that applies to account transaction restriction notifications and validates that:
	/// - all transaction modification values are valid
	DECLARE_STATELESS_VALIDATOR(AccountOperationRestrictionModificationValues, model::ModifyAccountOperationRestrictionsNotification)();

	/// Validator that applies to transaction notifications and validates that:
	/// - the signer is allowed to initiate a transaction of the specified transaction type
	DECLARE_STATEFUL_VALIDATOR(OperationRestriction, model::TransactionNotification)();

	/// Validator that applies to account transaction restriction notifications and validates that:
	/// - account operation restriction transactions are not blocked
	DECLARE_STATEFUL_VALIDATOR(AccountOperationRestrictionNoSelfBlocking, model::ModifyAccountOperationRestrictionValueNotification)();
}}
