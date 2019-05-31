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
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <sstream>

namespace catapult { namespace ionet {

#define TEST_CLASS NodeTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultMetadata) {
		// Act:
		NodeMetadata metadata;

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Zero, metadata.NetworkIdentifier);
		EXPECT_EQ("", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateMetadataWithNetworkIdentifier) {
		// Act:
		NodeMetadata metadata(model::NetworkIdentifier::Mijin_Test);

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, metadata.NetworkIdentifier);
		EXPECT_EQ("", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateMetadataWithNetworkIdentifierAndName) {
		// Act:
		NodeMetadata metadata(model::NetworkIdentifier::Mijin_Test, "alice");

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, metadata.NetworkIdentifier);
		EXPECT_EQ("alice", metadata.Name);
		EXPECT_EQ(NodeVersion(), metadata.Version);
		EXPECT_EQ(NodeRoles::None, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateCustomMetadata) {
		// Act:
		NodeMetadata metadata(model::NetworkIdentifier::Mijin_Test, "alice", NodeVersion(123), NodeRoles::Api);

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, metadata.NetworkIdentifier);
		EXPECT_EQ("alice", metadata.Name);
		EXPECT_EQ(NodeVersion(123), metadata.Version);
		EXPECT_EQ(NodeRoles::Api, metadata.Roles);
	}

	TEST(TEST_CLASS, CanCreateDefaultNode) {
		// Act:
		Node node;

		// Assert:
		EXPECT_EQ(Key(), node.identityKey());

		EXPECT_EQ("", node.endpoint().Host);
		EXPECT_EQ(0u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Zero, node.metadata().NetworkIdentifier);
		EXPECT_EQ("", node.metadata().Name);
		EXPECT_EQ(NodeVersion(), node.metadata().Version);
		EXPECT_EQ(NodeRoles::None, node.metadata().Roles);
	}

	TEST(TEST_CLASS, CanCreateCustomNode) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		Node node(key, { "bob.com", 1234 }, { model::NetworkIdentifier::Mijin_Test, "bob", NodeVersion(7), NodeRoles::Peer });

		// Assert:
		EXPECT_EQ(key, node.identityKey());

		EXPECT_EQ("bob.com", node.endpoint().Host);
		EXPECT_EQ(1234u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, node.metadata().NetworkIdentifier);
		EXPECT_EQ("bob", node.metadata().Name);
		EXPECT_EQ(NodeVersion(7), node.metadata().Version);
		EXPECT_EQ(NodeRoles::Peer, node.metadata().Roles);
	}

	// endregion

	// region comparison

	namespace {
		const char* Default_Key = "default";

		std::unordered_map<std::string, Node> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomByteArray<Key>();
			auto key2 = test::GenerateRandomByteArray<Key>();
			auto networkIdentifier1 = static_cast<model::NetworkIdentifier>(0x25);
			auto networkIdentifier2 = static_cast<model::NetworkIdentifier>(0x26);
			return {
				{ Default_Key, { key1, { "bob.com", 1234 }, NodeMetadata(networkIdentifier1, "bob") } },
				{ "diff-key", { key2, { "bob.com", 1234 }, NodeMetadata(networkIdentifier1, "bob") } },
				{ "diff-host", { key1, { "alice.com", 1234 }, NodeMetadata(networkIdentifier1, "bob") } },
				{ "diff-port", { key1, { "bob.com", 1233 }, NodeMetadata(networkIdentifier1, "bob") } },
				{ "diff-meta-network", { key1, { "bob.com", 1234 }, NodeMetadata(networkIdentifier2, "bob") } },
				{ "diff-meta-name", { key1, { "bob.com", 1234 }, NodeMetadata(networkIdentifier1, "alice") } },
				{ "diff-meta-version", { key1, { "bob.com", 1234 }, { networkIdentifier1, "bob", NodeVersion(1), NodeRoles::None } } },
				{ "diff-meta-roles", { key1, { "bob.com", 1234 }, { networkIdentifier1, "bob", NodeVersion(), NodeRoles::Peer } } }
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			// only significant differences are key and meta-network
			return { Default_Key, "diff-host", "diff-port", "diff-meta-name", "diff-meta-version", "diff-meta-roles" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region insertion operator

	namespace {
		void AssertOutputOperator(const Node& node, const std::string& expected) {
			// Act:
			auto str = test::ToString(node);

			// Assert:
			EXPECT_EQ(expected, str);
		}
	}

	TEST(TEST_CLASS, CanOutputNodeWithName) {
		// Arrange:
		Node node({ test::GenerateRandomByteArray<Key>(), { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableNameCharacters) {
		// Arrange:
		std::string name = "al\a" + std::string(1, '\0') + "ce\t";
		Node node({ test::GenerateRandomByteArray<Key>(), { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, name) });

		// Assert:
		AssertOutputOperator(node, "al??ce? @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableHostCharacters) {
		// Arrange:
		std::string host = "bo\a" + std::string(1, '\0') + "b.co\tm";
		Node node({ test::GenerateRandomByteArray<Key>(), { host, 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bo??b.co?m:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutName) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		auto expectedMijinAddress = "MAOJUDIG67LNG5WHL5MI4RAR5Y46RKTENICGQU5C";
		auto expectedTwentyFiveAddress = "EUOJUDIG67LNG5WHL5MI4RAR5Y46RKTENKUJJCQV";
#else
		auto expectedMijinAddress = "MCX7YGZ5D524BZVRCPJL3M34MV23QJKFRND6NWMJ";
		auto expectedTwentyFiveAddress = "EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UKO";
#endif

		// Assert: note that the public key -> address conversion is dependent on network
		auto key = crypto::ParseKey("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator(
				{ key, { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Mijin) },
				std::string(expectedMijinAddress) + " @ bob.com:1234");

		AssertOutputOperator(
				{ key, { "bob.com", 1234 }, NodeMetadata(static_cast<model::NetworkIdentifier>(0x25)) },
				std::string(expectedTwentyFiveAddress) + " @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutHost) {
		// Assert:
		Node node({ test::GenerateRandomByteArray<Key>(), NodeEndpoint(), NodeMetadata(model::NetworkIdentifier::Zero, "alice") });
		AssertOutputOperator(node, "alice");
	}

	// endregion

	// region hasher

	TEST(TEST_CLASS, Hasher_EqualNodesWithSameKeyReturnSameHash) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		Node node1({ key, NodeEndpoint(), NodeMetadata() });
		Node node2({ key, NodeEndpoint(), NodeMetadata() });

		// Sanity:
		EXPECT_EQ(node1, node2);

		// Act:
		auto result1 = NodeHasher()(node1);
		auto result2 = NodeHasher()(node2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Hasher_UnequalNodesWithSameKeyReturnSameHash) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		Node node1({ key, NodeEndpoint(), NodeMetadata(model::NetworkIdentifier::Zero) });
		Node node2({ key, NodeEndpoint(), NodeMetadata(model::NetworkIdentifier::Mijin) });

		// Sanity:
		EXPECT_NE(node1, node2);

		// Act:
		auto result1 = NodeHasher()(node1);
		auto result2 = NodeHasher()(node2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Hasher_NodesWithDifferentKeysReturnDifferentHash) {
		// Arrange:
		Node node1({ test::GenerateRandomByteArray<Key>(), NodeEndpoint(), NodeMetadata() });
		Node node2({ test::GenerateRandomByteArray<Key>(), NodeEndpoint(), NodeMetadata() });

		// Act:
		auto result1 = NodeHasher()(node1);
		auto result2 = NodeHasher()(node2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion
}}
