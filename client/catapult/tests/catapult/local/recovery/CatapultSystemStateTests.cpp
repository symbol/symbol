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

		constexpr uint8_t Flag_Broker = 0x01;
		constexpr uint8_t Flag_Server = 0x02;
		constexpr uint8_t Flag_Harvesters_Temp = 0x04;
		constexpr uint8_t Flag_Commit_Step = 0x08;

		struct TestOptions {
			uint8_t FileFlags;
			consumers::CommitOperationStep OperationStep;
		};

		class TestContext {
		public:
			explicit TestContext(const TestOptions& options)
					: m_dataDirectory(m_tempDir.name())
					, m_systemState(m_dataDirectory) {
				if (Flag_Broker & options.FileFlags)
					createLockFile("broker.lock");

				if (Flag_Server & options.FileFlags)
					createLockFile("server.lock");

				if (Flag_Harvesters_Temp & options.FileFlags)
					createPlaceholderFile("harvesters.dat.tmp");

				if (Flag_Commit_Step & options.FileFlags)
					io::IndexFile(m_dataDirectory.rootDir().file("commit_step.dat")).set(static_cast<uint64_t>(options.OperationStep));
			}

		public:
			auto& systemState() {
				return m_systemState;
			}

		public:
			bool exists(const std::string& filename) {
				return boost::filesystem::exists(m_dataDirectory.rootDir().file(filename));
			}

		private:
			void createLockFile(const std::string& filename) const {
				// simulate an abandoned lock file by creating an empty file (using FileLock will prevent delete on windows)
				createPlaceholderFile(filename);
			}

			void createPlaceholderFile(const std::string& filename) const {
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

	namespace {
		void AssertCanCreate(const TestOptions& options) {
			// Arrange:
			TestContext context(options);
			const auto& systemState = context.systemState();

			// Act + Assert:
			EXPECT_EQ(0 != (Flag_Broker & options.FileFlags), systemState.shouldRecoverBroker());
			EXPECT_EQ(0 != (Flag_Server & options.FileFlags), systemState.shouldRecoverServer());

			auto expectedOperationStep = Flag_Commit_Step & options.FileFlags
					? options.OperationStep
					: consumers::CommitOperationStep::All_Updated;
			EXPECT_EQ(expectedOperationStep, systemState.commitStep());
		}
	}

	TEST(TEST_CLASS, CanCreateAroundNoFiles) {
		AssertCanCreate({ 0x00, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyBrokerLock) {
		AssertCanCreate({ 0x01, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyServerLock) {
		AssertCanCreate({ 0x02, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyHarvestersTemp) {
		AssertCanCreate({ 0x04, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanCreateAroundOnlyCommitStep) {
		AssertCanCreate({ 0x08, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanCreateAroundAllFiles) {
		AssertCanCreate({ 0x0F, consumers::CommitOperationStep::Blocks_Written });
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

			EXPECT_FALSE(context.exists("harvesters.dat.tmp"));
		}
	}

	TEST(TEST_CLASS, CanResetFromNoFiles) {
		AssertCanResetFrom({ 0x00, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyBrokerLock) {
		AssertCanResetFrom({ 0x01, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyServerLock) {
		AssertCanResetFrom({ 0x02, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyHarvestersTemp) {
		AssertCanResetFrom({ 0x04, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromOnlyCommitStep) {
		AssertCanResetFrom({ 0x08, consumers::CommitOperationStep::Blocks_Written });
	}

	TEST(TEST_CLASS, CanResetFromAllFiles) {
		AssertCanResetFrom({ 0x0F, consumers::CommitOperationStep::Blocks_Written });
	}

	// endregion
}}
