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
/// Defines an account link validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_ACCOUNT_LINK_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, AccountLink, DESCRIPTION, CODE, None)

	/// Validation failed because main account is already linked to another account.
	DEFINE_ACCOUNT_LINK_RESULT(Link_Already_Exists, 1);

	/// Validation failed because unlink data is not consistent with existing account link.
	DEFINE_ACCOUNT_LINK_RESULT(Inconsistent_Unlink_Data, 2);

	/// Validation failed because main account is not linked to another account.
	DEFINE_ACCOUNT_LINK_RESULT(Unknown_Link, 3);

	/// Validation failed because link is attempting to convert ineligible account to remote.
	DEFINE_ACCOUNT_LINK_RESULT(Remote_Account_Ineligible, 4);

	/// Validation failed because remote is not allowed to sign a transaction.
	DEFINE_ACCOUNT_LINK_RESULT(Remote_Account_Signer_Prohibited, 5);

	/// Validation failed because remote is not allowed to participate in the transaction.
	DEFINE_ACCOUNT_LINK_RESULT(Remote_Account_Participant_Prohibited, 6);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
