/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/thread/ThreadGroup.h"
#include "catapult/utils/SpinLock.h"
#include "tests/int/stress/test/StressThreadLogger.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS StorageIntegrityTests

	namespace {
		size_t GetNumIterations() {
			return test::GetStressIterationCount() ? 10'000 : 500;
		}

		Height GetMaxHeight() {
			return Height(GetNumIterations() + 1);
		}

		void RunMultithreadedReadWriteTest(size_t numReaders) {
			// Arrange:
			// - prepare and create the storage
			test::TempDirectoryGuard tempDir;
			test::PrepareStorage(tempDir.name());

			auto stagingDirectory = std::filesystem::path(tempDir.name()) / "staging";
			std::filesystem::create_directory(stagingDirectory);

			BlockStorageCache storage(
					std::make_unique<FileBlockStorage>(tempDir.name(), test::File_Database_Batch_Size),
					std::make_unique<FileBlockStorage>(
							stagingDirectory.generic_string(),
							test::File_Database_Batch_Size,
							FileBlockStorageMode::None));

			// - note that there can only ever be a single writer at a time since only one modifier can be outstanding at once
			std::vector<Height> heights(numReaders);

			// Act: set up reader thread(s) that read blocks
			thread::ThreadGroup threads;
			for (auto r = 0u; r < numReaders; ++r) {
				threads.spawn([&, r] {
					test::StressThreadLogger logger("reader thread " + std::to_string(r));

					while (GetMaxHeight() != heights[r]) {
						auto view = storage.view();
						auto pBlock = view.loadBlock(view.chainHeight());
						heights[r] = pBlock->Height;
					}
				});
			}

			// - set up a writer thread that writes blocks
			threads.spawn([&] {
				test::StressThreadLogger logger("writer thread");

				for (auto i = 0u; i < GetNumIterations(); ++i) {
					logger.notifyIteration(i, GetNumIterations());
					auto pNextBlock = test::GenerateBlockWithTransactions(0, Height(2 + i));

					auto modifier = storage.modifier();
					modifier.saveBlock(test::BlockToBlockElement(*pNextBlock));
					modifier.commit();
				}
			});

			// - wait for all threads
			threads.join();

			// Assert: all readers were able to observe the last height
			EXPECT_EQ(GetMaxHeight(), storage.view().chainHeight());

			for (const auto& height : heights)
				EXPECT_EQ(GetMaxHeight(), height);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, StorageIsThreadSafeWithSingleReaderSingleWriter) {
		RunMultithreadedReadWriteTest(1);
	}

	NO_STRESS_TEST(TEST_CLASS, StorageIsThreadSafeWithMultipleReadersSingleWriter) {
		RunMultithreadedReadWriteTest(test::GetNumDefaultPoolThreads());
	}
}}
