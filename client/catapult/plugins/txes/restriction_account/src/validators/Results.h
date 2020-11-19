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
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif

/// Defines a restriction account validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_RESTRICTION_ACCOUNT_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, RestrictionAccount, DESCRIPTION, CODE, None)

	/// Validation failed because the account restriction flags are invalid.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Invalid_Restriction_Flags, 1);

	/// Validation failed because a modification action is invalid.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Invalid_Modification_Action, 2);

	/// Validation failed because a modification address is invalid.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Invalid_Modification_Address, 3);

	/// Validation failed because the operation type is incompatible.
	/// \note This indicates that the existing restrictions have a different operation type than that specified in the notification.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Modification_Operation_Type_Incompatible, 4);

	/// Validation failed because a modification is redundant.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Redundant_Modification, 5);

	/// Validation failed because a value is not in the container.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Invalid_Modification, 6);

	/// Validation failed because the transaction has too many modifications.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Modification_Count_Exceeded, 7);

	/// Validation failed because the transaction has no modifications.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(No_Modifications, 8);

	/// Validation failed because the resulting account restriction has too many values.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Values_Count_Exceeded, 9);

	/// Validation failed because the account restriction value is invalid.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Invalid_Value, 10);

	/// Validation failed because the addresses involved in the transaction are not allowed to interact.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Address_Interaction_Prohibited, 11);

	/// Validation failed because the mosaic transfer is prohibited by the recipient.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Mosaic_Transfer_Prohibited, 12);

	/// Validation failed because the operation type is not allowed to be initiated by the signer.
	DEFINE_RESTRICTION_ACCOUNT_RESULT(Operation_Type_Prohibited, 13);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
