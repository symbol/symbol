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

#include "Conversions.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/HexParser.h"
#include <sstream>

namespace catapult { namespace test {

	std::string ToHexString(const std::vector<uint8_t>& data) {
		std::stringstream out;
		out << utils::HexFormat(data.data(), data.data() + data.size());
		return out.str();
	}

	std::vector<uint8_t> HexStringToVector(const std::string& hexString) {
		std::vector<uint8_t> result;
		if (hexString.size() % 2)
			CATAPULT_THROW_RUNTIME_ERROR_1("hexString has odd number of characters", hexString.size());

		result.resize(hexString.size() / 2);
		utils::ParseHexStringIntoContainer(hexString.c_str(), hexString.size(), result);
		return result;
	}
}}
