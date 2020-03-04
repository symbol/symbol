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

#include "catapult/config/CatapultDataDirectory.h"
#include "tests/int/node/test/LocalNodeApiTraits.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/int/node/test/PeerLocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeRequestTests

	namespace {
		using TestContext = test::PeerLocalNodeTestContext;

		uint64_t ReadIndexFileValue(const std::string& indexFilePath) {
			return io::IndexFile(indexFilePath).get();
		}

		// region TestCatapultDataDirectory

		class TestCatapultDataDirectory {
		public:
			explicit TestCatapultDataDirectory(const boost::filesystem::path& directory) : m_dataDirectory(directory)
			{}

		public:
			std::string commitStep() const {
				return m_dataDirectory.rootDir().file("commit_step.dat");
			}

			std::string stateChangeWriterIndex() const {
				return m_dataDirectory.spoolDir("state_change").file("index_server.dat");
			}

			std::string stateChangeReaderIndex() const {
				return m_dataDirectory.spoolDir("state_change").file("index_server_r.dat");
			}

		private:
			config::CatapultDataDirectory m_dataDirectory;
		};

		// endregion
	}

	// region push

	namespace {
		template<typename TCheckSyncFiles>
		void AssertCanPushBlockToLocalNode(test::NodeFlag nodeFlag, TCheckSyncFiles checkSyncFiles) {
			// Arrange:
			TestContext context(nodeFlag);
			auto dataDirectory = TestCatapultDataDirectory(context.dataDirectory());
			test::WaitForBoot(context);

			// Sanity: two files are produced when booting with nemesis height
			EXPECT_EQ(Height(1), context.height());
			EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());
			EXPECT_TRUE(boost::filesystem::exists(dataDirectory.commitStep()));
			EXPECT_EQ(2u, ReadIndexFileValue(dataDirectory.stateChangeWriterIndex()));

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo = test::PushValidBlock(connection);

			// - wait for the chain height to change and for all height readers to disconnect
			context.waitForHeight(Height(2));
			auto chainHeight = context.height();
			WAIT_FOR_ONE_EXPR(context.stats().NumActiveReaders);

			// Assert: the chain height is two
			EXPECT_EQ(Height(2), chainHeight);

			// - a single block element was added
			auto stats = context.stats();
			EXPECT_EQ(1u, stats.NumAddedBlockElements);
			EXPECT_EQ(0u, stats.NumAddedTransactionElements);

			// - sync updated files as expected
			checkSyncFiles(context, dataDirectory);

			// - the connection is still active
			context.assertSingleReaderConnection();
		}
	}

	TEST(TEST_CLASS, CanPushBlockToLocalNodeWithCacheDatabaseStorageDisabled) {
		AssertCanPushBlockToLocalNode(test::NodeFlag::Regular, [](const auto& context, const auto&) {
			// Assert: supplemental state should not have been updated
			EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());
		});
	}

	TEST(TEST_CLASS, CanPushBlockToLocalNodeWithCacheDatabaseStorageEnabled) {
		AssertCanPushBlockToLocalNode(test::NodeFlag::Cache_Database_Storage, [](const auto& context, const auto&) {
			// Assert: supplemental state should have been updated
			EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());
		});
	}

	TEST(TEST_CLASS, CanPushBlockToLocalNodeWithoutAutoSyncCleanup) {
		AssertCanPushBlockToLocalNode(test::NodeFlag::Regular, [](const auto&, const auto& dataDirectory) {
			// Assert: state_change subscriber produces two files each in boot and per sync
			EXPECT_EQ(2u, ReadIndexFileValue(dataDirectory.commitStep()));
			EXPECT_EQ(4u, ReadIndexFileValue(dataDirectory.stateChangeWriterIndex()));
			EXPECT_FALSE(boost::filesystem::exists(dataDirectory.stateChangeReaderIndex()));
		});
	}

	TEST(TEST_CLASS, CanPushBlockToLocalNodeWithAutoSyncCleanup) {
		AssertCanPushBlockToLocalNode(test::NodeFlag::Auto_Sync_Cleanup, [](const auto&, const auto& dataDirectory) {
			// Assert: state_change subscriber produces two files each in boot and per sync and all files were consumed
			EXPECT_EQ(2u, ReadIndexFileValue(dataDirectory.commitStep()));
			EXPECT_EQ(4u, ReadIndexFileValue(dataDirectory.stateChangeWriterIndex()));
			EXPECT_EQ(4u, ReadIndexFileValue(dataDirectory.stateChangeReaderIndex()));
		});
	}

	TEST(TEST_CLASS, CanPushTransactionToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		test::ExternalSourceConnection connection(context.publicKey());
		auto pIo = test::PushValidTransaction(connection);
		WAIT_FOR_ONE_EXPR(context.stats().NumAddedTransactionElements);

		// Assert: a single transaction element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(1u, stats.NumAddedTransactionElements);

		// - the connection is still active
		context.assertSingleReaderConnection();
	}

	// endregion

	// region pull

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CanGetResponse) {
		// Assert: the connection is still active
		test::AssertCanGetResponse<TApiTraits>(TestContext(), TestContext::AssertSingleReaderConnection);
	}

	CHAIN_API_INT_INVALID_TRAITS_BASED_TEST(InvalidRequestTriggersException) {
		// Assert: the connection is still active
		test::AssertInvalidRequestTriggersException(
				TestContext(),
				TApiTraits::InitiateInvalidRequest,
				TestContext::AssertSingleReaderConnection);
	}

	// endregion
}}
