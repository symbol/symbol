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
#include "catapult/crypto_voting/OtsTypes.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	// region step identifier

	/// Finalization stages.
	enum class FinalizationStage : uint64_t {
		/// Prevote stage.
		Prevote,

		/// Precommit stage.
		Precommit,

		/// Number of stages.
		Count
	};

	/// Finalization step identifier.
	struct StepIdentifier {
		/// Finalization point.
		FinalizationPoint Point;

		/// Finalization stage.
		FinalizationStage Stage;

	public:
		/// Returns \c true if this step identifier is equal to \a rhs.
		bool operator==(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is not equal to \a rhs.
		bool operator!=(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is less than \a rhs.
		bool operator<(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is less than or equal to \a rhs.
		bool operator<=(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is greater than \a rhs.
		bool operator>(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is greater than or equal to \a rhs.
		bool operator>=(const StepIdentifier& rhs) const;
	};

	/// Insertion operator for outputting \a stepIdentifier to \a out.
	std::ostream& operator<<(std::ostream& out, const StepIdentifier& stepIdentifier);

	// endregion

#pragma pack(pop)

	/// Converts \a stepIdentifier to ots key identifier using \a dilution.
	crypto::OtsKeyIdentifier StepIdentifierToOtsKeyIdentifier(const StepIdentifier& stepIdentifier, uint64_t dilution);
}}
