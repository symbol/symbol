#include "src/validators/Validators.h"
#include "tests/test/LockDurationValidatorTests.h"

namespace catapult { namespace validators {

#define TEST_CLASS SecretLockDurationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(SecretLockDuration, BlockDuration(0))

	namespace {
		struct SecretTraits {
		public:
			using NotificationType = model::SecretLockDurationNotification;

			static auto CreateValidator(BlockDuration maxDuration) {
				return CreateSecretLockDurationValidator(maxDuration);
			}
		};
	}

	DEFINE_DURATION_VALIDATOR_TESTS(SecretTraits)
}}
