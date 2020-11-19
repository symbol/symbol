/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/local/server/NodeUtils.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS NodeUtilsTests

	// region ValidateNodes / AddLocalNode

	namespace {
		void ValidateNodesWithLengths(size_t peerHostSize, size_t peerNameSize) {
			// Arrange: configure single node with specified lengths
			CATAPULT_LOG(debug) << "seed with lengths: " << peerHostSize << ", " << peerNameSize;

			auto peerEndpoint = ionet::NodeEndpoint{ std::string(peerHostSize, 'q'), 1234 };
			auto peerMetadata = ionet::NodeMetadata(model::UniqueNetworkFingerprint(), std::string(peerNameSize, 'p'));
			std::vector<ionet::Node> nodes{
				ionet::Node({ Key{ { 1 } }, "1.2.3.4" }),
				ionet::Node({ Key{ { 4 } }, "3.3.3.3" }, peerEndpoint, peerMetadata),
				ionet::Node({ Key{ { 9 } }, "9.8.7.6" })
			};

			// Act:
			ValidateNodes(nodes);
		}
	}

	TEST(TEST_CLASS, ValidateNodes_SucceedsWhenMaxStringLengthsAreUsed) {
		EXPECT_NO_THROW(ValidateNodesWithLengths(255, 255));
	}

	TEST(TEST_CLASS, ValidateNodes_FailsWhenMaxStringLengthsAreExceeded) {
		EXPECT_THROW(ValidateNodesWithLengths(256, 255), catapult_invalid_argument);
		EXPECT_THROW(ValidateNodesWithLengths(255, 256), catapult_invalid_argument);
	}

	namespace {
		auto CreateCatapultConfiguration(const std::string& host, const std::string& name) {
			test::MutableCatapultConfiguration config;
			config.Node.Local.Host = host;
			config.Node.Local.FriendlyName = name;

			config.User.CertificateDirectory = test::GetDefaultCertificateDirectory();
			return config.ToConst();
		}

		void AddLocalNodeWithLengths(ionet::NodeContainer& nodes, size_t localHostSize, size_t localNameSize) {
			// Arrange: configure single node with specified lengths
			CATAPULT_LOG(debug) << "seed with lengths: " << localHostSize << ", " << localNameSize;

			auto config = CreateCatapultConfiguration(std::string(localHostSize, 'm'), std::string(localNameSize, 'l'));

			// Act:
			AddLocalNode(nodes, config);
		}
	}

	TEST(TEST_CLASS, AddLocalNode_SucceedsWhenMaxStringLengthsAreUsed) {
		// Arrange:
		ionet::NodeContainer nodes;
		auto caPublicKeyPemFilename = config::GetCaPublicKeyPemFilename(test::GetDefaultCertificateDirectory());
		auto caPublicKey = crypto::ReadPublicKeyFromPublicKeyPemFile(caPublicKeyPemFilename);

		// Act:
		AddLocalNodeWithLengths(nodes, 255, 255);

		// Assert: local node is added to nodes
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{ { caPublicKey, std::string(255, 'l'), ionet::NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	TEST(TEST_CLASS, AddLocalNode_FailsWhenMaxStringLengthsAreExceeded) {
		// Arrange:
		ionet::NodeContainer nodes;

		// Act + Assert:
		EXPECT_THROW(AddLocalNodeWithLengths(nodes, 256, 255), catapult_invalid_argument);
		EXPECT_THROW(AddLocalNodeWithLengths(nodes, 255, 256), catapult_invalid_argument);
	}

	// endregion

	// region GetBanSettings

	TEST(TEST_CLASS, CanGetBanSettingsFromBanningSubConfiguration) {
		// Arrange:
		config::NodeConfiguration::BanningSubConfiguration banConfig;
		banConfig.DefaultBanDuration = utils::TimeSpan::FromHours(12);
		banConfig.MaxBanDuration = utils::TimeSpan::FromMinutes(23);
		banConfig.KeepAliveDuration = utils::TimeSpan::FromSeconds(34);
		banConfig.MaxBannedNodes = 45;

		// Act:
		auto banSettings = GetBanSettings(banConfig);

		// Assert:
		EXPECT_EQ(utils::TimeSpan::FromHours(12), banSettings.DefaultBanDuration);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(23), banSettings.MaxBanDuration);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(34), banSettings.KeepAliveDuration);
		EXPECT_EQ(45u, banSettings.MaxBannedNodes);
	}

	// endregion
}}
