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

#include "StepIdentifier.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace model {

	// region step identifier

	StepIdentifier::StepIdentifier() : StepIdentifier(FinalizationEpoch(), FinalizationPoint(), FinalizationStage::Prevote)
	{}

	StepIdentifier::StepIdentifier(FinalizationEpoch epoch, FinalizationPoint point, FinalizationStage stage)
			: Epoch(epoch)
			, PointStage((point.unwrap() << 1) | (utils::to_underlying_type(stage) & 1))
	{}

	model::FinalizationRound StepIdentifier::Round() const {
		return { Epoch, FinalizationPoint(PointStage.unwrap() >> 1) };
	}

	model::FinalizationStage StepIdentifier::Stage() const {
		return static_cast<model::FinalizationStage>(PointStage.unwrap() & 1);
	}

	bool StepIdentifier::operator==(const StepIdentifier& rhs) const {
		return Epoch == rhs.Epoch && PointStage == rhs.PointStage;
	}

	bool StepIdentifier::operator!=(const StepIdentifier& rhs) const {
		return !(*this == rhs);
	}

	bool StepIdentifier::operator<(const StepIdentifier& rhs) const {
		return Epoch != rhs.Epoch ? Epoch < rhs.Epoch : PointStage < rhs.PointStage;
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
		auto isPrecommit = stepIdentifier.PointStage.unwrap() & 1;
		out << stepIdentifier.Round() << " " << (isPrecommit ? "precommit" : "prevote");
		return out;
	}

	// endregion

	// region StepIdentifierToBmKeyIdentifier

	crypto::BmKeyIdentifier StepIdentifierToBmKeyIdentifier(const StepIdentifier& stepIdentifier) {
		auto identifier = stepIdentifier.Epoch.unwrap();
		return { identifier };
	}

	// endregion
}}
