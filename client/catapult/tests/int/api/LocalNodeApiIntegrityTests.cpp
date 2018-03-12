#include "tests/test/int/LocalNodeApiTraits.h"
#include "tests/test/int/LocalNodeIntegrityTestUtils.h"
#include "tests/test/int/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeApiIntegrityTests

	namespace {
		class TestContext {
		public:
			TestContext() : m_context(test::NodeFlag::With_Partner | test::NodeFlag::Simulated_Api, { test::CreateLocalPartnerNode() })
			{}

			auto stats() const {
				return m_context.stats();
			}

		private:
			test::LocalNodeTestContext<test::LocalNodeApiTraits> m_context;
		};

		void AssertReaderConnection(const test::BasicLocalNodeStats& stats) {
			// Assert: the external reader connection is still active
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		}

		void AssertNoReaderConnection(const test::BasicLocalNodeStats& stats) {
			// Assert: the external reader connection is not active
			EXPECT_EQ(0u, stats.NumActiveReaders);
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
		WAIT_FOR_ZERO_EXPR(context.stats().NumActiveReaders);

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
}}
