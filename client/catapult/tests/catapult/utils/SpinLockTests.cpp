#include "catapult/utils/SpinLock.h"
#include "tests/test/nodeps/BasicLockTests.h"

namespace catapult { namespace utils {

	namespace {
		struct LockPolicy {
			using LockType = SpinLock;

			static auto ExclusiveLock(LockType& lock) {
				return std::make_unique<std::lock_guard<LockType>>(lock);
			}
		};
	}

	DEFINE_BASIC_LOCK_TESTS(SpinLockTests,)
}}
