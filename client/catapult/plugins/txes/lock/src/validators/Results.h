#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif

/// Defines a lock validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_LOCK_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Lock, DESCRIPTION, CODE, None)

	/// Validation failed because hash algorithm for lock type secret is invalid.
	DEFINE_LOCK_RESULT(Invalid_Hash_Algorithm, 1);

	/// Validation failed because lock does not allow the specified mosaic.
	DEFINE_LOCK_RESULT(Invalid_Mosaic_Id, 2);

	/// Validation failed because lock does not allow the specified amount.
	DEFINE_LOCK_RESULT(Invalid_Mosaic_Amount, 3);

	/// Validation failed because hash is already present in cache.
	DEFINE_LOCK_RESULT(Hash_Exists, 4);

	/// Validation failed because hash is not present in cache.
	DEFINE_LOCK_RESULT(Hash_Does_Not_Exist, 5);

	/// Validation failed because hash is not 0 padded.
	DEFINE_LOCK_RESULT(Missing_Secret_Padding, 6);

	/// Validation failed because hash is not implemented yet.
	DEFINE_LOCK_RESULT(Hash_Not_Implemented, 7);

	/// Validation failed because proof is too small or too large.
	DEFINE_LOCK_RESULT(Proof_Size_Out_Of_Bounds, 8);

	/// Validation failed because secret does not match proof.
	DEFINE_LOCK_RESULT(Secret_Mismatch, 9);

	/// Validation failed because secret is unknown.
	DEFINE_LOCK_RESULT(Unknown_Secret, 10);

	/// Validation failed because hash is inactive.
	DEFINE_LOCK_RESULT(Inactive_Hash, 11);

	/// Validation failed because secret is inactive.
	DEFINE_LOCK_RESULT(Inactive_Secret, 12);

	/// Validation failed because hash algorithm does not match.
	DEFINE_LOCK_RESULT(Hash_Algorithm_Mismatch, 13);

	/// Validation failed because hash has already been used.
	DEFINE_LOCK_RESULT(Hash_Already_Used, 14);

	/// Validation failed because secret has already been used.
	DEFINE_LOCK_RESULT(Secret_Already_Used, 15);

	/// Validation failed because duration is too long.
	DEFINE_LOCK_RESULT(Invalid_Duration, 16);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
