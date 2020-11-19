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

#include "nodediscovery/src/BatchPeersRequestor.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS BatchPeersRequestorTests

	namespace {
		net::PacketIoPickerContainer CreatePickerContainer(
				mocks::PickOneAwareMockPacketWriters& writers1,
				mocks::PickOneAwareMockPacketWriters& writers2,
				mocks::PickOneAwareMockPacketWriters& writers3) {
			// notice that varied roles are used because the requestor should ignore roles
			net::PacketIoPickerContainer container;
			container.insert(writers1, ionet::NodeRoles::None);
			container.insert(writers2, ionet::NodeRoles::Api | ionet::NodeRoles::Peer);
			container.insert(writers3, ionet::NodeRoles::Api);
			return container;
		}

		auto CaptureNodes(std::vector<ionet::NodeSet>& nodeSets) {
			return [&nodeSets](const auto& nodes) {
				nodeSets.push_back(nodes);
			};
		}

		struct TestContext {
		public:
			// notice that the container is held by value
			TestContext() : Requestor(CreatePickerContainer(Writers1, Writers2, Writers3), CaptureNodes(NodeSets))
			{}

		public:
			mocks::PickOneAwareMockPacketWriters Writers1;
			mocks::PickOneAwareMockPacketWriters Writers2;
			mocks::PickOneAwareMockPacketWriters Writers3;
			std::vector<ionet::NodeSet> NodeSets;
			BatchPeersRequestor Requestor;
		};

		auto CreateFailureMockPacketIo() {
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Write_Error);
			return pPacketIo;
		}

		auto CreateSuccessMockPacketIo(const Key& key, const std::string& name) {
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, [&key, name](const auto*) {
				// - push ping and pull peers packets are compatible, so create the former and change the type
				auto pPacket = test::CreateNodePushPingPacket(key, ionet::NodeVersion(1234), "", name);
				pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Peers;
				return pPacket;
			});

			return pPacketIo;
		}

		void AssertEqualRemoteApiResults(
				const std::vector<ionet::NodeInteractionResult>& expectedResults,
				const std::vector<ionet::NodeInteractionResult>& results) {
			ASSERT_EQ(expectedResults.size(), results.size());
			for (auto i = 0u; i < results.size(); ++i) {
				EXPECT_EQ(expectedResults[i].Identity.PublicKey, results[i].Identity.PublicKey) << "at index " << i;
				EXPECT_EQ(expectedResults[i].Identity.Host, results[i].Identity.Host) << "at index " << i;
				EXPECT_EQ(expectedResults[i].Code, results[i].Code) << "at index " << i;
			}
		}
	}

	TEST(TEST_CLASS, TimeoutIsPassedDownToPickers) {
		// Arrange:
		TestContext context;

		// Act:
		context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		const auto& expectedDurations = std::vector<utils::TimeSpan>{ utils::TimeSpan::FromMilliseconds(15) };
		EXPECT_EQ(expectedDurations, context.Writers1.pickOneDurations());
		EXPECT_EQ(expectedDurations, context.Writers2.pickOneDurations());
		EXPECT_EQ(expectedDurations, context.Writers3.pickOneDurations());
	}

	TEST(TEST_CLASS, NoPeersOfPeersAreFoundWhenNoPacketIosAreAvailable) {
		// Arrange:
		TestContext context;

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		EXPECT_TRUE(result.empty());
		EXPECT_TRUE(context.NodeSets.empty());
	}

	TEST(TEST_CLASS, NoPeersOfPeersAreFoundWhenApiFails) {
		// Arrange:
		TestContext context;

		// - configure one picker to return a packet io with a failure interaction
		auto partnerIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		context.Writers2.setPacketIo(CreateFailureMockPacketIo());
		context.Writers2.setNodeIdentity(partnerIdentity);

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		std::vector<ionet::NodeInteractionResult> expectedResult{ { partnerIdentity, ionet::NodeInteractionResultCode::Failure } };
		AssertEqualRemoteApiResults(expectedResult, result);

		EXPECT_TRUE(context.NodeSets.empty());
	}

	namespace {
		void AssertSingleNodeSet(
				const ionet::NodeSet& nodes,
				const model::NodeIdentity& expectedIdentity,
				const std::string& expectedName) {
			ASSERT_EQ(1u, nodes.size()) << expectedName;

			const auto& node = *nodes.cbegin();
			EXPECT_EQ(expectedIdentity.PublicKey, node.identity().PublicKey) << expectedName;
			EXPECT_EQ("", node.identity().Host) << expectedName;
			EXPECT_EQ(expectedName, node.metadata().Name) << expectedName;
		}
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceeds) {
		// Arrange:
		TestContext context;

		// - configure one picker to return a packet io with a successful interaction
		auto partnerIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		context.Writers2.setPacketIo(CreateSuccessMockPacketIo(partnerIdentity.PublicKey, "alice"));
		context.Writers2.setNodeIdentity(partnerIdentity);

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		std::vector<ionet::NodeInteractionResult> expectedResult{ { partnerIdentity, ionet::NodeInteractionResultCode::Success } };
		AssertEqualRemoteApiResults(expectedResult, result);

		ASSERT_EQ(1u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerIdentity, "alice");
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceedsForMultiplePickers) {
		// Arrange:
		TestContext context;

		// - configure two pickers to return a packet io with a successful interaction
		auto partnerIdentity1 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		context.Writers1.setPacketIo(CreateSuccessMockPacketIo(partnerIdentity1.PublicKey, "alice"));
		context.Writers1.setNodeIdentity(partnerIdentity1);

		auto partnerIdentity2 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "22.33.44.55" };
		context.Writers2.setPacketIo(CreateSuccessMockPacketIo(partnerIdentity2.PublicKey, "bob"));
		context.Writers2.setNodeIdentity(partnerIdentity2);

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		std::vector<ionet::NodeInteractionResult> expectedResult{
			{ partnerIdentity1, ionet::NodeInteractionResultCode::Success },
			{ partnerIdentity2, ionet::NodeInteractionResultCode::Success }
		};
		AssertEqualRemoteApiResults(expectedResult, result);

		ASSERT_EQ(2u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerIdentity1, "alice");
		AssertSingleNodeSet(context.NodeSets[1], partnerIdentity2, "bob");
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceedsForMultiplePickersAmidstSomeFailures) {
		// Arrange:
		TestContext context;

		// - configure two pickers to return a packet io with a successful interaction
		//   and one to return a packet io with a failure interaction
		auto partnerIdentity1 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		context.Writers1.setPacketIo(CreateSuccessMockPacketIo(partnerIdentity1.PublicKey, "alice"));
		context.Writers1.setNodeIdentity(partnerIdentity1);

		auto partnerIdentity2 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "22.33.44.55" };
		context.Writers2.setPacketIo(CreateFailureMockPacketIo());
		context.Writers2.setNodeIdentity(partnerIdentity2);

		auto partnerIdentity3 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "33.44.55.66" };
		context.Writers3.setPacketIo(CreateSuccessMockPacketIo(partnerIdentity3.PublicKey, "bob"));
		context.Writers3.setNodeIdentity(partnerIdentity3);

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		std::vector<ionet::NodeInteractionResult> expectedResult{
			{ partnerIdentity1, ionet::NodeInteractionResultCode::Success },
			{ partnerIdentity2, ionet::NodeInteractionResultCode::Failure },
			{ partnerIdentity3, ionet::NodeInteractionResultCode::Success }
		};
		AssertEqualRemoteApiResults(expectedResult, result);

		ASSERT_EQ(2u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerIdentity1, "alice");
		AssertSingleNodeSet(context.NodeSets[1], partnerIdentity3, "bob");
	}
}}
