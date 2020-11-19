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
	void AssertLockTypeTryLockFailsWhenLockIsLocked(Args&& ...args) {
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

#define MAKE_BASIC_LOCK_TEST(TEST_CLASS, TEST_NAME, ...) \
	TEST(TEST_CLASS, TEST_NAME) { test::AssertLockType##TEST_NAME<LockPolicy>(__VA_ARGS__); }

#define DEFINE_BASIC_LOCK_TESTS(TEST_CLASS, ...) \
	MAKE_BASIC_LOCK_TEST(TEST_CLASS, LockIsInitiallyUnlocked, __VA_ARGS__) \
	MAKE_BASIC_LOCK_TEST(TEST_CLASS, TryLockFailsWhenLockIsLocked, __VA_ARGS__) \
	MAKE_BASIC_LOCK_TEST(TEST_CLASS, LockCanBeUnlocked, __VA_ARGS__) \
	MAKE_BASIC_LOCK_TEST(TEST_CLASS, LockGuaranteesExclusiveAccess, __VA_ARGS__) \
	MAKE_BASIC_LOCK_TEST(TEST_CLASS, LockGuaranteesExclusiveAccessAfterLockUnlockCycles, __VA_ARGS__)
