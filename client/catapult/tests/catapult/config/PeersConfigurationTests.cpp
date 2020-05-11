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
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace config {

#define TEST_CLASS PeersConfigurationTests

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(0x25);
		constexpr auto Generation_Hash_Seed_String = "272C4ECC55B7A42A07478A9550543C62673D1599A8362CC662E019049B76B7F2";

		auto GetNetworkFingerprint() {
			return model::UniqueNetworkFingerprint(
					Network_Identifier,
					utils::ParseByteArray<GenerationHashSeed>(Generation_Hash_Seed_String));
		}

		void AssertIdentity(const model::NodeIdentity& identity, const Key& identityKey, const std::string& host) {
			EXPECT_EQ(identityKey, identity.PublicKey);
			EXPECT_EQ(host, identity.Host);
		}

		void AssertEndpoint(const ionet::NodeEndpoint& endpoint, const std::string& host, int port) {
			EXPECT_EQ(host, endpoint.Host);
			EXPECT_EQ(port, endpoint.Port);
		}

		void AssertMetadata(const ionet::NodeMetadata& metadata, const std::string& name, ionet::NodeRoles roles) {
			auto expectedGenerationHashSeed = utils::ParseByteArray<GenerationHashSeed>(Generation_Hash_Seed_String);

			EXPECT_EQ(name, metadata.Name);
			EXPECT_EQ(Network_Identifier, metadata.NetworkFingerprint.Identifier);
			EXPECT_EQ(expectedGenerationHashSeed, metadata.NetworkFingerprint.GenerationHashSeed);
			EXPECT_EQ(ionet::NodeVersion(), metadata.Version);
			EXPECT_EQ(roles, metadata.Roles);
		}
	}

	// region failure

	TEST(TEST_CLASS, ConfigFileRequiresKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, PeersEntryMustBeArray) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":false
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, PeersEntriesMustBeValid) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{}
			]
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AllRequiredLeafPropertiesMustBePresent) {
		// Arrange: missing endpoint.port
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja" },
					"metadata":{ "name":"bob" }
				}
			]
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AllRequiredPropertiesMustBePresent) {
		// Arrange: missing endpoint
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"metadata":{ "name":"bob" }
				}
			]
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, NodeRolesMustBeValid) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "name":"bob", "roles":"Omega" }
				}
			]
		})";

		// Act + Assert:
		EXPECT_THROW(LoadPeersFromStream(stream, GetNetworkFingerprint()), catapult_runtime_error);
	}

	// endregion

	TEST(TEST_CLASS, CanParseMinimalConfiguration) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[]
		})";

		// Act:
		auto nodes = LoadPeersFromStream(stream, GetNetworkFingerprint());

		// Assert:
		EXPECT_EQ(0u, nodes.size());
	}

	TEST(TEST_CLASS, CanParseKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{
					"publicKey":"1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "name":"bob", "roles":"Api" }
				}
			]
		})";

		// Act:
		auto nodes = LoadPeersFromStream(stream, GetNetworkFingerprint());

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = utils::ParseByteArray<Key>("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF");

		const auto& node = *nodes.cbegin();
		AssertIdentity(node.identity(), expectedKey, "bob.nem.ninja");
		AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
		AssertMetadata(node.metadata(), "bob", ionet::NodeRoles::Api);
		EXPECT_EQ("bob @ bob.nem.ninja:12345", test::ToString(node));
	}

	TEST(TEST_CLASS, CanParseKnownPeersWithoutNames) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
				{
					"publicKey":"1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751",
					"endpoint":{ "host":"bob.nem.ninja", "port":12345 },
					"metadata":{ "roles":"Peer" }
				}
			]
		})";

		// Act:
		auto nodes = LoadPeersFromStream(stream, GetNetworkFingerprint());

		// Assert:
		ASSERT_EQ(1u, nodes.size());
		auto expectedKey = utils::ParseByteArray<Key>("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");

		const auto& node = *nodes.cbegin();
		AssertIdentity(node.identity(), expectedKey, "bob.nem.ninja");
		AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
		AssertMetadata(node.metadata(), "", ionet::NodeRoles::Peer);

		auto expectedAddress = "EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UI";
		EXPECT_EQ(std::string(expectedAddress) + " @ bob.nem.ninja:12345", test::ToString(node));
	}

	TEST(TEST_CLASS, CanParseMultipleKnownPeers) {
		// Arrange:
		std::stringstream stream;
		stream << R"({
			"knownPeers":[
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
			]
		})";

		// Act:
		auto nodes = LoadPeersFromStream(stream, GetNetworkFingerprint());

		// Assert:
		ASSERT_EQ(2u, nodes.size());
		auto iter = nodes.cbegin();
		{
			const auto& node = *iter++;
			auto expectedKey = utils::ParseByteArray<Key>("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF");

			AssertIdentity(node.identity(), expectedKey, "bob.nem.ninja");
			AssertEndpoint(node.endpoint(), "bob.nem.ninja", 12345);
			AssertMetadata(node.metadata(), "bob", ionet::NodeRoles::Peer);
			EXPECT_EQ("bob @ bob.nem.ninja:12345", test::ToString(node));
		}

		{
			const auto& node = *iter++;
			auto expectedKey = utils::ParseByteArray<Key>("FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321");

			AssertIdentity(node.identity(), expectedKey, "123.456.789.1011");
			AssertEndpoint(node.endpoint(), "123.456.789.1011", 5432);
			AssertMetadata(node.metadata(), "foobar", ionet::NodeRoles::Api);
			EXPECT_EQ("foobar @ 123.456.789.1011:5432", test::ToString(node));
		}
	}

	TEST(TEST_CLASS, CanLoadPeersFromResourcesDirectory) {
		// Arrange: attempt to load from the "real" resources directory
		auto resourcesPath = boost::filesystem::path("../resources");
		for (const auto filename : { "peers-p2p.json", "peers-api.json" }) {
			CATAPULT_LOG(debug) << "parsing peers from " << filename;

			// Act:
			auto peers = LoadPeersFromPath((resourcesPath / filename).generic_string(), GetNetworkFingerprint());

			// Assert:
			EXPECT_FALSE(peers.empty());
		}
	}
}}
