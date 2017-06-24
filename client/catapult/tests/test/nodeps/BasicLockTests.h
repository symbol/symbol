#pragma once
#include "LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	template<typename TLockPolicy, typename ...Args>
	void AssertLockTypeLockIsInitiallyUnlocked(Args&& ...args) {
		// Arrange:
		typename TLockPolicy::LockType lock{ std::forward<Args>(args)... };

		// Assert:
		EXPECT_TRUE(lock.try_lock());
	}

	template<typename TLockPolicy, typename ...Args>
	void AssertLockTypeTryLockFailsIfLockIsLocked(Args&& ...args) {
		// Arrange:
		typename TLockPolicy::LockType lock{ std::forward<Args>(args)... };
		ASSERT_TRUE(lock.try_lock());

		// Assert:
		EXPECT_FALSE(lock.try_lock());
	}

	template<typename TLockPolicy, typename ...Args>
	void AssertLockTypeLockCanBeUnlocked(Args&& ...args) {
		// Arrange:
		typename TLockPolicy::LockType lock{ std::forward<Args>(args)... };
		ASSERT_TRUE(lock.try_lock());

		// Act:
		lock.unlock();

		// Assert:
		EXPECT_TRUE(lock.try_lock());
	}

	template<typename TLockPolicy, typename ...Args>
	void AssertLockTypeLockGuaranteesExclusiveAccess(Args&& ...args) {
		// Arrange:
		typename TLockPolicy::LockType lock{ std::forward<Args>(args)... };

		// Assert:
		test::AssertLockGuaranteesExclusiveAccess<TLockPolicy>(lock);
	}

	template<typename TLockPolicy, typename ...Args>
	void AssertLockTypeLockGuaranteesExclusiveAccessAfterLockUnlockCycles(Args&& ...args) {
		// Arrange:
		typename TLockPolicy::LockType lock{ std::forward<Args>(args)... };

		// Assert:
		test::AssertLockGuaranteesExclusiveAccessAfterLockUnlockCycles<TLockPolicy>(lock);
	}
}}

#define DEFINE_BASIC_LOCK_TEST(TEST_CLASS, TEST_NAME, ...) \
	TEST(TEST_CLASS, TEST_NAME) { test::AssertLockType##TEST_NAME<LockPolicy>(__VA_ARGS__); }

#define DEFINE_BASIC_LOCK_TESTS(TEST_CLASS, ...) \
	DEFINE_BASIC_LOCK_TEST(TEST_CLASS, LockIsInitiallyUnlocked, __VA_ARGS__) \
	DEFINE_BASIC_LOCK_TEST(TEST_CLASS, TryLockFailsIfLockIsLocked, __VA_ARGS__) \
	DEFINE_BASIC_LOCK_TEST(TEST_CLASS, LockCanBeUnlocked, __VA_ARGS__) \
	DEFINE_BASIC_LOCK_TEST(TEST_CLASS, LockGuaranteesExclusiveAccess, __VA_ARGS__) \
	DEFINE_BASIC_LOCK_TEST(TEST_CLASS, LockGuaranteesExclusiveAccessAfterLockUnlockCycles, __VA_ARGS__)
