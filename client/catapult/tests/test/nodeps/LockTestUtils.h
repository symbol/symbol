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

#pragma once
#include "catapult/thread/ThreadGroup.h"
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <thread>

namespace catapult { namespace test {

	/// Number of default threads used in lock tests.
	const auto Num_Default_Lock_Threads = 2 * GetNumDefaultPoolThreads();

	/// Stateful information used in a lock test.
	struct LockTestState {
	public:
		/// Creates state.
		LockTestState() : ShouldBlock(true), NumValueChanges(0)
		{}

	public:
		/// Flag that is used to block threads.
		std::atomic_bool ShouldBlock;

		/// Number of value changes detected.
		std::atomic<uint32_t> NumValueChanges;

	public:
		/// Increments \a counter for the thread identified by \a id and waits for ShouldBlock to be set to \c false.
		template<typename TCounter>
		void incrementCounterAndBlock(TCounter& counter, size_t id) {
			CATAPULT_LOG(trace) << "entered lock " << id;
			++counter;
			++NumValueChanges;
			block();
			CATAPULT_LOG(trace) << "exiting lock " << id;
		}

		/// Sets the specified \a value to \a desired and waits for ShouldBlock to be set to \c false.
		template<typename TValue>
		void setValueAndBlock(TValue& value, TValue desired) {
			CATAPULT_LOG(trace) << "entered lock " << desired;
			value = desired;
			++NumValueChanges;
			block();
			CATAPULT_LOG(trace) << "exiting lock " << desired;
		}

		/// Waits for ShouldBlock to be set to \c false.
		void block() {
			while (ShouldBlock)
			{}
		}

		/// Waits for NumValueChanges to be at least one.
		void waitForValueChangeWithPause() {
			// wait for the counter to be incremented and then pause
			CATAPULT_LOG(debug) << "waiting for increment";
			WAIT_FOR_EXPR(NumValueChanges > 0);
			CATAPULT_LOG(debug) << "pausing";
			Pause();
			CATAPULT_LOG(debug) << "done pausing";
		}
	};

	/// RAII structure for gracefully tearing down a lock test.
	struct LockTestGuard {
	public:
		/// Thread group used by the current test.
		thread::ThreadGroup Threads;

	public:
		/// Creates a guard around \a state.
		explicit LockTestGuard(LockTestState& state) : m_state(state)
		{}

		/// Destroys guard.
		~LockTestGuard() {
			m_state.ShouldBlock = false;
			Threads.join();
		}

	private:
		LockTestState& m_state;
	};

	/// Asserts that \a source can create multiple sub-objects (via \a createSubObject) on multiple threads
	/// that are validated by \a verifySubObject.
	template<typename TSource, typename TCreateSubObject, typename TVerifySubObject>
	void CanCreateSubObjectOnMultipleThreads(TSource&& source, TCreateSubObject createSubObject, TVerifySubObject verifySubObject) {
		// Arrange:
		constexpr auto Num_Sub_Objects = 10u;
		std::atomic<size_t> numSubObjects(0);
		thread::ThreadGroup threads;

		// Act: create a subobject on multiple threads
		for (auto i = 0u; i < Num_Sub_Objects; ++i) {
			threads.spawn([&]() {
				auto subObject = createSubObject(source);
				++numSubObjects;

				WAIT_FOR_VALUE(Num_Sub_Objects, numSubObjects);

				// Assert:
				verifySubObject(subObject);
			});
		}

		threads.join();

		// Assert:
		EXPECT_EQ(Num_Sub_Objects, numSubObjects);
	}

	/// Asserts that \a lock can be used to acquire exclusive access to a shared resource.
	template<typename TLockPolicy, typename TLock>
	void AssertLockGuaranteesExclusiveAccess(TLock& lock) {
		// Arrange:
		uint32_t counter = 0u;
		LockTestState state;
		LockTestGuard testGuard(state);

		// - spawn multiple threads
		for (auto i = 0u; i < Num_Default_Lock_Threads; ++i) {
			testGuard.Threads.spawn([&, i] {
				// Act: acquire a lock and increment and block the counter
				auto lockGuard = TLockPolicy::ExclusiveLock(lock);
				state.incrementCounterAndBlock(counter, i);
			});
		}

		// - wait for the counter to be incremented
		state.waitForValueChangeWithPause();

		// Assert: only one thread accessed the counter
		EXPECT_EQ(1u, counter);
	}

	/// Asserts that \a lock can be used to acquire and release exclusive access to a shared resource.
	template<typename TLockPolicy, typename TLock>
	void AssertLockGuaranteesExclusiveAccessAfterLockUnlockCycles(TLock& lock) {
		// Arrange:
		uint32_t counter = 0u;

		// - spawn multiple threads
		thread::ThreadGroup threads;
		for (auto i = 0u; i < Num_Default_Lock_Threads; ++i) {
			threads.spawn([&, i] {
				// Act: acquire a lock and increment the counter
				auto guard = TLockPolicy::ExclusiveLock(lock);
				CATAPULT_LOG(debug) << "entered lock " << i;
				++counter;
				CATAPULT_LOG(debug) << "exiting lock " << i;
			});
		}

		// - wait for all threads
		threads.join();

		// Assert: all threads accessed the counter (individually)
		EXPECT_EQ(Num_Default_Lock_Threads, counter);
	}

	// region view / modifier lock provider utils

	/// Asserts that multiple views can be acquired from \a provider.
	template<typename TProvider>
	void AssertMultipleViewsCanBeAcquired(const TProvider& provider) {
		// Assert:
		CanCreateSubObjectOnMultipleThreads(
				provider,
				[](const auto& source) { return source.view(); },
				[](const auto&) {
					// do nothing - view was acquired
				});
	}

	/// Asserts that the lock obtained by \a acquireFirstLock prevents the lock obtained by \a acquireSecondLock
	/// from being acquired.
	template<typename TAcquireFirstLockFunc, typename TAcquireSecondLockFunc>
	void AssertExclusiveLocks(TAcquireFirstLockFunc acquireFirstLock, TAcquireSecondLockFunc acquireSecondLock) {
		// Arrange: get the first lock
		std::atomic<size_t> flag(0);
		{
			auto lock1 = acquireFirstLock();

			// - spawn another thread to acquire the second lock
			std::thread([&flag, acquireSecondLock]() {
				acquireSecondLock();
				flag = 1;
			}).detach();

			// Act: sleep a small amount
			Sleep(15);

			// Sanity: the flag should have its original value
			EXPECT_EQ(0u, flag);

			// Act: release the first lock
		}

		// - wait for the flag value to change
		WAIT_FOR_EXPR(0u != flag);

		// Assert: the other thread acquired the (second) lock
		EXPECT_EQ(1u, flag);
	}

	/// Asserts that \a provider view blocks a modifier.
	template<typename TProvider>
	void AssertModifierIsBlockedByView(TProvider&& provider) {
		// Assert:
		AssertExclusiveLocks(
			[&provider]() { return provider.view(); },
			[&provider]() { return provider.modifier(); });
	}

	/// Asserts that \a provider modifier blocks a view.
	template<typename TProvider>
	void AssertViewIsBlockedByModifier(TProvider&& provider) {
		// Assert:
		AssertExclusiveLocks(
			[&provider]() { return provider.modifier(); },
			[&provider]() { return provider.view(); });
	}

	/// Asserts that \a provider modifier blocks a modifier.
	template<typename TProvider>
	void AssertModifierIsBlockedByModifier(TProvider&& provider) {
		// Assert:
		AssertExclusiveLocks(
			[&provider]() { return provider.modifier(); },
			[&provider]() { return provider.modifier(); });
	}

/// Adds all view/modifier lock provider tests to the specified test class (\a TEST_CLASS).
#define DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS) \
	TEST(TEST_CLASS, MultipleViewsCanBeAcquired) { test::AssertMultipleViewsCanBeAcquired(*CreateLockProvider()); } \
	TEST(TEST_CLASS, ModifierIsBlockedByView) { test::AssertModifierIsBlockedByView(*CreateLockProvider()); } \
	TEST(TEST_CLASS, ViewIsBlockedByModifier) { test::AssertViewIsBlockedByModifier(*CreateLockProvider()); } \
	TEST(TEST_CLASS, ModifierIsBlockedByModifier) { test::AssertModifierIsBlockedByModifier(*CreateLockProvider()); }

	// endregion
}}
