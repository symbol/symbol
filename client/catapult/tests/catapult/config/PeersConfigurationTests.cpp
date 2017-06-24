#include "catapult/config/PeersConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		const auto Network_Identifier = static_cast<model::NetworkIdentifier>(0x25);

		void AssertEndpoint(const ionet::NodeEndpoint& endpoint, const std::string& host, int port) {
			EXPECT_EQ(host, endpoint.Host);
			EXPECT_EQ(port, endpoint.Port);
		}

		void AssertIdentity(const ionet::NodeIdentity& identity, const std::string& name, const Key& publicKey) {
			EXPECT_EQ(name, identity.Name);
			EXPECT_EQ(publicKey, identity.PublicKey);
		}
	}

	TEST(PeersConfigurationTests, ConfigFileRequiresKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{}
		)";

		// Act:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), std::runtime_error);
	}

	TEST(PeersConfigurationTests, CanParseMinimalConfiguration) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":false}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		EXPECT_EQ(0u, nodes.size());
	}

	TEST(PeersConfigurationTests, PeersEntriesMustBeValid) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{}
			]}
		)";

		// Act:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), std::runtime_error);
	}

	TEST(PeersConfigurationTests, CanParseKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"endpoint":{ "host": "bob.nem.ninja", "port":12345 },
					"identity":{ "name": "bob", "public-key": "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = test::ToArray<Key_Size>(
			"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");

		const auto& node = *nodes.cbegin();
		AssertEndpoint(node.Endpoint, "bob.nem.ninja", 12345);
		AssertIdentity(node.Identity, "bob", expectedKey);
		EXPECT_EQ("bob @ bob.nem.ninja", test::ToString(node));
	}

	TEST(PeersConfigurationTests, CanParseKnownPeersWithoutNames) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"endpoint":{ "host": "bob.nem.ninja", "port":12345 },
					"identity":{ "public-key": "1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = test::ToArray<Key_Size>(
			"1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");

		const auto& node = *nodes.cbegin();
		AssertEndpoint(node.Endpoint, "bob.nem.ninja", 12345);
		AssertIdentity(node.Identity, "", expectedKey);
		EXPECT_EQ("EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UKO @ bob.nem.ninja", test::ToString(node));
	}

	TEST(PeersConfigurationTests, CanParseMultipleKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"endpoint":{ "host": "bob.nem.ninja", "port":12345 },
					"identity":{ "name": "bob", "public-key": "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef" }
				},
				{
					"endpoint":{ "host": "123.456.789.1011", "port":5432 },
					"identity":{ "name": "foobar", "public-key": "fedcba0987654321fedcba0987654321fedcba0987654321fedcba0987654321" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(2u, nodes.size());
		auto it = nodes.cbegin();
		{
			const auto& node = *it++;
			auto expectedKey = test::ToArray<Key_Size>(
				"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");

			AssertEndpoint(node.Endpoint, "bob.nem.ninja", 12345);
			AssertIdentity(node.Identity, "bob", expectedKey);
			EXPECT_EQ("bob @ bob.nem.ninja", test::ToString(node));
		}

		{
			const auto& node = *it++;
			auto expectedKey = test::ToArray<Key_Size>(
				"fedcba0987654321fedcba0987654321fedcba0987654321fedcba0987654321");

			AssertEndpoint(node.Endpoint, "123.456.789.1011", 5432);
			AssertIdentity(node.Identity, "foobar", expectedKey);
			EXPECT_EQ("foobar @ 123.456.789.1011", test::ToString(node));
		}
	}
}}
