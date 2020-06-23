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
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif
/// Defines a core validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CORE_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Core, DESCRIPTION, CODE, None)

	/// Validation failed because the deadline passed.
	DEFINE_CORE_RESULT(Past_Deadline, 1);

	/// Validation failed because the deadline is too far in the future.
	DEFINE_CORE_RESULT(Future_Deadline, 2);

	/// Validation failed because the account has an insufficient balance.
	DEFINE_CORE_RESULT(Insufficient_Balance, 3);

	/// Validation failed because there are too many transactions in a block.
	DEFINE_CORE_RESULT(Too_Many_Transactions, 4);

	/// Validation failed because an entity originated from the nemesis account after the nemesis block.
	DEFINE_CORE_RESULT(Nemesis_Account_Signed_After_Nemesis_Block, 5);

	/// Validation failed because the entity has the wrong network specified.
	DEFINE_CORE_RESULT(Wrong_Network, 6);

	/// Validation failed because an address is invalid.
	DEFINE_CORE_RESULT(Invalid_Address, 7);

	/// Validation failed because entity version is invalid.
	DEFINE_CORE_RESULT(Invalid_Version, 8);

	/// Validation failed because a transaction fee is invalid.
	DEFINE_CORE_RESULT(Invalid_Transaction_Fee, 9);

	/// Validation failed because a block was harvested by an ineligible harvester.
	DEFINE_CORE_RESULT(Block_Harvester_Ineligible, 10);

	/// Validation failed because an address is zero.
	DEFINE_CORE_RESULT(Zero_Address, 11);

	/// Validation failed because a public key is zero.
	DEFINE_CORE_RESULT(Zero_Public_Key, 12);

	/// Validation failed because internal padding is nonzero.
	DEFINE_CORE_RESULT(Nonzero_Internal_Padding, 13);

	/// Validation failed because an address collision is detected.
	DEFINE_CORE_RESULT(Address_Collision, 14);

	// region key link results

	/// Validation failed because link action is invalid.
	DEFINE_CORE_RESULT(Invalid_Link_Action, 101);

	/// Validation failed because main account is already linked to another account.
	DEFINE_CORE_RESULT(Link_Already_Exists, 102);

	/// Validation failed because unlink data is not consistent with existing account link.
	DEFINE_CORE_RESULT(Inconsistent_Unlink_Data, 103);

	/// Validation failed because link range is invalid.
	DEFINE_CORE_RESULT(Invalid_Link_Range, 104);

	/// Validation failed because main account has too many links of the specified type.
	DEFINE_CORE_RESULT(Too_Many_Links, 105);

	// endregion

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
