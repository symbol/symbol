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
#include "tests/TestHarness.h"
#include <sstream>

namespace catapult { namespace ionet {

#define TEST_CLASS NodeTests

	// region constructor (NodeMetadata)

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

		EXPECT_EQ(model::NetworkIdentifier::Zero, node.metadata().NetworkIdentifier);
		EXPECT_EQ("", node.metadata().Name);
		EXPECT_EQ(NodeVersion(), node.metadata().Version);
		EXPECT_EQ(NodeRoles::None, node.metadata().Roles);
	}

	TEST(TEST_CLASS, CanCreateNodeWithIdentity) {
		// Act:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node({ identityKey, "11.22.33.44" });

		// Assert:
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);

		EXPECT_EQ("", node.endpoint().Host);
		EXPECT_EQ(0u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Zero, node.metadata().NetworkIdentifier);
		EXPECT_EQ("", node.metadata().Name);
		EXPECT_EQ(NodeVersion(), node.metadata().Version);
		EXPECT_EQ(NodeRoles::None, node.metadata().Roles);
	}

	TEST(TEST_CLASS, CanCreateCustomNode) {
		// Act:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node(
				{ identityKey, "11.22.33.44" },
				{ "bob.com", 1234 },
				{ model::NetworkIdentifier::Mijin_Test, "bob", NodeVersion(7), NodeRoles::Peer });

		// Assert:
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);

		EXPECT_EQ("bob.com", node.endpoint().Host);
		EXPECT_EQ(1234u, node.endpoint().Port);

		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, node.metadata().NetworkIdentifier);
		EXPECT_EQ("bob", node.metadata().Name);
		EXPECT_EQ(NodeVersion(7), node.metadata().Version);
		EXPECT_EQ(NodeRoles::Peer, node.metadata().Roles);
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
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node({ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableNameCharacters) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		std::string name = "al\a" + std::string(1, '\0') + "ce\t";
		Node node({ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, name) });

		// Assert:
		AssertOutputOperator(node, "al??ce? @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithUnprintableHostCharacters) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		std::string host = "bo\a" + std::string(1, '\0') + "b.co\tm";
		Node node({ { identityKey, "11.22.33.44" }, { host, 1234 }, NodeMetadata(model::NetworkIdentifier::Zero, "alice") });

		// Assert:
		AssertOutputOperator(node, "alice @ bo??b.co?m:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutName) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_KECCAK
		auto expectedMijinAddress = "MAOJUDIG67LNG5WHL5MI4RAR5Y46RKTENICGQU5C";
		auto expectedTwentyFiveAddress = "EUOJUDIG67LNG5WHL5MI4RAR5Y46RKTENKUJJCQV";
#else
		auto expectedMijinAddress = "MCX7YGZ5D524BZVRCPJL3M34MV23QJKFRND6NWMJ";
		auto expectedTwentyFiveAddress = "EWX7YGZ5D524BZVRCPJL3M34MV23QJKFRPLA5UKO";
#endif

		// Assert: note that the public key -> address conversion is dependent on network
		auto identityKey = crypto::ParseKey("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator(
				{ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, NodeMetadata(model::NetworkIdentifier::Mijin) },
				std::string(expectedMijinAddress) + " @ bob.com:1234");

		AssertOutputOperator(
				{ { identityKey, "11.22.33.44" }, { "bob.com", 1234 }, NodeMetadata(static_cast<model::NetworkIdentifier>(0x25)) },
				std::string(expectedTwentyFiveAddress) + " @ bob.com:1234");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutHost) {
		auto identityKey = test::GenerateRandomByteArray<Key>();
		Node node({ { identityKey, "11.22.33.44" }, NodeEndpoint(), NodeMetadata(model::NetworkIdentifier::Zero, "alice") });
		AssertOutputOperator(node, "alice");
	}

	// endregion
}}
