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

#include "CurveUtils.h"
#include "tests/test/nodeps/Conversions.h"

namespace catapult { namespace test {

	void ScalarAddGroupOrder(uint8_t* scalar) {
		// 2^252 + 27742317777372353535851937790883648493, little endian.
		auto groupOrder = test::HexStringToVector("EDD3F55C1A631258D69CF7A2DEF9DE1400000000000000000000000000000010");
		uint8_t r = 0;
		for (auto i = 0u; i < groupOrder.size(); ++i) {
			auto t = static_cast<uint16_t>(scalar[i] + groupOrder[i]);
			scalar[i] = static_cast<uint8_t>(scalar[i] + groupOrder[i] + r);
			r = static_cast<uint8_t>(t >> 8);
		}
	}
}}
