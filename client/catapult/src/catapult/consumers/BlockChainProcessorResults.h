#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace consumers {
#endif
/// Defines a chain validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_CHAIN_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Chain, DESCRIPTION, CODE, None)

	/// Validation failed because a block was received that did not link with the existing chain.
	DEFINE_CHAIN_RESULT(Unlinked, 102);

	/// Validation failed because a block was received that is not a hit.
	DEFINE_CHAIN_RESULT(Block_Not_Hit, 104);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
