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

#include "catapult/ionet/Node.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"
#include <sstream>

namespace catapult { namespace ionet {

#define TEST_CLASS NodeTests

	// region constructor (NodeMetadata)

	TEST(TEST_CLASS, CanCreateDefaultMetadata) {
		// Act:
		NodeMetadata metadata;

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Zero, metadata.NetworkFingerprint.Identifier);
		EXPECT_EQ(GenerationHashSeed(), metadata.NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateMetadataWithNetworkFingerprint) {
		// Arrange
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto networkFingerprint = model::UniqueNetworkFingerprint(model::NetworkIdentifier::Private_Test, generationHashSeed);

		// Act:
		NodeMetadata metadata(networkFingerprint);

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Private_Test, metadata.NetworkFingerprint.Identifier);
		EXPECT_EQ(generationHashSeed, metadata.NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateMetadataWithNetworkFingerprintAndName) {
		// Arrange
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto networkFingerprint = model::UniqueNetworkFingerprint(model::NetworkIdentifier::Private_Test, generationHashSeed);

		// Act:
		NodeMetadata metadata(networkFingerprint, "alice");

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Private_Test, metadata.NetworkFingerprint.Identifier);
		EXPECT_EQ(generationHashSeed, metadata.NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("alice", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateCustomMetadata) {
		// Arrange
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto networkFingerprint = model::UniqueNetworkFingerprint(model::NetworkIdentifier::Private_Test, generationHashSeed);

		// Act:
		NodeMetadata metadata(networkFingerprint, "alice", NodeVersion(123), NodeRoles::Api);

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Private_Test, metadata.NetworkFingerprint.Identifier);
		EXPECT_EQ(generationHashSeed, metadata.NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("alice", metadata.Name);
		EXPECT_EQ(NodeVersion(123), metadata.Version);
		EXPECT_EQ(NodeRoles::Api, metadata.Roles);
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultNode) {
		// Act:
		Node node;

		// Assert:
		EXPECT_EQ(Key(), node.identity().PublicKey);
		EXPECT_EQ("", node.identity().Host);

		EXPECT_EQ("", node.endpoint().Host);
		EXPECT_EQ(0u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Zero, node.metadata().NetworkFingerprint.Identifier);
		EXPECT_EQ(GenerationHashSeed(), node.metadata().NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("", node.metadata().Name);
		EXPECT_EQ(NodeVersion(), node.metadata().Version);
		EXPECT_EQ(NodeRoles::None, node.metadata().Roles);
	}

	TEST(TEST_CLASS, CanCreateNodeWithIdentity) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();

		// Act:
		Node node({ identityKey, "11.22.33.44" });

		// Assert:
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);

		EXPECT_EQ("", node.endpoint().Host);
		EXPECT_EQ(0u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Zero, node.metadata().NetworkFingerprint.Identifier);
		EXPECT_EQ(GenerationHashSeed(), node.metadata().NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("", node.metadata().Name);
		EXPECT_EQ(NodeVersion(), node.metadata().Version);
		EXPECT_EQ(NodeRoles::None, node.metadata().Roles);
	}

	TEST(TEST_CLASS, CanCreateCustomNode) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto networkFingerprint = model::UniqueNetworkFingerprint(model::NetworkIdentifier::Private_Test, generationHashSeed);

		// Act:
		Node node({ identityKey, "11.22.33.44" }, { "bob.com", 1234 }, { networkFingerprint, "bob", NodeVersion(7), NodeRoles::Peer });

		// Assert:
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);

		EXPECT_EQ("bob.com", node.endpoint().Host);
		EXPECT_EQ(1234u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Private_Test, node.metadata().NetworkFingerprint.Identifier);
		EXPECT_EQ(generationHashSeed, node.metadata().NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("bob", node.metadata().Name);
		EXPECT_EQ(NodeVersion(7), node.metadata().Version);
		EXPECT_EQ(NodeRoles::Peer, node.metadata().Roles);
	}

	// endregion

	// region insertion operator

	namespace {
		NodeMetadata CreateMetadata(model::NetworkIdentifier networkIdentifier, const std::string& name = "") {
			return NodeMetadata(model::UniqueNetworkFingerprint(networkIdentifier), name);
		}

		void AssertOutputOperator(const Node& node, const std::string& expected) {
			// Act:
			auto str = test::ToString(node);

			// Assert:
			EXPECT_EQ(expected, str);
		}
	}

	TEST(TEST_CLASS, CanOutputNodeWithName) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node({ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableNameCharacters) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		std::string name = "al\a" + std::string(1, '\0') + "ce\t";
		Node node({ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Zero, name) });

		// Assert:
		AssertOutputOperator(node, "al??ce? @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableHostCharacters) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		std::string host = "bo\a" + std::string(1, '\0') + "b.co\tm";
		Node node({ { identityKey, "11.22.33.44" }, { host, 1234 }, CreateMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bo??b.co?m:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutName) {
		// Arrange:
		auto expectedPrivateAddress = "PCX7YGZ5D524BZVRCPJL3M34MV23QJKFROQU2QY";
		auto expectedTwentyFiveAddress = "EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UI";

		// Assert: note that the public key -> address conversion is dependent on network
		auto identityKey = utils::ParseByteArray<Key>("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator(
				{ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) },
				std::string(expectedPrivateAddress) + " @ bob.com:1234");

		AssertOutputOperator(
				{ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(static_cast<model::NetworkIdentifier>(0x25)) },
				std::string(expectedTwentyFiveAddress) + " @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutHost) {
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node({ { identityKey, "11.22.33.44" }, NodeEndpoint(), CreateMetadata(model::NetworkIdentifier::Zero, "alice") });
		AssertOutputOperator(node, "alice");
	}

	// endregion
}}
