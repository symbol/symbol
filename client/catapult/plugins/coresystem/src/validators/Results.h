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

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
