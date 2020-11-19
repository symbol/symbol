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

/// Defines a lock secret validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_LOCK_SECRET_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, LockSecret, DESCRIPTION, CODE, None)

	/// Validation failed because hash algorithm for lock type secret is invalid.
	DEFINE_LOCK_SECRET_RESULT(Invalid_Hash_Algorithm, 1);

	/// Validation failed because hash is already present in cache.
	DEFINE_LOCK_SECRET_RESULT(Hash_Already_Exists, 2);

	/// Validation failed because proof is too small or too large.
	DEFINE_LOCK_SECRET_RESULT(Proof_Size_Out_Of_Bounds, 3);

	/// Validation failed because secret does not match proof.
	DEFINE_LOCK_SECRET_RESULT(Secret_Mismatch, 4);

	/// Validation failed because composite key is unknown.
	DEFINE_LOCK_SECRET_RESULT(Unknown_Composite_Key, 5);

	/// Validation failed because secret is inactive.
	DEFINE_LOCK_SECRET_RESULT(Inactive_Secret, 6);

	/// Validation failed because hash algorithm does not match.
	DEFINE_LOCK_SECRET_RESULT(Hash_Algorithm_Mismatch, 7);

	/// Validation failed because duration is too long.
	DEFINE_LOCK_SECRET_RESULT(Invalid_Duration, 8);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
