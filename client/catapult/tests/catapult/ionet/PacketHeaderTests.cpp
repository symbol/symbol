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

#include "catapult/ionet/PacketHeader.h"
#include "tests/TestHarness.h"
#include <limits>

namespace catapult { namespace ionet {

#define TEST_CLASS PacketHeaderTests

	// region IsPacketDataSizeValid

	TEST(TEST_CLASS, IsPacketDataSizeValid_ReturnsFalseWhenPacketSizeIsTooSmall) {
		// Arrange:
		for (auto size : std::initializer_list<uint32_t>{ 0, sizeof(PacketHeader) / 2, sizeof(PacketHeader) - 1 }){
			auto header = PacketHeader{ size, PacketType::Undefined };

			// Act + Assert:
			EXPECT_FALSE(IsPacketDataSizeValid(header, 1234)) << size;
		}
	}

	TEST(TEST_CLASS, IsPacketDataSizeValid_ReturnsTrueWhenPacketSizeIsInAllowableRange) {
		// Arrange:
		for (auto size : std::initializer_list<uint32_t>{ 0, 789, 1234 }) {
			auto header = PacketHeader{ SizeOf32<PacketHeader>() + size, PacketType::Undefined };

			// Act + Assert:
			EXPECT_TRUE(IsPacketDataSizeValid(header, 1234)) << size;
		}
	}

	TEST(TEST_CLASS, IsPacketDataSizeValid_ReturnsFalseWhenPacketSizeIsTooLarge) {
		// Arrange:
		for (auto size : std::initializer_list<uint32_t>{ 1235, 9999, std::numeric_limits<uint32_t>::max() }) {
			auto header = PacketHeader{ SizeOf32<PacketHeader>() + size, PacketType::Undefined };

			// Act + Assert:
			EXPECT_FALSE(IsPacketDataSizeValid(header, 1234)) << size;
		}
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputPacketHeader) {
		// Arrange:
		auto header = PacketHeader{ 572, PacketType::Push_Block };

		// Act:
		auto str = test::ToString(header);

		// Assert:
		EXPECT_EQ("packet Push_Block with size 572", str);
	}

	// endregion
}}
