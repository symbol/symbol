#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif
/// Defines a signature validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_SIGNATURE_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Signature, DESCRIPTION, CODE, None)

	/// Validation failed because the verification of the signature failed.
	DEFINE_SIGNATURE_RESULT(Not_Verifiable, 8);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
