#pragma once
#include "Results.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to all signature notifications and validates that:
	/// - signatures are valid
	DECLARE_STATELESS_VALIDATOR(Signature, model::SignatureNotification)();
}}
