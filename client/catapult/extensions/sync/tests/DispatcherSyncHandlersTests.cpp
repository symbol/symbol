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

#include "sync/src/DispatcherSyncHandlers.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace sync {

#define TEST_CLASS DispatcherSyncHandlersTests

	// region CreateCommitStepHandler - test context

	namespace {
		class CreateCommitStepHandlerTestContext {
		public:
			explicit CreateCommitStepHandlerTestContext(uint64_t syncIndexWriterValue)
					: m_dataDirectory(m_tempDir.name())
					, m_commitStep(CreateCommitStepHandler(m_dataDirectory)) {
				auto stateChangeDirectory = m_dataDirectory.spoolDir("state_change");
				boost::filesystem::create_directories(stateChangeDirectory.path());

				if (0 != syncIndexWriterValue)
					io::IndexFile(stateChangeDirectory.file("index_server.dat")).set(syncIndexWriterValue);
			}

		public:
			consumers::CommitOperationStep readCommitStep() const {
				return static_cast<consumers::CommitOperationStep>(io::IndexFile(m_dataDirectory.rootDir().file("commit_step.dat")).get());
			}

			bool existsIndexWriterValue() const {
				return indexWriterFile().exists();
			}

			uint64_t readIndexWriterValue() const {
				return indexWriterFile().get();
			}

		public:
			void commitStep(consumers::CommitOperationStep step) {
				m_commitStep(step);
			}

		private:
			io::IndexFile indexWriterFile() const {
				return io::IndexFile(m_dataDirectory.spoolDir("state_change").file("index.dat"));
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;
			consumers::BlockChainSyncHandlers::CommitStepFunc m_commitStep;
		};
	}

	// endregion

	// region CreateCommitStepHandler - tests

	namespace {
		void AssertOnlyCommitStepFileIsUpdated(consumers::CommitOperationStep step) {
			// Arrange:
			CreateCommitStepHandlerTestContext context(123);

			// Act:
			context.commitStep(step);

			// Assert:
			EXPECT_EQ(step, context.readCommitStep());
			EXPECT_FALSE(context.existsIndexWriterValue());
		}
	}

	TEST(TEST_CLASS, CommitStepFileIsUpdatedWhenOperationIsBlocksWritten) {
		AssertOnlyCommitStepFileIsUpdated(consumers::CommitOperationStep::Blocks_Written);
	}

	TEST(TEST_CLASS, CommitStepFileIsUpdatedWhenOperationIsStateWritten) {
		AssertOnlyCommitStepFileIsUpdated(consumers::CommitOperationStep::State_Written);
	}

	TEST(TEST_CLASS, CommitStepFileIsUpdatedWhenOperationIsAllUpdatedAndSyncIndexWriterFileDoesNotExist) {
		// Arrange: don't create sync index writer file
		CreateCommitStepHandlerTestContext context(0);

		// Act:
		context.commitStep(consumers::CommitOperationStep::All_Updated);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, context.readCommitStep());
		EXPECT_FALSE(context.existsIndexWriterValue());
	}

	TEST(TEST_CLASS, CommitStepFileAndIndexWriterFileAreUpdatedWhenOperationIsAllUpdatedAndSyncIndexWriterFileExists) {
		// Arrange:
		CreateCommitStepHandlerTestContext context(123);

		// Act:
		context.commitStep(consumers::CommitOperationStep::All_Updated);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, context.readCommitStep());
		EXPECT_TRUE(context.existsIndexWriterValue());
		EXPECT_EQ(123u, context.readIndexWriterValue());
	}

	// endregion

	// region AddSupplementalDataResiliency - test context

	namespace {
		class AddSupplementalDataResiliencyTestContext {
		public:
			struct Counters {
				size_t NumPreStateWrittenCalls = 0;
				size_t NumCommitStepCalls = 0;
			};

		public:
			AddSupplementalDataResiliencyTestContext()
					: m_dataDirectory(m_tempDir.name())
					, m_cache({}) {
				m_syncHandlers.PreStateWritten = [&counters = m_counters](const auto&, const auto&, auto) {
					++counters.NumPreStateWrittenCalls;
				};
				m_syncHandlers.CommitStep = [&counters = m_counters](auto) {
					++counters.NumCommitStepCalls;
				};

				AddSupplementalDataResiliency(m_syncHandlers, m_dataDirectory, m_cache, m_score);
			}

		public:
			const auto& counters() const {
				return m_counters;
			}

			const auto& syncHandlers() const {
				return m_syncHandlers;
			}

			std::string supplementalPath(const std::string& directory) const {
				return m_dataDirectory.dir(directory).file("supplemental.dat");
			}

		public:
			void runPreStateWrittenTest() {
				// Act:
				m_syncHandlers.PreStateWritten(m_cache.createDelta(), state::CatapultState(), Height());

				// Assert:
				EXPECT_EQ(1u, m_counters.NumPreStateWrittenCalls);
				EXPECT_TRUE(boost::filesystem::exists(supplementalPath("state.tmp")));
				EXPECT_FALSE(boost::filesystem::exists(supplementalPath("state")));
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;
			cache::CatapultCache m_cache;
			extensions::LocalNodeChainScore m_score;
			Counters m_counters;
			consumers::BlockChainSyncHandlers m_syncHandlers;
		};
	}

	// endregion

	// region AddSupplementalDataResiliency - tests

	TEST(TEST_CLASS, AddSupplementalDataResiliency_PreStateWrittenWritesSupplementalDataToTempDirectory) {
		// Arrange:
		AddSupplementalDataResiliencyTestContext context;

		// Act + Assert:
		context.runPreStateWrittenTest();
	}

	namespace {
		void AssertAddSupplementalDataResiliencyCommitStepDoesNothing(consumers::CommitOperationStep step) {
			// Arrange:
			AddSupplementalDataResiliencyTestContext context;
			context.runPreStateWrittenTest();

			// Act:
			context.syncHandlers().CommitStep(step);

			// Assert:
			EXPECT_EQ(1u, context.counters().NumPreStateWrittenCalls);
			EXPECT_EQ(1u, context.counters().NumCommitStepCalls);
			EXPECT_TRUE(boost::filesystem::exists(context.supplementalPath("state.tmp")));
			EXPECT_FALSE(boost::filesystem::exists(context.supplementalPath("state")));
		}
	}

	TEST(TEST_CLASS, AddSupplementalDataResiliency_CommitStepDoesNothingWhenOperationIsBlocksWritten) {
		AssertAddSupplementalDataResiliencyCommitStepDoesNothing(consumers::CommitOperationStep::Blocks_Written);
	}

	TEST(TEST_CLASS, AddSupplementalDataResiliency_CommitStepDoesNothingWhenOperationIsStateWritten) {
		AssertAddSupplementalDataResiliencyCommitStepDoesNothing(consumers::CommitOperationStep::State_Written);
	}

	TEST(TEST_CLASS, AddSupplementalDataResiliency_CommitStepMovesSupplementalDataWhenOperationIsAllUpdated) {
		// Arrange:
		AddSupplementalDataResiliencyTestContext context;
		context.runPreStateWrittenTest();

		// Act:
		context.syncHandlers().CommitStep(consumers::CommitOperationStep::All_Updated);

		// Assert:
		EXPECT_EQ(1u, context.counters().NumPreStateWrittenCalls);
		EXPECT_EQ(1u, context.counters().NumCommitStepCalls);
		EXPECT_FALSE(boost::filesystem::exists(context.supplementalPath("state.tmp")));
		EXPECT_TRUE(boost::filesystem::exists(context.supplementalPath("state")));
	}

	// endregion
}}
