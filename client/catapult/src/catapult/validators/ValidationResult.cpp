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

#include "ValidationResult.h"
#include "catapult/utils/HexFormatter.h"
#include <iostream>

namespace catapult { namespace validators {

#define DEFINE_CASE(RESULT) case utils::to_underlying_type(RESULT)

#define CASE_WELL_KNOWN_RESULT(CODE) DEFINE_CASE(ValidationResult::CODE): return #CODE

#define CUSTOM_RESULT_DEFINITION 1
#undef DEFINE_VALIDATION_RESULT

#define STR(SYMBOL) #SYMBOL
#define DEFINE_VALIDATION_RESULT(SEVERITY, FACILITY, DESCRIPTION, CODE, FLAGS) \
		DEFINE_CASE(MakeValidationResult((ResultSeverity::SEVERITY), (FacilityCode::FACILITY), CODE, (ResultFlags::FLAGS))): \
			return STR(SEVERITY##_##FACILITY##_##DESCRIPTION)

	namespace {
		const char* ToString(ValidationResult result) {
			switch (utils::to_underlying_type(result)) {
			// well known results (defined in enum)
			CASE_WELL_KNOWN_RESULT(Success);
			CASE_WELL_KNOWN_RESULT(Neutral);
			CASE_WELL_KNOWN_RESULT(Failure);

			// custom plugin results
			#include "plugins/coresystem/src/validators/Results.h"
			#include "plugins/services/hashcache/src/validators/Results.h"
			#include "plugins/services/signature/src/validators/Results.h"
			#include "plugins/txes/account_link/src/validators/Results.h"
			#include "plugins/txes/aggregate/src/validators/Results.h"
			#include "plugins/txes/lock_hash/src/validators/Results.h"
			#include "plugins/txes/lock_secret/src/validators/Results.h"
			#include "plugins/txes/metadata/src/validators/Results.h"
			#include "plugins/txes/mosaic/src/validators/Results.h"
			#include "plugins/txes/multisig/src/validators/Results.h"
			#include "plugins/txes/namespace/src/validators/Results.h"
			#include "plugins/txes/restriction_account/src/validators/Results.h"
			#include "plugins/txes/restriction_mosaic/src/validators/Results.h"
			#include "plugins/txes/transfer/src/validators/Results.h"
			#include "src/catapult/chain/ChainResults.h"
			#include "src/catapult/consumers/ConsumerResults.h"
			#include "src/catapult/extensions/Results.h"
			}

			return nullptr;
		}
	}

	std::ostream& operator<<(std::ostream& out, ValidationResult result) {
		auto pStr = ToString(result);
		if (pStr)
			out << pStr;
		else
			out << "ValidationResult<0x" << utils::HexFormat(utils::to_underlying_type(result)) << ">";

		return out;
	}
}}
