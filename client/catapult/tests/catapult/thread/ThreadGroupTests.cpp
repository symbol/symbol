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

#include "catapult/thread/ThreadGroup.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS ThreadGroupTests

	namespace {
		void SpawnThree(ThreadGroup& threads, std::atomic<size_t>& value) {
			threads.spawn([&value]() {
				test::Pause();
				value += 1;
			});

			threads.spawn([&value]() {
				value += 2;
			});

			threads.spawn([&value]() {
				test::Pause();
				value += 3;
			});
		}
	}

	TEST(TEST_CLASS, CanCreateEmpty) {
		// Act:
		ThreadGroup threads;

		// Assert:
		EXPECT_EQ(0u, threads.size());
	}

	TEST(TEST_CLASS, CanSpawnThreads) {
		// Arrange:
		std::atomic<size_t> value(0);
		{
			ThreadGroup threads;

			// Act:
			SpawnThree(threads, value);
			WAIT_FOR_VALUE(2u, value);

			// Assert:
			EXPECT_EQ(3u, threads.size());
			EXPECT_EQ(2u, value);
		}

		// Assert: destructor joins threads
		WAIT_FOR_VALUE(6u, value);
	}

	TEST(TEST_CLASS, CanJoinThreads) {
		// Arrange:
		std::atomic<size_t> value(0);
		ThreadGroup threads;
		SpawnThree(threads, value);

		// Act:
		threads.join();

		// Assert:
		EXPECT_EQ(3u, threads.size());
		EXPECT_EQ(6u, value);
	}

	TEST(TEST_CLASS, CanJoinThreadsMultipleTimes) {
		// Arrange:
		std::atomic<size_t> value(0);
		ThreadGroup threads;
		SpawnThree(threads, value);

		// Act:
		threads.join();
		threads.join();
		threads.join();

		// Assert:
		EXPECT_EQ(3u, threads.size());
		EXPECT_EQ(6u, value);
	}

	TEST(TEST_CLASS, CanJoinThreadsAfterSomeHaveCompleted) {
		// Arrange:
		std::atomic<size_t> value(0);
		ThreadGroup threads;
		SpawnThree(threads, value);

		// Act:
		WAIT_FOR_VALUE(2u, value);
		threads.join();

		// Assert:
		EXPECT_EQ(3u, threads.size());
		EXPECT_EQ(6u, value);
	}
}}
