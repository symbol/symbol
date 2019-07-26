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

#include "catapult/local/recovery/CatapultSystemState.h"
#include "catapult/io/IndexFile.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS CatapultSystemStateTests

	namespace {
		// region TestOptions / TestContext

		struct TestOptions {
			bool CreateBrokerLock;
			bool CreateServerLock;
			bool CreateCommitStep;
			consumers::CommitOperationStep OperationStep;
		};

		class TestContext {
		public:
			explicit TestContext(const TestOptions& options)
					: m_dataDirectory(m_tempDir.name())
					, m_systemState(m_dataDirectory) {
				if (options.CreateBrokerLock)
					createLockFile("broker.lock");

				if (options.CreateServerLock)
					createLockFile("server.lock");

				if (options.CreateCommitStep)
					io::IndexFile(m_dataDirectory.rootDir().file("commit_step.dat")).set(static_cast<uint64_t>(options.OperationStep));
			}

		public:
			auto& systemState() {
				return m_systemState;
			}

		private:
			void createLockFile(const std::string& filename) const {
				// simulate an abandoned lock file by creating an empty file (using FileLock will prevent delete on windows)
				io::RawFile(m_dataDirectory.rootDir().file(filename), io::OpenMode::Read_Write);
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;
			CatapultSystemState m_systemState;
		};

		// endregion
	}

	// region constructor / detection

	TEST(TEST_CLASS, CanCreateAroundNoFiles) {
		// Arrange:
		TestContext context({ false, false, false, consumers::CommitOperationStep::Blocks_Written });
		const auto& systemState = context.systemState();

		// Act + Assert:
		EXPECT_FALSE(systemState.shouldRecoverBroker());
		EXPECT_FALSE(systemState.shouldRecoverServer());
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, systemState.commitStep());
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyBrokerLock) {
		// Arrange:
		TestContext context({ true, false, false, consumers::CommitOperationStep::Blocks_Written });
		const auto& systemState = context.systemState();

		// Act + Assert:
		EXPECT_TRUE(systemState.shouldRecoverBroker());
		EXPECT_FALSE(systemState.shouldRecoverServer());
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, systemState.commitStep());
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyServerLock) {
		// Arrange:
		TestContext context({ false, true, false, consumers::CommitOperationStep::Blocks_Written });
		const auto& systemState = context.systemState();

		// Act + Assert:
		EXPECT_FALSE(systemState.shouldRecoverBroker());
		EXPECT_TRUE(systemState.shouldRecoverServer());
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, systemState.commitStep());
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyCommitStep) {
		// Arrange:
		TestContext context({ false, false, true, consumers::CommitOperationStep::Blocks_Written });
		const auto& systemState = context.systemState();

		// Act + Assert:
		EXPECT_FALSE(systemState.shouldRecoverBroker());
		EXPECT_FALSE(systemState.shouldRecoverServer());
		EXPECT_EQ(consumers::CommitOperationStep::Blocks_Written, systemState.commitStep());
	}

	TEST(TEST_CLASS, CanCreateAroundAllFiles) {
		// Arrange:
		TestContext context({ true, true, true, consumers::CommitOperationStep::Blocks_Written });
		const auto& systemState = context.systemState();

		// Act + Assert:
		EXPECT_TRUE(systemState.shouldRecoverBroker());
		EXPECT_TRUE(systemState.shouldRecoverServer());
		EXPECT_EQ(consumers::CommitOperationStep::Blocks_Written, systemState.commitStep());
	}

	// endregion

	// region reset

	namespace {
		void AssertCanResetFrom(const TestOptions& options) {
			// Arrange:
			TestContext context(options);
			auto& systemState = context.systemState();

			// Act:
			systemState.reset();

			// Assert:
			EXPECT_FALSE(systemState.shouldRecoverBroker());
			EXPECT_FALSE(systemState.shouldRecoverServer());
			EXPECT_EQ(consumers::CommitOperationStep::All_Updated, systemState.commitStep());
		}
	}

	TEST(TEST_CLASS, CanResetFromNoFiles) {
		AssertCanResetFrom({ false, false, false, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyBrokerLock) {
		AssertCanResetFrom({ true, false, false, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyServerLock) {
		AssertCanResetFrom({ false, true, false, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyCommitStep) {
		AssertCanResetFrom({ false, false, true, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromAllFiles) {
		AssertCanResetFrom({ true, true, true, consumers::CommitOperationStep::Blocks_Written });
	}

	// endregion
}}
