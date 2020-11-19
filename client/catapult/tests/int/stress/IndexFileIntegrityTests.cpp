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

#include "catapult/io/IndexFile.h"
#include "catapult/thread/ThreadGroup.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS IndexFileIntegrityTests

	namespace {
		uint64_t GetNumIterations() {
			return test::GetStressIterationCount() ? 15'000 : 1'500;
		}

		uint64_t ReadIndexFileValue(const std::string& indexFilename) {
			IndexFile indexFile(indexFilename, LockMode::None);
			return indexFile.get();
		}

		void AssertMixedOperationsDoNotCauseCrash(const consumer<IndexFile&, uint64_t>& operation) {
			// Arrange:
			test::TempFileGuard tempFile("foo.dat");
			{
				// - guarantee the existence of the file
				IndexFile indexFile(tempFile.name());
				indexFile.increment();
			}

			// Act: writer thread
			thread::ThreadGroup threads;
			threads.spawn([&tempFile, operation] {
				for (auto i = 0u; i < GetNumIterations(); ++i) {
					IndexFile indexFile(tempFile.name(), LockMode::None);
					operation(indexFile, i + 1);
				}
			});

			// - reader thread
			uint64_t lastValue = 0;
			bool isAnyDecreasing = false;
			threads.spawn([&tempFile, &lastValue, &isAnyDecreasing] {
				for (auto i = 0u; i < GetNumIterations(); ++i) {
					auto value = ReadIndexFileValue(tempFile.name());
					if (lastValue > value)
						isAnyDecreasing = true;

					lastValue = value;
				}
			});

			// - wait for all threads
			threads.join();

			// Assert:
			EXPECT_FALSE(isAnyDecreasing);
			EXPECT_LE(GetNumIterations(), ReadIndexFileValue(tempFile.name()));
		}
	}

	TEST(TEST_CLASS, GetAndSetFromDifferentThreadsDoNotCauseCrash) {
		AssertMixedOperationsDoNotCauseCrash([](auto& indexFile, auto i) { indexFile.set(i); });
	}

	TEST(TEST_CLASS, GetAndIncrementFromDifferentThreadsDoNotCauseCrash) {
		AssertMixedOperationsDoNotCauseCrash([](auto& indexFile, auto) { indexFile.increment(); });
	}
}}
