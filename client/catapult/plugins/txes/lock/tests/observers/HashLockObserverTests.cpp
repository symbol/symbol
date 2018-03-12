#include "src/observers/Observers.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/LockNotificationsTestUtils.h"
#include "tests/test/LockObserverTests.h"

namespace catapult { namespace observers {

#define TEST_CLASS HashLockObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(HashLock,)

	namespace {
		struct HashObserverTraits {
		public:
			using CacheType = cache::HashLockInfoCache;
			using NotificationType = model::HashLockNotification;
			using NotificationBuilder = test::HashLockNotificationBuilder;
			using ObserverTestContext = test::ObserverTestContextT<test::HashLockInfoCacheFactory>;

			static auto CreateObserver() {
				return CreateHashLockObserver();
			}

			static auto GenerateRandomLockInfo(const NotificationType& notification) {
				auto lockInfo = test::BasicHashLockInfoTestTraits::CreateLockInfo();
				lockInfo.Hash = notification.Hash;
				return lockInfo;
			}

			static const auto& ToKey(const NotificationType& notification) {
				return notification.Hash;
			}

			static void AssertAddedLockInfo(const model::HashLockInfo& lockInfo, const NotificationType& notification) {
				// Assert:
				EXPECT_EQ(notification.Hash, lockInfo.Hash);
			}
		};
	}

	DEFINE_LOCK_OBSERVER_TESTS(HashObserverTraits)
}}
