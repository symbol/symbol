#pragma once
#include "ValidatorTypes.h"
#include <functional>

namespace catapult { namespace validators {

	using SequentialValidationPolicyFunc = ValidationPolicyFunc<ValidationResult>;

	/// Creates a sequential validation policy.
	SequentialValidationPolicyFunc CreateSequentialValidationPolicy();
}}
