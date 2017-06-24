#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {
#endif
/// Defines a transfer validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_TRANSFER_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Transfer, DESCRIPTION, CODE, None)

	/// Validation failed because the message is too large.
	DEFINE_TRANSFER_RESULT(Message_Too_Large, 6);

	/// Validation failed because mosaics are out of order.
	DEFINE_TRANSFER_RESULT(Out_Of_Order_Mosaics, 200);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
