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

namespace catapult { namespace validators {

#endif

/// Defines a restriction mosaic validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_RESTRICTION_MOSAIC_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, RestrictionMosaic, DESCRIPTION, CODE, None)

	/// Validation failed because the mosaic restriction type is invalid.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Invalid_Restriction_Type, 1);

	/// Validation failed because specified previous value does not match current value.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Previous_Value_Mismatch, 2);

	/// Validation failed because specified previous value is nonzero.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Previous_Value_Must_Be_Zero, 3);

	/// Validation failed because the maximum number of restrictions would be exeeded.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Max_Restrictions_Exceeded, 4);

	/// Validation failed because nonexistent restriction cannot be deleted.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Cannot_Delete_Nonexistent_Restriction, 5);

	/// Validation failed because required global restriction does not exist.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Unknown_Global_Restriction, 6);

	/// Validation failed because mosaic has invalid global restriction.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Invalid_Global_Restriction, 7);

	/// Validation failed because account lacks proper permissions to move mosaic.
	DEFINE_RESTRICTION_MOSAIC_RESULT(Account_Unauthorized, 8);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
