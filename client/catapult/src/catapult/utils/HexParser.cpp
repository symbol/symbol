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

#include "HexParser.h"
#include <string>

namespace catapult { namespace utils {

	namespace {
		bool TryParseNibble(const char ch, int& nibble) {
			if (ch >= 'A' && ch <= 'F')
				nibble = ch - 'A' + 10;
			else if (ch >= 'a' && ch <= 'f')
				nibble = ch - 'a' + 10;
			else if (ch >= '0' && ch <= '9')
				nibble = ch - '0';
			else
				return false;

			return true;
		}
	}

	uint8_t ParseByte(char ch1, char ch2) {
		uint8_t by;
		if (!TryParseByte(ch1, ch2, by)) {
			auto byteString = std::string{ ch1, ch2 };
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown hex character in string", byteString);
		}

		return by;
	}

	bool TryParseByte(char ch1, char ch2, uint8_t& by) {
		int nibble1, nibble2;
		if (!TryParseNibble(ch1, nibble1) || !TryParseNibble(ch2, nibble2))
			return false;

		by = static_cast<uint8_t>(nibble1 << 4 | nibble2);
		return true;
	}
}}
