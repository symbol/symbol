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

#include "NameChecker.h"
#include <algorithm>

namespace catapult { namespace model {

	namespace {
		constexpr bool IsAlphaNumeric(uint8_t ch) {
			return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9');
		}

		constexpr bool IsValidChar(uint8_t ch) {
			return IsAlphaNumeric(ch) || '-' == ch || '_' == ch;
		}
	}

	bool IsValidName(const uint8_t* pName, size_t nameSize) {
		if (0 == nameSize)
			return false;

		return IsAlphaNumeric(pName[0]) && std::all_of(pName + 1, pName + nameSize, IsValidChar);
	}
}}
