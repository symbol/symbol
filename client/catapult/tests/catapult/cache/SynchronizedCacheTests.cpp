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

#include "catapult/cache/SynchronizedCache.h"
#include "catapult/thread/Future.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS SynchronizedCacheTests

	namespace {
		template<typename TView>
		void AssertContents(const TView& view, size_t expectedId) {
			// Assert:
			ASSERT_TRUE(!!view);
			EXPECT_EQ(expectedId, view->id());
			EXPECT_EQ(expectedId, (*view).id());
		}

		template<typename TView>
		void AssertContents(TView& view, size_t expectedId) {
			// Assert:
			AssertContents(const_cast<const TView&>(view), expectedId);
			EXPECT_EQ(expectedId, view->id());
			EXPECT_EQ(expectedId, (*view).id());
		}

		template<typename TView>
		void AssertDefaultContents(TView& view) {
			// Assert:
			AssertContents(view, 0);
		}
	}

	// region locked (read-only)

	TEST(TEST_CLASS, CanCreateLockedReadOnlyView) {
		// Act:
		test::SimpleCache cache;
		auto view = cache.createView();

		// Assert:
		AssertDefaultContents(view);
	}

	TEST(TEST_CLASS, CanMoveConstructLockedReadOnlyView) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto delta = cache.createDelta();
			delta->increment();
			delta->increment();
			cache.commit();
		}

		// Act:
		auto view1 = cache.createView();
		auto view2 = std::move(view1);

		// Assert:
		// - the moved (locked) view is simple and should retain its original value (move has no effect on source)
		// - the destination (locked) view should point to a different view (the one moved into it)
		AssertContents(view1, 2);
		AssertContents(view2, 2);
		EXPECT_NE(&(*view1), &(*view2));
	}

	TEST(TEST_CLASS, CanCreateMultipleLockedReadOnlyViews) {
		test::CanCreateSubObjectOnMultipleThreads(
				test::SimpleCache(),
				[](const auto& cache) { return cache.createView(); },
				[](const auto& view) { AssertDefaultContents(view); });
	}

	// endregion

	// region commit

	TEST(TEST_CLASS, CannotCommitChangesWhenNoDeltasAreOutstanding) {
		// Arrange:
		test::SimpleCache cache;

		// Act + Assert:
		EXPECT_THROW(cache.commit(), catapult_runtime_error);
	}

	// endregion

	// region createDelta

	TEST(TEST_CLASS, CanCreateDelta) {
		// Act:
		test::SimpleCache cache;
		auto delta = cache.createDelta();

		// Assert:
		AssertDefaultContents(delta);
	}

	TEST(TEST_CLASS, CannotCreateMultipleDeltas) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto delta = cache.createDelta();

			// Act + Assert:
			EXPECT_THROW(cache.createDelta(), catapult_runtime_error);
		}

		// Act: cache delta went out of scope, another delta is allowed
		auto delta = cache.createDelta();

		// Assert:
		AssertDefaultContents(delta);
	}

	TEST(TEST_CLASS, CanChangeDeltaWithoutCommitting) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto delta = cache.createDelta();

			// Act:
			delta->increment();
			delta->increment();

			// Assert:
			EXPECT_EQ(2u, delta->id());
		}

		EXPECT_EQ(0u, cache.createView()->id());
	}

	TEST(TEST_CLASS, CanCommitDeltaChanges) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto delta = cache.createDelta();

			// Act:
			delta->increment();
			delta->increment();
			cache.commit();

			// Assert:
			EXPECT_EQ(2u, delta->id());
		}

		EXPECT_EQ(2u, cache.createView()->id());
	}

	TEST(TEST_CLASS, OnlyChangesFromMostRecentDeltaAreCommitted) {
		// Arrange:
		test::SimpleCache cache;
		{
			// - create a delta with two increments and then release it
			auto delta = cache.createDelta();
			delta->increment();
			delta->increment();
		}

		{
			// Act: create a new delta with one increment and commit it
			auto delta = cache.createDelta();
			delta->increment();
			cache.commit();

			// Assert: only the changes from the last delta were committed
			EXPECT_EQ(1u, delta->id());
		}

		EXPECT_EQ(1u, cache.createView()->id());
	}

	// endregion

	// region createDetachedDelta

	TEST(TEST_CLASS, CanCreateAndLockDetachedDelta) {
		// Act:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		auto delta = lockableDelta.tryLock();

		// Assert:
		AssertDefaultContents(delta);
	}

	TEST(TEST_CLASS, CanLockDetachedDeltaAfterReset) {
		// Arrange:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		{
			auto delta = lockableDelta.tryLock();

			// - reset the delta
			EXPECT_TRUE(!!delta);
		}

		// Act: relock the delta
		auto delta = lockableDelta.tryLock();

		// Assert:
		AssertDefaultContents(delta);
	}

	TEST(TEST_CLASS, CanCreateMultipleDetachedDeltas) {
		// Arrange:
		test::SimpleCache cache;
		std::vector<decltype(cache.createDetachedDelta())> deltas;

		// Act:
		for (auto i = 0u; i < 10; ++i) {
			auto lockableDelta = cache.createDetachedDelta();
			lockableDelta.tryLock()->increment();
			deltas.push_back(std::move(lockableDelta));
		}

		// Assert: the detached deltas are independent (because different lockable caches are being locked)
		for (auto& lockableDelta : deltas) {
			auto delta = lockableDelta.tryLock();
			AssertContents(delta, 1);
		}
	}

	TEST(TEST_CLASS, CanLockDetachedDeltaMultipleTimes) {
		// Arrange:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();

		// Act:
		for (auto i = 0u; i < 10; ++i) {
			auto delta = lockableDelta.tryLock();
			delta->increment();
		}

		// Assert:
		AssertContents(lockableDelta.tryLock(), 10);
	}

	TEST(TEST_CLASS, CanChangeDetachedDeltaWithoutCommitting) {
		// Act:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		{
			auto delta = lockableDelta.tryLock();

			// Act:
			delta->increment();
			delta->increment();

			// Assert:
			EXPECT_EQ(2u, delta->id());
		}

		EXPECT_EQ(0u, cache.createView()->id());
	}

	TEST(TEST_CLASS, CannotCommitDetachedDeltaChanges) {
		// Arrange:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		{
			auto delta = lockableDelta.tryLock();

			// Act:
			delta->increment();
			delta->increment();
			EXPECT_THROW(cache.commit(), catapult_runtime_error);

			// Assert:
			EXPECT_EQ(2u, delta->id());
		}

		EXPECT_EQ(0u, cache.createView()->id());
	}

	TEST(TEST_CLASS, CommitInvalidatesLockableCache) {
		// Arrange:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		{
			auto delta = cache.createDelta();

			// Sanity:
			EXPECT_TRUE(!!delta);
			std::thread([&lockableDelta]() {
				// - need to lock on a separate thread because test thread owns delta
				EXPECT_TRUE(lockableDelta.tryLock());
			}).join();

			// Act:
			delta->increment();
			cache.commit();

			// Assert:
			EXPECT_TRUE(!!delta);
		}

		EXPECT_FALSE(lockableDelta.tryLock());
	}

	// endregion

	// region locking

	TEST(TEST_CLASS, CanGetMultipleViewsOfDifferentTypes) {
		// Arrange:
		static constexpr auto Num_Views = 7u;
		test::SimpleCache cache;
		std::atomic<size_t> numViews(0);
		thread::ThreadGroup threads;

		auto handler = [&numViews](const auto& view) {
			++numViews;
			WAIT_FOR_VALUE(Num_Views, numViews);

			// Assert:
			AssertDefaultContents(view);
		};

		// Act:
		for (auto i = 0u; i < (Num_Views - 1) / 2; ++i) {
			// - view
			threads.spawn([&cache, &handler]() { handler(cache.createView()); });

			// - detached delta
			threads.spawn([&cache, &handler]() {
				auto lockableDelta = cache.createDetachedDelta();
				auto detachedDelta = lockableDelta.tryLock();
				handler(detachedDelta);
			});
		}

		// - delta
		threads.spawn([&cache, &handler]() { handler(cache.createDelta()); });

		threads.join();

		// Assert:
		EXPECT_EQ(Num_Views, numViews);
	}

	TEST(TEST_CLASS, ViewBlocksCommit) {
		// Act:
		test::SimpleCache cache;
		test::AssertExclusiveLocks(
				[&cache]() { return cache.createView(); },
				[&cache]() {
					auto delta = cache.createDelta();
					cache.commit();
				});
	}

	TEST(TEST_CLASS, LockedDetachedDeltaBlocksCommit) {
		// Arrange:
		struct DetachedDeltaGuard {
		public:
			explicit DetachedDeltaGuard(LockableCacheDelta<test::SimpleCacheDelta>&& lockableDelta)
					: m_lockableDelta(std::move(lockableDelta))
					, m_delta(m_lockableDelta.tryLock())
			{}

		private:
			LockableCacheDelta<test::SimpleCacheDelta> m_lockableDelta;
			OptionalLockedCacheDelta<test::SimpleCacheDelta> m_delta;
		};

		// Act:
		test::SimpleCache cache;
		test::AssertExclusiveLocks(
				[&cache]() { return DetachedDeltaGuard(cache.createDetachedDelta()); },
				[&cache]() {
					auto delta = cache.createDelta();
					cache.commit();
				});
	}

	TEST(TEST_CLASS, UnlockedDetachedDeltaDoesNotBlockCommit) {
		// Arrange:
		test::SimpleCache cache;
		auto lockableDelta = cache.createDetachedDelta();
		lockableDelta.tryLock()->increment(); // lock is acquired only for the increment operation

		{
			auto delta = cache.createDelta();
			delta->increment();
			delta->increment();

			// Sanity:
			AssertContents(delta, 2);
			std::thread([&cache, &lockableDelta]() {
				// - need to lock on a separate thread because test thread owns delta
				AssertContents(cache.createView(), 0);
				AssertContents(lockableDelta.tryLock(), 1);
			}).join();

			// Act:
			cache.commit();

			// Assert:
			AssertContents(delta, 2);
		}

		AssertContents(cache.createView(), 2);
		EXPECT_FALSE(lockableDelta.tryLock());
	}

	namespace {
		struct CommitLockGuard {
		public:
			CommitLockGuard()
					: m_cache(m_flag)
					, m_delta(m_cache.createDelta())
			{}

		public:
			void lock() {
				m_cache.commit();
			}

			void waitForWaiter() {
				WAIT_FOR_ONE_EXPR(m_flag.state()->numWaiters());
			}

		public:
			void reset() {
				m_flag.state()->set();
			}

			test::SimpleCache& cache() {
				return m_cache;
			}

		private:
			test::AutoSetFlag m_flag;
			test::SimpleCache m_cache;
			LockedCacheDelta<test::SimpleCacheDelta> m_delta;
		};

		class ThreadGroup {
		public:
			~ThreadGroup() {
				CATAPULT_LOG(debug) << ">>> joining all threads";
				for (auto& thread : m_threads)
					thread.join();

				CATAPULT_LOG(debug) << "<<< joined all threads";
			}

		public:
			template<typename TFunc>
			void createThread(const std::string& name, TFunc func) {
				CATAPULT_LOG(debug) << "+ creating thread: " << name;
				m_threads.emplace_back([name, func]() mutable {
					CATAPULT_LOG(debug) << "> spawning thread: " << name;
					func();
					CATAPULT_LOG(debug) << "< destroying thread: " << name;
				});
			}

		private:
			std::vector<std::thread> m_threads;
		};

		template<typename TBlockedOperationFunc>
		void AssertCommitBlocksOperation(TBlockedOperationFunc blockedOperationFunc) {
			// Arrange: create the cache and start a (long) commit operation
			std::atomic<size_t> flag(0);
			CommitLockGuard commitLockGuard;
			auto& cache = commitLockGuard.cache();

			// - [A] spawn the thread that signals other threads once the lock is acquired
			thread::promise<bool> lockPromise;
			ThreadGroup threads;
			threads.createThread("signal lock", [&flag, &commitLockGuard, &lockPromise]() {
				// - wait for [B] to acquire the lock (in commit) and signal the lock was acquired
				commitLockGuard.waitForWaiter(); // waits for [B]
				flag = 1; // signals [C, main]

				// - block this thread until the lock is released
				auto lockFuture = lockPromise.get_future();
				lockFuture.get(); // waits for [B]
			});

			// - [B] spawn the lock acquisition on a separate thread because the lock should be acquired before signaling
			threads.createThread("lock acquisition", [&commitLockGuard, &lockPromise]() mutable {
				commitLockGuard.lock(); // signals [A], waits for [main]

				CATAPULT_LOG(debug) << "lock acquisition thread locked cache";
				lockPromise.set_value(true); // signals [A]
			});

			// - [C] spawn another thread to attempt the blocked operation
			threads.createThread("blocked operation", [&flag, &cache, blockedOperationFunc]() {
				// - wait for the lock to be acquired by [B]
				WAIT_FOR_ONE(flag); // waits for [A]

				// - run the blocked operation
				blockedOperationFunc(cache); // waits for [main]

				// - signal the blocked operation completed
				flag = 2; // signals [main]
			});

			// Act: wait for the value to change and then pause
			CATAPULT_LOG(debug) << "test thread waiting for flag";
			WAIT_FOR_ONE(flag); // waits for [A]

			// - wait a bit to see if flag changes
			test::Pause();

			// Sanity: the flag should have only been updated by [A]
			EXPECT_EQ(1u, flag);

			// Act: release the first (commit) lock
			commitLockGuard.reset(); // signals [B (releases lock), C (acquires lock)]

			// - wait for the flag value to change
			WAIT_FOR_VALUE(2u, flag); // waits for [C]

			// Assert: [C] acquired the (second) lock
			EXPECT_EQ(2u, flag);
		}
	}

	TEST(TEST_CLASS, CommitBlocksView) {
		// Act:
		AssertCommitBlocksOperation([](const auto& cache) {
			// Act:
			auto view = cache.createView();

			// Assert: view was created (since view is non-nullable, there is nothing else to check)
		});
	}

	TEST(TEST_CLASS, CommitBlocksDelta) {
		// Act:
		AssertCommitBlocksOperation([](auto& cache) {
			// Act + Assert:
			EXPECT_THROW(cache.createDelta(), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CommitBlocksDetachedDelta) {
		// Act:
		AssertCommitBlocksOperation([](const auto& cache) {
			// Act:
			auto lockableDelta = cache.createDetachedDelta();

			// Assert:
			EXPECT_TRUE(lockableDelta.tryLock());
		});
	}

	// endregion

	// region SynchronizedCacheWithInit

	namespace {
		class BasicSimpleCacheWithInit : public test::BasicSimpleCache {
		public:
			explicit BasicSimpleCacheWithInit(std::string& initString) : m_initString(initString)
			{}

		public:
			void init(const std::string& initString) {
				m_initString = initString;
			}

		private:
			std::string& m_initString;
		};

		class SimpleCacheWithInit : public SynchronizedCacheWithInit<BasicSimpleCacheWithInit> {
		public:
			SimpleCacheWithInit() : SynchronizedCacheWithInit(BasicSimpleCacheWithInit(m_initString))
			{}

		public:
			const std::string& initString() const {
				return m_initString;
			}

		private:
			std::string m_initString;
		};
	}

	TEST(TEST_CLASS, SynchronizedCacheWithInit_InitDelegatesToWrappedCache) {
		// Arrange:
		SimpleCacheWithInit cache;

		// Act:
		cache.init("init value");

		// Assert:
		EXPECT_EQ(cache.initString(), "init value");
	}

	TEST(TEST_CLASS, SynchronizedCacheWithInit_CannotCallInitMoreThanOnce) {
		// Arrange:
		SimpleCacheWithInit cache;
		cache.init("init value");

		// Act + Assert:
		EXPECT_THROW(cache.init("init value"), catapult_runtime_error);
		EXPECT_THROW(cache.init("init value 2"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, SynchronizedCacheWithInit_InitIsOptional) {
		// Arrange:
		SimpleCacheWithInit cache;

		// Act + Assert: no exception
		{
			auto delta = cache.createDelta();
		}

		{
			auto view = cache.createView();
		}

		// -  init wasn't called
		EXPECT_TRUE(cache.initString().empty());
	}

	// endregion
}}
