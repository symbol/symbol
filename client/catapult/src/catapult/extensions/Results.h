#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace extensions {

#endif
/// Defines an extension validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_EXTENSION_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Extension, DESCRIPTION, CODE, None)

	/// Validation failed because the partial transaction was pruned from the temporal cache.
	DEFINE_EXTENSION_RESULT(Partial_Transaction_Cache_Prune, 0x0101);

	/// Validation failed because the partial transaction was pruned from the temporal cache due to its dependency being removed.
	DEFINE_EXTENSION_RESULT(Partial_Transaction_Dependency_Removed, 0x0102);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
