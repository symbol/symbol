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

#include "StepIdentifier.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace model {

	// region step identifier

	bool StepIdentifier::operator==(const StepIdentifier& rhs) const {
		return Point == rhs.Point && Stage == rhs.Stage;
	}

	bool StepIdentifier::operator!=(const StepIdentifier& rhs) const {
		return !(*this == rhs);
	}

	bool StepIdentifier::operator<(const StepIdentifier& rhs) const {
		return Point < rhs.Point || (Point == rhs.Point && Stage < rhs.Stage);
	}

	bool StepIdentifier::operator<=(const StepIdentifier& rhs) const {
		return *this < rhs || *this == rhs;
	}

	bool StepIdentifier::operator>(const StepIdentifier& rhs) const {
		return !(*this <= rhs);
	}

	bool StepIdentifier::operator>=(const StepIdentifier& rhs) const {
		return !(*this < rhs);
	}

	std::ostream& operator<<(std::ostream& out, const StepIdentifier& stepIdentifier) {
		out << "(" << stepIdentifier.Point << ", " << utils::to_underlying_type(stepIdentifier.Stage) << ")";
		return out;
	}

	// endregion

	// region StepIdentifierToOtsKeyIdentifier

	crypto::OtsKeyIdentifier StepIdentifierToOtsKeyIdentifier(const StepIdentifier& stepIdentifier, uint64_t dilution) {
		// assume: Dilution < 1 is not allowed
		constexpr auto Num_Stages = utils::to_underlying_type(FinalizationStage::Count);
		auto identifier = stepIdentifier.Point.unwrap() * Num_Stages + utils::to_underlying_type(stepIdentifier.Stage);

		crypto::OtsKeyIdentifier keyIdentifier;
		keyIdentifier.BatchId = identifier / dilution;
		keyIdentifier.KeyId = identifier % dilution;
		return keyIdentifier;
	}

	// endregion
}}
