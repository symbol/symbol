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
/// Defines an aggregate validation result with \a DESCRIPTION and \a CODE.
#define DEFINE_AGGREGATE_RESULT(DESCRIPTION, CODE) DEFINE_VALIDATION_RESULT(Failure, Aggregate, DESCRIPTION, CODE, None)

	/// Validation failed because aggregate has too many transactions.
	DEFINE_AGGREGATE_RESULT(Too_Many_Transactions, 1);

	/// Validation failed because aggregate does not have any transactions.
	DEFINE_AGGREGATE_RESULT(No_Transactions, 2);

	/// Validation failed because aggregate has too many cosignatures.
	DEFINE_AGGREGATE_RESULT(Too_Many_Cosignatures, 3);

	/// Validation failed because redundant cosignatures are present.
	DEFINE_AGGREGATE_RESULT(Redundant_Cosignatures, 4);

	/// Validation failed because at least one cosignatory is ineligible.
	DEFINE_AGGREGATE_RESULT(Ineligible_Cosignatories, 5);

	/// Validation failed because at least one required cosignature is missing.
	DEFINE_AGGREGATE_RESULT(Missing_Cosignatures, 6);

	/// Validation failed because the aggregate transactions hash does not match the calculated value.
	DEFINE_AGGREGATE_RESULT(Transactions_Hash_Mismatch, 7);

#ifndef CUSTOM_RESULT_DEFINITION
}}
#endif
