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

#include "TimeSpan.h"
#include "IntegerMath.h"
#include "StreamFormatGuard.h"

namespace catapult { namespace utils {

	std::ostream& operator<<(std::ostream& out, const TimeSpan& timeSpan) {
		// calculate components
		auto remainder = timeSpan.millis();
		auto millis = DivideAndGetRemainder<uint64_t>(remainder, 1000);
		auto seconds = DivideAndGetRemainder<uint64_t>(remainder, 60);
		auto minutes = DivideAndGetRemainder<uint64_t>(remainder, 60);
		auto hours = remainder;

		// output as 00:00:00[.000]
		StreamFormatGuard guard(out, std::ios::dec, '0');
		out << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds;

		if (0 != millis)
			out << "." << std::setw(3) << millis;

		return out;
	}
}}
