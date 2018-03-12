#include "src/validators/Validators.h"
#include "tests/test/CacheUniqueTestUtils.h"
#include "tests/test/LockNotificationsTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS HashCacheUniqueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(HashCacheUnique,)

	namespace {
		struct HashCacheTraits {
		public:
			using DescriptorType = test::BasicHashLockInfoTestTraits;
			using NotificationType = model::HashLockNotification;
			using NotificationBuilder = test::HashLockNotificationBuilder;
			using CacheFactory = test::HashLockInfoCacheFactory;

			static constexpr auto Failure = Failure_Lock_Hash_Exists;

			static auto CreateValidator() {
				return CreateHashCacheUniqueValidator();
			}
		};
	}

	DEFINE_CACHE_UNIQUE_TESTS(HashCacheTraits)
}}
