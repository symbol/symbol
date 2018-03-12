#include "src/observers/Observers.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/LockNotificationsTestUtils.h"
#include "tests/test/LockObserverTests.h"

namespace catapult { namespace observers {

#define TEST_CLASS SecretLockObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(SecretLock,)

	namespace {
		struct SecretObserverTraits {
		public:
			using CacheType = cache::SecretLockInfoCache;
			using NotificationType = model::SecretLockNotification;
			using NotificationBuilder = test::SecretLockNotificationBuilder;
			using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

			static auto CreateObserver() {
				return CreateSecretLockObserver();
			}

			static auto GenerateRandomLockInfo(const NotificationType& notification) {
				auto lockInfo = test::BasicSecretLockInfoTestTraits::CreateLockInfo();
				lockInfo.Secret = notification.Secret;
				return lockInfo;
			}

			static const auto& ToKey(const NotificationType& notification) {
				return notification.Secret;
			}

			static void AssertAddedLockInfo(const model::SecretLockInfo& lockInfo, const NotificationType& notification) {
				// Assert:
				EXPECT_EQ(notification.HashAlgorithm, lockInfo.HashAlgorithm);
				EXPECT_EQ(notification.Secret, lockInfo.Secret);
				EXPECT_EQ(notification.Recipient, lockInfo.Recipient);
			}
		};
	}

	DEFINE_LOCK_OBSERVER_TESTS(SecretObserverTraits)
}}
