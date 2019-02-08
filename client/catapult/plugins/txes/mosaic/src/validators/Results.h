/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
/// Defines a mosaic validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_MOSAIC_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Mosaic, DESCRIPTION, CODE, None)

	// region common

	/// Validation failed because the duration has an invalid value.
	DEFINE_MOSAIC_RESULT(Invalid_Duration, 1);

	/// Validation failed because the name is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Name, 2);

	/// Validation failed because the name and id don't match.
	DEFINE_MOSAIC_RESULT(Name_Id_Mismatch, 3);

	/// Validation failed because the parent is expired.
	DEFINE_MOSAIC_RESULT(Expired, 4);

	/// Validation failed because the parent owner conflicts with the child owner.
	DEFINE_MOSAIC_RESULT(Owner_Conflict, 5);

	/// Validation failed because the id is not the expected id generated from signer and nonce.
	DEFINE_MOSAIC_RESULT(Id_Mismatch, 6);

	// endregion

	// region mosaic

	/// Validation failed because the existing parent id does not match the supplied parent id.
	DEFINE_MOSAIC_RESULT(Parent_Id_Conflict, 100);

	/// Validation failed because a mosaic property is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Property, 101);

	/// Validation failed because the mosaic flags are invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Flags, 102);

	/// Validation failed because the mosaic divisibility is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Divisibility, 103);

	/// Validation failed because the mosaic supply change direction is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Supply_Change_Direction, 104);

	/// Validation failed because the mosaic supply change amount is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Supply_Change_Amount, 105);

	/// Validation failed because the mosaic id is invalid.
	DEFINE_MOSAIC_RESULT(Invalid_Id, 107);

	/// Validation failed because mosaic modification is not allowed.
	DEFINE_MOSAIC_RESULT(Modification_Disallowed, 150);

	/// Validation failed because mosaic modification would not result in any changes.
	DEFINE_MOSAIC_RESULT(Modification_No_Changes, 151);

	/// Validation failed because the mosaic supply is immutable.
	DEFINE_MOSAIC_RESULT(Supply_Immutable, 161);

	/// Validation failed because the resulting mosaic supply is negative.
	DEFINE_MOSAIC_RESULT(Supply_Negative, 162);

	/// Validation failed because the resulting mosaic supply exceeds the maximum allowed value.
	DEFINE_MOSAIC_RESULT(Supply_Exceeded, 163);

	/// Validation failed because the mosaic is not transferable.
	DEFINE_MOSAIC_RESULT(Non_Transferable, 164);

	/// Validation failed because the credit of the mosaic would exceed the maximum of different mosaics an account is allowed to own.
	DEFINE_MOSAIC_RESULT(Max_Mosaics_Exceeded, 170);

	// endregion

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
