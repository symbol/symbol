#include "catapult/io/FileLock.h"
#include "tests/test/nodeps/BasicLockTests.h"
#include "tests/test/nodeps/Filesystem.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

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

	TEST(TEST_CLASS, LockFileIsNotRemovedIfProcessExits) {
		// Arrange:
		TempFileGuard guard("test.lock");

		// Act: create a lock, but don't let the process run the dtor
		ASSERT_DEATH({
			FileLock lock(guard.name());
			lock.lock();
			_exit(1);
		}, "");

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, LockFileCannotBeAcquiredFromOtherProcess) {
		// Arrange: it seems arrange is executed in both parent and child process
		// so try_lock() is used to avoid locking the child in an infinite loop
		TempFileGuard guard("test.lock");
		FileLock lock(guard.name());
		lock.try_lock();

		// Sanity:
		ASSERT_TRUE(boost::filesystem::exists(guard.name()));

		// Act: create a new process, that tries to acquire same lock
		ASSERT_EXIT({
			FileLock descendantLock(guard.name());
			auto isLockAcquired = descendantLock.try_lock();
			exit(isLockAcquired ? 123 : 0);
		}, ::testing::ExitedWithCode(0), "");

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}}
