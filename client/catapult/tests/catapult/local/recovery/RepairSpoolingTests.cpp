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

#include "catapult/local/recovery/RepairSpooling.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS RepairSpoolingTests

	namespace {
		// region TestContext

		std::vector<std::string> GetAllQueueNames() {
			return {
				"block_change", "finalization", "transaction_status",
				"partial_transactions_change", "unconfirmed_transactions_change",
				"block_recover",
				"block_sync", "state_change"
			};
		}

		enum class SetupMode { Create_Directories, None };

		class TestContext {
		public:
			TestContext(SetupMode mode, uint64_t indexValue) : m_dataDirectory(m_tempDir.name()) {
				if (SetupMode::None == mode)
					return;

				// initialize all directories with an index file with a predefined value
				// and an unrelated marker file
				for (const auto& queueName : GetAllQueueNames()) {
					auto directory = m_dataDirectory.spoolDir(queueName);
					boost::filesystem::create_directories(directory.path());

					io::IndexFile(directory.file("index.dat")).set(indexValue);
					io::IndexFile(directory.file("marker")).set(0);
				}

				io::IndexFile(rootMarkerFilename()).set(0);
			}

		public:
			void repair(consumers::CommitOperationStep commitStep) {
				return RepairSpooling(m_dataDirectory, commitStep);
			}

		public:
			std::string rootMarkerFilename() const {
				return m_dataDirectory.rootDir().file("marker");
			}

			bool exists(const std::string& queueName) const {
				return boost::filesystem::exists(m_dataDirectory.spoolDir(queueName).path());
			}

			size_t countRootFiles() const {
				auto begin = boost::filesystem::directory_iterator(m_dataDirectory.rootDir().path());
				auto end = boost::filesystem::directory_iterator();
				return static_cast<size_t>(std::count_if(begin, end, [](const auto& entry) {
					return boost::filesystem::is_regular_file(entry.path());
				}));
			}

			size_t countFiles(const std::string& queueName) const {
				auto begin = boost::filesystem::directory_iterator(m_dataDirectory.spoolDir(queueName).path());
				auto end = boost::filesystem::directory_iterator();
				return static_cast<size_t>(std::distance(begin, end));
			}

			uint64_t readIndex(const std::string& queueName, const std::string& indexName) const {
				return io::IndexFile(m_dataDirectory.spoolDir(queueName).file(indexName)).get();
			}

			void setIndex(const std::string& queueName, const std::string& indexName, uint64_t indexValue) {
				io::IndexFile(m_dataDirectory.spoolDir(queueName).file(indexName)).set(indexValue);
			}

			void removeIndex(const std::string& queueName, const std::string& indexName) {
				boost::filesystem::remove(m_dataDirectory.spoolDir(queueName).file(indexName));
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;
		};

		// endregion

		// region test utils

		void AssertPurged(const TestContext& context, const std::string& queueName) {
			EXPECT_TRUE(context.exists(queueName)) << queueName;
			EXPECT_EQ(0u, context.countFiles(queueName)) << queueName;
		}

		void AssertRetained(
				const TestContext& context,
				const std::string& queueName,
				size_t numExpectedFiles,
				uint64_t expectedIndexValue) {
			EXPECT_TRUE(context.exists(queueName)) << queueName;
			EXPECT_EQ(numExpectedFiles, context.countFiles(queueName)) << queueName;
			EXPECT_EQ(expectedIndexValue, context.readIndex(queueName, "index.dat")) << queueName;
			EXPECT_EQ(0u, context.readIndex(queueName, "marker")) << queueName;
		}

		// endregion

		// region traits

		template<consumers::CommitOperationStep CommitStep>
		struct NotStateWrittenTraits {
		public:
			static constexpr auto Commit_Step = CommitStep;

		public:
			static std::vector<std::string> GetPurgedQueueNames() {
				return { "partial_transactions_change", "unconfirmed_transactions_change", "block_recover", "block_sync" };
			}

			static std::vector<std::string> GetRetainedQueueNames() {
				return { "block_change", "finalization", "transaction_status" };
			}

			static uint64_t GetExpectedStateChangeIndexServerValue(uint64_t indexValue, uint64_t) {
				return indexValue;
			}
		};

		using AllUpdatedTraits = NotStateWrittenTraits<consumers::CommitOperationStep::All_Updated>;
		using BlocksWrittenTraits = NotStateWrittenTraits<consumers::CommitOperationStep::Blocks_Written>;

		struct StateWrittenTraits {
		public:
			static constexpr auto Commit_Step = consumers::CommitOperationStep::State_Written;

		public:
			static std::vector<std::string> GetPurgedQueueNames() {
				return { "partial_transactions_change", "unconfirmed_transactions_change", "block_recover" };
			}

			static std::vector<std::string> GetRetainedQueueNames() {
				return { "block_change", "finalization", "transaction_status", "block_sync" };
			}

			static uint64_t GetExpectedStateChangeIndexServerValue(uint64_t, uint64_t indexServerValue) {
				return indexServerValue;
			}
		};

#define COMMIT_STEP_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_AllUpdated) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AllUpdatedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BlocksWritten) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlocksWrittenTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StateWritten) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StateWrittenTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion
	}

	// region tests

	namespace {
		template<typename TTraits>
		void RepairAndCheckNonStateChangeDirectories(TestContext& context, uint64_t expectedIndexValue, size_t numRepairs = 1) {
			// Sanity:
			EXPECT_EQ(7u, TTraits::GetPurgedQueueNames().size() + TTraits::GetRetainedQueueNames().size());
			EXPECT_EQ(1u, context.countRootFiles());

			// Act:
			for (auto i = 0u; i < numRepairs; ++i)
				context.repair(TTraits::Commit_Step);

			// Assert: check purged directories
			for (const auto& purgedQueueName : TTraits::GetPurgedQueueNames())
				AssertPurged(context, purgedQueueName);

			// - only marker left in root dir
			EXPECT_EQ(1u, context.countRootFiles());
			EXPECT_TRUE(boost::filesystem::exists(context.rootMarkerFilename()));

			// - check retained directories
			for (const auto& retainedQueueName : TTraits::GetRetainedQueueNames())
				AssertRetained(context, retainedQueueName, 2, expectedIndexValue);
		}

		template<typename TTraits>
		void AssertCanRepairSpoolingWhenAllDirectoriesArePresent(size_t numRepairs) {
			// Arrange:
			TestContext context(SetupMode::Create_Directories, 111);
			context.setIndex("state_change", "index_server.dat", 123);

			// Act:
			RepairAndCheckNonStateChangeDirectories<TTraits>(context, 111, numRepairs);

			// - check state_change directory
			AssertRetained(context, "state_change", 3, 111);
			EXPECT_EQ(TTraits::GetExpectedStateChangeIndexServerValue(111, 123), context.readIndex("state_change", "index_server.dat"));
		}
	}

	COMMIT_STEP_TEST(CanRepairSpoolingWhenAllDirectoriesArePresent) {
		AssertCanRepairSpoolingWhenAllDirectoriesArePresent<TTraits>(1);
	}

	COMMIT_STEP_TEST(RepairSpoolingWhenAllDirectoriesArePresentIsIdempotent) {
		AssertCanRepairSpoolingWhenAllDirectoriesArePresent<TTraits>(3);
	}

	COMMIT_STEP_TEST(CanRepairSpoolingWhenAllDirectoriesArePresentAndStateChangeIndexDoesNotExist) {
		// Arrange:
		TestContext context(SetupMode::Create_Directories, 111);
		context.setIndex("state_change", "index_server.dat", 123);
		context.removeIndex("state_change", "index.dat");

		// Act:
		RepairAndCheckNonStateChangeDirectories<TTraits>(context, 111);

		// - check state_change directory (index.dat should not be created and index_server.dat value should be unchanged)
		EXPECT_TRUE(context.exists("state_change"));
		EXPECT_EQ(2u, context.countFiles("state_change"));
		EXPECT_EQ(0u, context.readIndex("state_change", "marker"));
		EXPECT_EQ(123u, context.readIndex("state_change", "index_server.dat"));
	}

	COMMIT_STEP_TEST(CanRepairSpoolingWhenAllDirectoriesArePresentAndStateChangeIndexServerDoesNotExist) {
		// Arrange: don't create index_server.dat
		TestContext context(SetupMode::Create_Directories, 111);

		// Act:
		RepairAndCheckNonStateChangeDirectories<TTraits>(context, 111);

		// - check state_change directory
		if (consumers::CommitOperationStep::State_Written == TTraits::Commit_Step) {
			// - index_server.dat should not be created and index.dat value should be unchanged
			AssertRetained(context, "state_change", 2, 111);
		} else {
			// - index_server.dat should be created with index.dat value
			AssertRetained(context, "state_change", 3, 111);
			EXPECT_EQ(111u, context.readIndex("state_change", "index_server.dat"));
		}
	}

	COMMIT_STEP_TEST(RepairHasNoEffectWhenNoSpoolingDirectoriesArePresent) {
		// Arrange:
		TestContext context(SetupMode::None, 111);

		// Sanity:
		EXPECT_EQ(8u, GetAllQueueNames().size());

		// Act:
		context.repair(TTraits::Commit_Step);

		// Assert: no directories should have been created
		for (const auto& queueName : GetAllQueueNames())
			EXPECT_FALSE(context.exists(queueName)) << queueName;
	}

	// endregion
}}
