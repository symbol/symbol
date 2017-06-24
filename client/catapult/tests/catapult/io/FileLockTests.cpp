#include "catapult/io/FileLock.h"
#include "tests/test/nodeps/BasicLockTests.h"
#include "tests/test/nodeps/Filesystem.h"

using catapult::test::TempFileGuard;

namespace catapult { namespace io {

	namespace {
		struct LockPolicy {
			using LockType = FileLock;

			static auto ExclusiveLock(LockType& lock) {
				return std::make_unique<std::lock_guard<LockType>>(lock);
			}
		};
	}

	DEFINE_BASIC_LOCK_TESTS(FileLockTests, TempFileGuard("test.lock").name())
}}
