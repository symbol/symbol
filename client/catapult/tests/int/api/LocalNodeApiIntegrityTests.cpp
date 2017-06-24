#include "plugins/mongo/coremongo/src/AggregateExternalCacheStorage.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/ionet/Packet.h"
#include "catapult/local/api/LocalNode.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/int/Configuration.h"
#include "tests/test/int/LocalNodeApiTraits.h"
#include "tests/test/int/LocalNodeIntegrityTestUtils.h"
#include "tests/test/int/LocalNodeTestUtils.h"
#include "tests/test/local/mocks/MockChainScoreProvider.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

#define TEST_CLASS LocalNodeApiIntegrityTests

	namespace {
		class TestContext {
		public:
			TestContext() : m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				auto config = test::LoadLocalNodeConfigurationWithNemesisPluginExtensions(m_tempDir.name());

				auto pStorage = std::make_unique<mocks::MemoryBasedStorage>();
				m_pChainScoreProvider = std::make_shared<mocks::MockChainScoreProvider>();

				auto pUnconfirmedTransactionsCache = test::CreateUnconfirmedTransactionsCache();
				auto viewProvider = test::CreateViewProvider(*pUnconfirmedTransactionsCache);
				m_pLocalNode = CreateLocalNode(
						m_serverKeyPair,
						std::move(config),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeApiIntegrityTests"),
						std::move(pStorage),
						decltype(m_pChainScoreProvider)(m_pChainScoreProvider), // create give-away copy
						std::make_unique<mongo::plugins::AggregateExternalCacheStorage>(
								mongo::plugins::AggregateExternalCacheStorage::StorageContainer()),
						viewProvider,
						std::move(pUnconfirmedTransactionsCache));
			}

			BootedLocalNode& localNode() const {
				return *m_pLocalNode;
			}

			test::BasicLocalNodeStats stats() const {
				return test::CountersToBasicLocalNodeStats(localNode().counters());
			}

		private:
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			std::shared_ptr<mocks::MockChainScoreProvider> m_pChainScoreProvider;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};

		void AssertReaderConnection(const test::BasicLocalNodeStats& stats) {
			// Assert: the external reader connection is still active
			EXPECT_EQ(2u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		}

		void AssertNoReaderConnection(const test::BasicLocalNodeStats& stats) {
			// Assert: the external reader connection is not active (only self connection)
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		}
	}

	// region push

	TEST(TEST_CLASS, CannotPushBlockToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		// - note that push valid block will create a new reader connection, increasing the number of readers from 1 (self) to 2
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidBlock(connection);

		// - wait for the external reader to be closed by the server
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveReaders);

		// Assert: no block element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is no longer active
		AssertNoReaderConnection(stats);
	}

	TEST(TEST_CLASS, CanPushTransactionToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidTransaction(connection);
		WAIT_FOR_ONE_EXPR(context.stats().NumAddedTransactionElements);

		// Assert: a single transaction element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(1u, stats.NumAddedTransactionElements);

		// - the connection is still active
		AssertReaderConnection(stats);
	}

	// endregion

	// region pull (unsupported)

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CannotGetResponse) {
		// Assert: the connection is no longer active
		test::AssertInvalidRequestTriggersException(TestContext(), TApiTraits::InitiateValidRequest, AssertNoReaderConnection);
	}

	// endregion
}}}
