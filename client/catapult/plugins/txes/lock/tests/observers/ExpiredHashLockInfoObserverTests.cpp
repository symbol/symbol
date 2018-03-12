#include "src/observers/Observers.h"
#include "tests/test/ExpiredLockInfoObserverTests.h"
#include "tests/test/LockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredHashLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredHashLockInfo, )

	namespace {
		struct ExpiredHashLockInfoTraits : public test::BasicHashLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::HashLockInfoCacheFactory>;

		public:
			static MosaicId LockInfoMosaicId() {
				return Xem_Id;
			}

			static auto CreateObserver() {
				return CreateExpiredHashLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::HashLockInfoCache>();
			}

			static Amount GetExpectedExpiringLockOwnerBalance(observers::NotifyMode, Amount initialBalance, Amount) {
				return initialBalance;
			}

			static Amount GetExpectedBlockSignerBalance(observers::NotifyMode mode, Amount initialBalance, Amount delta, size_t count) {
				// expiring hash lock is paid to block signer
				auto adjustedDelta = Amount(count * delta.unwrap());
				return observers::NotifyMode::Commit == mode ? initialBalance + adjustedDelta : initialBalance - adjustedDelta;
			}
		};
	}

	DEFINE_EXPIRED_LOCK_INFO_OBSERVER_TESTS(ExpiredHashLockInfoTraits)
}}
