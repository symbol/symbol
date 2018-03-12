#include "src/validators/Validators.h"
#include "tests/test/CacheUniqueTestUtils.h"
#include "tests/test/LockNotificationsTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS SecretCacheUniqueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(SecretCacheUnique,)

	namespace {
		struct SecretCacheTraits {
		public:
			using DescriptorType = test::BasicSecretLockInfoTestTraits;
			using NotificationType = model::SecretLockNotification;
			using NotificationBuilder = test::SecretLockNotificationBuilder;
			using CacheFactory = test::SecretLockInfoCacheFactory;

			static constexpr auto Failure = Failure_Lock_Hash_Exists;

			static auto CreateValidator() {
				return CreateSecretCacheUniqueValidator();
			}
		};
	}

	DEFINE_CACHE_UNIQUE_TESTS(SecretCacheTraits)
}}
