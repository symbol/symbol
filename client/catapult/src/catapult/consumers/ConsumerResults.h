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

namespace catapult { namespace consumers {

#endif
/// Defines a failure consumer validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CONSUMER_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Consumer, DESCRIPTION, CODE, None)

/// Defines a neutral consumer validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_NEUTRAL_CONSUMER_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Neutral, Consumer, DESCRIPTION, CODE, Verbose)

	/// Validation failed because the consumer input is empty.
	DEFINE_CONSUMER_RESULT(Empty_Input, 1);

	/// Validation failed because the block transactions hash does not match the calculated value.
	DEFINE_CONSUMER_RESULT(Block_Transactions_Hash_Mismatch, 2);

	/// Validation failed because an entity hash is present in the recency cache.
	DEFINE_NEUTRAL_CONSUMER_RESULT(Hash_In_Recency_Cache, 3);

	/// Validation failed because the chain is internally improperly linked.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Improper_Link, 4);

	/// Validation failed because the chain part contains duplicate transactions.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Duplicate_Transactions, 5);

	/// Validation failed because the chain part does not link to the current chain.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Unlinked, 6);

	/// Validation failed because the remote chain difficulties do not match the calculated difficulties.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Difficulties_Mismatch, 7);

	/// Validation failed because the remote chain score is not better.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Score_Not_Better, 8);

	/// Validation failed because the remote chain is too far behind.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Too_Far_Behind, 9);

	/// Validation failed because the remote chain timestamp is too far in the future.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Too_Far_In_Future, 10);

	/// Validation failed because the verification of the signature failed during a batch operation.
	DEFINE_CONSUMER_RESULT(Batch_Signature_Not_Verifiable, 11);

	/// Validation failed because the chain has an improper importance link.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Improper_Importance_Link, 12);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
