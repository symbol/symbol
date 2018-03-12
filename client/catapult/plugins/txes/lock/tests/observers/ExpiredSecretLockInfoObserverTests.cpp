#include "src/observers/Observers.h"
#include "tests/test/ExpiredLockInfoObserverTests.h"
#include "tests/test/LockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredSecretLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredSecretLockInfo, )

	namespace {
		struct ExpiredSecretLockInfoTraits : public test::BasicSecretLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

		public:
			static MosaicId LockInfoMosaicId() {
				return MosaicId(123);
			}

			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::SecretLockInfoCache>();
			}

			static Amount GetExpectedExpiringLockOwnerBalance(observers::NotifyMode mode, Amount initialBalance, Amount delta) {
				// expiring secret lock is paid to lock creator
				return observers::NotifyMode::Commit == mode ? initialBalance + delta : initialBalance - delta;
			}

			static Amount GetExpectedBlockSignerBalance(observers::NotifyMode, Amount initialBalance, Amount, size_t) {
				return initialBalance;
			}
		};
	}

	DEFINE_EXPIRED_LOCK_INFO_OBSERVER_TESTS(ExpiredSecretLockInfoTraits)
}}
