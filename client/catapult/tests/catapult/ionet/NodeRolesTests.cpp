#include "catapult/ionet/NodeRoles.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeRolesTests

	TEST(TEST_CLASS, CanParseEmptyString) {
		// Act:
		NodeRoles roles;
		auto isParsed = TryParseValue("", roles);

		// Assert:
		EXPECT_TRUE(isParsed);
		EXPECT_EQ(NodeRoles::None, roles);
	}

	TEST(TEST_CLASS, CanParseSingleValue) {
		// Act:
		NodeRoles roles;
		auto isParsed = TryParseValue("Peer", roles);

		// Assert:
		EXPECT_TRUE(isParsed);
		EXPECT_EQ(NodeRoles::Peer, roles);
	}

	TEST(TEST_CLASS, CanParseMultipleValues) {
		// Act:
		NodeRoles roles;
		auto isParsed = TryParseValue("Peer,Api", roles);

		// Assert:
		EXPECT_TRUE(isParsed);
		EXPECT_EQ(static_cast<NodeRoles>(3), roles);
	}

	TEST(TEST_CLASS, CannotParseMalformedSet) {
		// Act:
		auto roles = NodeRoles::None;
		auto isParsed = TryParseValue("Peer,,Api", roles);

		// Assert:
		EXPECT_FALSE(isParsed);
		EXPECT_EQ(NodeRoles::None, roles);
	}

	TEST(TEST_CLASS, CannotParseSetWithUnknownRole) {
		// Act:
		auto roles = NodeRoles::None;
		auto isParsed = TryParseValue("Peer,Api2,Api", roles);

		// Assert:
		EXPECT_FALSE(isParsed);
		EXPECT_EQ(NodeRoles::None, roles);
	}
}}
