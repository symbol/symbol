#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {
#endif
	/// Validation failed because the entity hash is already known.
	DEFINE_VALIDATION_RESULT(Failure, Hash, Exists, 7, Verbose);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
