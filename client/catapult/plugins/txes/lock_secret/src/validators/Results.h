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

/// Defines a lock secret validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_LOCKSECRET_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, LockSecret, DESCRIPTION, CODE, None)

	/// Validation failed because hash algorithm for lock type secret is invalid.
	DEFINE_LOCKSECRET_RESULT(Invalid_Hash_Algorithm, 1);

	/// Validation failed because hash is already present in cache.
	DEFINE_LOCKSECRET_RESULT(Hash_Exists, 2);

	/// Validation failed because hash is not implemented yet.
	DEFINE_LOCKSECRET_RESULT(Hash_Not_Implemented, 3);

	/// Validation failed because proof is too small or too large.
	DEFINE_LOCKSECRET_RESULT(Proof_Size_Out_Of_Bounds, 4);

	/// Validation failed because secret does not match proof.
	DEFINE_LOCKSECRET_RESULT(Secret_Mismatch, 5);

	/// Validation failed because secret is unknown.
	DEFINE_LOCKSECRET_RESULT(Unknown_Secret, 6);

	/// Validation failed because secret is inactive.
	DEFINE_LOCKSECRET_RESULT(Inactive_Secret, 7);

	/// Validation failed because hash algorithm does not match.
	DEFINE_LOCKSECRET_RESULT(Hash_Algorithm_Mismatch, 8);

	/// Validation failed because duration is too long.
	DEFINE_LOCKSECRET_RESULT(Invalid_Duration, 9);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
