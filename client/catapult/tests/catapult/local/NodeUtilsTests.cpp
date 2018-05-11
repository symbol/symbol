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

#include "catapult/local/NodeUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS NodeUtilsTests

	namespace {
		auto CreateLocalNodeConfiguration(const std::string& bootKey, const std::string& host, const std::string& name) {
			auto nodeConfig = config::NodeConfiguration::Uninitialized();
			nodeConfig.Local.Host = host;
			nodeConfig.Local.FriendlyName = name;

			auto userConfig = config::UserConfiguration::Uninitialized();
			userConfig.BootKey = bootKey;

			return config::LocalNodeConfiguration(
					model::BlockChainConfiguration::Uninitialized(),
					std::move(nodeConfig),
					config::LoggingConfiguration::Uninitialized(),
					std::move(userConfig));
		}
	}

	TEST(TEST_CLASS, SeedNodeContainerAddsOnlyLocalNodeWhenThereAreNoStaticNodes) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key_Size);
		auto config = CreateLocalNodeConfiguration(hexPrivateKey, "127.0.0.1", "LOCAL");
		extensions::LocalNodeBootstrapper bootstrapper(config, "", "bootstrapper");

		// Act:
		ionet::NodeContainer nodes;
		SeedNodeContainer(nodes, bootstrapper);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), "LOCAL", ionet::NodeSource::Local }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	TEST(TEST_CLASS, SeedNodeContainerAddsLocalAndStaticNodes) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key_Size);
		auto config = CreateLocalNodeConfiguration(hexPrivateKey, "127.0.0.1", "LOCAL");
		extensions::LocalNodeBootstrapper bootstrapper(config, "", "bootstrapper");

		auto keys = test::GenerateRandomDataVector<Key>(3);
		bootstrapper.addStaticNodes({
			test::CreateNamedNode(keys[0], "alice"),
			test::CreateNamedNode(keys[1], "bob"),
			test::CreateNamedNode(keys[2], "charlie")
		});

		// Act:
		ionet::NodeContainer nodes;
		SeedNodeContainer(nodes, bootstrapper);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(4u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), "LOCAL", ionet::NodeSource::Local },
			{ keys[0], "alice", ionet::NodeSource::Static },
			{ keys[1], "bob", ionet::NodeSource::Static },
			{ keys[2], "charlie", ionet::NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	namespace {
		void SeedWithLengths(
				ionet::NodeContainer& nodes,
				const std::string& bootKey,
				const Key& peerKey,
				size_t localHostSize,
				size_t localNameSize,
				size_t peerHostSize,
				size_t peerNameSize) {
			// Arrange:
			CATAPULT_LOG(debug)
					<< "seed with lengths: " << localHostSize << ", " << localNameSize
					<< ", " << peerHostSize << ", " << peerNameSize;

			auto config = CreateLocalNodeConfiguration(bootKey, std::string(localHostSize, 'm'), std::string(localNameSize, 'l'));
			extensions::LocalNodeBootstrapper bootstrapper(config, "", "bootstrapper");

			auto peerEndpoint = ionet::NodeEndpoint{ std::string(peerHostSize, 'q'), 1234 };
			auto peerMetadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, std::string(peerNameSize, 'p'));
			bootstrapper.addStaticNodes({ ionet::Node(peerKey, peerEndpoint, peerMetadata) });

			// Act:
			SeedNodeContainer(nodes, bootstrapper);
		}
	}

	TEST(TEST_CLASS, SeedNodeContainerSucceedsIfMaxStringLengthsAreUsed) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key_Size);
		auto peerKey = test::GenerateRandomData<Key_Size>();

		// Act:
		ionet::NodeContainer nodes;
		SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 255, 255);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(2u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), std::string(255, 'l'), ionet::NodeSource::Local },
			{ peerKey, std::string(255, 'p'), ionet::NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	TEST(TEST_CLASS, SeedNodeContainerFailsIfMaxStringLengthsAreExceeded) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key_Size);
		auto peerKey = test::GenerateRandomData<Key_Size>();
		ionet::NodeContainer nodes;

		// Act + Assert:
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 256, 255, 255, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 256, 255, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 256, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 255, 256), catapult_invalid_argument);
	}
}}
