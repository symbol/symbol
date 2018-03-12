#pragma once
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace test {

	/// Creates a validation result with \a severity and \a code.
	constexpr validators::ValidationResult MakeValidationResult(validators::ResultSeverity severity, uint16_t code) {
		return MakeValidationResult(severity, validators::FacilityCode::Chain, code, validators::ResultFlags::None);
	}
}}
