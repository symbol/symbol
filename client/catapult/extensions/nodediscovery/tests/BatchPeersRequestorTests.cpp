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
		EXPECT_FALSE(result);
		EXPECT_TRUE(context.NodeSets.empty());
	}

	TEST(TEST_CLASS, NoPeersOfPeersAreFoundWhenApiFails) {
		// Arrange:
		TestContext context;

		// - configure one picker to return a packet io with a failure interaction
		context.Writers2.setPacketIo(CreateFailureMockPacketIo());

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_TRUE(context.NodeSets.empty());
	}

	namespace {
		void AssertSingleNodeSet(const ionet::NodeSet& nodes, const Key& expectedKey, const std::string& expectedName) {
			ASSERT_EQ(1u, nodes.size()) << expectedName;

			const auto& node = *nodes.cbegin();
			EXPECT_EQ(expectedKey, node.identityKey()) << expectedName;
			EXPECT_EQ(expectedName, node.metadata().Name) << expectedName;
		}
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceeds) {
		// Arrange:
		TestContext context;

		// - configure one picker to return a packet io with a successful interaction
		auto partnerKey = test::GenerateRandomData<Key_Size>();
		context.Writers2.setPacketIo(CreateSuccessMockPacketIo(partnerKey, "alice"));

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		EXPECT_TRUE(result);
		ASSERT_EQ(1u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerKey, "alice");
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceedsForMultiplePickers) {
		// Arrange:
		TestContext context;

		// - configure two pickers to return a packet io with a successful interaction
		auto partnerKey1 = test::GenerateRandomData<Key_Size>();
		context.Writers1.setPacketIo(CreateSuccessMockPacketIo(partnerKey1, "alice"));

		auto partnerKey2 = test::GenerateRandomData<Key_Size>();
		context.Writers2.setPacketIo(CreateSuccessMockPacketIo(partnerKey2, "bob"));

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		EXPECT_TRUE(result);
		ASSERT_EQ(2u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerKey1, "alice");
		AssertSingleNodeSet(context.NodeSets[1], partnerKey2, "bob");
	}

	TEST(TEST_CLASS, PeersOfPeersAreFoundWhenApiSucceedsForMultiplePickersAmidstSomeFailures) {
		// Arrange:
		TestContext context;

		// - configure two pickers to return a packet io with a successful interaction
		//   and one to return a packet io with a failure interaction
		auto partnerKey1 = test::GenerateRandomData<Key_Size>();
		context.Writers1.setPacketIo(CreateSuccessMockPacketIo(partnerKey1, "alice"));

		context.Writers2.setPacketIo(CreateFailureMockPacketIo());

		auto partnerKey2 = test::GenerateRandomData<Key_Size>();
		context.Writers3.setPacketIo(CreateSuccessMockPacketIo(partnerKey2, "bob"));

		// Act:
		auto result = context.Requestor.findPeersOfPeers(utils::TimeSpan::FromMilliseconds(15)).get();

		// Assert:
		EXPECT_TRUE(result);
		ASSERT_EQ(2u, context.NodeSets.size());

		AssertSingleNodeSet(context.NodeSets[0], partnerKey1, "alice");
		AssertSingleNodeSet(context.NodeSets[1], partnerKey2, "bob");
	}
}}
