#include "catapult/ionet/Node.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <sstream>

namespace catapult { namespace ionet {

	TEST(NodeTests, CanCreateDefaultNode) {
		// Act:
		Node node;

		// Assert:
		EXPECT_EQ("", node.Endpoint.Host);
		EXPECT_EQ(0u, node.Endpoint.Port);
		EXPECT_EQ(Key{}, node.Identity.PublicKey);
		EXPECT_EQ("", node.Identity.Name);
		EXPECT_EQ(model::NetworkIdentifier::Zero, node.NetworkIdentifier);
	}

	TEST(NodeTests, CanCreateCustomNode) {
		// Act:
		auto key = test::GenerateRandomData<Key_Size>();
		Node node = { { "bob.com", 1234 }, { key, "bob" }, model::NetworkIdentifier::Mijin_Test };

		// Assert:
		EXPECT_EQ("bob.com", node.Endpoint.Host);
		EXPECT_EQ(1234u, node.Endpoint.Port);
		EXPECT_EQ(key, node.Identity.PublicKey);
		EXPECT_EQ("bob", node.Identity.Name);
		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, node.NetworkIdentifier);
	}

	namespace {
		const char* Default_Key = "default";

		std::unordered_map<std::string, Node> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomData<Key_Size>();
			auto key2 = test::GenerateRandomData<Key_Size>();
			auto networkIdentifier1 = static_cast<model::NetworkIdentifier>(0x25);
			auto networkIdentifier2 = static_cast<model::NetworkIdentifier>(0x26);
			return {
				{ Default_Key, { { "bob.com", 1234 }, { key1, "bob" }, networkIdentifier1 } },
				{ "diff-host", { { "alice.com", 1234 }, { key1, "bob" }, networkIdentifier1 } },
				{ "diff-port", { { "bob.com", 1233 }, { key1, "bob" }, networkIdentifier1 } },
				{ "diff-key", { { "bob.com", 1234 }, { key2, "bob" }, networkIdentifier1 } },
				{ "diff-name", { { "bob.com", 1234 }, { key1, "alice" }, networkIdentifier1 } },
				{ "diff-network-id", { { "bob.com", 1234 }, { key1, "bob" }, networkIdentifier2 } },
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "diff-host", "diff-port", "diff-name" };
		}
	}

	TEST(NodeTests, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(
			Default_Key,
			GenerateEqualityInstanceMap(),
			GetEqualTags());
	}

	TEST(NodeTests, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			Default_Key,
			GenerateEqualityInstanceMap(),
			GetEqualTags());
	}

	namespace {
		void AssertOutputOperator(const Node& node, const std::string& expected) {
			// Act:
			auto str = test::ToString(node);

			// Assert:
			EXPECT_EQ(expected, str);
		}
	}

	TEST(NodeTests, CanOutputNodeWithName) {
		// Assert:
		AssertOutputOperator(
				{ { "bob.com", 1234 }, { test::GenerateRandomData<Key_Size>(), "alice" }, model::NetworkIdentifier::Zero },
				"alice @ bob.com");
	}

	TEST(NodeTests, CanOutputNodeWithUnprintableNameCharacters) {
		// Assert:
		AssertOutputOperator(
				{ { "bob.com", 1234 }, { test::GenerateRandomData<Key_Size>(), "al\ace\t" }, model::NetworkIdentifier::Zero },
				"al?ce? @ bob.com");
	}

	TEST(NodeTests, CanOutputNodeWithoutName) {
		// Assert: note that the public key -> address conversion is dependent on network
		auto key = crypto::ParseKey("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator(
				{ { "bob.com", 1234 }, { key, "" }, model::NetworkIdentifier::Mijin },
				"MCX7YGZ5D524BZVRCPJL3M34MV23QJKFRND6NWMJ @ bob.com");

		AssertOutputOperator(
				{ { "bob.com", 1234 }, { key, "" }, static_cast<model::NetworkIdentifier>(0x25) },
				"EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UKO @ bob.com");
	}

	TEST(NodeTests, CanOutputNodeWithoutHost) {
		// Assert:
		AssertOutputOperator(
				{ {}, { test::GenerateRandomData<Key_Size>(), "alice" }, model::NetworkIdentifier::Zero },
				"alice");
	}
}}
