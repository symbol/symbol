#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/SpinLock.h"
#include "tests/int/stress/utils/StressThreadLogger.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace io {

	namespace {
#ifdef STRESS
		constexpr size_t Num_Iterations = 10'000;
#else
		constexpr size_t Num_Iterations = 500;
#endif
		constexpr Height Max_Height(Num_Iterations + 1);

		void RunMultithreadedReadWriteTest(size_t numReaders) {
			// Arrange:
			// - prepare and create the storage
			test::TempDirectoryGuard tempDir;
			test::PrepareStorage(tempDir.name());
			BlockStorageCache storage(std::make_unique<FileBasedStorage>(tempDir.name()));

			// - note that there can only ever be a single writer at a time since only one modifier can be outstanding at once
			std::vector<Height> heights(numReaders);

			// Act: set up reader thread(s) that read blocks
			boost::thread_group threads;
			for (auto r = 0u; r < numReaders; ++r) {
				threads.create_thread([&, r] {
					test::StressThreadLogger logger("reader thread " + std::to_string(r));

					while (Max_Height != heights[r]) {
						auto view = storage.view();
						auto pBlock = view.loadBlock(view.chainHeight());
						heights[r] = pBlock->Height;
					}
				});
			}

			// - set up a writer thread that writes blocks
			threads.create_thread([&] {
				test::StressThreadLogger logger("writer thread");

				for (auto i = 0u; i < Num_Iterations; ++i) {
					logger.notifyIteration(i, Num_Iterations);

					auto modifier = storage.modifier();
					auto pNextBlock = test::GenerateVerifiableBlockAtHeight(Height(2 + i));
					modifier.saveBlock(test::BlockToBlockElement(*pNextBlock));
				}
			});

			// - wait for all threads
			threads.join_all();

			// Assert: all readers were able to observe the last height
			EXPECT_EQ(Max_Height, storage.view().chainHeight());

			for (const auto& height : heights)
				EXPECT_EQ(Max_Height, height);
		}
	}

	NO_STRESS_TEST(StorageTests, StorageIsThreadSafeWithSingleReaderSingleWriter) {
		// Assert:
		RunMultithreadedReadWriteTest(1);
	}

	NO_STRESS_TEST(StorageTests, StorageIsThreadSafeWithMultipleReadersSingleWriter) {
		// Assert:
		RunMultithreadedReadWriteTest(test::GetNumDefaultPoolThreads());
	}
}}
