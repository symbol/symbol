#include "catapult/local/SyncScheduledTasks.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/net/PacketWriters.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

	// region CreateConnectPeersTask

	// region MockPacketWriters

	namespace {
		class MockPacketWriters : public net::PacketWriters {
		public:
			void connect(const ionet::Node& node, const ConnectCallback& callback) override {
				m_nodes.push_back(node);

				// call the callback from a separate thread after some delay
				std::thread([callback]() {
					test::Pause();
					callback(net::PeerConnectResult::Accepted);
				}).detach();
			}

		public:
			const std::vector<ionet::Node>& connectedNodes() const {
				return m_nodes;
			}

		// region not implemented

		public:
			size_t numActiveConnections() const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			size_t numActiveWriters() const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			size_t numAvailableWriters() const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			void broadcast(const ionet::PacketPayload&) override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			ionet::NodePacketIoPair pickOne(const utils::TimeSpan&) override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			void accept(
					const std::shared_ptr<net::AsyncTcpServerAcceptContext>&,
					const ConnectCallback&) override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

			void shutdown() override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

		// endregion

		private:
			std::vector<ionet::Node> m_nodes;
		};
	}

	// endregion

	TEST(SyncScheduledTasksTests, CreateConnectPeersTask_CanCreateTask) {
		// Arrange:
		auto peers = std::vector<ionet::Node>();
		MockPacketWriters writers;

		// Act:
		auto task = CreateConnectPeersTask(peers, writers);

		// Assert:
		EXPECT_EQ("connect peers task", task.Name);
		EXPECT_EQ(utils::TimeSpan::FromMilliseconds(10), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(1), task.RepeatDelay);
	}

	TEST(SyncScheduledTasksTests, CreateConnectPeersTask_SucceedsWhenThereAreNoPeers) {
		// Arrange:
		auto peers = std::vector<ionet::Node>();
		MockPacketWriters writers;
		auto task = CreateConnectPeersTask(peers, writers);

		// Act:
		auto result = task.Callback().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);
		EXPECT_TRUE(writers.connectedNodes().empty());
	}

	TEST(SyncScheduledTasksTests, CreateConnectPeersTask_SucceedsWhenThereArePeers) {
		// Arrange:
		auto peers = std::vector<ionet::Node>{
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()),
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()),
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>())
		};
		MockPacketWriters writers;
		auto task = CreateConnectPeersTask(peers, writers);

		// Act:
		auto result = task.Callback().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);
		ASSERT_EQ(peers.size(), writers.connectedNodes().size());
		for (auto i = 0u; i < peers.size(); ++i)
			EXPECT_EQ(peers[i], writers.connectedNodes()[i]) << "node at " << i;
	}

	// endregion

	// region CreateSynchronizerTask

	TEST(SyncScheduledTasksTests, CreateSynchronizerTask_CanCreateTask) {
		// Arrange:
		auto config = test::CreateUninitializedLocalNodeConfiguration();
		auto pStorage = mocks::CreateMemoryBasedStorageCache(3);
		model::TransactionRegistry registry;
		test::DefaultPacketWritersHolder packetWritersHolder;

		// Act:
		auto task = CreateSynchronizerTask(
				config,
				*pStorage,
				registry,
				packetWritersHolder.get(),
				[]() { return model::ChainScore(); },
				[]() { return model::ShortHashRange(); },
				[](const auto&, const auto&) { return 0u; },
				[](const auto&) {});

		// Assert:
		EXPECT_EQ("synchronizer task", task.Name);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(3), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(3), task.RepeatDelay);
	}

	// endregion
}}
