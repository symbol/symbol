#include "catapult/api/ChainPackets.h"
#include "catapult/ionet/Packet.h"
#include "catapult/local/p2p/LocalNode.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/int/Configuration.h"
#include "tests/test/int/LocalNodeApiTraits.h"
#include "tests/test/int/LocalNodeIntegrityTestUtils.h"
#include "tests/test/int/LocalNodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

#define TEST_CLASS LocalNodeIntegrityTests

	namespace {
		class TestContext {
		public:
			TestContext() : m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				m_pLocalNode = CreateLocalNode(
						m_serverKeyPair,
						test::LoadLocalNodeConfigurationWithNemesisPluginExtensions(m_tempDir.name()),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeIntegrityTests"));
			}

			BootedLocalNode& localNode() const {
				return *m_pLocalNode;
			}

			test::PeerLocalNodeStats stats() const {
				return test::CountersToPeerLocalNodeStats(localNode().counters());
			}

		private:
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};

		Height RetrieveHeight() {
			test::ExternalSourceConnection connection;

			std::atomic<Height::ValueType> height(0);
			std::atomic_bool gotHeight(false);
			connection.apiCall([&](const auto& remoteApi) {
				remoteApi.chainInfo().then([&gotHeight, &height](auto&& infoFuture) {
					height = infoFuture.get().Height.unwrap();
					gotHeight = true;
				});
			});
			WAIT_FOR(gotHeight);
			return Height(height);
		}

		void AssertReaderConnection(const test::PeerLocalNodeStats& stats) {
			// Assert: the external reader connection is still active
			EXPECT_EQ(2u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		}
	}

	// region push

	TEST(TEST_CLASS, CanPushBlockToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Sanity:
		EXPECT_EQ(Height(1), RetrieveHeight());

		// Act:
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidBlock(connection);

		// - wait for the chain height to change and for all height readers to disconnect
		WAIT_FOR_VALUE_EXPR(RetrieveHeight(), Height(2));
		auto chainHeight = RetrieveHeight();
		WAIT_FOR_VALUE_EXPR(context.stats().NumActiveReaders, 2u);

		// Assert: the chain height is two
		EXPECT_EQ(Height(2), chainHeight);

		// - a single block element was added
		auto stats = context.stats();
		EXPECT_EQ(1u, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is still active
		AssertReaderConnection(stats);
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

	// region pull

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CanGetResponse) {
		// Assert: the connection is still active
		test::AssertCanGetResponse<TApiTraits>(TestContext(), AssertReaderConnection);
	}

	CHAIN_API_INT_INVALID_TRAITS_BASED_TEST(InvalidRequestTriggersException) {
		// Assert: the connection is still active
		test::AssertInvalidRequestTriggersException(TestContext(), TApiTraits::InitiateInvalidRequest, AssertReaderConnection);
	}

	// endregion
}}}
