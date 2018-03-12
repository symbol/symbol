#include "catapult/ionet/NodePacketIoPair.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodePacketIoPairTests

	TEST(TEST_CLASS, CanCreateEmptyPair) {
		// Act:
		NodePacketIoPair pair;

		// Assert:
		EXPECT_EQ(Node(), pair.node());
		EXPECT_FALSE(!!pair.io());
		EXPECT_FALSE(!!pair);
	}

	TEST(TEST_CLASS, CanCreateNonEmptyPair) {
		// Act:
		auto node = Node(test::GenerateRandomData<Key_Size>(), ionet::NodeEndpoint(), ionet::NodeMetadata());
		auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
		NodePacketIoPair pair(node, pPacketIo);

		// Assert:
		EXPECT_EQ(node, pair.node());
		EXPECT_EQ(pPacketIo, pair.io());
		EXPECT_TRUE(!!pair);
	}
}}
