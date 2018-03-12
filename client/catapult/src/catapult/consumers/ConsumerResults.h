#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace consumers {

#endif
/// Defines a consumer validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CONSUMER_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Consumer, DESCRIPTION, CODE, None)

/// Defines a verbose consumer validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CONSUMER_RESULT_VERBOSE(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Consumer, DESCRIPTION, CODE, Verbose)

	/// Validation failed because the consumer input is empty.
	DEFINE_CONSUMER_RESULT(Empty_Input, 0x0001);

	/// Validation failed because the block transactions hash does not match the calculated value.
	DEFINE_CONSUMER_RESULT(Block_Transactions_Hash_Mismatch, 0x1001);

	/// Validation failed because an entity hash is present in the recency cache.
	DEFINE_CONSUMER_RESULT_VERBOSE(Hash_In_Recency_Cache, 0x1002);

	/// Validation failed because the chain part has too many blocks.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Too_Many_Blocks, 0x2001);

	/// Validation failed because the chain is internally improperly linked.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Improper_Link, 0x2002);

	/// Validation failed because the chain part contains duplicate transactions.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Duplicate_Transactions, 0x2003);

	/// Validation failed because the chain part does not link to the current chain.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Unlinked, 0x3001);

	/// Validation failed because the remote chain difficulties do not match the calculated difficulties.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Mismatched_Difficulties, 0x3002);

	/// Validation failed because the remote chain score is not better.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Score_Not_Better, 0x3003);

	/// Validation failed because the remote chain is too far behind.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Too_Far_Behind, 0x3004);

	/// Validation failed because the remote chain timestamp is too far in the future.
	DEFINE_CONSUMER_RESULT(Remote_Chain_Too_Far_In_Future, 0x3005);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
