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

#include "XemUnit.h"
#include "ConfigurationValueParsers.h"
#include "StreamFormatGuard.h"

namespace catapult { namespace utils {

	std::ostream& operator<<(std::ostream& out, XemUnit unit) {
		StreamFormatGuard guard(out, std::ios::dec, '0');
		out << unit.xem();
		if (unit.isFractional())
			out << '.' << std::setw(6) << (unit.m_value % XemUnit::Microxem_Per_Xem);

		return out;
	}

	bool TryParseValue(const std::string& str, XemUnit& parsedValue) {
		Amount microxem;
		if (!TryParseValue(str, microxem))
			return false;

		XemUnit xemUnit(microxem);
		if (xemUnit.isFractional())
			return false;

		parsedValue = xemUnit;
		return true;
	}
}}
