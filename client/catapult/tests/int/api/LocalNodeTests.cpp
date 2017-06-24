#include "plugins/mongo/coremongo/src/ExternalCacheStorage.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/api/LocalNode.h"
#include "catapult/model/ChainScore.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/int/LocalNodeTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/local/mocks/MockChainScoreProvider.h"
#include "tests/test/mongo/mocks/MockExternalCacheStorage.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	using NodeFlag = catapult::test::NodeFlag;

	namespace {
		class TestContext {
		public:
			TestContext(NodeFlag nodeFlag) : m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				auto config = test::LoadLocalNodeConfiguration(static_cast<uint64_t>(nodeFlag), m_tempDir.name());
				m_expectedHistorySize = model::CalculateDifficultyHistorySize(config.BlockChain);

				auto pStorage = std::make_unique<mocks::MemoryBasedStorage>();
				m_pChainScoreProvider = std::make_shared<mocks::MockChainScoreProvider>();

				auto pExternalCacheStorage = std::make_unique<mocks::MockExternalCacheStorage<0>>();
				m_pExternalCacheStorage = pExternalCacheStorage.get();

				auto pUnconfirmedTransactionsCache = test::CreateUnconfirmedTransactionsCache();
				auto viewProvider = test::CreateViewProvider(*pUnconfirmedTransactionsCache);
				m_pLocalNode = CreateLocalNode(
						m_serverKeyPair,
						std::move(config),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeTests-api"),
						std::move(pStorage),
						decltype(m_pChainScoreProvider)(m_pChainScoreProvider), // create give-away copy
						std::move(pExternalCacheStorage),
						viewProvider,
						std::move(pUnconfirmedTransactionsCache));
			}

		public:
			BootedLocalNode& localNode() {
				return *m_pLocalNode;
			}

			test::BasicLocalNodeStats stats() const {
				return test::CountersToBasicLocalNodeStats(m_pLocalNode->counters());
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

		public:
			void assertShutdown() const {
				// Assert:
				auto stats = this->stats();
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumActiveReaders);
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumActiveWriters);
				EXPECT_EQ(Sentinel_Stats_Value, stats.NumScheduledTasks);
			}

			void assertStateLoaded() const {
				EXPECT_EQ(1u, m_pExternalCacheStorage->numLoadAllCalls());
				EXPECT_EQ(Height(1), m_pExternalCacheStorage->chainHeight());
			}

		private:
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			uint64_t m_expectedHistorySize;
			std::shared_ptr<mocks::MockChainScoreProvider> m_pChainScoreProvider;
			mocks::MockExternalCacheStorage<0>* m_pExternalCacheStorage;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};
	}

	// region basic tests

	TEST(LocalNodeTests, CanBootLocalNodeWithoutPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithoutPeers<TestContext>([](const auto& context, const auto&) {
			context.assertStateLoaded();
		});
	}

	TEST(LocalNodeTests, CanBootLocalNodeWithPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithPeers<TestContext>([](const auto& context, const auto&) {
			context.assertStateLoaded();
		});
	}

	TEST(LocalNodeTests, CanShutdownNode) {
		// Assert:
		test::AssertCanShutdownLocalNode<TestContext>();
	}

	TEST(LocalNodeTests, AllPeriodicTasksAreScheduled) {
		// Assert:
		test::AssertLocalNodeSchedulesTasks<TestContext>(2);
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
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
	}

	// endregion

	// region connection tests

	TEST(LocalNodeTests, CannotConnectToApiPort) {
		// Assert:
		test::AssertConnectionError<TestContext>(test::Local_Node_Api_Port);
	}

	namespace {
		template<typename THandler>
		void RunExternalReaderTest(THandler handler) {
			// Arrange: boot a local node and wait for the node to connect to the peer
			TestContext context(NodeFlag::Regular);
			context.waitForNumActiveReaders(1);
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(test::Local_Node_Port);
			context.waitForNumActiveReaders(2);
			handler(context);
		}
	}

	TEST(LocalNodeTests, CanConnectToLocalNodeAsReader) {
		// Act:
		RunExternalReaderTest([](auto& context) {
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(2u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		});
	}

	TEST(LocalNodeTests, CanShutdownLocalNodeWithExternalConnections) {
		// Act:
		RunExternalReaderTest([](auto& context) {
			// Act: shutdown the local node
			CATAPULT_LOG(debug) << "shutting down local node";
			context.localNode().shutdown();

			// Assert: the shutdown completed successfully without hanging
			context.assertShutdown();
		});
	}

	// endregion
}}}
