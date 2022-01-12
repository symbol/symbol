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

#include "FinalizationRound.h"

namespace catapult { namespace model {

	bool FinalizationRound::operator==(const FinalizationRound& rhs) const {
		return Epoch == rhs.Epoch && Point == rhs.Point;
	}

	bool FinalizationRound::operator!=(const FinalizationRound& rhs) const {
		return !(*this == rhs);
	}

	bool FinalizationRound::operator<(const FinalizationRound& rhs) const {
		return Epoch != rhs.Epoch ? Epoch < rhs.Epoch : Point < rhs.Point;
	}

	bool FinalizationRound::operator<=(const FinalizationRound& rhs) const {
		return *this < rhs || *this == rhs;
	}

	bool FinalizationRound::operator>(const FinalizationRound& rhs) const {
		return !(*this <= rhs);
	}

	bool FinalizationRound::operator>=(const FinalizationRound& rhs) const {
		return !(*this < rhs);
	}

	FinalizationRound FinalizationRound::operator+(FinalizationPoint point) const {
		return { Epoch, Point + point };
	}

	FinalizationRound FinalizationRound::operator-(FinalizationPoint point) const {
		return { Epoch, Point - point };
	}

	std::ostream& operator<<(std::ostream& out, const FinalizationRound& round) {
		out << "(" << round.Epoch << ", " << round.Point << ")";
		return out;
	}
}}
