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

#include "catapult/utils/SpinReaderWriterLock.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS SpinReaderWriterLockTests

	TEST(TEST_CLASS, LockIsInitiallyUnlocked) {
		// Act:
		SpinReaderWriterLock lock;

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanAcquireReaderLock) {
		// Act:
		SpinReaderWriterLock lock;
		auto readLock = lock.acquireReader();

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_TRUE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanReleaseReaderLock) {
		// Act:
		SpinReaderWriterLock lock;
		{
			auto readLock = lock.acquireReader();
		}

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanReleaseReaderLockAfterMove) {
		// Act:
		SpinReaderWriterLock lock;
		{
			auto readLock = lock.acquireReader();
			auto readLock2 = std::move(readLock);
		}

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanPromoteReaderLockToWriterLock) {
		// Act:
		SpinReaderWriterLock lock;
		auto readLock = lock.acquireReader();
		auto writeLock = readLock.promoteToWriter();

		// Assert:
		EXPECT_TRUE(lock.isWriterPending());
		EXPECT_TRUE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanDemoteWriterLockToReaderLock) {
		// Act:
		SpinReaderWriterLock lock;
		auto readLock = lock.acquireReader();
		{
			auto writeLock = readLock.promoteToWriter();
		}

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_TRUE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanReleaseWriterLock) {
		// Act:
		SpinReaderWriterLock lock;
		{
			auto readLock = lock.acquireReader();
			auto writeLock = readLock.promoteToWriter();
		}

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CanReleaseWriterLockAfterMove) {
		// Act:
		SpinReaderWriterLock lock;
		{
			auto readLock = lock.acquireReader();
			auto writeLock = readLock.promoteToWriter();
			auto writeLock2 = std::move(writeLock);
		}

		// Assert:
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, CannotPromoteReaderLockToWriterLockMultipleTimes) {
		// Arrange:
		SpinReaderWriterLock lock;
		auto readLock = lock.acquireReader();
		auto writeLock = readLock.promoteToWriter();

		// Act + Assert:
		EXPECT_THROW(readLock.promoteToWriter(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanPromoteReaderLockToWriterLockAfterDemotion) {
		// Act: acquire a reader and then promote, demote, promote
		SpinReaderWriterLock lock;
		auto readLock = lock.acquireReader();
		{
			auto writeLock = readLock.promoteToWriter();
		}

		auto writeLock = readLock.promoteToWriter();

		// Assert:
		EXPECT_TRUE(lock.isWriterPending());
		EXPECT_TRUE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, MultipleThreadsCanAquireReaderLock) {
		// Arrange:
		SpinReaderWriterLock lock;
		std::atomic<uint32_t> counter(0);
		test::LockTestState state;
		test::LockTestGuard testGuard(state);

		for (auto i = 0u; i < test::Num_Default_Lock_Threads; ++i) {
			testGuard.Threads.create_thread([&, i] {
				// Act: acquire a reader and increment the counter
				auto readLock = lock.acquireReader();
				state.incrementCounterAndBlock(counter, i);
			});
		}

		// - wait for the counter to be incremented by all readers
		CATAPULT_LOG(debug) << "waiting for readers";
		WAIT_FOR_VALUE(test::Num_Default_Lock_Threads, counter);

		// Assert: all threads were able to access the counter
		EXPECT_EQ(test::Num_Default_Lock_Threads, counter);
		EXPECT_FALSE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_TRUE(lock.isReaderActive());
	}

	namespace {
		struct ExclusiveLockGuard {
		public:
			explicit ExclusiveLockGuard(SpinReaderWriterLock& lock)
					: m_readLock(lock.acquireReader())
					, m_writeLock(m_readLock.promoteToWriter())
			{}

		private:
			SpinReaderWriterLock::ReaderLockGuard m_readLock;
			SpinReaderWriterLock::WriterLockGuard m_writeLock;
		};

		struct LockPolicy {
			using LockType = SpinReaderWriterLock;

			static auto ExclusiveLock(LockType& lock) {
				return ExclusiveLockGuard(lock);
			}
		};
	}

	TEST(TEST_CLASS, LockGuaranteesExclusiveWriterAccess) {
		// Arrange:
		SpinReaderWriterLock lock;

		// Assert:
		test::AssertLockGuaranteesExclusiveAccess<LockPolicy>(lock);
	}

	TEST(TEST_CLASS, LockGuaranteesExclusiveWriterAccessAfterLockUnlockCycles) {
		// Arrange:
		SpinReaderWriterLock lock;

		// Assert:
		test::AssertLockGuaranteesExclusiveAccessAfterLockUnlockCycles<LockPolicy>(lock);
	}

	TEST(TEST_CLASS, ReaderBlocksWriter) {
		// Arrange:
		SpinReaderWriterLock lock;
		char value = '\0';
		test::LockTestState state;
		test::LockTestGuard testGuard(state);

		// Act: spawn the reader thread
		testGuard.Threads.create_thread([&] {
			// - acquire a reader and then spawn thread that takes a write lock
			auto readLock = lock.acquireReader();
			testGuard.Threads.create_thread([&] {
				// - the writer should be blocked because the outer thread is holding a read lock
				auto readLock2 = lock.acquireReader();
				auto writeLock2 = readLock2.promoteToWriter();
				state.setValueAndBlock(value, 'w');
			});

			state.setValueAndBlock(value, 'r');
		});

		// - wait for the value to be set
		state.waitForValueChangeWithPause();

		// Assert: only the reader was executed
		EXPECT_EQ(1u, state.NumValueChanges);
		EXPECT_EQ('r', value);
		EXPECT_TRUE(lock.isWriterPending());
		EXPECT_FALSE(lock.isWriterActive());
		EXPECT_TRUE(lock.isReaderActive());
	}

	TEST(TEST_CLASS, WriterBlocksReader) {
		// Arrange:
		SpinReaderWriterLock lock;
		char value = '\0';
		test::LockTestState state;
		test::LockTestGuard testGuard(state);

		// Act: spawn the writer thread
		testGuard.Threads.create_thread([&] {
			// - acquire a writer and then spawn thread that takes a read lock
			auto readLock = lock.acquireReader();
			auto writeLock = readLock.promoteToWriter();
			testGuard.Threads.create_thread([&] {
				// - the reader should be blocked because the outer thread is holding a write lock
				auto readLock2 = lock.acquireReader();
				state.setValueAndBlock(value, 'r');
			});

			state.setValueAndBlock(value, 'w');
		});

		// - wait for the value to be set
		state.waitForValueChangeWithPause();

		// Assert: only the writer was executed
		EXPECT_EQ(1u, state.NumValueChanges);
		EXPECT_EQ('w', value);
		EXPECT_TRUE(lock.isWriterPending());
		EXPECT_TRUE(lock.isWriterActive());
		EXPECT_FALSE(lock.isReaderActive());
	}

	namespace {
		struct ReaderWriterRaceState : public test::LockTestState {
		public:
			SpinReaderWriterLock Lock;
			std::atomic<char> ReleasedThreadId;
			std::atomic<uint32_t> NumWaitingThreads;
			std::atomic<uint32_t> NumReaderThreads;

		public:
			ReaderWriterRaceState() : ReleasedThreadId('\0'), NumWaitingThreads(0), NumReaderThreads(0)
			{}

		public:
			auto acquireReader() {
				++NumWaitingThreads;
				auto readLock = Lock.acquireReader();
				++NumReaderThreads;
				return readLock;
			}

		public:
			void doWriterWork() {
				auto readLock = acquireReader();
				doWriterWork(std::move(readLock));
			}

			void doWriterWork(SpinReaderWriterLock::ReaderLockGuard&& readLock) {
				auto writeLock = readLock.promoteToWriter();

				setReleasedThreadId('w');
				block();
			}

			void doReaderWork() {
				auto readLock = acquireReader();

				setReleasedThreadId('r');
				block();
			}

			void waitForReleasedThread() {
				WAIT_FOR_EXPR('\0' != ReleasedThreadId);
			}

		private:
			void setReleasedThreadId(char ch) {
				char expected = '\0';
				ReleasedThreadId.compare_exchange_strong(expected, ch);
			}
		};
	}

	TEST(TEST_CLASS, WriterIsPreferredToReader) {
		// Arrange:
		//  M: |ReadLock     |      # M acquires ReadLock while other threads are spawned
		//  W:   |WriteLock**  |    # when M ReadLock is released, pending writer is unblocked
		//  R:     |ReadLock***  |  # when W WriteLock is released, pending reader2 is unblocked
		ReaderWriterRaceState state;
		test::LockTestGuard testGuard(state);

		// Act: spawn a reader thread
		testGuard.Threads.create_thread([&] {
			// - acquire a reader lock
			auto readLock = state.Lock.acquireReader();

			// - spawn a thread that will acquire a writer lock
			testGuard.Threads.create_thread([&] {
				state.doWriterWork();
			});

			// - spawn a thread that will acquire a reader lock after a writer is pending
			testGuard.Threads.create_thread([&] {
				WAIT_FOR_EXPR(state.Lock.isWriterPending());
				state.doReaderWork();
			});

			// - block until both the reader and writer threads are pending
			WAIT_FOR_VALUE(2u, state.NumWaitingThreads);

			// - wait a bit in case the state changes due to a bug
			test::Pause();
		});

		// - wait for releasedThreadId to be set
		state.waitForReleasedThread();

		// Assert: the writer was released first (the reader was blocked by the pending writer)
		EXPECT_EQ('w', state.ReleasedThreadId);
	}

	TEST(TEST_CLASS, WriterIsBlockedByAllPendingReaders) {
		// Arrange:
		//  M: |ReadLock       |        # M acquires ReadLock while other threads are spawned
		//  W:   |ReadLock           |  # when M ReadLock is released, pending reader1 is unblocked
		//  R:     |ReadLock       |    # when M ReadLock is released, pending reader2 is unblocked
		//  W:       [WriteLock****  |  # when R ReadLock is released, pending writer is unblocked
		//                              # (note that promotion is blocked by R ReadLock)
		ReaderWriterRaceState state;
		test::LockTestGuard testGuard(state);

		// Act: spawn a reader thread
		testGuard.Threads.create_thread([&] {
			// Act: acquire a reader lock
			auto readLock = state.Lock.acquireReader();

			// - spawn a thread that will acquire a writer lock after multiple readers (including itself) are active
			testGuard.Threads.create_thread([&] {
				auto writerThreadReadLock = state.acquireReader();
				WAIT_FOR_VALUE(2u, state.NumReaderThreads);
				state.doWriterWork(std::move(writerThreadReadLock));
			});

			// - spawn a thread that will acquire a reader lock after the writer thread
			testGuard.Threads.create_thread([&] {
				WAIT_FOR_ONE(state.NumReaderThreads);
				state.doReaderWork();
			});

			// - block until both the reader and writer threads have acquired a reader lock
			WAIT_FOR_VALUE(2u, state.NumReaderThreads);

			// - wait a bit in case the state changes due to a bug
			test::Pause();
		});

		// - wait for releasedThreadId to be set
		state.waitForReleasedThread();

		// Assert: the reader was released first (the writer was blocked by the reader)
		EXPECT_EQ('r', state.ReleasedThreadId);
	}
}}
