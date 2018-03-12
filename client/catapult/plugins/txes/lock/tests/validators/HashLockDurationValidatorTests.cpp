#include "src/validators/Validators.h"
#include "tests/test/LockDurationValidatorTests.h"

namespace catapult { namespace validators {

#define TEST_CLASS HashLockDurationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(HashLockDuration, BlockDuration(0))

	namespace {
		struct HashTraits {
		public:
			using NotificationType = model::HashLockDurationNotification;

			static auto CreateValidator(BlockDuration maxDuration) {
				return CreateHashLockDurationValidator(maxDuration);
			}
		};
	}

	DEFINE_DURATION_VALIDATOR_TESTS(HashTraits)
}}
