#include "catapult/local/p2p/LocalNode.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/io/RawFile.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/int/Configuration.h"
#include "tests/test/int/LocalNodeTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace local { namespace p2p {

	using NodeFlag = catapult::test::NodeFlag;

	namespace {
		class TestContext {
		public:
			TestContext(NodeFlag nodeFlag) : m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				m_pLocalNode = reboot(createConfig(nodeFlag));
			}

		public:
			BootedLocalNode& localNode() {
				return *m_pLocalNode;
			}

			test::PeerLocalNodeStats stats() const {
				return test::CountersToPeerLocalNodeStats(m_pLocalNode->counters());
			}

		public:
			config::LocalNodeConfiguration createConfig(NodeFlag nodeFlag) {
				return test::LoadLocalNodeConfiguration(static_cast<uint64_t>(nodeFlag), m_tempDir.name());
			}

			std::unique_ptr<BootedLocalNode> reboot(config::LocalNodeConfiguration&& config) const {
				// in order for the nemesis block to be processed, at least the transfer plugin needs to be loaded
				// notice that the api LocalNodeTests do not actually process the nemesis block, so they don't need plugin extensions
				test::AddNemesisPluginExtensions(const_cast<model::BlockChainConfiguration&>(config.BlockChain));

				return CreateLocalNode(
						m_serverKeyPair,
						std::move(config),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeTests-p2p"));
			}

		public:
			void waitForNumActiveReaders(size_t value) const {
				WAIT_FOR_VALUE_EXPR(stats().NumActiveReaders, value);
			}

			void waitForNumActiveWriters(size_t value) const {
				WAIT_FOR_VALUE_EXPR(stats().NumActiveWriters, value);
			}

			void waitForNumScheduledTasks(size_t value) const {
				WAIT_FOR_VALUE_EXPR(stats().NumScheduledTasks, value);
			}

			void waitForNumActiveBroadcastWriters(size_t value) const {
				WAIT_FOR_VALUE_EXPR(stats().NumActiveBroadcastWriters, value);
			}

		public:
			bool hasSavedState() const {
				return boost::filesystem::exists(getSupplementalStatePath());
			}

			Height loadSavedChainHeight() const {
				auto path = getSupplementalStatePath();
				io::RawFile file(path.string(), io::OpenMode::Read_Only);
				file.seek(file.size() - sizeof(Height));
				return io::Read<Height>(file);
			}

		public:
			void assertShutdown() const {
				// Assert:
				auto stats = this->stats();
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumActiveReaders);
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumActiveWriters);
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumScheduledTasks);
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumActiveBroadcastWriters);
			}

		private:
			boost::filesystem::path getSupplementalStatePath() const {
				return boost::filesystem::path(m_tempDir.name()) / "state" / "supplemental.dat";
			}

		private:
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};
	}

	// region basic tests

	TEST(LocalNodeTests, CanBootLocalNodeWithoutPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithoutPeers<TestContext>([](const auto&, const auto& stats) {
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(LocalNodeTests, CanBootLocalNodeWithPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithPeers<TestContext>([](const auto&, const auto& stats) {
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(LocalNodeTests, CanShutdownNode) {
		// Assert:
		test::AssertCanShutdownLocalNode<TestContext>();
	}

	TEST(LocalNodeTests, AllPeriodicTasksAreScheduled) {
		// Assert:
		test::AssertLocalNodeSchedulesTasks<TestContext>(5);
	}

	TEST(LocalNodeTests, AllCounterGroupsAreRegistered) {
		// Act:
		TestContext context(NodeFlag::Regular);

		const auto& counters = context.localNode().counters();
		for (const auto& counter : counters)
			CATAPULT_LOG(debug) << "counter: " << counter.id().name();

		// Assert: check candidate counters
		EXPECT_TRUE(test::HasCounter(counters, "ACNTST C")) << "cache counters";
		EXPECT_TRUE(test::HasCounter(counters, "TX ELEMENTS")) << "basic local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "UNLKED ACCTS")) << "peer local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "UNCNFRMTX C")) << "peer local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
	}

	// endregion

	// region auto harvest tests

	namespace {
		constexpr bool Auto_Harvest = true;

		void AssertNumUnlockedAccounts(bool shouldAutoHarvest, size_t expectedNumUnlockedAccounts) {
			// Arrange:
			auto flags = shouldAutoHarvest ? NodeFlag::Auto_Harvest : NodeFlag::Regular;

			// Act
			TestContext context(flags);
			context.waitForNumActiveReaders(0);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(expectedNumUnlockedAccounts, stats.NumUnlockedAccounts);
		}
	}

	TEST(LocalNodeTests, BootUnlocksAccountIfAutoHarvestIsEnabled) {
		// Assert:
		AssertNumUnlockedAccounts(Auto_Harvest, 1u);
	}

	TEST(LocalNodeTests, BootDoesNotUnlockAccountIfAutoHarvestIsDisabled) {
		// Assert:
		AssertNumUnlockedAccounts(!Auto_Harvest, 0u);
	}

	// endregion

	// region connection tests

	namespace {
		template<typename THandler>
		void RunExternalConnectionTest(unsigned short port, THandler handler) {
			// Arrange: boot a local node and wait for the node to connect to the peer
			TestContext context(NodeFlag::Regular);
			context.waitForNumActiveReaders(1);
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(port);
			handler(context);
		}
	}

	TEST(LocalNodeTests, CanConnectToLocalNodeAsReader) {
		// Act:
		RunExternalConnectionTest(test::Local_Node_Port, [](auto& context) {
			context.waitForNumActiveReaders(2);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(2u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(LocalNodeTests, CanConnectToLocalNodeAsBroadcastWriter) {
		// Act:
		RunExternalConnectionTest(test::Local_Node_Api_Port, [](auto& context) {
			context.waitForNumActiveBroadcastWriters(1);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(1u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(LocalNodeTests, CanShutdownLocalNodeWithExternalConnections) {
		// Arrange: boot a local node and wait for the node to connect to the peer
		TestContext context(NodeFlag::Regular);
		context.waitForNumActiveReaders(1);
		context.waitForNumActiveWriters(1);

		// Act: create external connections to the node
		auto connection1 = test::CreateExternalConnection(test::Local_Node_Port);
		auto connection2 = test::CreateExternalConnection(test::Local_Node_Api_Port);
		context.waitForNumActiveReaders(2);
		context.waitForNumActiveBroadcastWriters(1);

		// Act: shutdown the local node
		CATAPULT_LOG(debug) << "shutting down local node";
		context.localNode().shutdown();

		// Assert: the shutdown completed successfully without hanging
		context.assertShutdown();
	}

	// endregion

	// region state saving

	TEST(LocalNodeTests, ShutdownSavesStateToDiskOnSuccessfulBoot) {
		// Arrange:
		TestContext context(NodeFlag::Regular);

		// Sanity:
		EXPECT_FALSE(context.hasSavedState());

		// Act:
		context.localNode().shutdown();

		// Assert:
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());
	}

	TEST(LocalNodeTests, ShutdownDoesNotSaveStateToDiskOnFailedBoot) {
		// Arrange: create saved state
		TestContext context(NodeFlag::Regular);
		context.localNode().shutdown();

		// Sanity:
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());

		// - prepare bad config
		auto badConfig = context.createConfig(NodeFlag::Regular);
		const_cast<model::BlockChainConfiguration&>(badConfig.BlockChain).Plugins.emplace(
				"catapult.plugins.awesome",
				utils::ConfigurationBag({}));

		// Act: simulate a boot failure by specifying an incorrect plugin
		EXPECT_THROW(context.reboot(std::move(badConfig)), catapult_invalid_argument);

		// Assert: the config was not overwritten
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());
	}

	// endregion
}}}
