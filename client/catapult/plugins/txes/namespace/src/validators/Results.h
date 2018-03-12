#pragma once
#ifndef CUSTOM_RESULT_DEFINITION
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

#endif
/// Defines a namespace validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_NAMESPACE_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Namespace, DESCRIPTION, CODE, None)

/// Defines a mosaic validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_MOSAIC_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Mosaic, DESCRIPTION, CODE, None)

/// Defines a namespace and mosaic validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_NAMESPACE_MOSAIC_RESULT(DESCRIPTION, CODE) \
	DEFINE_NAMESPACE_RESULT(DESCRIPTION, CODE); \
	DEFINE_MOSAIC_RESULT(DESCRIPTION, CODE)

	// region common

	/// Validation failed because the duration has an invalid value.
	DEFINE_NAMESPACE_MOSAIC_RESULT(Invalid_Duration, 1);

	/// Validation failed because the name is invalid.
	DEFINE_NAMESPACE_MOSAIC_RESULT(Invalid_Name, 2);

	/// Validation failed because the name and id don't match.
	DEFINE_NAMESPACE_MOSAIC_RESULT(Name_Id_Mismatch, 3);

	/// Validation failed because the parent is expired.
	DEFINE_NAMESPACE_MOSAIC_RESULT(Expired, 4);

	/// Validation failed because the parent owner conflicts with the child owner.
	DEFINE_NAMESPACE_MOSAIC_RESULT(Owner_Conflict, 5);

	// endregion

	// region namespace

	/// Validation failed because the namespace type is invalid.
	DEFINE_NAMESPACE_RESULT(Invalid_Namespace_Type, 100);

	/// Validation failed because the root namespace has a reserved name.
	DEFINE_NAMESPACE_RESULT(Root_Name_Reserved, 101);

	/// Validation failed because the resulting namespace would exceed the maximum allowed namespace depth.
	DEFINE_NAMESPACE_RESULT(Too_Deep, 102);

	/// Validation failed because the namespace parent is unknown.
	DEFINE_NAMESPACE_RESULT(Parent_Unknown, 103);

	/// Validation failed because the namespace already exists.
	DEFINE_NAMESPACE_RESULT(Already_Exists, 150);

	/// Validation failed because the namespace is already active.
	DEFINE_NAMESPACE_RESULT(Already_Active, 151);

	/// Validation failed because an eternal namespace was received after the nemesis block.
	DEFINE_NAMESPACE_RESULT(Eternal_After_Nemesis_Block, 152);

	/// Validation failed because the maximum number of children for a root namespace was exceeded.
	DEFINE_NAMESPACE_RESULT(Max_Children_Exceeded, 153);

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

	/// Validation failed because the mosaic has a reserved name.
	DEFINE_MOSAIC_RESULT(Name_Reserved, 106);

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

	/// Validation failed because the credit of the mosaic would exceed the maximimun of different mosaics an account is allowed to own.
	DEFINE_MOSAIC_RESULT(Max_Mosaics_Exceeded, 170);

	// endregion

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
