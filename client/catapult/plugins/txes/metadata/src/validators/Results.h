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
/// Defines a metadata validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_METADATA_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Metadata, DESCRIPTION, CODE, None)

	/// Validation failed because the metadata value is too small.
	DEFINE_METADATA_RESULT(Value_Too_Small, 1);

	/// Validation failed because the metadata value is too large.
	DEFINE_METADATA_RESULT(Value_Too_Large, 2);

	/// Validation failed because the metadata value size delta is larger in magnitude than the value size.
	DEFINE_METADATA_RESULT(Value_Size_Delta_Too_Large, 3);

	/// Validation failed because the metadata value size delta does not match expected value based on the current state.
	DEFINE_METADATA_RESULT(Value_Size_Delta_Mismatch, 4);

	/// Validation failed because a metadata value change (truncation) is irreversible.
	DEFINE_METADATA_RESULT(Value_Change_Irreversible, 5);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
