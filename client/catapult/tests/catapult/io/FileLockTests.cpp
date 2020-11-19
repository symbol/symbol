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

#include "catapult/io/FileLock.h"
#include "tests/test/nodeps/BasicLockTests.h"
#include "tests/test/nodeps/Filesystem.h"
#include <filesystem>

using catapult::test::TempFileGuard;

namespace catapult { namespace io {

#define TEST_CLASS FileLockTests

	namespace {
		struct LockPolicy {
			using LockType = FileLock;

			static auto ExclusiveLock(LockType& lock) {
				return std::make_unique<std::lock_guard<LockType>>(lock);
			}
		};
	}

	DEFINE_BASIC_LOCK_TESTS(TEST_CLASS, TempFileGuard("test.lock").name())

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

	TEST(TEST_CLASS, LockFileIsNotRemovedWhenProcessExits) {
		// Arrange:
		TempFileGuard guard("test.lock");

		// Act: create a lock, but don't let the process run the dtor
		ASSERT_DEATH({
			FileLock lock(guard.name());
			lock.lock();
			_exit(1);
		}, "");

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, LockFileCanBeRemovedAfterOriginalProcessExits) {
		// Arrange:
		TempFileGuard guard("test.lock");

		// - create a lock, but don't let the process run the dtor
		ASSERT_DEATH({
			FileLock lock(guard.name());
			lock.lock();
			_exit(1);
		}, "");

		// Sanity:
		EXPECT_TRUE(std::filesystem::exists(guard.name()));

		// Act:
		std::filesystem::remove(guard.name());

		// Assert:
		EXPECT_FALSE(std::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, LockFileCannotBeAcquiredFromOtherProcess) {
		// Arrange: it seems arrange is executed in both parent and child process
		// so try_lock() is used to avoid locking the child in an infinite loop
		TempFileGuard guard("test.lock");
		FileLock lock(guard.name());
		lock.try_lock();

		// Sanity:
		ASSERT_TRUE(std::filesystem::exists(guard.name()));

		// Act: create a new process, that tries to acquire same lock
		ASSERT_EXIT({
			FileLock descendantLock(guard.name());
			auto isLockAcquired = descendantLock.try_lock();
			exit(isLockAcquired ? 123 : 0);
		}, ::testing::ExitedWithCode(0), "");

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(guard.name()));
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}}
