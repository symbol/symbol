#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif
/// Defines a core validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CORE_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Core, DESCRIPTION, CODE, None)

	/// Validation failed because the deadline passed.
	DEFINE_CORE_RESULT(Past_Deadline, 3);

	/// Validation failed because the deadline is too far in the future.
	DEFINE_CORE_RESULT(Future_Deadline, 4);

	/// Validation failed because the account has an insufficient balance.
	DEFINE_CORE_RESULT(Insufficient_Balance, 5);

	/// Validation failed because there are too many transactions in a block.
	DEFINE_CORE_RESULT(Too_Many_Transactions, 15);

	/// Validation failed because an entity originated from the nemesis account after the nemesis block.
	DEFINE_CORE_RESULT(Nemesis_Account_Signed_After_Nemesis_Block, 18);

	/// Validation failed because the entity has the wrong network specified.
	DEFINE_CORE_RESULT(Wrong_Network, 20);

	/// Validation failed because an address is invalid.
	DEFINE_CORE_RESULT(Invalid_Address, 21);

	/// Validation failed because a block was harvested by an ineligible harvester.
	DEFINE_CORE_RESULT(Block_Harvester_Ineligible, 105);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
