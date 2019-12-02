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

#include "catapult/extensions/CommitStepHandler.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS CommitStepHandlerTests

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
}}
