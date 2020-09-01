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
#include <iosfwd>

namespace catapult {
	namespace model {
		class FinalizationContext;
		struct FinalizationProof;
	}
}

namespace catapult { namespace chain {

	// region VerifyFinalizationProofResult

#define VERIFY_FINALIZATION_PROOF_RESULT_LIST \
	/* Proof version is not supported. */ \
	ENUM_VALUE(Failure_Invalid_Version) \
	\
	/* Proof point does not match context. */ \
	ENUM_VALUE(Failure_Invalid_Point) \
	\
	/* Proof height is invalid. */ \
	ENUM_VALUE(Failure_Invalid_Height) \
	\
	/* Proof hash is invalid. */ \
	ENUM_VALUE(Failure_Invalid_Hash) \
	\
	/* Proof does not represent a valid precommit. */ \
	ENUM_VALUE(Failure_No_Precommit) \
	\
	/* Proof contains an invalid message. */ \
	ENUM_VALUE(Failure_Invalid_Messsage) \
	\
	/* Proof was successfully verified. */ \
	ENUM_VALUE(Success)

#define ENUM_VALUE(LABEL) LABEL,
	/// Verify finalization proof results.
	enum class VerifyFinalizationProofResult {
		VERIFY_FINALIZATION_PROOF_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, VerifyFinalizationProofResult value);

	// endregion

	/// Verifies \a proof given \a context.
	VerifyFinalizationProofResult VerifyFinalizationProof(
			const model::FinalizationProof& proof,
			const model::FinalizationContext& context);
}}
