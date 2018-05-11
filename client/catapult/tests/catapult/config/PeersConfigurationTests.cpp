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

#include "catapult/config/PeersConfiguration.h"
#include "tests/TestHarness.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace config {

#define TEST_CLASS PeersConfigurationTests

	namespace {
		const auto Network_Identifier = static_cast<model::NetworkIdentifier>(0x25);

		void AssertEndpoint(const ionet::NodeEndpoint& endpoint, const std::string& host, int port) {
			EXPECT_EQ(host, endpoint.Host);
			EXPECT_EQ(port, endpoint.Port);
		}

		void AssertMetadata(const ionet::NodeMetadata& metadata, const std::string& name, ionet::NodeRoles roles) {
			EXPECT_EQ(name, metadata.Name);
			EXPECT_EQ(Network_Identifier, metadata.NetworkIdentifier);
			EXPECT_EQ(ionet::NodeVersion(), metadata.Version);
			EXPECT_EQ(roles, metadata.Roles);
		}
	}

	// region failure

	TEST(TEST_CLASS, ConfigFileRequiresKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, PeersEntryMustBeArray) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":false}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, PeersEntriesMustBeValid) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{}
			]}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AllRequiredLeafPropertiesMustBePresent) {
		// Arrange: missing endpoint.port
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja" },
					"metadata":{ "name":"bob" }
				}
			]}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AllRequiredPropertiesMustBePresent) {
		// Arrange: missing endpoint
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"metadata":{ "name":"bob" }
				}
			]}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, NodeRolesMustBeValid) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "name":"bob", "roles":"Omega" }
				}
			]}
		)";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, Network_Identifier), catapult_runtime_error);
	}

	// endregion

	TEST(TEST_CLASS, CanParseMinimalConfiguration) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		EXPECT_EQ(0u, nodes.size());
	}

	TEST(TEST_CLASS, CanParseKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "name":"bob", "roles":"Api" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = test::ToArray<Key_Size>("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF");

		const auto& node = *nodes.cbegin();
		EXPECT_EQ(expectedKey, node.identityKey());
		AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
		AssertMetadata(node.metadata(), "bob", ionet::NodeRoles::Api);
		EXPECT_EQ("bob @ bob.nem.ninja", test::ToString(node));
	}

	TEST(TEST_CLASS, CanParseKnownPeersWithoutNames) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "roles":"Peer" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = test::ToArray<Key_Size>("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");

		const auto& node = *nodes.cbegin();
		EXPECT_EQ(expectedKey, node.identityKey());
		AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
		AssertMetadata(node.metadata(), "", ionet::NodeRoles::Peer);

#ifdef SIGNATURE_SCHEME_NIS1
		auto expectedAddress = "EUOJUDIG67LNG5WHL5MI4RAR5Y46RKTENKUJJCQV";
#else
		auto expectedAddress = "EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UKO";
#endif
		EXPECT_EQ(std::string(expectedAddress) + " @ bob.nem.ninja", test::ToString(node));
	}

	TEST(TEST_CLASS, CanParseMultipleKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"(
			{"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "name":"bob", "roles":"Peer" }
				},
				{
					"publicKey":"FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321",
					"endpoint":{ "host":"123.456.789.1011", "port":5432 },
					"metadata":{ "name":"foobar", "roles":"Api" }
				}
			]}
		)";

		// Act:
		auto nodes = LoadPeersFromStream(stream, Network_Identifier);

		// Assert:
		ASSERT_EQ(2u, nodes.size());
		auto iter = nodes.cbegin();
		{
			const auto& node = *iter++;
			auto expectedKey = test::ToArray<Key_Size>("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF");

			EXPECT_EQ(expectedKey, node.identityKey());
			AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
			AssertMetadata(node.metadata(), "bob", ionet::NodeRoles::Peer);
			EXPECT_EQ("bob @ bob.nem.ninja", test::ToString(node));
		}

		{
			const auto& node = *iter++;
			auto expectedKey = test::ToArray<Key_Size>("FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321");

			EXPECT_EQ(expectedKey, node.identityKey());
			AssertEndpoint(node.endpoint(), "123.456.789.1011", 5432);
			AssertMetadata(node.metadata(), "foobar", ionet::NodeRoles::Api);
			EXPECT_EQ("foobar @ 123.456.789.1011", test::ToString(node));
		}
	}

	TEST(TEST_CLASS, CanLoadPeersFromResourcesDirectory) {
		// Arrange: attempt to load from the "real" resources directory
		auto resourcesPath = boost::filesystem::path("../resources");
		for (const auto filename : { "peers-p2p.json", "peers-api.json" }) {
			CATAPULT_LOG(debug) << "parsing peers from " << filename;

			// Act:
			auto peers = LoadPeersFromPath((resourcesPath / filename).generic_string(), Network_Identifier);

			// Assert:
			EXPECT_FALSE(peers.empty());
		}
	}
}}
