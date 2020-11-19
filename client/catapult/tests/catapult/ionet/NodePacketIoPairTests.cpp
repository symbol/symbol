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

#include "catapult/ionet/NodePacketIoPair.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodePacketIoPairTests

	TEST(TEST_CLASS, CanCreateEmptyPair) {
		// Act:
		NodePacketIoPair pair;

		// Assert:
		EXPECT_EQ(Key(), pair.node().identity().PublicKey);
		EXPECT_EQ("", pair.node().identity().Host);

		EXPECT_FALSE(!!pair.io());
		EXPECT_FALSE(!!pair);
	}

	TEST(TEST_CLASS, CanCreatePairWithValues) {
		// Act:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto node = Node({ identityKey, "11.22.33.44" });
		auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
		NodePacketIoPair pair(node, pPacketIo);

		// Assert:
		EXPECT_EQ(identityKey, pair.node().identity().PublicKey);
		EXPECT_EQ("11.22.33.44", pair.node().identity().Host);

		EXPECT_EQ(pPacketIo, pair.io());
		EXPECT_TRUE(!!pair);
	}
}}
